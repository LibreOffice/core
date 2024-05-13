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

#include <sal/config.h>
#include <sal/log.hxx>

#include <string_view>

#include "passwordcontainer.hxx"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <o3tl/string_view.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/MasterPasswordRequest.hpp>
#include <com/sun/star/task/NoMasterException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <rtl/cipher.h>
#include <rtl/digest.h>
#include <rtl/byteseq.hxx>
#include <rtl/ustrbuf.hxx>

using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;

static OUString createIndex(const std::vector< OUString >& lines)
{
    OUStringBuffer aResult;

    for( size_t i = 0; i < lines.size(); i++ )
    {
        if( i )
            aResult.append("__");
        OString line = OUStringToOString( lines[i], RTL_TEXTENCODING_UTF8 );
        const char* pLine = line.getStr();

        while( *pLine )
        {
            if (rtl::isAsciiAlphanumeric(static_cast<unsigned char>(*pLine)))
            {
                aResult.append(*pLine);
            }
            else
            {
                aResult.append("_" + OUString::number(*pLine, 16) );
            }

            pLine++;
        }
    }

    return aResult.makeStringAndClear();
}


static std::vector< OUString > getInfoFromInd( std::u16string_view aInd )
{
    std::vector< OUString > aResult;
    bool aStart = true;

    OString line = OUStringToOString( aInd, RTL_TEXTENCODING_ASCII_US );
    const char* pLine = line.getStr();
    do
    {
        OUStringBuffer newItem;
        if( !aStart )
            pLine += 2;
        else
            aStart = false;

        while( *pLine && ( pLine[0] != '_' || pLine[1] != '_' ))
            if( *pLine != '_' )
            {
                newItem.append( *pLine );
                pLine++;
            }
            else
            {
                OUString aNum;
                for( int i = 1; i < 3; i++ )
                {
                    if( !pLine[i]
                      ||  ( ( pLine[i] < '0' || pLine[i] > '9' )
                         && ( pLine[i] < 'a' || pLine[i] > 'f' )
                         && ( pLine[i] < 'A' || pLine[i] > 'F' ) ) )
                    {
                        OSL_FAIL( "Wrong index syntax!" );
                        return aResult;
                    }

                    aNum += OUStringChar( pLine[i] );
                }

                newItem.append( sal_Unicode( aNum.toUInt32( 16 ) ) );
                pLine += 3;
            }

        aResult.push_back( newItem.makeStringAndClear() );
    } while( pLine[0] == '_' && pLine[1] == '_' );

    if( *pLine )
        OSL_FAIL( "Wrong index syntax!" );

    return aResult;
}


static bool shorterUrl( OUString& aURL )
{
    sal_Int32 aInd = aURL.lastIndexOf( '/' );
    if( aInd > 0 && aURL.indexOf( "://" ) != aInd-2 )
    {
        aURL = aURL.copy( 0, aInd );
        return true;
    }

    return false;
}


static OUString getAsciiLine( const ::rtl::ByteSequence& buf )
{
    OUString aResult;

    ::rtl::ByteSequence outbuf( buf.getLength()*2+1 );

    for( int ind = 0; ind < buf.getLength(); ind++ )
    {
        outbuf[ind*2]   = ( static_cast<sal_uInt8>(buf[ind]) >> 4 ) + 'a';
        outbuf[ind*2+1] = ( static_cast<sal_uInt8>(buf[ind]) & 0x0f ) + 'a';
    }
    outbuf[buf.getLength()*2] = '\0';

    aResult = OUString::createFromAscii( reinterpret_cast<char*>(outbuf.getArray()) );

    return aResult;
}


static ::rtl::ByteSequence getBufFromAsciiLine( std::u16string_view line )
{
    OSL_ENSURE( line.size() % 2 == 0, "Wrong syntax!" );
    OString tmpLine = OUStringToOString( line, RTL_TEXTENCODING_ASCII_US );
    ::rtl::ByteSequence aResult(line.size()/2);

    for( int ind = 0; ind < tmpLine.getLength()/2; ind++ )
    {
        aResult[ind] = ( static_cast<sal_uInt8>( tmpLine[ind*2] - 'a' ) << 4 ) | static_cast<sal_uInt8>( tmpLine[ind*2+1] - 'a' );
    }

    return aResult;
}


PasswordMap StorageItem::getInfo()
{
    PasswordMap aResult;

    const Sequence< OUString > aNodeNames     = ConfigItem::GetNodeNames( u"Store"_ustr );
    sal_Int32 aNodeCount = aNodeNames.getLength();
    Sequence< OUString > aPropNames( aNodeCount * 2);

    std::transform(aNodeNames.begin(), aNodeNames.end(), aPropNames.getArray(),
        [](const OUString& rName) -> OUString {
            return "Store/Passwordstorage['" + rName + "']/Password"; });
    std::transform(aNodeNames.begin(), aNodeNames.end(), aPropNames.getArray() + aNodeCount,
        [](const OUString& rName) -> OUString {
            return "Store/Passwordstorage['" + rName + "']/InitializationVector"; });

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aPropNames );

    if( aPropertyValues.getLength() != aNodeCount * 2)
    {
        OSL_FAIL( "Problems during reading" );
        return aResult;
    }

    for( sal_Int32 aNodeInd = 0; aNodeInd < aNodeCount; ++aNodeInd )
    {
        std::vector< OUString > aUrlUsr = getInfoFromInd( aNodeNames[aNodeInd] );

        if( aUrlUsr.size() == 2 )
        {
            OUString aUrl  = aUrlUsr[0];
            OUString aName = aUrlUsr[1];

            OUString aEPasswd;
            OUString aIV;
            aPropertyValues[aNodeInd] >>= aEPasswd;
            aPropertyValues[aNodeInd + aNodeCount] >>= aIV;

            PasswordMap::iterator aIter = aResult.find( aUrl );
            if( aIter != aResult.end() )
                aIter->second.emplace_back( aName, aEPasswd, aIV );
            else
            {
                NamePasswordRecord aNewRecord( aName, aEPasswd, aIV );
                std::vector< NamePasswordRecord > listToAdd( 1, aNewRecord );

                aResult.insert( PairUrlRecord( aUrl, listToAdd ) );
            }
        }
        else
            OSL_FAIL( "Wrong index syntax!" );
    }

    return aResult;
}


