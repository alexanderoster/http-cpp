//
// The MIT License (MIT)
//
// Copyright (c) 2013-2014 by Konstantin (Kosta) Baumann & Autodesk Inc.
//
// Permission is hereby granted, free of charge,  to any person obtaining a copy of
// this software and  associated documentation  files  (the "Software"), to deal in
// the  Software  without  restriction,  including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software,  and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this  permission notice  shall be included in all
// copies or substantial portions of the Software.
//
// THE  SOFTWARE  IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER
// IN  AN  ACTION  OF  CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "utils.hpp"

#include <cassert>

static const char HEX[] = "0123456789ABCDEF";

static inline bool is_hex(
    int const h
) {
    switch(h) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            return true;
        default:
            return false;
    }
}

static inline int hex2int(
    int const h
) {
    assert(is_hex(h));
    switch(h) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return (h - '0');
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            return ((h - 'A') + 10);
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            return ((h - 'a') + 10);
        default:
            return 0;
    }
}

namespace {
    enum encode_part {
        encode_all_part,
        encode_path_part,
        encode_value_part,
        encode_key_part
    };
}

static std::string encode(
    std::string const& str,
    encode_part const part
) {
    std::string escaped;
    escaped.reserve(3 * str.size());

    for(unsigned char const c : str) {
        bool enc = true;
        switch(c) {
            // these don't need to be encoded
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            case '~': case '-': case '_': case '.':
                        enc = false; break;

            case '=':   enc = (part == encode_key_part);  break; // encode for keys
            case '&':   enc = (part != encode_path_part); break; // encode for keys and values
            case '+':   enc = (part != encode_path_part); break; // encode for keys and values
            case '/':   enc = (part != encode_path_part); break; // encode for keys and values

            // by default encode all other chars not handle above
            default:    enc = true; break;
        }

        if(enc || (part == encode_all_part)) {
            const int high = (c & 0xF0) >> 4;
            const int low  = (c & 0x0F);

            escaped += '%';
            escaped += HEX[high];
            escaped += HEX[low];
        } else {
            escaped += c;
        }
    }

    return escaped;
}

std::string http::encode_path(std::string const& path) {
    return encode(path, encode_path_part);
}

std::string http::encode_key(std::string const& key) {
    return encode(key, encode_key_part);
}

std::string http::encode_value(std::string const& value) {
    return encode(value, encode_value_part);
}

std::string http::encode_all(std::string const& str) {
    return encode(str, encode_all_part);
}

std::string http::decode(
    std::string const& str
) {
    std::string unescaped;
    unescaped.reserve(str.size());

    for(size_t i = 0, iEnd = str.size(); i < iEnd; ++i) {
        const auto c = str[i];
        if((c == '%') && (i + 2 < iEnd) && is_hex(str[i+1]) && is_hex(str[i+2])) {
            const int high = hex2int(str[i+1]);
            const int low  = hex2int(str[i+2]);
            unescaped += static_cast<unsigned char>(high << 4 | low);
            i += 2;
        } else {
            unescaped += c;
        }
    }

    return unescaped;
}
