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


#include "passwordcontainer.hxx"

#include <unotools/pathoptions.hxx>
#include <cppuhelper/factory.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/MasterPasswordRequest.hpp>
#include <com/sun/star/task/NoMasterException.hpp>

#include <rtl/cipher.h>
#include <rtl/digest.h>
#include <rtl/byteseq.hxx>

using namespace std;
using namespace osl;
using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;


static OUString createIndex( vector< OUString > lines )
{
    OString aResult;
    const sal_Char* pLine;

    for( unsigned int i = 0; i < lines.size(); i++ )
    {
        if( i )
            aResult += OString( "__" );
        OString line = OUStringToOString( lines[i], RTL_TEXTENCODING_UTF8 );
        pLine = line.getStr();

        while( *pLine )
        {
            if (comphelper::string::isalnumAscii(*pLine))
            {
                aResult += OString( *pLine );
            }
            else
            {
                aResult += OString("_");
                aResult += OString::number(  *pLine, 16 );
            }

            pLine++;
        }
    }

    return OUString::createFromAscii( aResult.getStr() );
}


static vector< OUString > getInfoFromInd( OUString aInd )
{
    vector< OUString > aResult;
    sal_Bool aStart = sal_True;

    OString line = OUStringToOString( aInd, RTL_TEXTENCODING_ASCII_US );
    const sal_Char* pLine = line.getStr();
    do
    {
        OUString newItem;
        if( !aStart )
            pLine += 2;
        else
            aStart = sal_False;

        while( *pLine && !( pLine[0] == '_' && pLine[1] == '_' ))
            if( *pLine != '_' )
            {
                newItem += OUString( (sal_Unicode) *pLine );
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
                        OSL_FAIL( "Wrong index syntax!\n" );
                        return aResult;
                    }

                    aNum += OUString( (sal_Unicode) pLine[i] );
                }

                newItem += OUString( (sal_Unicode) aNum.toUInt32( 16 ) );
                pLine += 3;
            }

        aResult.push_back( newItem );
    } while( pLine[0] == '_' && pLine[1] == '_' );

    if( *pLine )
        OSL_FAIL( "Wrong index syntax!\n" );

    return aResult;
}


static sal_Bool shorterUrl( OUString& aURL )
{
    sal_Int32 aInd = aURL.lastIndexOf( sal_Unicode( '/' ) );
    if( aInd > 0 && aURL.indexOf( "://" ) != aInd-2 )
    {
        aURL = aURL.copy( 0, aInd );
        return sal_True;
    }

    return sal_False;
}


static OUString getAsciiLine( const ::rtl::ByteSequence& buf )
{
    OUString aResult;

    ::rtl::ByteSequence outbuf( buf.getLength()*2+1 );

    for( int ind = 0; ind < buf.getLength(); ind++ )
    {
        outbuf[ind*2]   = ( ((sal_uInt8)buf[ind]) >> 4 ) + 'a';
        outbuf[ind*2+1] = ( ((sal_uInt8)buf[ind]) & 0x0f ) + 'a';
    }
    outbuf[buf.getLength()*2] = '\0';

    aResult = OUString::createFromAscii( (sal_Char*)outbuf.getArray() );

    return aResult;
}


static ::rtl::ByteSequence getBufFromAsciiLine( OUString line )
{
    OSL_ENSURE( line.getLength() % 2 == 0, "Wrong syntax!\n" );
    OString tmpLine = OUStringToOString( line, RTL_TEXTENCODING_ASCII_US );
    ::rtl::ByteSequence aResult(line.getLength()/2);

    for( int ind = 0; ind < tmpLine.getLength()/2; ind++ )
    {
        aResult[ind] = ( (sal_uInt8)( tmpLine.getStr()[ind*2] - 'a' ) << 4 ) | (sal_uInt8)( tmpLine.getStr()[ind*2+1] - 'a' );
    }

    return aResult;
}


static Sequence< OUString > copyVectorToSequence( const vector< OUString >& original )
{
    Sequence< OUString > newOne ( original.size() );
    for( unsigned int i = 0; i < original.size() ; i++ )
        newOne[i] = original[i];

    return newOne;
}

static vector< OUString > copySequenceToVector( const Sequence< OUString >& original )
{
    vector< OUString > newOne ( original.getLength() );
    for( int i = 0; i < original.getLength() ; i++ )
        newOne[i] = original[i];

    return newOne;
}


PassMap StorageItem::getInfo()
{
    PassMap aResult;

    Sequence< OUString > aNodeNames     = ConfigItem::GetNodeNames( OUString("Store") );
    sal_Int32 aNodeCount = aNodeNames.getLength();
    Sequence< OUString > aPropNames( aNodeCount );
    sal_Int32 aNodeInd;

    for( aNodeInd = 0; aNodeInd < aNodeCount; ++aNodeInd )
    {
        aPropNames[aNodeInd]  = OUString("Store/Passwordstorage['");
        aPropNames[aNodeInd] += aNodeNames[aNodeInd];
        aPropNames[aNodeInd] += OUString("']/Password");
    }

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aPropNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_ENSURE( aPropertyValues.getLength() == aNodeNames.getLength(), "Problems during reading\n" );
        return aResult;
    }

    for( aNodeInd = 0; aNodeInd < aNodeCount; ++aNodeInd )
    {
        vector< OUString > aUrlUsr = getInfoFromInd( aNodeNames[aNodeInd] );

        if( aUrlUsr.size() == 2 )
        {
            OUString aUrl  = aUrlUsr[0];
            OUString aName = aUrlUsr[1];

            OUString aEPasswd;
            aPropertyValues[aNodeInd] >>= aEPasswd;

            PassMap::iterator aIter = aResult.find( aUrl );
            if( aIter != aResult.end() )
                aIter->second.push_back( NamePassRecord( aName, aEPasswd ) );
            else
            {
                NamePassRecord aNewRecord( aName, aEPasswd );
                list< NamePassRecord > listToAdd( 1, aNewRecord );

                aResult.insert( PairUrlRecord( aUrl, listToAdd ) );
            }
        }
        else
            OSL_FAIL( "Wrong index sintax!\n" );
    }

    return aResult;
}