void StorageItem::setUseStorage( bool bUse )
{
    ConfigItem::SetModified();
    ConfigItem::PutProperties( { u"UseStorage"_ustr }, { uno::Any(bUse) } );
}


bool StorageItem::useStorage()
{
    Sequence<OUString> aNodeNames { u"UseStorage"_ustr };

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aNodeNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_FAIL( "Problems during reading" );
        return false;
    }

    bool aResult = false;
    aPropertyValues[0] >>= aResult;

    return aResult;
}


sal_Int32 StorageItem::getStorageVersion()
{
    Sequence<OUString> aNodeNames { u"StorageVersion"_ustr };

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aNodeNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_FAIL( "Problems during reading" );
        return 0;
    }

    sal_Int32 nResult = 0;
    aPropertyValues[0] >>= nResult;

    return nResult;
}

bool StorageItem::getEncodedMasterPassword( OUString& aResult, OUString& aResultIV )
{
    if( hasEncoded )
    {
        aResult = mEncoded;
        aResultIV = mEncodedIV;
        return true;
    }

    Sequence< OUString > aNodeNames{ u"HasMaster"_ustr, u"Master"_ustr, u"MasterInitializationVector"_ustr };

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aNodeNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_FAIL( "Problems during reading" );
        return false;
    }

    aPropertyValues[0] >>= hasEncoded;
    aPropertyValues[1] >>= mEncoded;
    aPropertyValues[2] >>= mEncodedIV;

    aResult = mEncoded;
    aResultIV = mEncodedIV;

    return hasEncoded;
}


void StorageItem::setEncodedMasterPassword( const OUString& aEncoded, const OUString& aEncodedIV, bool bAcceptEmpty )
{
    bool bHasMaster = ( !aEncoded.isEmpty() || bAcceptEmpty );

    ConfigItem::SetModified();
    ConfigItem::PutProperties( { u"HasMaster"_ustr, u"Master"_ustr, u"MasterInitializationVector"_ustr, u"StorageVersion"_ustr },
                               { uno::Any(bHasMaster), uno::Any(aEncoded),
                                 uno::Any(aEncodedIV), uno::Any(nCurrentStorageVersion) } );

    hasEncoded = bHasMaster;
    mEncoded = aEncoded;
    mEncodedIV = aEncodedIV;
}


void StorageItem::remove( const OUString& aURL, const OUString& aName )
{
    Sequence< OUString > sendSeq { createIndex( { aURL, aName } ) };

    ConfigItem::ClearNodeElements( u"Store"_ustr, sendSeq );
}


void StorageItem::clear()
{
    ConfigItem::ClearNodeSet( u"Store"_ustr );
}


void StorageItem::update( const OUString& aURL, const NamePasswordRecord& aRecord )
{
    if ( !aRecord.HasPasswords( PERSISTENT_RECORD ) )
    {
        OSL_FAIL( "Unexpected storing of a record!" );
        return;
    }

    Sequence< beans::PropertyValue > sendSeq{ comphelper::makePropertyValue(
        "Store/Passwordstorage['" + createIndex( { aURL, aRecord.GetUserName() } ) + "']/InitializationVector",
        aRecord.GetPersistentIV()), comphelper::makePropertyValue(
        "Store/Passwordstorage['" + createIndex( { aURL, aRecord.GetUserName() } ) + "']/Password",
        aRecord.GetPersistentPasswords()) };

    ConfigItem::SetModified();
    ConfigItem::SetSetProperties( u"Store"_ustr, sendSeq );
}


void StorageItem::Notify( const Sequence< OUString >& )
{
    // this feature still should not be used
    if( mainCont )
        mainCont->Notify();
}


void StorageItem::ImplCommit()
{
    // Do nothing, we stored everything we want already
}


PasswordContainer::PasswordContainer( const Reference<XComponentContext>& rxContext )
{
    // m_pStorageFile->Notify() can be called
    std::unique_lock aGuard( mMutex );

    mComponent.set( rxContext->getServiceManager(), UNO_QUERY );
    mComponent->addEventListener( this );

    m_xStorageFile.emplace( this, "Office.Common/Passwords" );
    if( m_xStorageFile->useStorage() )
        m_aContainer = m_xStorageFile->getInfo();
}


PasswordContainer::~PasswordContainer()
{
    std::unique_lock aGuard( mMutex );

    m_xStorageFile.reset();

    if( mComponent.is() )
    {
        mComponent->removeEventListener(this);
        mComponent.clear();
    }
}

void SAL_CALL PasswordContainer::disposing( const EventObject& )
{
    std::unique_lock aGuard( mMutex );

    m_xStorageFile.reset();

    if( mComponent.is() )
    {
        //mComponent->removeEventListener(this);
        mComponent.clear();
    }
}

