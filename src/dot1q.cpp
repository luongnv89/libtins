/*
 * Copyright (c) 2012, Nasel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
#include <stdexcept>
#include <cstring>
#include <cassert>
#include "dot1q.h"
#include "internals.h"

namespace Tins {

Dot1Q::Dot1Q() 
: _header() 
{
    
}

Dot1Q::Dot1Q(const uint8_t *buffer, uint32_t total_sz) {
    if(total_sz < sizeof(_header))
        throw std::runtime_error("Not enough size for a Dot1Q header");
    std::memcpy(&_header, buffer, sizeof(_header));
    buffer += sizeof(_header);
    total_sz -= sizeof(_header);
    
    if(total_sz) {
        inner_pdu(
            Internals::pdu_from_flag(
                (Constants::Ethernet::e)payload_type(), 
                buffer, 
                total_sz
            )
        );
    }
}

void Dot1Q::priority(small_uint<3> new_priority) {
    _header.priority = new_priority;
}

void Dot1Q::cfi(small_uint<1> new_cfi) {
    _header.cfi = new_cfi;
}

void Dot1Q::id(small_uint<12> new_id) {
    uint16_t value = new_id;
    _header.idL = new_id & 0xff;
    _header.idH = new_id >> 8;
}

void Dot1Q::payload_type(uint16_t new_type) {
    _header.type = Endian::host_to_be(new_type);
}

uint32_t Dot1Q::header_size() const {
    return sizeof(_header);
}

uint32_t Dot1Q::trailer_size() const {
    uint32_t total_size = sizeof(_header);
    if(inner_pdu())
        total_size += inner_pdu()->size();
    return (total_size > 50) ? 0 : (50 - total_size);
}

void Dot1Q::write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *) {
    uint32_t trailer = trailer_size();
    #ifdef TINS_DEBUG
        assert(total_sz >= sizeof(_header) + trailer);
    #endif
    if ((payload_type() == 0) && inner_pdu()) {
        Constants::Ethernet::e flag = Internals::pdu_flag_to_ether_type(
            inner_pdu()->pdu_type()
        );
        payload_type(static_cast<uint16_t>(flag));
    }
    std::memcpy(buffer, &_header, sizeof(_header));
    
    buffer += sizeof(_header) + inner_pdu()->size();
    std::fill(buffer, buffer + trailer, 0);
}
}