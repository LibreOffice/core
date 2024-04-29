/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_gpgme.h>

#include <algorithm>
#include <string_view>

#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/hash.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <rtl/digest.h>
#include <rtl/random.h>
#include <string.h>

#if HAVE_FEATURE_GPGME
# include <context.h>
# include <data.h>
# include <decryptionresult.h>
#endif

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::task::PasswordRequestMode;
using ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER;
using ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER;
using ::com::sun::star::task::XInteractionHandler;

using namespace ::com::sun::star;

namespace comphelper {


static uno::Sequence< sal_Int8 > GeneratePBKDF2Hash( std::u16string_view aPassword, const uno::Sequence< sal_Int8 >& aSalt, sal_Int32 nCount, sal_Int32 nHashLength )
{
    uno::Sequence< sal_Int8 > aResult;

    if ( !aPassword.empty() && aSalt.hasElements() && nCount && nHashLength )
    {
        OString aBytePass = OUStringToOString( aPassword, RTL_TEXTENCODING_UTF8 );
        // FIXME this is subject to the SHA1-bug tdf#114939 - see also
        // RequestPassword() in filedlghelper.cxx
        aResult.realloc( 16 );
        rtl_digest_PBKDF2( reinterpret_cast < sal_uInt8 * > ( aResult.getArray() ),
                           aResult.getLength(),
                           reinterpret_cast < const sal_uInt8 * > ( aBytePass.getStr() ),
                           aBytePass.getLength(),
                           reinterpret_cast < const sal_uInt8 * > ( aSalt.getConstArray() ),
                           aSalt.getLength(),
                           nCount );
    }

    return aResult;
}


IDocPasswordVerifier::~IDocPasswordVerifier()
{
}


uno::Sequence< beans::PropertyValue > DocPasswordHelper::GenerateNewModifyPasswordInfo( std::u16string_view aPassword )
{
    uno::Sequence< beans::PropertyValue > aResult;

    uno::Sequence< sal_Int8 > aSalt = GenerateRandomByteSequence( 16 );
    sal_Int32 const nPBKDF2IterationCount = 100000;

    uno::Sequence< sal_Int8 > aNewHash = GeneratePBKDF2Hash(aPassword, aSalt, nPBKDF2IterationCount, 16);
    if ( aNewHash.hasElements() )
    {
        aResult = { comphelper::makePropertyValue(u"algorithm-name"_ustr, u"PBKDF2"_ustr),
                    comphelper::makePropertyValue(u"salt"_ustr, aSalt),
                    comphelper::makePropertyValue(u"iteration-count"_ustr, nPBKDF2IterationCount),
                    comphelper::makePropertyValue(u"hash"_ustr, aNewHash) };
    }

    return aResult;
}


uno::Sequence<beans::PropertyValue>
DocPasswordHelper::GenerateNewModifyPasswordInfoOOXML(std::u16string_view aPassword)
{
    uno::Sequence<beans::PropertyValue> aResult;

    if (!aPassword.empty())
    {
        uno::Sequence<sal_Int8> aSalt = GenerateRandomByteSequence(16);
        OUStringBuffer aBuffer(22);
        comphelper::Base64::encode(aBuffer, aSalt);
        OUString sSalt = aBuffer.makeStringAndClear();

        sal_Int32 const nIterationCount = 100000;
        OUString sAlgorithm(u"SHA-512"_ustr);

        const OUString sHash(GetOoxHashAsBase64(OUString(aPassword), sSalt, nIterationCount,
                                                comphelper::Hash::IterCount::APPEND, sAlgorithm));

        if (!sHash.isEmpty())
        {
            aResult = { comphelper::makePropertyValue(u"algorithm-name"_ustr, sAlgorithm),
                        comphelper::makePropertyValue(u"salt"_ustr, sSalt),
                        comphelper::makePropertyValue(u"iteration-count"_ustr, nIterationCount),
                        comphelper::makePropertyValue(u"hash"_ustr, sHash) };
        }
    }

    return aResult;
}


uno::Sequence< beans::PropertyValue > DocPasswordHelper::ConvertPasswordInfo( const uno::Sequence< beans::PropertyValue >& aInfo )
{
    uno::Sequence< beans::PropertyValue > aResult;
    OUString sAlgorithm, sHash, sSalt, sCount;
    sal_Int32 nAlgorithm = 0;

    for ( const auto & prop : aInfo )
    {
        if ( prop.Name == "cryptAlgorithmSid" )
        {
            prop.Value >>= sAlgorithm;
            nAlgorithm = sAlgorithm.toInt32();
        }
        else if ( prop.Name == "salt" )
            prop.Value >>= sSalt;
        else if ( prop.Name == "cryptSpinCount" )
            prop.Value >>= sCount;
        else if ( prop.Name == "hash" )
            prop.Value >>= sHash;
    }

    if (nAlgorithm == 1)
        sAlgorithm = "MD2";
    else if (nAlgorithm == 2)
        sAlgorithm = "MD4";
    else if (nAlgorithm == 3)
        sAlgorithm = "MD5";
    else if (nAlgorithm == 4)
        sAlgorithm = "SHA-1";
    else if (nAlgorithm == 5)
        sAlgorithm = "MAC";
    else if (nAlgorithm == 6)
        sAlgorithm = "RIPEMD";
    else if (nAlgorithm == 7)
        sAlgorithm = "RIPEMD-160";
    else if (nAlgorithm == 9)
        sAlgorithm = "HMAC";
    else if (nAlgorithm == 12)
        sAlgorithm = "SHA-256";
    else if (nAlgorithm == 13)
        sAlgorithm = "SHA-384";
    else if (nAlgorithm == 14)
        sAlgorithm = "SHA-512";

    if ( !sCount.isEmpty() )
    {
        sal_Int32 nCount = sCount.toInt32();
        aResult = { comphelper::makePropertyValue(u"algorithm-name"_ustr, sAlgorithm),
                    comphelper::makePropertyValue(u"salt"_ustr, sSalt),
                    comphelper::makePropertyValue(u"iteration-count"_ustr, nCount),
                    comphelper::makePropertyValue(u"hash"_ustr, sHash) };
    }

    return aResult;
}


bool DocPasswordHelper::IsModifyPasswordCorrect( std::u16string_view aPassword, const uno::Sequence< beans::PropertyValue >& aInfo )
{
    bool bResult = false;
    if ( !aPassword.empty() && aInfo.hasElements() )
    {
        OUString sAlgorithm;
        uno::Any aSalt, aHash;
        sal_Int32 nCount = 0;

        for ( const auto & prop : aInfo )
        {
            if ( prop.Name == "algorithm-name" )
                prop.Value >>= sAlgorithm;
            else if ( prop.Name == "salt" )
                aSalt = prop.Value;
            else if ( prop.Name == "iteration-count" )
                prop.Value >>= nCount;
            else if ( prop.Name == "hash" )
                aHash = prop.Value;
        }

        if ( sAlgorithm == "PBKDF2" )
        {
            uno::Sequence<sal_Int8> aIntSalt, aIntHash;
            aSalt >>= aIntSalt;
            aHash >>= aIntHash;
            if (aIntSalt.hasElements() && nCount > 0 && aIntHash.hasElements())
            {
                uno::Sequence<sal_Int8> aNewHash
                    = GeneratePBKDF2Hash(aPassword, aIntSalt, nCount, aIntHash.getLength());
                for (sal_Int32 nInd = 0; nInd < aNewHash.getLength() && nInd < aIntHash.getLength()
                                         && aNewHash[nInd] == aIntHash[nInd];
                     nInd++)
                {
                    if (nInd == aNewHash.getLength() - 1 && nInd == aIntHash.getLength() - 1)
                        bResult = true;
                }
            }
        }
        else if (nCount > 0)
        {
            OUString sSalt, sHash;
            aSalt >>= sSalt;
            aHash >>= sHash;
            if (!sSalt.isEmpty() && !sHash.isEmpty())
            {
                const OUString aNewHash(GetOoxHashAsBase64(OUString(aPassword), sSalt, nCount,
                                                           comphelper::Hash::IterCount::APPEND,
                                                           sAlgorithm));
                if (!aNewHash.isEmpty())
                    bResult = aNewHash == sHash;
            }
        }
    }

    return bResult;
}


sal_uInt32 DocPasswordHelper::GetWordHashAsUINT32(
                std::u16string_view aUString )
{
    static const sal_uInt16 pInitialCode[] = {
        0xE1F0, // 1
        0x1D0F, // 2
        0xCC9C, // 3
        0x84C0, // 4
        0x110C, // 5
        0x0E10, // 6
        0xF1CE, // 7
        0x313E, // 8
        0x1872, // 9
        0xE139, // 10
        0xD40F, // 11
        0x84F9, // 12
        0x280C, // 13
        0xA96A, // 14
        0x4EC3  // 15
    };

    static const sal_uInt16 pEncryptionMatrix[15][7] = {
        { 0xAEFC, 0x4DD9, 0x9BB2, 0x2745, 0x4E8A, 0x9D14, 0x2A09}, // last-14
        { 0x7B61, 0xF6C2, 0xFDA5, 0xEB6B, 0xC6F7, 0x9DCF, 0x2BBF}, // last-13
        { 0x4563, 0x8AC6, 0x05AD, 0x0B5A, 0x16B4, 0x2D68, 0x5AD0}, // last-12
        { 0x0375, 0x06EA, 0x0DD4, 0x1BA8, 0x3750, 0x6EA0, 0xDD40}, // last-11
        { 0xD849, 0xA0B3, 0x5147, 0xA28E, 0x553D, 0xAA7A, 0x44D5}, // last-10
        { 0x6F45, 0xDE8A, 0xAD35, 0x4A4B, 0x9496, 0x390D, 0x721A}, // last-9
        { 0xEB23, 0xC667, 0x9CEF, 0x29FF, 0x53FE, 0xA7FC, 0x5FD9}, // last-8
        { 0x47D3, 0x8FA6, 0x8FA6, 0x1EDA, 0x3DB4, 0x7B68, 0xF6D0}, // last-7
        { 0xB861, 0x60E3, 0xC1C6, 0x93AD, 0x377B, 0x6EF6, 0xDDEC}, // last-6
        { 0x45A0, 0x8B40, 0x06A1, 0x0D42, 0x1A84, 0x3508, 0x6A10}, // last-5
        { 0xAA51, 0x4483, 0x8906, 0x022D, 0x045A, 0x08B4, 0x1168}, // last-4
        { 0x76B4, 0xED68, 0xCAF1, 0x85C3, 0x1BA7, 0x374E, 0x6E9C}, // last-3
        { 0x3730, 0x6E60, 0xDCC0, 0xA9A1, 0x4363, 0x86C6, 0x1DAD}, // last-2
        { 0x3331, 0x6662, 0xCCC4, 0x89A9, 0x0373, 0x06E6, 0x0DCC}, // last-1
        { 0x1021, 0x2042, 0x4084, 0x8108, 0x1231, 0x2462, 0x48C4}  // last
    };

    sal_uInt32 nResult = 0;
    size_t nLen = aUString.size();

    if ( nLen )
    {
        if ( nLen > 15 )
            nLen = 15;

        sal_uInt16 nHighResult = pInitialCode[nLen - 1];
        sal_uInt16 nLowResult = 0;

        for ( size_t nInd = 0; nInd < nLen; nInd++ )
        {
            // NO Encoding during conversion!
            // The specification says that the low byte should be used in case it is not NULL
            char nHighChar = static_cast<char>( aUString[nInd] >> 8 );
            char nLowChar = static_cast<char>( aUString[nInd] & 0xFF );
            char nChar = nLowChar ? nLowChar : nHighChar;

            for ( int nMatrixInd = 0; nMatrixInd < 7; ++nMatrixInd )
            {
                if ( ( nChar & ( 1 << nMatrixInd ) ) != 0 )
                    nHighResult = nHighResult ^ pEncryptionMatrix[15 - nLen + nInd][nMatrixInd];
            }

            nLowResult = ( ( ( nLowResult >> 14 ) & 0x0001 ) | ( ( nLowResult << 1 ) & 0x7FFF ) ) ^ nChar;
        }

        nLowResult = static_cast<sal_uInt16>( ( ( ( nLowResult >> 14 ) & 0x001 ) | ( ( nLowResult << 1 ) & 0x7FF ) ) ^ nLen ^ 0xCE4B );

        nResult = ( nHighResult << 16 ) | nLowResult;
    }

    return nResult;
}


sal_uInt16 DocPasswordHelper::GetXLHashAsUINT16(
                std::u16string_view aUString,
                rtl_TextEncoding nEnc )
{
    sal_uInt16 nResult = 0;

    OString aString = OUStringToOString( aUString, nEnc );

    if ( !aString.isEmpty() && aString.getLength() <= SAL_MAX_UINT16 )
    {
        for ( sal_Int32 nInd = aString.getLength() - 1; nInd >= 0; nInd-- )
        {
            nResult = ( ( nResult >> 14 ) & 0x01 ) | ( ( nResult << 1 ) & 0x7FFF );
            nResult ^= aString[nInd];
        }

        nResult = ( ( nResult >> 14 ) & 0x01 ) | ( ( nResult << 1 ) & 0x7FFF );
        nResult ^= ( 0x8000 | ( 'N' << 8 ) | 'K' );
        nResult ^= aString.getLength();
    }

    return nResult;
}


Sequence< sal_Int8 > DocPasswordHelper::GetXLHashAsSequence(
                std::u16string_view aUString )
{
    sal_uInt16 nHash = GetXLHashAsUINT16( aUString );
    return {sal_Int8(nHash >> 8), sal_Int8(nHash & 0xFF)};
}


std::vector<unsigned char> DocPasswordHelper::GetOoxHashAsVector(
        const OUString& rPassword,
        const std::vector<unsigned char>& rSaltValue,
        sal_uInt32 nSpinCount,
        comphelper::Hash::IterCount eIterCount,
         std::u16string_view rAlgorithmName)
{
    comphelper::HashType eType;
    if (rAlgorithmName == u"SHA-512" || rAlgorithmName == u"SHA512")
        eType = comphelper::HashType::SHA512;
    else if (rAlgorithmName == u"SHA-256" || rAlgorithmName == u"SHA256")
        eType = comphelper::HashType::SHA256;
    else if (rAlgorithmName == u"SHA-384" || rAlgorithmName == u"SHA384")
        eType = comphelper::HashType::SHA384;
    else if (rAlgorithmName == u"SHA-1" || rAlgorithmName == u"SHA1") // "SHA1" might be in the wild
        eType = comphelper::HashType::SHA1;
    else if (rAlgorithmName == u"MD5")
        eType = comphelper::HashType::MD5;
    else
        return std::vector<unsigned char>();

    return comphelper::Hash::calculateHash( rPassword, rSaltValue, nSpinCount, eIterCount, eType);
}


css::uno::Sequence<sal_Int8> DocPasswordHelper::GetOoxHashAsSequence(
        const OUString& rPassword,
        std::u16string_view rSaltValue,
        sal_uInt32 nSpinCount,
        comphelper::Hash::IterCount eIterCount,
        std::u16string_view rAlgorithmName)
{
    std::vector<unsigned char> aSaltVec;
    if (!rSaltValue.empty())
    {
        css::uno::Sequence<sal_Int8> aSaltSeq;
        comphelper::Base64::decode( aSaltSeq, rSaltValue);
        aSaltVec = comphelper::sequenceToContainer<std::vector<unsigned char>>( aSaltSeq);
    }

    std::vector<unsigned char> hash( GetOoxHashAsVector( rPassword, aSaltVec, nSpinCount, eIterCount, rAlgorithmName));

    return comphelper::containerToSequence<sal_Int8>( hash);
}

OUString DocPasswordHelper::GetOoxHashAsBase64(
        const OUString& rPassword,
        std::u16string_view rSaltValue,
        sal_uInt32 nSpinCount,
        comphelper::Hash::IterCount eIterCount,
        std::u16string_view rAlgorithmName)
{
    css::uno::Sequence<sal_Int8> aSeq( GetOoxHashAsSequence( rPassword, rSaltValue, nSpinCount,
                eIterCount, rAlgorithmName));

    OUStringBuffer aBuf((aSeq.getLength()+2)/3*4);
    comphelper::Base64::encode( aBuf, aSeq);
    return aBuf.makeStringAndClear();
}


/*static*/ uno::Sequence< sal_Int8 > DocPasswordHelper::GenerateRandomByteSequence( sal_Int32 nLength )
{
    uno::Sequence< sal_Int8 > aResult( nLength );

    rtlRandomPool aRandomPool = rtl_random_createPool ();
    if (rtl_random_getBytes(aRandomPool, aResult.getArray(), nLength) != rtl_Random_E_None)
    {
        throw uno::RuntimeException(u"rtl_random_getBytes failed"_ustr);
    }
    rtl_random_destroyPool ( aRandomPool );

    return aResult;
}


/*static*/ uno::Sequence< sal_Int8 > DocPasswordHelper::GenerateStd97Key( std::u16string_view aPassword, const uno::Sequence< sal_Int8 >& aDocId )
{
    uno::Sequence< sal_Int8 > aResultKey;
    if ( !aPassword.empty() && aDocId.getLength() == 16 )
    {
        sal_uInt16 pPassData[16] = {};

        sal_Int32 nPassLen = std::min< sal_Int32 >( aPassword.size(), 15 );
        memcpy( pPassData, aPassword.data(), nPassLen * sizeof(pPassData[0]) );

        aResultKey = GenerateStd97Key( pPassData, aDocId );
    }

    return aResultKey;
}


/*static*/ uno::Sequence< sal_Int8 > DocPasswordHelper::GenerateStd97Key( const sal_uInt16 pPassData[16], const uno::Sequence< sal_Int8 >& aDocId )
{
    uno::Sequence< sal_Int8 > aResultKey;

    if ( aDocId.getLength() == 16 )
        aResultKey = GenerateStd97Key(pPassData, reinterpret_cast<const sal_uInt8*>(aDocId.getConstArray()));

    return aResultKey;
}


/*static*/ uno::Sequence< sal_Int8 > DocPasswordHelper::GenerateStd97Key( const sal_uInt16 pPassData[16], const sal_uInt8 pDocId[16] )
{
    uno::Sequence< sal_Int8 > aResultKey;
    if ( pPassData[0] )
    {
        sal_uInt8 pKeyData[64] = {};

        sal_Int32 nInd = 0;

        // Fill PassData into KeyData.
        for ( nInd = 0; nInd < 16 && pPassData[nInd]; nInd++)
        {
            pKeyData[2*nInd] = sal::static_int_cast< sal_uInt8 >( (pPassData[nInd] >> 0) & 0xff );
            pKeyData[2*nInd + 1] = sal::static_int_cast< sal_uInt8 >( (pPassData[nInd] >> 8) & 0xff );
        }

        pKeyData[2*nInd] = 0x80;
        pKeyData[56] = sal::static_int_cast< sal_uInt8 >( nInd << 4 );

        // Fill raw digest of KeyData into KeyData.
        rtlDigest hDigest = rtl_digest_create ( rtl_Digest_AlgorithmMD5 );
        (void)rtl_digest_updateMD5 (
            hDigest, pKeyData, sizeof(pKeyData));
        (void)rtl_digest_rawMD5 (
            hDigest, pKeyData, RTL_DIGEST_LENGTH_MD5);

        // Update digest with KeyData and Unique.
        for ( nInd = 0; nInd < 16; nInd++ )
        {
            rtl_digest_updateMD5( hDigest, pKeyData, 5 );
            rtl_digest_updateMD5( hDigest, pDocId, 16 );
        }

        // Update digest with padding.
        pKeyData[16] = 0x80;
        memset( pKeyData + 17, 0, sizeof(pKeyData) - 17 );
        pKeyData[56] = 0x80;
        pKeyData[57] = 0x0a;

        rtl_digest_updateMD5( hDigest, &(pKeyData[16]), sizeof(pKeyData) - 16 );

        // Fill raw digest of above updates
        aResultKey.realloc( RTL_DIGEST_LENGTH_MD5 );
        rtl_digest_rawMD5 ( hDigest, reinterpret_cast<sal_uInt8*>(aResultKey.getArray()), aResultKey.getLength() );

        // Erase KeyData array and leave.
        rtl_secureZeroMemory (pKeyData, sizeof(pKeyData));

        rtl_digest_destroy(hDigest);
    }

    return aResultKey;
}


/*static*/ css::uno::Sequence< css::beans::NamedValue > DocPasswordHelper::requestAndVerifyDocPassword(
        IDocPasswordVerifier& rVerifier,
        const css::uno::Sequence< css::beans::NamedValue >& rMediaEncData,
        const OUString& rMediaPassword,
        const Reference< XInteractionHandler >& rxInteractHandler,
        const OUString& rDocumentUrl,
        DocPasswordRequestType eRequestType,
        const std::vector< OUString >* pDefaultPasswords,
        bool* pbIsDefaultPassword )
{
    css::uno::Sequence< css::beans::NamedValue > aEncData;
    OUString aPassword;
    DocPasswordVerifierResult eResult = DocPasswordVerifierResult::WrongPassword;

    sal_Int32 nMediaEncDataCount = rMediaEncData.getLength();

    // tdf#93389: if the document is being restored from autorecovery, we need to add encryption
    // data also for real document type.
    // TODO: get real filter name here (from CheckPasswd_Impl), to only add necessary data
    bool bForSalvage = false;
    if (nMediaEncDataCount)
    {
        for (auto& val : rMediaEncData)
        {
            if (val.Name == "ForSalvage")
            {
                --nMediaEncDataCount; // don't consider this element below
                val.Value >>= bForSalvage;
                break;
            }
        }
    }

    // first, try provided default passwords
    if( pbIsDefaultPassword )
        *pbIsDefaultPassword = false;
    if( pDefaultPasswords )
    {
        for( const auto& rPassword : *pDefaultPasswords )
        {
            OSL_ENSURE( !rPassword.isEmpty(), "DocPasswordHelper::requestAndVerifyDocPassword - unexpected empty default password" );
            if( !rPassword.isEmpty() )
            {
                eResult = rVerifier.verifyPassword( rPassword, aEncData );
                if (eResult == DocPasswordVerifierResult::OK)
                {
                    aPassword = rPassword;
                    if (pbIsDefaultPassword)
                        *pbIsDefaultPassword = true;
                }
                if( eResult != DocPasswordVerifierResult::WrongPassword )
                    break;
            }
        }
    }

    // try media encryption data (skip, if result is OK or ABORT)
    if( eResult == DocPasswordVerifierResult::WrongPassword )
    {
        if (nMediaEncDataCount)
        {
            eResult = rVerifier.verifyEncryptionData( rMediaEncData );
            if( eResult == DocPasswordVerifierResult::OK )
                aEncData = rMediaEncData;
        }
    }

    // try media password (skip, if result is OK or ABORT)
    if( eResult == DocPasswordVerifierResult::WrongPassword )
    {
        if( !rMediaPassword.isEmpty() )
        {
            eResult = rVerifier.verifyPassword( rMediaPassword, aEncData );
            if (eResult == DocPasswordVerifierResult::OK)
                aPassword = rMediaPassword;
        }
    }

    // request a password (skip, if result is OK or ABORT)
    if( (eResult == DocPasswordVerifierResult::WrongPassword) && rxInteractHandler.is() ) try
    {
        PasswordRequestMode eRequestMode = PasswordRequestMode_PASSWORD_ENTER;
        while( eResult == DocPasswordVerifierResult::WrongPassword )
        {
            rtl::Reference<DocPasswordRequest> pRequest = new DocPasswordRequest( eRequestType, eRequestMode, rDocumentUrl );
            rxInteractHandler->handle( pRequest );
            if( pRequest->isPassword() )
            {
                if( !pRequest->getPassword().isEmpty() )
                    eResult = rVerifier.verifyPassword( pRequest->getPassword(), aEncData );
                if (eResult == DocPasswordVerifierResult::OK)
                    aPassword = pRequest->getPassword();
            }
            else
            {
                eResult = DocPasswordVerifierResult::Abort;
            }
            eRequestMode = PasswordRequestMode_PASSWORD_REENTER;
        }
    }
    catch( Exception& )
    {
    }

    if (eResult == DocPasswordVerifierResult::OK && !aPassword.isEmpty())
    {
        if (std::none_of(std::cbegin(aEncData), std::cend(aEncData),
                         [](const css::beans::NamedValue& val) {
                             return val.Name == PACKAGE_ENCRYPTIONDATA_SHA256UTF8;
                         }))
        {
            // tdf#118639: We need ODF encryption data for autorecovery, where password
            // will already be unavailable, so generate and append it here
            aEncData = comphelper::concatSequences(
                aEncData, OStorageHelper::CreatePackageEncryptionData(aPassword));
        }

        if (bForSalvage)
        {
            // TODO: add individual methods for different target filter, and only call what's needed

            // 1. Prepare binary MS formats encryption data
            auto aUniqueID = GenerateRandomByteSequence(16);
            auto aEnc97Key = GenerateStd97Key(aPassword, aUniqueID);
            // 2. Add MS binary and OOXML encryption data to result
            aEncData = comphelper::concatSequences(
                aEncData, std::initializer_list<beans::NamedValue>{
                              { u"STD97EncryptionKey"_ustr, css::uno::Any(aEnc97Key) },
                              { u"STD97UniqueID"_ustr, css::uno::Any(aUniqueID) },
                              { u"OOXPassword"_ustr, css::uno::Any(aPassword) },
                          });
        }
    }

    return (eResult == DocPasswordVerifierResult::OK) ? aEncData : uno::Sequence< beans::NamedValue >();
}

/*static*/ uno::Sequence< css::beans::NamedValue >
    DocPasswordHelper::decryptGpgSession(
        const uno::Sequence< uno::Sequence< beans::NamedValue > >& rGpgProperties )
{
#if HAVE_FEATURE_GPGME
    if ( !rGpgProperties.hasElements() )
        return uno::Sequence< beans::NamedValue >();

    uno::Sequence< beans::NamedValue > aEncryptionData;
    std::unique_ptr<GpgME::Context> ctx;
    GpgME::initializeLibrary();
    GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
    if (err)
        throw uno::RuntimeException(u"The GpgME library failed to initialize for the OpenPGP protocol."_ustr);

    ctx.reset( GpgME::Context::createForProtocol(GpgME::OpenPGP) );
    if (ctx == nullptr)
        throw uno::RuntimeException(u"The GpgME library failed to initialize for the OpenPGP protocol."_ustr);
    ctx->setArmor(false);

    for (auto& rSequence : rGpgProperties)
    {
        if (rSequence.getLength() == 3)
        {
            // take CipherValue and try to decrypt that - stop after
            // the first successful decryption

            // ctx is setup now, let's decrypt the lot!
            uno::Sequence < sal_Int8 > aVector;
            rSequence[2].Value >>= aVector;

            GpgME::Data cipher(
                reinterpret_cast<const char*>(aVector.getConstArray()),
                size_t(aVector.getLength()), false);
            GpgME::Data plain;

            GpgME::DecryptionResult crypt_res = ctx->decrypt(
                cipher, plain);

            // NO_SECKEY -> skip
            // BAD_PASSPHRASE -> retry?

            off_t result = plain.seek(0,SEEK_SET);
            (void) result;
            assert(result == 0);
            int len=0, curr=0; char buf;
            while( (curr=plain.read(&buf, 1)) )
                len += curr;

            if(crypt_res.error() || !len)
                continue; // can't use this key, take next one

            uno::Sequence < sal_Int8 > aKeyValue(len);
            result = plain.seek(0,SEEK_SET);
            assert(result == 0);
            if( plain.read(aKeyValue.getArray(), len) != len )
                throw uno::RuntimeException(u"The GpgME library failed to read the encrypted value."_ustr);

            SAL_INFO("comphelper.crypto", "Extracted gpg session key of length: " << len);

            aEncryptionData = { { PACKAGE_ENCRYPTIONDATA_SHA256UTF8, uno::Any(aKeyValue) } };
            break;
        }
    }

    if ( aEncryptionData.hasElements() )
    {
        uno::Sequence< beans::NamedValue > aContainer{
            { u"GpgInfos"_ustr, uno::Any(rGpgProperties) }, { u"EncryptionKey"_ustr, uno::Any(aEncryptionData) }
        };

        return aContainer;
    }
#else
    (void)rGpgProperties;
#endif
    return uno::Sequence< beans::NamedValue >();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