std::vector< OUString > PasswordContainer::DecodePasswords( std::u16string_view aLine, std::u16string_view aIV, std::u16string_view aMasterPasswd, css::task::PasswordRequestMode mode )
{
    if( !aMasterPasswd.empty() )
    {
        rtlCipher aDecoder = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream );
        OSL_ENSURE( aDecoder, "Can't create decoder" );

        if( aDecoder )
        {
            OSL_ENSURE( aMasterPasswd.size() == RTL_DIGEST_LENGTH_MD5 * 2, "Wrong master password format!" );

            unsigned char code[RTL_DIGEST_LENGTH_MD5];
            for( int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ind++ )
                code[ ind ] = static_cast<char>(o3tl::toUInt32(aMasterPasswd.substr( ind*2, 2 ), 16));

            unsigned char iv[RTL_DIGEST_LENGTH_MD5] = {0};
            if (!aIV.empty())
            {
                for( int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ind++ )
                {
                    auto tmp = aIV.substr( ind*2, 2 );
                    iv[ ind ] = static_cast<char>(rtl_ustr_toInt64_WithLength(tmp.data(), 16, tmp.size()));
                }
            }

            rtlCipherError result = rtl_cipher_init (
                    aDecoder, rtl_Cipher_DirectionDecode,
                    code, RTL_DIGEST_LENGTH_MD5, iv, RTL_DIGEST_LENGTH_MD5 );

            if( result == rtl_Cipher_E_None )
            {
                ::rtl::ByteSequence aSeq = getBufFromAsciiLine( aLine );

                ::rtl::ByteSequence resSeq( aSeq.getLength() );

                rtl_cipher_decode ( aDecoder, aSeq.getArray(), aSeq.getLength(),
                                                        reinterpret_cast<sal_uInt8*>(resSeq.getArray()), resSeq.getLength() );

                OUString aPasswd( reinterpret_cast<char*>(resSeq.getArray()), resSeq.getLength(), RTL_TEXTENCODING_UTF8 );

                rtl_cipher_destroy (aDecoder);

                return getInfoFromInd( aPasswd );
            }

            rtl_cipher_destroy (aDecoder);
        }
    }
    else
    {
        OSL_FAIL( "No master password provided!" );
        // throw special exception
    }

    // problems with decoding
    OSL_FAIL( "Problem with decoding" );
    throw css::task::NoMasterException(
        u"Can't decode!"_ustr, css::uno::Reference<css::uno::XInterface>(), mode);
}

OUString PasswordContainer::EncodePasswords(const std::vector< OUString >& lines, std::u16string_view aIV, std::u16string_view aMasterPasswd)
{
    if( !aMasterPasswd.empty() )
    {
        OString aSeq = OUStringToOString( createIndex( lines ), RTL_TEXTENCODING_UTF8 );

        rtlCipher aEncoder = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream );
        OSL_ENSURE( aEncoder, "Can't create encoder" );

        if( aEncoder )
        {
            OSL_ENSURE( aMasterPasswd.size() == RTL_DIGEST_LENGTH_MD5 * 2, "Wrong master password format!" );

            unsigned char code[RTL_DIGEST_LENGTH_MD5];
            for( int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ind++ )
                code[ ind ] = static_cast<char>(o3tl::toUInt32(aMasterPasswd.substr( ind*2, 2 ), 16));

            unsigned char iv[RTL_DIGEST_LENGTH_MD5] = {0};
            if (!aIV.empty())
            {
                for( int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ind++ )
                {
                    auto tmp = aIV.substr( ind*2, 2 );
                    iv[ ind ] = static_cast<char>(rtl_ustr_toInt64_WithLength(tmp.data(), 16, tmp.size()));
                }
            }

            rtlCipherError result = rtl_cipher_init (
                    aEncoder, rtl_Cipher_DirectionEncode,
                    code, RTL_DIGEST_LENGTH_MD5, iv, RTL_DIGEST_LENGTH_MD5 );

            if( result == rtl_Cipher_E_None )
            {
                ::rtl::ByteSequence resSeq(aSeq.getLength()+1);

                result = rtl_cipher_encode ( aEncoder, aSeq.getStr(), aSeq.getLength()+1,
                                                        reinterpret_cast<sal_uInt8*>(resSeq.getArray()), resSeq.getLength() );

/*
                //test
                rtlCipherError result = rtl_cipher_init (
                    aEncoder, rtl_Cipher_DirectionDecode,
                    code, RTL_DIGEST_LENGTH_MD5, NULL, 0 );


                if( result == rtl_Cipher_E_None )
                {
                    OUString testOU = getAsciiLine( resSeq );
                    ::rtl::ByteSequence aSeq1 = getBufFromAsciiLine( testOU );

                    ::rtl::ByteSequence resSeq1( aSeq1.getLength() );

                    if( resSeq.getLength() == aSeq1.getLength() )
                    {
                        for( int ind = 0; ind < aSeq1.getLength(); ind++ )
                            if( resSeq[ind] != aSeq1[ind] )
                                testOU = "";
                    }

                    result = rtl_cipher_decode ( aEncoder, (sal_uInt8*)aSeq1.getArray(), aSeq1.getLength(),
                                                        (sal_uInt8*)resSeq1.getArray(), resSeq1.getLength() );

                    OUString aPasswd( ( char* )resSeq1.getArray(), resSeq1.getLength(), RTL_TEXTENCODING_UTF8 );
                }
*/

                rtl_cipher_destroy (aEncoder);

                if( result == rtl_Cipher_E_None )
                    return getAsciiLine( resSeq );

            }

            rtl_cipher_destroy (aEncoder);
        }
    }
    else
    {
        OSL_FAIL( "No master password provided!" );
        // throw special exception
    }

    // problems with encoding
    OSL_FAIL( "Problem with encoding" );
    throw RuntimeException(u"Can't encode!"_ustr );
}

void PasswordContainer::UpdateVector( const OUString& aURL, std::vector< NamePasswordRecord >& toUpdate, NamePasswordRecord const & aRecord, bool writeFile )
{
    for (auto & aNPIter : toUpdate)
        if( aNPIter.GetUserName() == aRecord.GetUserName() )
        {
            if( aRecord.HasPasswords( MEMORY_RECORD ) )
                aNPIter.SetMemoryPasswords( aRecord.GetMemoryPasswords() );

            if( aRecord.HasPasswords( PERSISTENT_RECORD ) )
            {
                aNPIter.SetPersistentPasswords( aRecord.GetPersistentPasswords(), aRecord.GetPersistentIV() );

                if( writeFile )
                {
                    // the password must be already encoded
                    m_xStorageFile->update( aURL, aRecord ); // change existing ( aURL, aName ) record in the configfile
                }
            }

            return;
        }


    if( aRecord.HasPasswords( PERSISTENT_RECORD ) && writeFile )
    {
        // the password must be already encoded
        m_xStorageFile->update( aURL, aRecord ); // add new aName to the existing url
    }

    toUpdate.insert( toUpdate.begin(), aRecord );
}