void StorageItem::setUseStorage( bool bUse )
{
    Sequence< OUString > sendNames(1);
    Sequence< uno::Any > sendVals(1);

    sendNames[0] = OUString("UseStorage");

    sendVals[0] <<= bUse;

    ConfigItem::SetModified();
    ConfigItem::PutProperties( sendNames, sendVals );
}


bool StorageItem::useStorage()
{
    Sequence< OUString > aNodeNames( 1 );
    aNodeNames[0] = OUString("UseStorage");

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aNodeNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_ENSURE( aPropertyValues.getLength() == aNodeNames.getLength(), "Problems during reading\n" );
        return sal_False;
    }

    bool aResult = false;
    aPropertyValues[0] >>= aResult;

    return aResult;
}


bool StorageItem::getEncodedMP( OUString& aResult )
{
    if( hasEncoded )
    {
        aResult = mEncoded;
        return true;
    }

    Sequence< OUString > aNodeNames( 2 );
    aNodeNames[0] = OUString("HasMaster");
    aNodeNames[1] = OUString("Master");

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aNodeNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_ENSURE( aPropertyValues.getLength() == aNodeNames.getLength(), "Problems during reading\n" );
        return false;
    }

    aPropertyValues[0] >>= hasEncoded;
    aPropertyValues[1] >>= mEncoded;

    aResult = mEncoded;

    return hasEncoded;
}


void StorageItem::setEncodedMP( const OUString& aEncoded, bool bAcceptEmpty )
{
    Sequence< OUString > sendNames(2);
    Sequence< uno::Any > sendVals(2);

    sendNames[0] = OUString("HasMaster");
    sendNames[1] = OUString("Master");

    sal_Bool bHasMaster = ( !aEncoded.isEmpty() || bAcceptEmpty );
    sendVals[0] <<= bHasMaster;
    sendVals[1] <<= aEncoded;

    ConfigItem::SetModified();
    ConfigItem::PutProperties( sendNames, sendVals );

    hasEncoded = bHasMaster;
    mEncoded = aEncoded;
}


void StorageItem::remove( const OUString& aURL, const OUString& aName )
{
    vector < OUString > forIndex;
    forIndex.push_back( aURL );
    forIndex.push_back( aName );

    Sequence< OUString > sendSeq(1);

    sendSeq[0] = createIndex( forIndex );

    ConfigItem::ClearNodeElements( OUString("Store"), sendSeq );
}


void StorageItem::clear()
{
    Sequence< OUString > sendSeq(1);

    ConfigItem::ClearNodeSet( OUString("Store") );
}


void StorageItem::update( const OUString& aURL, const NamePassRecord& aRecord )
{
    if ( !aRecord.HasPasswords( PERSISTENT_RECORD ) )
    {
        OSL_FAIL( "Unexpected storing of a record!" );
        return;
    }

    vector < OUString > forIndex;
    forIndex.push_back( aURL );
    forIndex.push_back( aRecord.GetUserName() );

    Sequence< beans::PropertyValue > sendSeq(1);

    sendSeq[0].Name  = OUString("Store/Passwordstorage['");
    sendSeq[0].Name += createIndex( forIndex );
    sendSeq[0].Name += OUString("']/Password");

    sendSeq[0].Value <<= aRecord.GetPersPasswords();

    ConfigItem::SetModified();
    ConfigItem::SetSetProperties( OUString("Store"), sendSeq );
}


void StorageItem::Notify( const Sequence< OUString >& )
{
    // this feature still should not be used
    if( mainCont )
        mainCont->Notify();
}


void StorageItem::Commit()
{
    // Do nothing, we stored everything we want already
}


PasswordContainer::PasswordContainer( const Reference<XMultiServiceFactory>& xServiceFactory ):
    m_pStorageFile( NULL )
{
    // m_pStorageFile->Notify() can be called
    ::osl::MutexGuard aGuard( mMutex );

    mComponent = Reference< XComponent >( xServiceFactory, UNO_QUERY );
    mComponent->addEventListener( this );

    m_pStorageFile = new StorageItem( this, OUString("Office.Common/Passwords") );
    if( m_pStorageFile )
        if( m_pStorageFile->useStorage() )
            m_aContainer = m_pStorageFile->getInfo();
}


PasswordContainer::~PasswordContainer()
{
    ::osl::MutexGuard aGuard( mMutex );

    if( m_pStorageFile )
    {
        delete m_pStorageFile;
        m_pStorageFile = NULL;
    }

    if( mComponent.is() )
    {
        mComponent->removeEventListener(this);
        mComponent.clear();
    }
}


void SAL_CALL PasswordContainer::disposing( const EventObject& ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if( m_pStorageFile )
    {
        delete m_pStorageFile;
        m_pStorageFile = NULL;
    }

    if( mComponent.is() )
    {
        //mComponent->removeEventListener(this);
        mComponent.clear();
    }
}


