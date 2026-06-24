/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Implementation of WOPI proof key verification.
 * Classes: ProofKey
 */

#include <config.h>

#include "ProofKey.hpp"

#include <common/Log.hpp>
#include <common/Util.hpp>
#include <wsd/Exceptions.hpp>

#include <common/base64.hpp>

#include <Poco/Crypto/RSADigestEngine.h>
#include <Poco/Crypto/RSAKey.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <vector>

namespace
{

std::vector<unsigned char> getBytesLE(const unsigned char* bytesInHostOrder, const size_t n)
{
    std::vector<unsigned char> ret(n);
#if !defined __BYTE_ORDER__
    static_assert(false, "Byte order is not detected on this platform!");
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    std::copy_n(bytesInHostOrder, n, ret.begin());
#else
    std::copy_n(bytesInHostOrder, n, ret.rbegin());
#endif
    return ret;
}

std::vector<unsigned char> getBytesBE(const unsigned char* bytesInHostOrder, const size_t n)
{
    std::vector<unsigned char> ret(n);
#if !defined __BYTE_ORDER__
    static_assert(false, "Byte order is not detected on this platform!");
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    std::copy_n(bytesInHostOrder, n, ret.rbegin());
#else
    std::copy_n(bytesInHostOrder, n, ret.begin());
#endif
    return ret;
}

// Returns passed number as vector of bytes (little-endian)
template <typename T>
std::vector<unsigned char> ToLEBytes(const T& x)
{
    return getBytesLE(reinterpret_cast<const unsigned char*>(&x), sizeof(x));
}

// Returns passed number as vector of bytes (network order = big-endian)
template <typename T>
std::vector<unsigned char> ToNetworkOrderBytes(const T& x)
{
    return getBytesBE(reinterpret_cast<const unsigned char*>(&x), sizeof(x));
}

} // namespace

std::vector<unsigned char> Proof::Base64ToBytes(const std::string &str)
{
    std::string decoded;
    macaron::Base64::Decode(str, decoded);
    return std::vector<unsigned char>(decoded.begin(), decoded.end());
}

void Proof::initialize()
{
    if ( _pKey)
    {
        const auto m = _pKey->modulus();
        const auto e = _pKey->encryptionExponent();
        const auto capiBlob = RSA2CapiBlob(m, e);

        const auto sv = BytesToBase64(capiBlob);
        const auto sm = BytesToBase64(m);
        const auto se = BytesToBase64(e);

        _aAttribs.emplace_back("value", sv);
        _aAttribs.emplace_back("modulus", sm);
        _aAttribs.emplace_back("exponent", se);

        // TODO: implement proper rotation; for now, just duplicate * to old*

        _aAttribs.emplace_back("oldvalue", sv);
        _aAttribs.emplace_back("oldmodulus", sm);
        _aAttribs.emplace_back("oldexponent", se);
    }

}

Proof::Proof(Type)
    : _pKey(new Poco::Crypto::RSAKey(
                 Poco::Crypto::RSAKey::KeyLength::KL_4096,
                 Poco::Crypto::RSAKey::Exponent::EXP_LARGE))
{
    initialize();
}

Proof::Proof()
    : _pKey([]() -> Poco::Crypto::RSAKey* {
        const auto keyPath = ProofKeyPath();
        try
        {
            return new Poco::Crypto::RSAKey("", keyPath);
        }
        catch (const Poco::FileNotFoundException& e)
        {
            std::string msg = e.displayText() +
                "\nNo proof-key will be present in discovery."
                "\nIf you need to use WOPI security, generate an RSA key using this command:"
                "\n    sudo coolconfig generate-proof-key"
                "\nor if your config dir is not /etc, you can run ssh-keygen manually:"
                "\n    ssh-keygen -t rsa -N \"\" -m PEM -f \"" + keyPath + "\""
                "\nNote: the proof_key file must be readable by the coolwsd process.";
            LOG_WRN(msg);
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERR("Could not open proof RSA key, Poco exception: " << e.displayText());
        }
        catch (const std::exception& e)
        {
            LOG_ERR("Could not open proof RSA key, standard exception: " << e.what());
        }
        catch (...)
        {
            LOG_ERR("Could not open proof RSA key: unknown exception");
        }
        return nullptr;
    }())
{
    initialize();
}

std::string Proof::ProofKeyPath()
{
    static const std::string keyPath =
#if ENABLE_DEBUG
        DEBUG_ABSSRCDIR
#else
        COOLWSD_CONFIGDIR
#endif
        "/proof_key";
    return keyPath;
}

// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-mqqb/ade9efde-3ec8-4e47-9ae9-34b64d8081bb
std::vector<unsigned char> Proof::RSA2CapiBlob(const std::vector<unsigned char>& modulus,
                                               const std::vector<unsigned char>& exponent)
{
    // Exponent might have arbitrary length in OpenSSL; we need exactly 4
    if (exponent.size() > 4)
        throw ParseError("Proof key public exponent is longer than 4 bytes.");
    // make sure exponent length is correct; assume we are passed big-endian vectors
    // GCC 12 doesn't like it when the capacity of exponent is > ours.
    // std::copy gives the following compile-time error:
    // error: writing 16 bytes into a region of size 4
    std::vector<unsigned char> exponent32LE(exponent.capacity());
    exponent32LE.resize(4);
    std::copy(exponent.rbegin(), exponent.rend(), exponent32LE.begin());

    std::vector<unsigned char> capiBlob = {
        0x06, 0x02, 0x00, 0x00,
        0x00, 0xA4, 0x00, 0x00,
        0x52, 0x53, 0x41, 0x31,
    };
    // modulus size in bits - 4 bytes (little-endian)
    const auto bitLen = ToLEBytes<std::uint32_t>(modulus.size() * 8);
    capiBlob.reserve(capiBlob.size() + bitLen.size() + exponent32LE.size() + modulus.size());
    std::copy(bitLen.begin(), bitLen.end(), std::back_inserter(capiBlob));
    // exponent - 4 bytes (little-endian)
    std::copy(exponent32LE.begin(), exponent32LE.end(), std::back_inserter(capiBlob));
    // modulus (passed big-endian, stored little-endian)
    std::copy(modulus.rbegin(), modulus.rend(), std::back_inserter(capiBlob));
    return capiBlob;
}

int64_t Proof::DotNetTicks(const std::chrono::system_clock::time_point& utc)
{
    // Get time point for Unix epoch; unfortunately from_time_t isn't constexpr
    const auto unxEpoch(std::chrono::system_clock::from_time_t(0));
    const auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(utc - unxEpoch);
    return duration_ns.count() / 100 + 621355968000000000;
}

std::vector<unsigned char> Proof::GetProof(const std::string& access_token, const std::string& uri,
                                           int64_t ticks)
{
    assert(access_token.size() <= static_cast<size_t>(std::numeric_limits<int32_t>::max()));
    std::string uri_upper = uri;
    for (auto& c : uri_upper)
        if (c >= 'a' && c <= 'z')
            c -= 'a' - 'A';
    assert(uri_upper.size() <= static_cast<size_t>(std::numeric_limits<int32_t>::max()));
    const auto access_token_size = ToNetworkOrderBytes<int32_t>(access_token.size());
    const auto uri_size = ToNetworkOrderBytes<int32_t>(uri_upper.size());
    const auto ticks_bytes = ToNetworkOrderBytes(ticks);
    const auto ticks_size = ToNetworkOrderBytes<int32_t>(ticks_bytes.size());
    const size_t size = access_token_size.size() + access_token.size()
                        + uri_size.size() + uri_upper.size() + ticks_size.size()
                        + ticks_bytes.size();
    std::vector<unsigned char> buf(size);
    auto pos = std::copy(access_token_size.begin(), access_token_size.end(), buf.begin());
    pos = std::copy(access_token.begin(), access_token.end(), pos);
    pos = std::copy(uri_size.begin(), uri_size.end(), pos);
    pos = std::copy(uri_upper.begin(), uri_upper.end(), pos);
    pos = std::copy(ticks_size.begin(), ticks_size.end(), pos);
    std::copy(ticks_bytes.begin(), ticks_bytes.end(), pos);
    return buf;
}

std::string Proof::SignProof(const std::vector<unsigned char>& proof) const
{
    assert( _pKey);
    // One per DocumentBroker that uses this via WopiStorage
    static thread_local Poco::Crypto::RSADigestEngine digestEngine(* _pKey, "SHA256");
    digestEngine.reset();
    digestEngine.update(proof.data(), proof.size());
    return BytesToBase64(digestEngine.signature());
}

VecOfStringPairs Proof::GetProofHeaders(const std::string& access_token, const std::string& uri) const
{
    VecOfStringPairs vec;
    if ( _pKey)
    {
        int64_t ticks = DotNetTicks(std::chrono::system_clock::now());
        vec.emplace_back("X-WOPI-TimeStamp", std::to_string(ticks));
        const auto proof = SignProof(GetProof(access_token, uri, ticks));
        vec.emplace_back("X-WOPI-Proof", proof);
        // TODO: implement proper rotation; for now, just duplicate X-WOPI-Proof to X-WOPI-ProofOld
        vec.emplace_back("X-WOPI-ProofOld", proof);
    }
    return vec;
}

const Proof& GetProof()
{
    static const Proof proof;
    return proof;
}

VecOfStringPairs GetProofHeaders(const std::string& access_token, const std::string& uri)
{
    return GetProof().GetProofHeaders(access_token, uri);
}

const VecOfStringPairs& GetProofKeyAttributes()
{
    return GetProof().GetProofKeyAttributes();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