UserRecord PasswordContainer::CopyToUserRecord( const NamePasswordRecord& aRecord, bool& io_bTryToDecode, const Reference< XInteractionHandler >& aHandler )
{
    ::std::vector< OUString > aPasswords;
    if( aRecord.HasPasswords( MEMORY_RECORD ) )
        aPasswords = aRecord.GetMemoryPasswords();

    if( io_bTryToDecode && aRecord.HasPasswords( PERSISTENT_RECORD ) )
    {
        try
        {
            ::std::vector< OUString > aDecodedPasswords = DecodePasswords( aRecord.GetPersistentPasswords(), aRecord.GetPersistentIV(),
                                                                           GetMasterPassword( aHandler ), css::task::PasswordRequestMode_PASSWORD_ENTER );
            aPasswords.insert( aPasswords.end(), aDecodedPasswords.begin(), aDecodedPasswords.end() );
        }
        catch( NoMasterException& )
        {
            // if master password could not be detected the entry will be just ignored
            io_bTryToDecode = false;
        }
    }

    return UserRecord( aRecord.GetUserName(), comphelper::containerToSequence( aPasswords ) );
}


Sequence< UserRecord > PasswordContainer::CopyToUserRecordSequence( const std::vector< NamePasswordRecord >& original, const Reference< XInteractionHandler >& aHandler )
{
    Sequence< UserRecord >     aResult( original.size() );
    auto aResultRange = asNonConstRange(aResult);
    sal_uInt32 nInd = 0;
    bool bTryToDecode = true;

    for (auto const& aNPIter : original)
    {
        aResultRange[nInd] = CopyToUserRecord( aNPIter, bTryToDecode, aHandler );
        ++nInd;
    }

    return aResult;
}


void SAL_CALL PasswordContainer::add( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, const Reference< XInteractionHandler >& aHandler )
{
    std::unique_lock aGuard( mMutex );

    PrivateAdd( Url, UserName, Passwords, MEMORY_RECORD, aHandler );
}


void SAL_CALL PasswordContainer::addPersistent( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, const Reference< XInteractionHandler >& aHandler  )
{
    std::unique_lock aGuard( mMutex );

    PrivateAdd( Url, UserName, Passwords, PERSISTENT_RECORD, aHandler );
}

OUString PasswordContainer::createIV()
{
    unsigned char iv[RTL_DIGEST_LENGTH_MD5];
    if (rtl_random_getBytes(nullptr, iv, RTL_DIGEST_LENGTH_MD5) != rtl_Random_E_None)
    {
        throw uno::RuntimeException(u"rtl_random_getBytes failed"_ustr);
    }
    OUStringBuffer aBuffer;
    for (sal_uInt8 i : iv)
    {
        aBuffer.append(OUString::number(i >> 4, 16) + OUString::number(i & 15, 16));
    }
    return aBuffer.makeStringAndClear();
}

void PasswordContainer::PrivateAdd( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, char Mode, const Reference< XInteractionHandler >& aHandler )
{
    NamePasswordRecord aRecord( UserName );
    ::std::vector< OUString > aStorePass = comphelper::sequenceToContainer< std::vector<OUString> >( Passwords );

    if( Mode == PERSISTENT_RECORD )
    {
        OUString sIV = createIV();
        OUString sEncodedPasswords = EncodePasswords(aStorePass, sIV, GetMasterPassword(aHandler));
        aRecord.SetPersistentPasswords(sEncodedPasswords, sIV);
    }
    else if( Mode == MEMORY_RECORD )
        aRecord.SetMemoryPasswords( std::move(aStorePass) );
    else
    {
        OSL_FAIL( "Unexpected persistence status!" );
        return;
    }

    if( !m_aContainer.empty() )
    {
        PasswordMap::iterator aIter = m_aContainer.find( Url );

        if( aIter != m_aContainer.end() )
        {
            UpdateVector( aIter->first, aIter->second, aRecord, true );
            return;
        }
    }

    std::vector< NamePasswordRecord > listToAdd( 1, aRecord );
    m_aContainer.insert( PairUrlRecord( Url, listToAdd ) );

    if( Mode == PERSISTENT_RECORD && m_xStorageFile && m_xStorageFile->useStorage() )
        m_xStorageFile->update( Url, aRecord );

}


UrlRecord SAL_CALL PasswordContainer::find( const OUString& aURL, const Reference< XInteractionHandler >& aHandler  )
{
    return find( aURL, u"", false, aHandler );
}


UrlRecord SAL_CALL PasswordContainer::findForName( const OUString& aURL, const OUString& aName, const Reference< XInteractionHandler >& aHandler  )
{
    return find( aURL, aName, true, aHandler );
}


Sequence< UserRecord > PasswordContainer::FindUsr( const std::vector< NamePasswordRecord >& userlist, std::u16string_view aName, const Reference< XInteractionHandler >& aHandler )
{
    for (auto const& aNPIter : userlist)
    {
        if( aNPIter.GetUserName() == aName )
        {
            bool bTryToDecode = true;
            Sequence< UserRecord > aResult { CopyToUserRecord( aNPIter, bTryToDecode, aHandler ) };

            return aResult;
        }
    }

    return Sequence< UserRecord >();
}


bool PasswordContainer::createUrlRecord(
    const PasswordMap::iterator & rIter,
    bool bName,
    std::u16string_view aName,
    const Reference< XInteractionHandler >& aHandler,
    UrlRecord & rRec )
{
    if ( bName )
    {
        Sequence< UserRecord > aUsrRec
            = FindUsr( rIter->second, aName, aHandler );
        if( aUsrRec.hasElements() )
        {
            rRec = UrlRecord( rIter->first, aUsrRec );
            return true;
        }
    }
    else
    {
        rRec = UrlRecord(
            rIter->first,
            CopyToUserRecordSequence( rIter->second, aHandler ) );
        return true;
    }
    return false;
}