vector< OUString > PasswordContainer::DecodePasswords( const OUString& aLine, const OUString& aMasterPasswd ) throw(RuntimeException)
{
    if( !aMasterPasswd.isEmpty() )
    {
        rtlCipher aDecoder = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream );
        OSL_ENSURE( aDecoder, "Can't create decoder\n" );

        if( aDecoder )
        {
            OSL_ENSURE( aMasterPasswd.getLength() == RTL_DIGEST_LENGTH_MD5 * 2, "Wrong master password format!\n" );

            unsigned char code[RTL_DIGEST_LENGTH_MD5];
            for( int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ind++ )
                code[ ind ] = (char)(aMasterPasswd.copy( ind*2, 2 ).toUInt32(16));

            rtlCipherError result = rtl_cipher_init (
                    aDecoder, rtl_Cipher_DirectionDecode,
                    code, RTL_DIGEST_LENGTH_MD5, NULL, 0 );

            if( result == rtl_Cipher_E_None )
            {
                ::rtl::ByteSequence aSeq = getBufFromAsciiLine( aLine );

                ::rtl::ByteSequence resSeq( aSeq.getLength() );

                result = rtl_cipher_decode ( aDecoder, (sal_uInt8*)aSeq.getArray(), aSeq.getLength(),
                                                        (sal_uInt8*)resSeq.getArray(), resSeq.getLength() );

                OUString aPasswd( ( sal_Char* )resSeq.getArray(), resSeq.getLength(), RTL_TEXTENCODING_UTF8 );

                rtl_cipher_destroy (aDecoder);

                return getInfoFromInd( aPasswd );
            }

            rtl_cipher_destroy (aDecoder);
        }
    }
    else
    {
        OSL_FAIL( "No master password provided!\n" );
        // throw special exception
    }

    // problems with decoding
    OSL_FAIL( "Problem with decoding\n" );
    throw RuntimeException("Can't decode!", Reference< XInterface >() );
}



OUString PasswordContainer::EncodePasswords( vector< OUString > lines, const OUString& aMasterPasswd ) throw(RuntimeException)
{
    if( !aMasterPasswd.isEmpty() )
    {
        OString aSeq = OUStringToOString( createIndex( lines ), RTL_TEXTENCODING_UTF8 );

        rtlCipher aEncoder = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream );
        OSL_ENSURE( aEncoder, "Can't create encoder\n" );

        if( aEncoder )
        {
            OSL_ENSURE( aMasterPasswd.getLength() == RTL_DIGEST_LENGTH_MD5 * 2, "Wrong master password format!\n" );

            unsigned char code[RTL_DIGEST_LENGTH_MD5];
            for( int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ind++ )
                code[ ind ] = (char)(aMasterPasswd.copy( ind*2, 2 ).toUInt32(16));

            rtlCipherError result = rtl_cipher_init (
                    aEncoder, rtl_Cipher_DirectionEncode,
                    code, RTL_DIGEST_LENGTH_MD5, NULL, 0 );

            if( result == rtl_Cipher_E_None )
            {
                ::rtl::ByteSequence resSeq(aSeq.getLength()+1);

                result = rtl_cipher_encode ( aEncoder, (sal_uInt8*)aSeq.getStr(), aSeq.getLength()+1,
                                                        (sal_uInt8*)resSeq.getArray(), resSeq.getLength() );

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
                                testOU = OUString();
                    }

                    result = rtl_cipher_decode ( aEncoder, (sal_uInt8*)aSeq1.getArray(), aSeq1.getLength(),
                                                        (sal_uInt8*)resSeq1.getArray(), resSeq1.getLength() );

                    OUString aPasswd( ( sal_Char* )resSeq1.getArray(), resSeq1.getLength(), RTL_TEXTENCODING_UTF8 );
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
        OSL_FAIL( "No master password provided!\n" );
        // throw special exception
    }

    // problems with encoding
    OSL_FAIL( "Problem with encoding\n" );
    throw RuntimeException("Can't encode!", Reference< XInterface >() );
}


void PasswordContainer::UpdateVector( const OUString& aURL, list< NamePassRecord >& toUpdate, NamePassRecord& aRecord, bool writeFile ) throw(RuntimeException)
{
    for( list< NamePassRecord >::iterator aNPIter = toUpdate.begin(); aNPIter != toUpdate.end(); ++aNPIter )
        if( aNPIter->GetUserName().equals( aRecord.GetUserName() ) )
        {
            if( aRecord.HasPasswords( MEMORY_RECORD ) )
                aNPIter->SetMemPasswords( aRecord.GetMemPasswords() );

            if( aRecord.HasPasswords( PERSISTENT_RECORD ) )
            {
                aNPIter->SetPersPasswords( aRecord.GetPersPasswords() );

                if( writeFile )
                {
                    // the password must be already encoded
                    m_pStorageFile->update( aURL, aRecord ); // change existing ( aURL, aName ) record in the configfile
                }
            }

            return;
        }


    if( aRecord.HasPasswords( PERSISTENT_RECORD ) && writeFile )
    {
        // the password must be already encoded
        m_pStorageFile->update( aURL, aRecord ); // add new aName to the existing url
    }

    toUpdate.insert( toUpdate.begin(), aRecord );
}