UrlRecord PasswordContainer::find(
    const OUString& aURL,
    std::u16string_view aName,
    bool bName, // only needed to support empty user names
    const Reference< XInteractionHandler >& aHandler  )
{
    std::unique_lock aGuard( mMutex );

    if( !m_aContainer.empty() && !aURL.isEmpty() )
    {
        OUString aUrl( aURL );

        // each iteration remove last '/...' section from the aUrl
        // while it's possible, up to the most left '://'
        do
        {
            // first look for <url>/somename and then look for <url>/somename/...
            PasswordMap::iterator aIter = m_aContainer.find( aUrl );
            if( aIter != m_aContainer.end() )
            {
                UrlRecord aRec;
                if ( createUrlRecord( aIter, bName, aName, aHandler, aRec ) )
                  return aRec;
            }
            else
            {
                OUString tmpUrl( aUrl );
                if ( !tmpUrl.endsWith("/") )
                    tmpUrl += "/";

                aIter = m_aContainer.lower_bound( tmpUrl );
                if( aIter != m_aContainer.end() && aIter->first.match( tmpUrl ) )
                {
                    UrlRecord aRec;
                    if ( createUrlRecord( aIter, bName, aName, aHandler, aRec ) )
                      return aRec;
                }
            }
        }
        while( shorterUrl( aUrl ) && !aUrl.isEmpty() );
    }

    return UrlRecord();
}

OUString PasswordContainer::GetDefaultMasterPassword()
{
    OUStringBuffer aResult;
    for ( sal_Int32 nInd = 0; nInd < RTL_DIGEST_LENGTH_MD5; nInd++ )
        aResult.append("aa");

    return aResult.makeStringAndClear();
}

OUString PasswordContainer::RequestPasswordFromUser( PasswordRequestMode aRMode, const uno::Reference< task::XInteractionHandler >& xHandler )
{
    // empty string means that the call was cancelled or just failed
    OUString aResult;

    if ( xHandler.is() )
    {
        ::rtl::Reference< MasterPasswordRequest_Impl > xRequest = new MasterPasswordRequest_Impl( aRMode );

        xHandler->handle( xRequest );

        ::rtl::Reference< ucbhelper::InteractionContinuation > xSelection = xRequest->getSelection();

        if ( xSelection.is() )
        {
            Reference< XInteractionAbort > xAbort( xSelection.get(), UNO_QUERY );
            if ( !xAbort.is() )
            {
                const ::rtl::Reference< ucbhelper::InteractionSupplyAuthentication > & xSupp
                            = xRequest->getAuthenticationSupplier();

                aResult = xSupp->getPassword();
            }
        }
    }

    return aResult;
}

// Mangle the key to match an old bug
static OUString ReencodeAsOldHash(std::u16string_view rPass)
{
    OUStringBuffer aBuffer;
    for (int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ++ind)
    {
        auto tmp = rPass.substr(ind * 2, 2);
        unsigned char i = static_cast<char>(rtl_ustr_toInt64_WithLength(tmp.data(), 16, tmp.size()));
        aBuffer.append(OUStringChar(static_cast< sal_Unicode >('a' + (i >> 4)))
                + OUStringChar(static_cast< sal_Unicode >('a' + (i & 15))));
    }
    return aBuffer.makeStringAndClear();
}

OUString const & PasswordContainer::GetMasterPassword( const Reference< XInteractionHandler >& aHandler )
{
    PasswordRequestMode aRMode = PasswordRequestMode_PASSWORD_ENTER;
    if( !m_xStorageFile || !m_xStorageFile->useStorage() )
        throw NoMasterException(u"Password storing is not active!"_ustr, Reference< XInterface >(), aRMode );

    if( m_aMasterPassword.isEmpty() && aHandler.is() )
    {
        OUString aEncodedMP, aEncodedMPIV;
        bool bDefaultPassword = false;

        if( !m_xStorageFile->getEncodedMasterPassword( aEncodedMP, aEncodedMPIV ) )
            aRMode = PasswordRequestMode_PASSWORD_CREATE;
        else if ( aEncodedMP.isEmpty() )
        {
            m_aMasterPassword = GetDefaultMasterPassword();
            bDefaultPassword = true;
        }

        if ( !bDefaultPassword )
        {
            bool bAskAgain = false;
            do {
                bAskAgain = false;

                OUString aPass = RequestPasswordFromUser( aRMode, aHandler );
                if ( !aPass.isEmpty() )
                {
                    if( aRMode == PasswordRequestMode_PASSWORD_CREATE )
                    {
                        m_aMasterPassword = aPass;
                        std::vector< OUString > aMaster( 1, m_aMasterPassword );

                        OUString sIV = createIV();
                        m_xStorageFile->setEncodedMasterPassword(EncodePasswords(aMaster, sIV, m_aMasterPassword), sIV);
                    }
                    else
                    {
                        if (m_xStorageFile->getStorageVersion() == 0)
                            aPass = ReencodeAsOldHash(aPass);

                        std::vector< OUString > aRM( DecodePasswords( aEncodedMP, aEncodedMPIV, aPass, aRMode ) );
                        if( aRM.empty() || aPass != aRM[0] )
                        {
                            bAskAgain = true;
                            aRMode = PasswordRequestMode_PASSWORD_REENTER;
                        }
                        else
                            m_aMasterPassword = aPass;
                    }
                }

            } while( bAskAgain );
        }
    }

    if ( m_aMasterPassword.isEmpty() )
        throw NoMasterException(u"No master password!"_ustr, Reference< XInterface >(), aRMode );

    return m_aMasterPassword;
}


void SAL_CALL PasswordContainer::remove( const OUString& aURL, const OUString& aName )
{
    std::unique_lock aGuard( mMutex );

    OUString aUrl( aURL );
    if( m_aContainer.empty() )
        return;

    PasswordMap::iterator aIter = m_aContainer.find( aUrl );

    if( aIter == m_aContainer.end() )
    {
        if( aUrl.endsWith("/") )
            aUrl = aUrl.copy( 0, aUrl.getLength() - 1 );
        else
            aUrl += "/";

        aIter = m_aContainer.find( aUrl );
    }

    if( aIter == m_aContainer.end() )
        return;

    auto aNPIter = std::find_if(aIter->second.begin(), aIter->second.end(),
        [&aName](const NamePasswordRecord& rNPRecord) { return rNPRecord.GetUserName() == aName; });

    if (aNPIter != aIter->second.end())
    {
        if( aNPIter->HasPasswords( PERSISTENT_RECORD ) && m_xStorageFile )
            m_xStorageFile->remove( aURL, aName ); // remove record ( aURL, aName )

        // the iterator will not be used any more so it can be removed directly
        aIter->second.erase( aNPIter );

        if( aIter->second.empty() )
            m_aContainer.erase( aIter );
    }
}


void SAL_CALL PasswordContainer::removePersistent( const OUString& aURL, const OUString& aName )
{
    std::unique_lock aGuard( mMutex );

    OUString aUrl( aURL );
    if( m_aContainer.empty() )
        return;

    PasswordMap::iterator aIter = m_aContainer.find( aUrl );

    if( aIter == m_aContainer.end() )
    {
        if( aUrl.endsWith("/") )
            aUrl = aUrl.copy( 0, aUrl.getLength() - 1 );
        else
            aUrl += "/";

        aIter = m_aContainer.find( aUrl );
    }

    if( aIter == m_aContainer.end() )
        return;

    auto aNPIter = std::find_if(aIter->second.begin(), aIter->second.end(),
        [&aName](const NamePasswordRecord& rNPRecord) { return rNPRecord.GetUserName() == aName; });

    if (aNPIter == aIter->second.end())
        return;

    if( aNPIter->HasPasswords( PERSISTENT_RECORD ) )
    {
        // TODO/LATER: should the password be converted to MemoryPassword?
        aNPIter->RemovePasswords( PERSISTENT_RECORD );

        if ( m_xStorageFile )
            m_xStorageFile->remove( aURL, aName ); // remove record ( aURL, aName )
    }

    if( !aNPIter->HasPasswords( MEMORY_RECORD ) )
        aIter->second.erase( aNPIter );

    if( aIter->second.empty() )
        m_aContainer.erase( aIter );
}

void SAL_CALL PasswordContainer::removeAllPersistent()
{
    std::unique_lock aGuard(mMutex);
    removeAllPersistent(aGuard);
}

void PasswordContainer::removeAllPersistent(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if( m_xStorageFile )
        m_xStorageFile->clear();

    for( PasswordMap::iterator aIter = m_aContainer.begin(); aIter != m_aContainer.end(); )
    {
        for( std::vector< NamePasswordRecord >::iterator aNPIter = aIter->second.begin(); aNPIter != aIter->second.end(); )
        {
            if( aNPIter->HasPasswords( PERSISTENT_RECORD ) )
            {
                // TODO/LATER: should the password be converted to MemoryPassword?
                aNPIter->RemovePasswords( PERSISTENT_RECORD );

                if ( m_xStorageFile )
                    m_xStorageFile->remove( aIter->first, aNPIter->GetUserName() ); // remove record ( aURL, aName )
            }

            if( !aNPIter->HasPasswords( MEMORY_RECORD ) )
            {
                aNPIter = aIter->second.erase(aNPIter);
            }
            else
                ++aNPIter;
        }

        if( aIter->second.empty() )
        {
            aIter = m_aContainer.erase(aIter);
        }
        else
            ++aIter;
    }
}

Sequence< UrlRecord > SAL_CALL PasswordContainer::getAllPersistent( const Reference< XInteractionHandler >& xHandler )
{
    std::unique_lock aGuard( mMutex );
    return getAllPersistent(aGuard, xHandler);
}

Sequence< UrlRecord > PasswordContainer::getAllPersistent( std::unique_lock<std::mutex>& /*rGuard*/, const Reference< XInteractionHandler >& xHandler )
{
    Sequence< UrlRecord > aResult;

    for( const auto& rEntry : m_aContainer )
    {
        Sequence< UserRecord > aUsers;
        for (auto const& aNP : rEntry.second)
            if( aNP.HasPasswords( PERSISTENT_RECORD ) )
            {
                sal_Int32 oldLen = aUsers.getLength();
                aUsers.realloc( oldLen + 1 );
                aUsers.getArray()[ oldLen ] = UserRecord( aNP.GetUserName(), comphelper::containerToSequence( DecodePasswords( aNP.GetPersistentPasswords(), aNP.GetPersistentIV(),
                                                                                                                               GetMasterPassword( xHandler ), css::task::PasswordRequestMode_PASSWORD_ENTER ) ) );
            }

        if( aUsers.hasElements() )
        {
            sal_Int32 oldLen = aResult.getLength();
            aResult.realloc( oldLen + 1 );
            aResult.getArray()[ oldLen ] = UrlRecord( rEntry.first, aUsers );
        }
    }

    return aResult;
}

sal_Bool SAL_CALL PasswordContainer::authorizateWithMasterPassword( const uno::Reference< task::XInteractionHandler >& xHandler )
{
    std::unique_lock aGuard( mMutex );
    return authorizateWithMasterPassword(aGuard, xHandler);
}