UserRecord PasswordContainer::CopyToUserRecord( const NamePassRecord& aRecord, bool& io_bTryToDecode, const Reference< XInteractionHandler >& aHandler )
{
    ::std::vector< OUString > aPasswords;
    if( aRecord.HasPasswords( MEMORY_RECORD ) )
        aPasswords = aRecord.GetMemPasswords();

    if( io_bTryToDecode && aRecord.HasPasswords( PERSISTENT_RECORD ) )
    {
        try
        {
            ::std::vector< OUString > aDecodedPasswords = DecodePasswords( aRecord.GetPersPasswords(), GetMasterPassword( aHandler ) );
            aPasswords.insert( aPasswords.end(), aDecodedPasswords.begin(), aDecodedPasswords.end() );
        }
        catch( NoMasterException& )
        {
            // if master password could not be detected the entry will be just ignored
            io_bTryToDecode = false;
        }
    }

    return UserRecord( aRecord.GetUserName(), copyVectorToSequence( aPasswords ) );
}


Sequence< UserRecord > PasswordContainer::CopyToUserRecordSequence( const list< NamePassRecord >& original, const Reference< XInteractionHandler >& aHandler ) throw(RuntimeException)
{
    Sequence< UserRecord >     aResult( original.size() );
    sal_uInt32 nInd = 0;
    bool bTryToDecode = true;

    for( list< NamePassRecord >::const_iterator aNPIter = original.begin();
         aNPIter != original.end();
         ++aNPIter, ++nInd )
    {
        aResult[nInd] = CopyToUserRecord( *aNPIter, bTryToDecode, aHandler );
    }

    return aResult;
}


void SAL_CALL PasswordContainer::add( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, const Reference< XInteractionHandler >& aHandler ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    PrivateAdd( Url, UserName, Passwords, MEMORY_RECORD, aHandler );
}


void SAL_CALL PasswordContainer::addPersistent( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, const Reference< XInteractionHandler >& aHandler  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    PrivateAdd( Url, UserName, Passwords, PERSISTENT_RECORD, aHandler );
}


void PasswordContainer::PrivateAdd( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, char Mode, const Reference< XInteractionHandler >& aHandler ) throw(RuntimeException)
{
    NamePassRecord aRecord( UserName );
    ::std::vector< OUString > aStorePass = copySequenceToVector( Passwords );

    if( Mode == PERSISTENT_RECORD )
        aRecord.SetPersPasswords( EncodePasswords( aStorePass, GetMasterPassword( aHandler ) ) );
    else if( Mode == MEMORY_RECORD )
        aRecord.SetMemPasswords( aStorePass );
    else
    {
        OSL_FAIL( "Unexpected persistence status!" );
        return;
    }

    if( !m_aContainer.empty() )
    {
        PassMap::iterator aIter = m_aContainer.find( Url );

        if( aIter != m_aContainer.end() )
        {
            UpdateVector( aIter->first, aIter->second, aRecord, sal_True );
            return;
        }
    }

    list< NamePassRecord > listToAdd( 1, aRecord );
    m_aContainer.insert( PairUrlRecord( Url, listToAdd ) );

    if( Mode == PERSISTENT_RECORD && m_pStorageFile && m_pStorageFile->useStorage() )
        m_pStorageFile->update( Url, aRecord );

}



UrlRecord SAL_CALL PasswordContainer::find( const OUString& aURL, const Reference< XInteractionHandler >& aHandler  ) throw(RuntimeException)
{
    return find( aURL, OUString(), false, aHandler );
}


UrlRecord SAL_CALL PasswordContainer::findForName( const OUString& aURL, const OUString& aName, const Reference< XInteractionHandler >& aHandler  ) throw(RuntimeException)
{
    return find( aURL, aName, true, aHandler );
}


Sequence< UserRecord > PasswordContainer::FindUsr( const list< NamePassRecord >& userlist, const OUString& aName, const Reference< XInteractionHandler >& aHandler ) throw(RuntimeException)
{
    sal_uInt32 nInd = 0;
    for( list< NamePassRecord >::const_iterator aNPIter = userlist.begin();
         aNPIter != userlist.end();
         ++aNPIter, ++nInd )
    {
        if( aNPIter->GetUserName().equals( aName ) )
        {
            Sequence< UserRecord > aResult(1);
            bool bTryToDecode = true;
            aResult[0] = CopyToUserRecord( *aNPIter, bTryToDecode, aHandler );

            return aResult;
        }
    }

    return Sequence< UserRecord >();
}


bool PasswordContainer::createUrlRecord(
    const PassMap::iterator & rIter,
    bool bName,
    const OUString & aName,
    const Reference< XInteractionHandler >& aHandler,
    UrlRecord & rRec )
        throw( RuntimeException )
{
    if ( bName )
    {
        Sequence< UserRecord > aUsrRec
            = FindUsr( rIter->second, aName, aHandler );
        if( aUsrRec.getLength() )
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
    const OUString& aName,
    bool bName, // only needed to support empty user names
    const Reference< XInteractionHandler >& aHandler  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if( !m_aContainer.empty() && !aURL.isEmpty() )
    {
        OUString aUrl( aURL );

        // each iteration remove last '/...' section from the aUrl
        // while it's possible, up to the most left '://'
        do
        {
            // first look for <url>/somename and then look for <url>/somename/...
            PassMap::iterator aIter = m_aContainer.find( aUrl );
            if( aIter != m_aContainer.end() )
            {
                UrlRecord aRec;
                if ( createUrlRecord( aIter, bName, aName, aHandler, aRec ) )
                  return aRec;
            }
            else
            {
                OUString tmpUrl( aUrl );
                if ( tmpUrl.getStr()[tmpUrl.getLength() - 1] != (sal_Unicode)'/' )
                    tmpUrl += OUString("/");

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
    OUString aResult;
    for ( sal_Int32 nInd = 0; nInd < RTL_DIGEST_LENGTH_MD5; nInd++ )
        aResult += OUString( "aa"  );

    return aResult;
}

OUString PasswordContainer::RequestPasswordFromUser( PasswordRequestMode aRMode, const uno::Reference< task::XInteractionHandler >& xHandler )
{
    // empty string means that the call was cancelled or just failed
    OUString aResult;

    if ( xHandler.is() )
    {
        ::rtl::Reference< MasterPasswordRequest_Impl > xRequest = new MasterPasswordRequest_Impl( aRMode );

        xHandler->handle( xRequest.get() );

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


OUString PasswordContainer::GetMasterPassword( const Reference< XInteractionHandler >& aHandler ) throw(RuntimeException)
{
    PasswordRequestMode aRMode = PasswordRequestMode_PASSWORD_ENTER;
    if( !m_pStorageFile || !m_pStorageFile->useStorage() )
        throw NoMasterException("Password storing is not active!", Reference< XInterface >(), aRMode );

    if( m_aMasterPasswd.isEmpty() && aHandler.is() )
    {
        OUString aEncodedMP;
        sal_Bool bAskAgain = sal_False;
        sal_Bool bDefaultPassword = sal_False;

        if( !m_pStorageFile->getEncodedMP( aEncodedMP ) )
            aRMode = PasswordRequestMode_PASSWORD_CREATE;
        else if ( aEncodedMP.isEmpty() )
        {
            m_aMasterPasswd = GetDefaultMasterPassword();
            bDefaultPassword = sal_True;
        }

        if ( !bDefaultPassword )
        {
            do {
                bAskAgain = sal_False;

                OUString aPass = RequestPasswordFromUser( aRMode, aHandler );
                if ( !aPass.isEmpty() )
                {
                    if( aRMode == PasswordRequestMode_PASSWORD_CREATE )
                    {
                        m_aMasterPasswd = aPass;
                        vector< OUString > aMaster( 1, m_aMasterPasswd );

                        m_pStorageFile->setEncodedMP( EncodePasswords( aMaster, m_aMasterPasswd ) );
                    }
                    else
                    {
                        vector< OUString > aRM( DecodePasswords( aEncodedMP, aPass ) );
                        if( aRM.empty() || !aPass.equals( aRM[0] ) )
                        {
                            bAskAgain = sal_True;
                            aRMode = PasswordRequestMode_PASSWORD_REENTER;
                        }
                        else
                            m_aMasterPasswd = aPass;
                    }
                }

            } while( bAskAgain );
        }
    }

    if ( m_aMasterPasswd.isEmpty() )
        throw NoMasterException("No master password!", Reference< XInterface >(), aRMode );

    return m_aMasterPasswd;
}


void SAL_CALL PasswordContainer::remove( const OUString& aURL, const OUString& aName ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    OUString aUrl( aURL );
    if( !m_aContainer.empty() )
    {
        PassMap::iterator aIter = m_aContainer.find( aUrl );

        if( aIter == m_aContainer.end() )
        {
            sal_Int32 aInd = aUrl.lastIndexOf( sal_Unicode( '/' ) );
            if( aInd > 0 && aUrl.getLength()-1 == aInd )
                aUrl = aUrl.copy( 0, aUrl.getLength() - 1 );
            else
                aUrl += OUString("/");

            aIter = m_aContainer.find( aUrl );
        }

        if( aIter != m_aContainer.end() )
        {
            for( list< NamePassRecord >::iterator aNPIter = aIter->second.begin(); aNPIter != aIter->second.end(); ++aNPIter )
                if( aNPIter->GetUserName().equals( aName ) )
                {
                    if( aNPIter->HasPasswords( PERSISTENT_RECORD ) && m_pStorageFile )
                        m_pStorageFile->remove( aURL, aName ); // remove record ( aURL, aName )

                    // the iterator will not be used any more so it can be removed directly
                    aIter->second.erase( aNPIter );

                    if( aIter->second.begin() == aIter->second.end() )
                        m_aContainer.erase( aIter );

                    return;
                }
        }
    }
}


void SAL_CALL PasswordContainer::removePersistent( const OUString& aURL, const OUString& aName ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    OUString aUrl( aURL );
    if( !m_aContainer.empty() )
    {
        PassMap::iterator aIter = m_aContainer.find( aUrl );

        if( aIter == m_aContainer.end() )
        {
            sal_Int32 aInd = aUrl.lastIndexOf( sal_Unicode( '/' ) );
            if( aInd > 0 && aUrl.getLength()-1 == aInd )
                aUrl = aUrl.copy( 0, aUrl.getLength() - 1 );
            else
                aUrl += OUString("/");

            aIter = m_aContainer.find( aUrl );
        }

        if( aIter != m_aContainer.end() )
        {
            for( list< NamePassRecord >::iterator aNPIter = aIter->second.begin(); aNPIter != aIter->second.end(); ++aNPIter )
                if( aNPIter->GetUserName().equals( aName ) )
                {
                    if( aNPIter->HasPasswords( PERSISTENT_RECORD ) )
                    {
                        // TODO/LATER: should the password be converted to MemoryPassword?
                        aNPIter->RemovePasswords( PERSISTENT_RECORD );

                        if ( m_pStorageFile )
                            m_pStorageFile->remove( aURL, aName ); // remove record ( aURL, aName )
                    }

                    if( !aNPIter->HasPasswords( MEMORY_RECORD ) )
                        aIter->second.erase( aNPIter );

                    if( aIter->second.begin() == aIter->second.end() )
                        m_aContainer.erase( aIter );

                    return;
                }
        }
    }
}

void SAL_CALL PasswordContainer::removeAllPersistent() throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if( m_pStorageFile )
        m_pStorageFile->clear();

    for( PassMap::iterator aIter = m_aContainer.begin(); aIter != m_aContainer.end(); )
    {
        for( list< NamePassRecord >::iterator aNPIter = aIter->second.begin(); aNPIter != aIter->second.end(); )
        {
            if( aNPIter->HasPasswords( PERSISTENT_RECORD ) )
            {
                // TODO/LATER: should the password be converted to MemoryPassword?
                aNPIter->RemovePasswords( PERSISTENT_RECORD );

                if ( m_pStorageFile )
                    m_pStorageFile->remove( aIter->first, aNPIter->GetUserName() ); // remove record ( aURL, aName )
            }

            if( !aNPIter->HasPasswords( MEMORY_RECORD ) )
            {
                list< NamePassRecord >::iterator aIterToDelete( aNPIter );
                ++aNPIter;
                aIter->second.erase( aIterToDelete );
            }
            else
                ++aNPIter;
        }

        if( aIter->second.begin() == aIter->second.end() )
        {
            PassMap::iterator aIterToDelete( aIter );
            ++aIter;
            m_aContainer.erase( aIterToDelete );
        }
        else
            ++aIter;
    }
}

Sequence< UrlRecord > SAL_CALL PasswordContainer::getAllPersistent( const Reference< XInteractionHandler >& xHandler ) throw(RuntimeException)
{
    Sequence< UrlRecord > aResult;

    ::osl::MutexGuard aGuard( mMutex );
    for( PassMap::iterator aIter = m_aContainer.begin(); aIter != m_aContainer.end(); ++aIter )
    {
        Sequence< UserRecord > aUsers;
        for( list< NamePassRecord >::iterator aNPIter = aIter->second.begin(); aNPIter != aIter->second.end(); ++aNPIter )
            if( aNPIter->HasPasswords( PERSISTENT_RECORD ) )
            {
                sal_Int32 oldLen = aUsers.getLength();
                aUsers.realloc( oldLen + 1 );
                aUsers[ oldLen ] = UserRecord( aNPIter->GetUserName(), copyVectorToSequence( DecodePasswords( aNPIter->GetPersPasswords(), GetMasterPassword( xHandler ) ) ) );
            }

        if( aUsers.getLength() )
        {
            sal_Int32 oldLen = aResult.getLength();
            aResult.realloc( oldLen + 1 );
            aResult[ oldLen ] = UrlRecord( aIter->first, aUsers );
        }
    }

    return aResult;
}

sal_Bool SAL_CALL PasswordContainer::authorizateWithMasterPassword( const uno::Reference< task::XInteractionHandler >& xHandler )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = sal_False;
    OUString aEncodedMP;
    uno::Reference< task::XInteractionHandler > xTmpHandler = xHandler;
    ::osl::MutexGuard aGuard( mMutex );

    // the method should fail if there is no master password
    if( m_pStorageFile && m_pStorageFile->useStorage() && m_pStorageFile->getEncodedMP( aEncodedMP ) )
    {
        if ( aEncodedMP.isEmpty() )
        {
            // this is a default master password
            // no UI is necessary
            bResult = sal_True;
        }
        else
        {
            if ( !xTmpHandler.is() )
            {
                uno::Reference< lang::XMultiServiceFactory > xFactory( mComponent, uno::UNO_QUERY_THROW );
                uno::Reference< uno::XComponentContext > xContext( comphelper::getComponentContext(xFactory) );
                xTmpHandler.set( InteractionHandler::createWithParent(xContext, 0), uno::UNO_QUERY_THROW );
            }

            if ( !m_aMasterPasswd.isEmpty() )
            {
                // there is a password, it should be just rechecked
                PasswordRequestMode aRMode = PasswordRequestMode_PASSWORD_ENTER;
                OUString aPass;

                do {
                    aPass = RequestPasswordFromUser( aRMode, xTmpHandler );
                    bResult = ( !aPass.isEmpty() && aPass.equals( m_aMasterPasswd ) );
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
    throw (uno::RuntimeException)
{
    sal_Bool bResult = sal_False;
    uno::Reference< task::XInteractionHandler > xTmpHandler = xHandler;
    ::osl::MutexGuard aGuard( mMutex );

    if ( m_pStorageFile && m_pStorageFile->useStorage() )
    {
        if ( !xTmpHandler.is() )
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( mComponent, uno::UNO_QUERY_THROW );
            uno::Reference< uno::XComponentContext > xContext( comphelper::getComponentContext(xFactory) );
            xTmpHandler.set( InteractionHandler::createWithParent(xContext, 0), uno::UNO_QUERY_THROW );
        }

        sal_Bool bCanChangePassword = sal_True;
        // if there is already a stored master password it should be entered by the user before the change happen
        OUString aEncodedMP;
        if( !m_aMasterPasswd.isEmpty() || m_pStorageFile->getEncodedMP( aEncodedMP ) )
            bCanChangePassword = authorizateWithMasterPassword( xTmpHandler );

        if ( bCanChangePassword )
        {
            // ask for the new password, but do not set it
            PasswordRequestMode aRMode = PasswordRequestMode_PASSWORD_CREATE;
            OUString aPass = RequestPasswordFromUser( aRMode, xTmpHandler );

            if ( !aPass.isEmpty() )
            {
                // get all the persistent entries if it is possible
                Sequence< UrlRecord > aPersistent = getAllPersistent( uno::Reference< task::XInteractionHandler >() );

                // remove the master password and the entries persistence
                removeMasterPassword();

                // store the new master password
                m_aMasterPasswd = aPass;
                vector< OUString > aMaster( 1, m_aMasterPasswd );
                m_pStorageFile->setEncodedMP( EncodePasswords( aMaster, m_aMasterPasswd ) );

                // store all the entries with the new password
                for ( int nURLInd = 0; nURLInd < aPersistent.getLength(); nURLInd++ )
                    for ( int nNameInd = 0; nNameInd< aPersistent[nURLInd].UserList.getLength(); nNameInd++ )
                        addPersistent( aPersistent[nURLInd].Url,
                                       aPersistent[nURLInd].UserList[nNameInd].UserName,
                                       aPersistent[nURLInd].UserList[nNameInd].Passwords,
                                       uno::Reference< task::XInteractionHandler >() );

                bResult = sal_True;
            }
        }
    }

    return bResult;
}

void SAL_CALL PasswordContainer::removeMasterPassword()
    throw (uno::RuntimeException)
{
    // remove all the stored passwords and the master password
    removeAllPersistent();

    ::osl::MutexGuard aGuard( mMutex );
    if ( m_pStorageFile )
    {
        m_aMasterPasswd = OUString();
        m_pStorageFile->setEncodedMP( OUString() ); // let the master password be removed from configuration
    }
}

::sal_Bool SAL_CALL PasswordContainer::hasMasterPassword(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if ( !m_pStorageFile )
        throw uno::RuntimeException();

    OUString aEncodedMP;
    return ( m_pStorageFile->useStorage() && m_pStorageFile->getEncodedMP( aEncodedMP ) );
}

::sal_Bool SAL_CALL PasswordContainer::allowPersistentStoring( ::sal_Bool bAllow )
    throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if ( !m_pStorageFile )
        throw uno::RuntimeException();

    if ( !bAllow )
        removeMasterPassword();

    if (m_pStorageFile->useStorage() == static_cast<bool>(bAllow))
        return bAllow;

    m_pStorageFile->setUseStorage( bAllow );
    return !bAllow;
}

::sal_Bool SAL_CALL PasswordContainer::isPersistentStoringAllowed()
    throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if ( !m_pStorageFile )
        throw uno::RuntimeException();

    return m_pStorageFile->useStorage();
}

::sal_Bool SAL_CALL PasswordContainer::useDefaultMasterPassword( const uno::Reference< task::XInteractionHandler >& xHandler )
    throw ( uno::RuntimeException )
{
    sal_Bool bResult = sal_False;
    uno::Reference< task::XInteractionHandler > xTmpHandler = xHandler;
    ::osl::MutexGuard aGuard( mMutex );

    if ( m_pStorageFile && m_pStorageFile->useStorage() )
    {
        if ( !xTmpHandler.is() )
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( mComponent, uno::UNO_QUERY_THROW );
            uno::Reference< uno::XComponentContext > xContext( comphelper::getComponentContext(xFactory) );
            xTmpHandler.set( InteractionHandler::createWithParent(xContext, 0), uno::UNO_QUERY_THROW );
        }

        sal_Bool bCanChangePassword = sal_True;
        // if there is already a stored nondefault master password it should be entered by the user before the change happen
        OUString aEncodedMP;
        if( m_pStorageFile->getEncodedMP( aEncodedMP ) && !aEncodedMP.isEmpty() )
            bCanChangePassword = authorizateWithMasterPassword( xTmpHandler );

        if ( bCanChangePassword )
        {
            // generate the default password
            OUString aPass = GetDefaultMasterPassword();
            if ( !aPass.isEmpty() )
            {
                // get all the persistent entries if it is possible
                Sequence< UrlRecord > aPersistent = getAllPersistent( uno::Reference< task::XInteractionHandler >() );

                // remove the master password and the entries persistence
                removeMasterPassword();

                // store the empty string to flag the default master password
                m_aMasterPasswd = aPass;
                m_pStorageFile->setEncodedMP( OUString(), sal_True );

                // store all the entries with the new password
                for ( int nURLInd = 0; nURLInd < aPersistent.getLength(); nURLInd++ )
                    for ( int nNameInd = 0; nNameInd< aPersistent[nURLInd].UserList.getLength(); nNameInd++ )
                        addPersistent( aPersistent[nURLInd].Url,
                                       aPersistent[nURLInd].UserList[nNameInd].UserName,
                                       aPersistent[nURLInd].UserList[nNameInd].Passwords,
                                       uno::Reference< task::XInteractionHandler >() );

                bResult = sal_True;
            }
        }
    }

    return bResult;

}

::sal_Bool SAL_CALL PasswordContainer::isDefaultMasterPasswordUsed()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( mMutex );

    if ( !m_pStorageFile )
        throw uno::RuntimeException();

    OUString aEncodedMP;
    return ( m_pStorageFile->useStorage() && m_pStorageFile->getEncodedMP( aEncodedMP ) && aEncodedMP.isEmpty() );
}


void SAL_CALL PasswordContainer::addUrl( const OUString& Url, ::sal_Bool MakePersistent )
    throw (uno::RuntimeException)
{
    mUrlContainer.add( Url, MakePersistent );
}

OUString SAL_CALL PasswordContainer::findUrl( const OUString& Url )
    throw (uno::RuntimeException)
{
    return mUrlContainer.find( Url );
}

void SAL_CALL PasswordContainer::removeUrl( const OUString& Url )
    throw (uno::RuntimeException)
{
    mUrlContainer.remove( Url );
}

uno::Sequence< OUString > SAL_CALL PasswordContainer::getUrls( ::sal_Bool OnlyPersistent )
    throw (uno::RuntimeException)
{
    return mUrlContainer.list( OnlyPersistent );
}


void PasswordContainer::Notify()
{
    ::osl::MutexGuard aGuard( mMutex );

    PassMap::iterator aIter;

    // remove the cached persistent values in the memory
    for( aIter = m_aContainer.begin(); aIter != m_aContainer.end(); ++aIter )
    {
        for( list< NamePassRecord >::iterator aNPIter = aIter->second.begin(); aNPIter != aIter->second.end(); )
        {
            if( aNPIter->HasPasswords( PERSISTENT_RECORD ) )
            {
                aNPIter->RemovePasswords( PERSISTENT_RECORD );

                if ( m_pStorageFile )
                    m_pStorageFile->remove( aIter->first, aNPIter->GetUserName() ); // remove record ( aURL, aName )
            }

            if( !aNPIter->HasPasswords( MEMORY_RECORD ) )
            {
                list< NamePassRecord >::iterator aIterToDelete( aNPIter );
                ++aNPIter;
                aIter->second.erase( aIterToDelete );
            }
            else
                ++aNPIter;
        }
    }

    PassMap addon;
    if( m_pStorageFile )
        addon = m_pStorageFile->getInfo();

    for( aIter = addon.begin(); aIter != addon.end(); ++aIter )
    {
        PassMap::iterator aSearchIter = m_aContainer.find( aIter->first );
        if( aSearchIter != m_aContainer.end() )
            for( list< NamePassRecord >::iterator aNPIter = aIter->second.begin(); aNPIter != aIter->second.end(); ++aNPIter )
                UpdateVector( aSearchIter->first, aSearchIter->second, *aNPIter, sal_False );
        else
            m_aContainer.insert( PairUrlRecord( aIter->first, aIter->second ) );
    }
}


OUString SAL_CALL PasswordContainer::getImplementationName(  ) throw(uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}


sal_Bool SAL_CALL PasswordContainer::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    if ( ServiceName.compareToAscii("com.sun.star.task.PasswordContainer") == 0 )
        return sal_True;
    else
        return sal_False;
}


Sequence< OUString > SAL_CALL PasswordContainer::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}


Sequence< OUString > SAL_CALL PasswordContainer::impl_getStaticSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    Sequence< OUString > aRet(1);
    *aRet.getArray() = OUString("com.sun.star.task.PasswordContainer");
    return aRet;
}


OUString SAL_CALL PasswordContainer::impl_getStaticImplementationName() throw(uno::RuntimeException)
{
    return OUString("stardiv.svl.PasswordContainer");
}


Reference< XInterface > SAL_CALL PasswordContainer::impl_createInstance( const Reference< XMultiServiceFactory >& xServiceManager ) throw( RuntimeException )
{
    return Reference< XInterface >( *new PasswordContainer( xServiceManager ) );
}


Reference< XSingleServiceFactory > SAL_CALL PasswordContainer::impl_createFactory( const Reference< XMultiServiceFactory >& ServiceManager ) throw(RuntimeException)
{
    Reference< XSingleServiceFactory > xReturn( ::cppu::createOneInstanceFactory( ServiceManager,
                                                        PasswordContainer::impl_getStaticImplementationName(),
                                                        PasswordContainer::impl_createInstance,
                                                        PasswordContainer::impl_getStaticSupportedServiceNames()));
    return xReturn ;

}


MasterPasswordRequest_Impl::MasterPasswordRequest_Impl( PasswordRequestMode Mode )
{
    MasterPasswordRequest aRequest;

    aRequest.Classification = InteractionClassification_ERROR;
    aRequest.Mode = Mode;

    setRequest( makeAny( aRequest ) );

    // Fill continuations...
    Sequence< RememberAuthentication > aRememberModes( 1 );
    aRememberModes[ 0 ] = RememberAuthentication_NO;

    m_xAuthSupplier
        = new ::ucbhelper::InteractionSupplyAuthentication(
                this,
                sal_False, // bCanSetRealm
                sal_False,  // bCanSetUserName
                sal_True,  // bCanSetPassword
                sal_False, // bCanSetAccount
                aRememberModes, // rRememberPasswordModes
                RememberAuthentication_NO, // eDefaultRememberPasswordMode
                aRememberModes, // rRememberAccountModes
                RememberAuthentication_NO, // eDefaultRememberAccountMode
                sal_False, // bCanUseSystemCredentials
                sal_False  // bDefaultUseSystemCredentials
            );

    Sequence<
        Reference< XInteractionContinuation > > aContinuations( 3 );
    aContinuations[ 0 ] = new ::ucbhelper::InteractionAbort( this );
    aContinuations[ 1 ] = new ::ucbhelper::InteractionRetry( this );
    aContinuations[ 2 ] = m_xAuthSupplier.get();

    setContinuations( aContinuations );
}


extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL passwordcontainer_component_getFactory (
    const sal_Char * pImplementationName,
    SAL_UNUSED_PARAMETER void * pServiceManager,
    SAL_UNUSED_PARAMETER void * /* pRegistryKey */)
{
    void * pResult = 0;
    if (pServiceManager)
    {
        Reference< XSingleServiceFactory > xFactory;
        if (PasswordContainer::impl_getStaticImplementationName().compareToAscii (pImplementationName) == 0)
        {
            xFactory = PasswordContainer::impl_createFactory (
                reinterpret_cast< XMultiServiceFactory* >(pServiceManager));
        }
        if (xFactory.is())
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