bool PasswordContainer::authorizateWithMasterPassword( std::unique_lock<std::mutex>& /*rGuard*/, const uno::Reference< task::XInteractionHandler >& xHandler )
{
    bool bResult = false;
    OUString aEncodedMP, aEncodedMPIV;
    uno::Reference< task::XInteractionHandler > xTmpHandler = xHandler;

    // the method should fail if there is no master password
    if( m_xStorageFile && m_xStorageFile->useStorage() && m_xStorageFile->getEncodedMasterPassword( aEncodedMP, aEncodedMPIV ) )
    {
        if ( aEncodedMP.isEmpty() )
        {
            // this is a default master password
            // no UI is necessary
            bResult = true;
        }
        else
        {
            if ( !xTmpHandler.is() )
            {
                uno::Reference< lang::XMultiServiceFactory > xFactory( mComponent, uno::UNO_QUERY_THROW );
                uno::Reference< uno::XComponentContext > xContext( comphelper::getComponentContext(xFactory) );
                xTmpHandler.set( InteractionHandler::createWithParent(xContext, nullptr), uno::UNO_QUERY_THROW );
            }

            if ( !m_aMasterPassword.isEmpty() )
            {
                // there is a password, it should be just rechecked
                PasswordRequestMode aRMode = PasswordRequestMode_PASSWORD_ENTER;
                OUString aPass;

                do {
                    aPass = RequestPasswordFromUser( aRMode, xTmpHandler );

                    if (!aPass.isEmpty() && m_xStorageFile->getStorageVersion() == 0)
                    {
                        aPass = ReencodeAsOldHash(aPass);
                    }

                    bResult = ( !aPass.isEmpty() && aPass == m_aMasterPassword );
                    aRMode = PasswordRequestMode_PASSWORD_REENTER; // further questions with error notification
                } while( !bResult && !aPass.isEmpty() );
            }
            else
            {
                try
                {
                    // ask for the password, if user provide no correct password an exception will be thrown
                    bResult = !GetMasterPassword( xTmpHandler ).isEmpty();
                }
                catch( uno::Exception& )
                {}
            }
        }
    }

    return bResult;
}

sal_Bool SAL_CALL PasswordContainer::changeMasterPassword( const uno::Reference< task::XInteractionHandler >& xHandler )
{
    bool bResult = false;
    uno::Reference< task::XInteractionHandler > xTmpHandler = xHandler;
    std::unique_lock aGuard( mMutex );

    if ( m_xStorageFile && m_xStorageFile->useStorage() )
    {
        if ( !xTmpHandler.is() )
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( mComponent, uno::UNO_QUERY_THROW );
            uno::Reference< uno::XComponentContext > xContext( comphelper::getComponentContext(xFactory) );
            xTmpHandler.set( InteractionHandler::createWithParent(xContext, nullptr), uno::UNO_QUERY_THROW );
        }

        bool bCanChangePassword = true;
        // if there is already a stored master password it should be entered by the user before the change happen
        OUString aEncodedMP, aEncodedMPIV;
        if( !m_aMasterPassword.isEmpty() || m_xStorageFile->getEncodedMasterPassword( aEncodedMP, aEncodedMPIV ) )
            bCanChangePassword = authorizateWithMasterPassword( aGuard, xTmpHandler );

        if ( bCanChangePassword )
        {
            // ask for the new password, but do not set it
            OUString aPass = RequestPasswordFromUser( PasswordRequestMode_PASSWORD_CREATE, xTmpHandler );

            if ( !aPass.isEmpty() )
            {
                // get all the persistent entries if it is possible
                const Sequence< UrlRecord > aPersistent = getAllPersistent( aGuard, uno::Reference< task::XInteractionHandler >() );

                // remove the master password and the entries persistence
                removeMasterPassword(aGuard);

                // store the new master password
                m_aMasterPassword = aPass;
                std::vector< OUString > aMaster( 1, m_aMasterPassword );
                OUString aIV = createIV();
                m_xStorageFile->setEncodedMasterPassword(EncodePasswords(aMaster, aIV, m_aMasterPassword), aIV);

                // store all the entries with the new password
                for ( const auto& rURL : aPersistent )
                    for ( const auto& rUser : rURL.UserList )
                        PrivateAdd( rURL.Url, rUser.UserName, rUser.Passwords, PERSISTENT_RECORD,
                                       uno::Reference< task::XInteractionHandler >() );

                bResult = true;
            }
        }
    }

    return bResult;
}

void SAL_CALL PasswordContainer::removeMasterPassword()
{
    std::unique_lock aGuard(mMutex);
    removeMasterPassword(aGuard);
}

void PasswordContainer::removeMasterPassword(std::unique_lock<std::mutex>& rGuard)
{
    // remove all the stored passwords and the master password
    removeAllPersistent(rGuard);

    if ( m_xStorageFile )
    {
        m_aMasterPassword.clear();
        m_xStorageFile->setEncodedMasterPassword( OUString(), OUString() ); // let the master password be removed from configuration
    }
}

sal_Bool SAL_CALL PasswordContainer::hasMasterPassword(  )
{
    std::unique_lock aGuard( mMutex );

    if ( !m_xStorageFile )
        throw uno::RuntimeException(u"storage file not set"_ustr);

    OUString aEncodedMP, aEncodedMPIV;
    return ( m_xStorageFile->useStorage() && m_xStorageFile->getEncodedMasterPassword( aEncodedMP, aEncodedMPIV ) );
}

sal_Bool SAL_CALL PasswordContainer::allowPersistentStoring( sal_Bool bAllow )
{
    std::unique_lock aGuard( mMutex );

    if ( !m_xStorageFile )
        throw uno::RuntimeException(u"storage file not set"_ustr);

    if ( !bAllow )
        removeMasterPassword(aGuard);

    if (m_xStorageFile->useStorage() == static_cast<bool>(bAllow))
        return bAllow;

    m_xStorageFile->setUseStorage( bAllow );
    return !bAllow;
}

sal_Bool SAL_CALL PasswordContainer::isPersistentStoringAllowed()
{
    std::unique_lock aGuard( mMutex );

    if ( !m_xStorageFile )
        throw uno::RuntimeException(u"storage file not set"_ustr);

    return m_xStorageFile->useStorage();
}

sal_Bool SAL_CALL PasswordContainer::useDefaultMasterPassword( const uno::Reference< task::XInteractionHandler >& xHandler )
{
    bool bResult = false;
    uno::Reference< task::XInteractionHandler > xTmpHandler = xHandler;
    std::unique_lock aGuard( mMutex );

    if ( m_xStorageFile && m_xStorageFile->useStorage() )
    {
        if ( !xTmpHandler.is() )
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( mComponent, uno::UNO_QUERY_THROW );
            uno::Reference< uno::XComponentContext > xContext( comphelper::getComponentContext(xFactory) );
            xTmpHandler.set( InteractionHandler::createWithParent(xContext, nullptr), uno::UNO_QUERY_THROW );
        }

        bool bCanChangePassword = true;
        // if there is already a stored nondefault master password it should be entered by the user before the change happen
        OUString aEncodedMP, aEncodedMPIV;
        if( m_xStorageFile->getEncodedMasterPassword( aEncodedMP, aEncodedMPIV ) && !aEncodedMP.isEmpty() )
            bCanChangePassword = authorizateWithMasterPassword( aGuard, xTmpHandler );

        if ( bCanChangePassword )
        {
            // generate the default password
            OUString aPass = GetDefaultMasterPassword();
            if ( !aPass.isEmpty() )
            {
                // get all the persistent entries if it is possible
                const Sequence< UrlRecord > aPersistent = getAllPersistent( aGuard, uno::Reference< task::XInteractionHandler >() );

                // remove the master password and the entries persistence
                removeMasterPassword(aGuard);

                // store the empty string to flag the default master password
                m_aMasterPassword = aPass;
                m_xStorageFile->setEncodedMasterPassword( OUString(), OUString(), true );

                // store all the entries with the new password
                for ( const auto& rURL : aPersistent )
                    for ( const auto& rUser : rURL.UserList )
                        PrivateAdd( rURL.Url, rUser.UserName, rUser.Passwords, PERSISTENT_RECORD,
                                   uno::Reference< task::XInteractionHandler >() );

                bResult = true;
            }
        }
    }

    return bResult;

}

sal_Bool SAL_CALL PasswordContainer::isDefaultMasterPasswordUsed()
{
    std::unique_lock aGuard( mMutex );

    if ( !m_xStorageFile )
        throw uno::RuntimeException(u"storage file not set"_ustr);

    OUString aEncodedMP, aEncodedMPIV;
    return ( m_xStorageFile->useStorage() && m_xStorageFile->getEncodedMasterPassword( aEncodedMP, aEncodedMPIV ) && aEncodedMP.isEmpty() );
}


void SAL_CALL PasswordContainer::addUrl( const OUString& Url, sal_Bool MakePersistent )
{
    mUrlContainer.add( Url, MakePersistent );
}

OUString SAL_CALL PasswordContainer::findUrl( const OUString& Url )
{
    return mUrlContainer.find( Url );
}

void SAL_CALL PasswordContainer::removeUrl( const OUString& Url )
{
    mUrlContainer.remove( Url );
}

uno::Sequence< OUString > SAL_CALL PasswordContainer::getUrls( sal_Bool OnlyPersistent )
{
    return mUrlContainer.list( OnlyPersistent );
}


void PasswordContainer::Notify()
{
    std::unique_lock aGuard( mMutex );

    // remove the cached persistent values in the memory
    for( auto& rEntry : m_aContainer )
    {
        for( std::vector< NamePasswordRecord >::iterator aNPIter = rEntry.second.begin(); aNPIter != rEntry.second.end(); )
        {
            if( aNPIter->HasPasswords( PERSISTENT_RECORD ) )
            {
                aNPIter->RemovePasswords( PERSISTENT_RECORD );

                if ( m_xStorageFile )
                    m_xStorageFile->remove( rEntry.first, aNPIter->GetUserName() ); // remove record ( aURL, aName )
            }

            if( !aNPIter->HasPasswords( MEMORY_RECORD ) )
            {
                aNPIter = rEntry.second.erase(aNPIter);
            }
            else
                ++aNPIter;
        }
    }

    PasswordMap addon;
    if( m_xStorageFile )
        addon = m_xStorageFile->getInfo();

    for( const auto& rEntry : addon )
    {
        PasswordMap::iterator aSearchIter = m_aContainer.find( rEntry.first );
        if( aSearchIter != m_aContainer.end() )
            for (auto const& aNP : rEntry.second)
                UpdateVector( aSearchIter->first, aSearchIter->second, aNP, false );
        else
            m_aContainer.insert( PairUrlRecord( rEntry.first, rEntry.second ) );
    }
}

OUString SAL_CALL PasswordContainer::getImplementationName(  )
{
    return u"stardiv.svl.PasswordContainer"_ustr;
}

sal_Bool SAL_CALL PasswordContainer::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this,  ServiceName );
}

Sequence< OUString > SAL_CALL PasswordContainer::getSupportedServiceNames(  )
{
    return { u"com.sun.star.task.PasswordContainer"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
svl_PasswordContainer_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new PasswordContainer(context));
}


MasterPasswordRequest_Impl::MasterPasswordRequest_Impl( PasswordRequestMode Mode )
{
    MasterPasswordRequest aRequest;

    aRequest.Classification = InteractionClassification_ERROR;
    aRequest.Mode = Mode;

    setRequest( Any( aRequest ) );

    // Fill continuations...
    Sequence< RememberAuthentication > aRememberModes{ RememberAuthentication_NO };

    m_xAuthSupplier
        = new ::ucbhelper::InteractionSupplyAuthentication(
                this,
                false, // bCanSetRealm
                false,  // bCanSetUserName
                true,  // bCanSetPassword
                false, // bCanSetAccount
                aRememberModes, // rRememberPasswordModes
                RememberAuthentication_NO, // eDefaultRememberPasswordMode
                aRememberModes, // rRememberAccountModes
                RememberAuthentication_NO, // eDefaultRememberAccountMode
                false // bCanUseSystemCredentials
            );

    Sequence<
        Reference< XInteractionContinuation > > aContinuations{
            new ::ucbhelper::InteractionAbort( this ),
            new ::ucbhelper::InteractionRetry( this ),
            m_xAuthSupplier
    };

    setContinuations( aContinuations );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
