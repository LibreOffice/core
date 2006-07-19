/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: passwordcontainer.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-19 17:08:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "passwordcontainer.hxx"

#include "pathoptions.hxx"

#ifndef _CPPUHELPER_FACTORY_HXX_
#include "cppuhelper/factory.hxx"
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_MASTERPASSWORDREQUEST_HPP_
#include <com/sun/star/task/MasterPasswordRequest.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_NOMASTEREXCEPTION_HPP_
#include <com/sun/star/task/NoMasterException.hpp>
#endif

#include <rtl/cipher.h>
#include <rtl/digest.h>
#include <rtl/byteseq.hxx>

#ifndef _TOOLS_INETSTRM_HXX
// @@@ #include <inetstrm.hxx>
#endif

using namespace std;
using namespace osl;
using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

static ::rtl::OUString createIndex( vector< ::rtl::OUString > lines )
{
    ::rtl::OString aResult;
    const sal_Char* pLine;

    for( unsigned int i = 0; i < lines.size(); i++ )
    {
        if( i )
            aResult += ::rtl::OString( "__" );
        ::rtl::OString line = ::rtl::OUStringToOString( lines[i], RTL_TEXTENCODING_UTF8 );
        pLine = line.getStr();

        while( *pLine )
        {
            if( ( *pLine >= 'A' && *pLine <= 'Z' )
                || ( *pLine >= 'a' && *pLine <= 'z' )
                || ( *pLine >= '0' && *pLine <= '9' ) )
            {
                aResult += ::rtl::OString::valueOf( *pLine );
            }
            else
            {
                aResult += ::rtl::OString("_");
                aResult += ::rtl::OString::valueOf( (sal_Int32) *pLine, 16 );
            }

            pLine++;
        }
    }

    return ::rtl::OUString::createFromAscii( aResult.getStr() );
}

//-------------------------------------------------------------------------

static vector< ::rtl::OUString > getInfoFromInd( ::rtl::OUString aInd )
{
    vector< ::rtl::OUString > aResult;
    sal_Bool aStart = sal_True;

    ::rtl::OString line = ::rtl::OUStringToOString( aInd, RTL_TEXTENCODING_ASCII_US );
    const sal_Char* pLine = line.getStr();
    do
    {
        ::rtl::OUString newItem;
        if( !aStart )
            pLine += 2;
        else
            aStart = sal_False;

        while( *pLine && !( pLine[0] == '_' && pLine[1] == '_' ))
            if( *pLine != '_' )
            {
                newItem += ::rtl::OUString::valueOf( (sal_Unicode) *pLine );
                pLine++;
            }
            else
            {
                ::rtl::OUString aNum;
                for( int i = 1; i < 3; i++ )
                {
                    if( !pLine[i]
                      ||  ( ( pLine[i] < '0' || pLine[i] > '9' )
                         && ( pLine[i] < 'a' || pLine[i] > 'f' )
                         && ( pLine[i] < 'A' || pLine[i] > 'F' ) ) )
                    {
                        OSL_ENSURE( sal_False, "Wrong index syntax!\n" );
                        return aResult;
                    }

                    aNum += ::rtl::OUString::valueOf( (sal_Unicode) pLine[i] );
                }

                newItem += ::rtl::OUString::valueOf( (sal_Unicode) aNum.toInt32( 16 ) );
                pLine += 3;
            }

        aResult.push_back( newItem );
    } while( pLine[0] == '_' && pLine[1] == '_' );

    if( *pLine )
        OSL_ENSURE( sal_False, "Wrong index syntax!\n" );

    return aResult;
}

//-------------------------------------------------------------------------

static sal_Bool shorterUrl( ::rtl::OUString& url )
{
    sal_Int32 aInd = url.lastIndexOf( sal_Unicode( '/' ) );
    if( aInd > 0  && url.indexOf( ::rtl::OUString::createFromAscii( "://" ) ) != aInd-2 )
    {
        url = url.copy( 0, aInd );
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

static ::rtl::OUString getAsciiLine( const ::rtl::ByteSequence& buf )
{
    ::rtl::OUString aResult;

    ::rtl::ByteSequence outbuf( buf.getLength()*2+1 );

    for( int ind = 0; ind < buf.getLength(); ind++ )
    {
        outbuf[ind*2]   = ( ((sal_uInt8)buf[ind]) >> 4 ) + 'a';
        outbuf[ind*2+1] = ( ((sal_uInt8)buf[ind]) & 0x0f ) + 'a';
    }
    outbuf[buf.getLength()*2] = '\0';

    aResult = ::rtl::OUString::createFromAscii( (sal_Char*)outbuf.getArray() );

    return aResult;
}

//-------------------------------------------------------------------------

static ::rtl::ByteSequence getBufFromAsciiLine( ::rtl::OUString line )
{
    OSL_ENSURE( line.getLength() % 2 == 0, "Wrong syntax!\n" );
    ::rtl::OString tmpLine = ::rtl::OUStringToOString( line, RTL_TEXTENCODING_ASCII_US );
    ::rtl::ByteSequence aResult(line.getLength()/2);

    for( int ind = 0; ind < tmpLine.getLength()/2; ind++ )
    {
        aResult[ind] = ( (sal_uInt8)( tmpLine.getStr()[ind*2] - 'a' ) << 4 ) | (sal_uInt8)( tmpLine.getStr()[ind*2+1] - 'a' );
    }

    return aResult;
}

//-------------------------------------------------------------------------

static Sequence< ::rtl::OUString > copyVectorToSequence( const vector< ::rtl::OUString >& original )
{
    Sequence< ::rtl::OUString > newOne ( original.size() );
    for( unsigned int i = 0; i < original.size() ; i++ )
        newOne[i] = original[i];

    return newOne;
}

static vector< ::rtl::OUString > copySequenceToVector( const Sequence< ::rtl::OUString >& original )
{
    vector< ::rtl::OUString > newOne ( original.getLength() );
    for( int i = 0; i < original.getLength() ; i++ )
        newOne[i] = original[i];

    return newOne;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

PassMap StorageItem::getInfo()
{
    PassMap aResult;

    Sequence< ::rtl::OUString > aNodeNames   = ConfigItem::GetNodeNames( ::rtl::OUString::createFromAscii("Store") );
    sal_Int32 aNodeCount = aNodeNames.getLength();
    Sequence< ::rtl::OUString > aPropNames( aNodeCount );
    sal_Int32 aNodeInd;

    for( aNodeInd = 0; aNodeInd < aNodeCount; ++aNodeInd )
    {
        aPropNames[aNodeInd]  = ::rtl::OUString::createFromAscii( "Store/" );
        aPropNames[aNodeInd] += aNodeNames[aNodeInd];
        aPropNames[aNodeInd] += ::rtl::OUString::createFromAscii( "/Password" );
    }

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aPropNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_ENSURE( aPropertyValues.getLength() == aNodeNames.getLength(), "Problems during reading\n" );
        return aResult;
    }

    for( aNodeInd = 0; aNodeInd < aNodeCount; ++aNodeInd )
    {
        vector< ::rtl::OUString > aUrlUsr = getInfoFromInd( aNodeNames[aNodeInd] );

        if( aUrlUsr.size() == 2 )
        {
            ::rtl::OUString aUrl  = aUrlUsr[0];
            ::rtl::OUString aName = aUrlUsr[1];

            vector< ::rtl::OUString > aPass;
            ::rtl::OUString aEPasswd;

            aPropertyValues[aNodeInd] >>= aEPasswd;
            aPass.push_back( aEPasswd );

            PassMap::iterator aIter = aResult.find( aUrl );
            if( aIter != aResult.end() )
                aIter->second.push_back( NamePassRecord( aName, aPass, PERSISTENT_RECORD ) );
            else
            {
                NamePassRecord aNewRecord( aName, aPass, PERSISTENT_RECORD );
                vector< NamePassRecord > vectorToAdd( 1, aNewRecord );

                aResult.insert( PairUrlRecord( aUrl, vectorToAdd ) );
            }
        }
        else
            OSL_ENSURE( sal_False, "Wrong index sintax!\n" );
    }

    return aResult;
}

//-------------------------------------------------------------------------

sal_Bool StorageItem::useStorage()
{
    Sequence< ::rtl::OUString > aNodeNames( 1 );
    aNodeNames[0] = ::rtl::OUString::createFromAscii( "UseStorage" );

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aNodeNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_ENSURE( aPropertyValues.getLength() == aNodeNames.getLength(), "Problems during reading\n" );
        return sal_False;
    }

    sal_Bool aResult = false;
    aPropertyValues[0] >>= aResult;

    return aResult;
}

//-------------------------------------------------------------------------

sal_Bool StorageItem::getEncodedMP( ::rtl::OUString& aResult )
{
    if( hasEncoded )
    {
        aResult = mEncoded;
        return sal_True;
    }

    Sequence< ::rtl::OUString > aNodeNames( 2 );
    aNodeNames[0] = ::rtl::OUString::createFromAscii( "HasMaster" );
    aNodeNames[1] = ::rtl::OUString::createFromAscii( "Master" );

    Sequence< Any > aPropertyValues = ConfigItem::GetProperties( aNodeNames );

    if( aPropertyValues.getLength() != aNodeNames.getLength() )
    {
        OSL_ENSURE( aPropertyValues.getLength() == aNodeNames.getLength(), "Problems during reading\n" );
        return sal_False;
    }

    aPropertyValues[0] >>= hasEncoded;
    aPropertyValues[1] >>= mEncoded;

    aResult = mEncoded;

    return hasEncoded;
}

//-------------------------------------------------------------------------

void StorageItem::setEncodedMP( const ::rtl::OUString& aEncoded )
{
    Sequence< ::rtl::OUString > sendNames(2);
    Sequence< ::com::sun::star::uno::Any > sendVals(2);

    sendNames[0] = ::rtl::OUString::createFromAscii( "HasMaster" );
    sendNames[1] = ::rtl::OUString::createFromAscii( "Master" );

    sendVals[0] <<= sal_True;
    sendVals[1] <<= aEncoded;

    ConfigItem::SetModified();
    ConfigItem::PutProperties( sendNames, sendVals );

    hasEncoded = sal_True;
    mEncoded = aEncoded;
}

//-------------------------------------------------------------------------

void StorageItem::remove( const ::rtl::OUString& url, const ::rtl::OUString& name )
{
    vector < ::rtl::OUString > forIndex;
    forIndex.push_back( url );
    forIndex.push_back( name );

    Sequence< ::rtl::OUString > sendSeq(1);

    sendSeq[0]  = ::rtl::OUString::createFromAscii( "Store/" );
    sendSeq[0] += createIndex( forIndex );

    ConfigItem::ClearNodeElements( ::rtl::OUString::createFromAscii( "Store" ), sendSeq );
}

//-------------------------------------------------------------------------

void StorageItem::clear()
{
    Sequence< ::rtl::OUString > sendSeq(1);

    ConfigItem::ClearNodeSet( ::rtl::OUString::createFromAscii( "Store" ) );
}

//-------------------------------------------------------------------------

void StorageItem::update( const ::rtl::OUString& url, const NamePassRecord& rec )
{
    vector < ::rtl::OUString > forIndex;
    forIndex.push_back( url );
    forIndex.push_back( rec.mName );

    Sequence< ::com::sun::star::beans::PropertyValue > sendSeq(1);

    sendSeq[0].Name  = ::rtl::OUString::createFromAscii( "Store/" );
    sendSeq[0].Name += createIndex( forIndex );
    sendSeq[0].Name += ::rtl::OUString::createFromAscii( "/Password" );

    if( rec.mPass.size() )
    {
        sendSeq[0].Value <<= rec.mPass[0];

        ConfigItem::SetModified();
        ConfigItem::SetSetProperties( ::rtl::OUString::createFromAscii( "Store" ), sendSeq );
    }
    else
        OSL_ENSURE( sal_False, "No password!\n" );
}

//-------------------------------------------------------------------------

void StorageItem::Notify( const Sequence< ::rtl::OUString >& )
{
    // this feature still should not be used
    if( mainCont )
        mainCont->Notify();
}

//-------------------------------------------------------------------------

void StorageItem::Commit()
{
    // Do nothing, we stored everything we want already
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

PasswordContainer::PasswordContainer( const Reference<XMultiServiceFactory>& xServiceFactory ):
    storageFile( NULL ),
    hasMasterPasswd( sal_False )
{
    // storageFile->Notify() can be called
    ::osl::MutexGuard aGuard( mMutex );

    mComponent = Reference< XComponent >( xServiceFactory, UNO_QUERY );
    mComponent->addEventListener( this );

    storageFile = new StorageItem( this, ::rtl::OUString::createFromAscii( "Office.Common/Passwords" ) );
    if( storageFile )
        if( storageFile->useStorage() )
            container = storageFile->getInfo();
        else
        {
            delete storageFile;
            storageFile = NULL;
        }
}

//-------------------------------------------------------------------------

PasswordContainer::~PasswordContainer()
{
    ::osl::MutexGuard aGuard( mMutex );

    if( storageFile )
    {
        delete storageFile;
        storageFile = NULL;
    }

    if( mComponent.is() )
    {
        mComponent->removeEventListener(this);
        mComponent = Reference< XComponent >();
    }
}

//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::disposing( const EventObject& ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if( storageFile )
    {
        delete storageFile;
        storageFile = NULL;
    }

    if( mComponent.is() )
    {
        //mComponent->removeEventListener(this);
        mComponent = Reference< XComponent >();
    }
}

//-------------------------------------------------------------------------

vector< ::rtl::OUString > PasswordContainer::decodePasswords( ::rtl::OUString aLine, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{
    getMasterPassword( Handler );

    if( hasMasterPasswd )
    {
        rtlCipher aDecoder = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream );
        OSL_ENSURE( aDecoder, "Can't create decoder\n" );

        if( aDecoder )
        {
            OSL_ENSURE( masterPasswd.getLength() == RTL_DIGEST_LENGTH_MD5 * 2, "Wrong master password format!\n" );

            unsigned char code[RTL_DIGEST_LENGTH_MD5];
            for( int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ind++ )
                code[ ind ] = (char)(masterPasswd.copy( ind*2, 2 ).toInt32(16));

            rtlCipherError result = rtl_cipher_init (
                    aDecoder, rtl_Cipher_DirectionDecode,
                    code, RTL_DIGEST_LENGTH_MD5, NULL, 0 );

            if( result == rtl_Cipher_E_None )
            {
                ::rtl::ByteSequence aSeq = getBufFromAsciiLine( aLine );

                ::rtl::ByteSequence resSeq( aSeq.getLength() );

                result = rtl_cipher_decode ( aDecoder, (sal_uInt8*)aSeq.getArray(), aSeq.getLength(),
                                                        (sal_uInt8*)resSeq.getArray(), resSeq.getLength() );

                ::rtl::OUString aPasswd( ( sal_Char* )resSeq.getArray(), resSeq.getLength(), RTL_TEXTENCODING_UTF8 );

                rtl_cipher_destroy (aDecoder);

                return getInfoFromInd( aPasswd );
            }

            rtl_cipher_destroy (aDecoder);
        }
    }
    else
    {
        OSL_ENSURE( sal_False, "No master password provided!\n" );
        // throw special exception
    }

    // problems with decoding
    OSL_ENSURE( sal_False, "Problem with decoding\n" );
    throw RuntimeException( ::rtl::OUString::createFromAscii( "Can't decode!" ), Reference< XInterface >() );
}


//-------------------------------------------------------------------------

::rtl::OUString PasswordContainer::encodePasswords( vector< ::rtl::OUString > lines, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{
    getMasterPassword( Handler );

    if( hasMasterPasswd )
    {
        ::rtl::OString aSeq = ::rtl::OUStringToOString( createIndex( lines ), RTL_TEXTENCODING_UTF8 );

        rtlCipher aEncoder = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream );
        OSL_ENSURE( aEncoder, "Can't create encoder\n" );

        if( aEncoder )
        {
            OSL_ENSURE( masterPasswd.getLength() == RTL_DIGEST_LENGTH_MD5 * 2, "Wrong master password format!\n" );

            unsigned char code[RTL_DIGEST_LENGTH_MD5];
            for( int ind = 0; ind < RTL_DIGEST_LENGTH_MD5; ind++ )
                code[ ind ] = (char)(masterPasswd.copy( ind*2, 2 ).toInt32(16));

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
                    ::rtl::OUString testOU = getAsciiLine( resSeq );
                    ::rtl::ByteSequence aSeq1 = getBufFromAsciiLine( testOU );

                    ::rtl::ByteSequence resSeq1( aSeq1.getLength() );

                    if( resSeq.getLength() == aSeq1.getLength() )
                    {
                        for( int ind = 0; ind < aSeq1.getLength(); ind++ )
                            if( resSeq[ind] != aSeq1[ind] )
                                testOU = ::rtl::OUString();
                    }

                    result = rtl_cipher_decode ( aEncoder, (sal_uInt8*)aSeq1.getArray(), aSeq1.getLength(),
                                                        (sal_uInt8*)resSeq1.getArray(), resSeq1.getLength() );

                    ::rtl::OUString aPasswd( ( sal_Char* )resSeq1.getArray(), resSeq1.getLength(), RTL_TEXTENCODING_UTF8 );
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
        OSL_ENSURE( sal_False, "No master password provided!\n" );
        // throw special exception
    }

    // problems with encoding
    OSL_ENSURE( sal_False, "Problem with encoding\n" );
    throw RuntimeException( ::rtl::OUString::createFromAscii( "Can't encode!" ), Reference< XInterface >() );
}

//-------------------------------------------------------------------------

void PasswordContainer::updateVector( const ::rtl::OUString& url, vector< NamePassRecord >& toUpdate, NamePassRecord& rec, sal_Bool writeFile, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{
    sal_Int32 oldLen = toUpdate.size();

    for( int i = 0; i < oldLen; i++ )
        if( toUpdate[i].mName.equals( rec.mName ) )
        {
            if( toUpdate[i].mStatus == PERSISTENT_RECORD )
                rec.mStatus = PERSISTENT_RECORD;

            if( rec.mStatus == PERSISTENT_RECORD && writeFile && storageFile )
            {
                ::rtl::OUString aEncPass = encodePasswords( rec.mPass, Handler );
                rec.mPass = vector< ::rtl::OUString >( 1, aEncPass );
                storageFile->update( url, rec ); // change existing ( url, name ) record in the configfile
            }

            toUpdate[i] = rec;

            return;
        }


    if( rec.mStatus == PERSISTENT_RECORD && writeFile && storageFile)
    {
        ::rtl::OUString aEncPass = encodePasswords( rec.mPass, Handler );
        rec.mPass = vector< ::rtl::OUString >( 1, aEncPass );
        storageFile->update( url, rec ); // add new name to the existing url
    }

    toUpdate.insert( toUpdate.begin(), rec );
}

//-------------------------------------------------------------------------

Sequence< UserRecord > PasswordContainer::copyToUserRecordSequence( const vector< NamePassRecord >& original, const Reference< XInteractionHandler >& Handler ) throw(RuntimeException)
{
    Sequence< UserRecord >     aResult( original.size() );
    for( unsigned int i = 0; i < original.size(); i++ )
    {
        if( original[i].mStatus == PERSISTENT_RECORD )
        {
            OSL_ENSURE( original[i].mPass.size(), "No encripted password!\n" );
            aResult[i] = UserRecord( original[i].mName, copyVectorToSequence( decodePasswords( original[i].mPass[0], Handler ) ) );
        }
        else
            aResult[i] = UserRecord( original[i].mName, copyVectorToSequence( original[i].mPass ) );
    }

    return aResult;
}

//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::add( const ::rtl::OUString& Url, const ::rtl::OUString& UserName, const Sequence< ::rtl::OUString >& Passwords, const Reference< XInteractionHandler >& Handler ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    privateAdd( Url, UserName, Passwords, SINGLE_RECORD, Handler );
}

//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::addPersistent( const ::rtl::OUString& Url, const ::rtl::OUString& UserName, const Sequence< ::rtl::OUString >& Passwords, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    privateAdd( Url, UserName, Passwords, PERSISTENT_RECORD, Handler );
}

//-------------------------------------------------------------------------

void PasswordContainer::privateAdd( const ::rtl::OUString& Url, const ::rtl::OUString& UserName, const Sequence< ::rtl::OUString >& Passwords, char Mode, const Reference< XInteractionHandler >& Handler ) throw(RuntimeException)
{
    vector< ::rtl::OUString > storePass = copySequenceToVector( Passwords );

    if( !container.empty() )
    {
        PassMap::iterator aIter = container.find( Url );

        if( aIter != container.end() )
        {
            NamePassRecord aNamePassRecord( UserName, storePass, Mode );
            updateVector( aIter->first, aIter->second, aNamePassRecord, sal_True, Handler );
            return;
        }
    }

    if( Mode == PERSISTENT_RECORD && storageFile )
    {
        ::rtl::OUString aEncPass = encodePasswords( storePass, Handler );
        storePass = vector< ::rtl::OUString >( 1, aEncPass );
    }

    NamePassRecord aNewRecord( UserName, storePass, Mode );
    vector< NamePassRecord > vectorToAdd( 1, aNewRecord );
    container.insert( PairUrlRecord( Url, vectorToAdd ) );

    if( Mode == PERSISTENT_RECORD && storageFile )
        storageFile->update( Url, aNewRecord );

}

//-------------------------------------------------------------------------


UrlRecord SAL_CALL PasswordContainer::find( const ::rtl::OUString& url, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if( !container.empty() )
    {
        ::rtl::OUString aUrl( url );
        PassMap::iterator aIter = container.find( aUrl );

        if( aIter != container.end() )
            return UrlRecord( aIter->first, copyToUserRecordSequence( aIter->second, Handler ) );

        // each iteration remove last '/...' section from the aUrl
        // while it's possible, up to the most left '://'
        while( shorterUrl( aUrl ) )
        {
            // first look for <url>/somename and then look for <url>/somename/...
            aIter = container.find( aUrl );
            if( aIter != container.end() )
                return UrlRecord( aIter->first, copyToUserRecordSequence( aIter->second, Handler ) );
            else
            {
                ::rtl::OUString tmpUrl( aUrl );
                tmpUrl += ::rtl::OUString::createFromAscii( "/" );

                aIter = container.lower_bound( aUrl );
                if( aIter != container.end() )
                    return UrlRecord( aIter->first, copyToUserRecordSequence( aIter->second, Handler ) );
            }
        }
    }

    return UrlRecord();
}

//-------------------------------------------------------------------------

Sequence< UserRecord > PasswordContainer::findUsr( const vector< NamePassRecord >& userlist, const ::rtl::OUString& name, const Reference< XInteractionHandler >& Handler ) throw(RuntimeException)
{
    for( unsigned int i = 0; i < userlist.size(); i++ )
        if( userlist[i].mName.equals( name ) )
        {
            Sequence< UserRecord > aResult(1);
            if( userlist[i].mStatus == PERSISTENT_RECORD )
            {
                OSL_ENSURE( userlist[i].mPass.size(), "No encripted password!\n" );
                *aResult.getArray() = UserRecord( name, copyVectorToSequence( decodePasswords( userlist[i].mPass[0], Handler ) ) );
            }
            else
                *aResult.getArray() = UserRecord( name, copyVectorToSequence( userlist[i].mPass ) );

            return aResult;
        }

    return Sequence< UserRecord >();
}

//-------------------------------------------------------------------------

UrlRecord SAL_CALL PasswordContainer::findForName( const ::rtl::OUString& url, const ::rtl::OUString& name, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{

    ::osl::MutexGuard aGuard( mMutex );
    if( !container.empty() )
    {
        ::rtl::OUString aUrl( url );
        PassMap::iterator aIter = container.find( aUrl );

        if( aIter != container.end() )
        {
            Sequence< UserRecord > aUsrRec = findUsr( aIter->second, name, Handler );
            if( aUsrRec.getLength() )
                return UrlRecord( aIter->first, aUsrRec );
        }

        // each iteration remove last '/...' section from the aUrl
        // while it's possible, up to the most left '://'
        while( shorterUrl( aUrl ) )
        {
            // first look for <url>/somename and then look for <url>/somename/...
            aIter = container.find( aUrl );
            if( aIter != container.end() )
            {
                Sequence< UserRecord > aUsrRec = findUsr( aIter->second, name, Handler );
                if( aUsrRec.getLength() )
                    return UrlRecord( aIter->first, aUsrRec );
            }
            else
            {
                ::rtl::OUString tmpUrl( aUrl );
                tmpUrl += ::rtl::OUString::createFromAscii( "/" );

                aIter = container.lower_bound( aUrl );
                if( aIter != container.end() )
                {
                    Sequence< UserRecord > aUsrRec = findUsr( aIter->second, name, Handler );
                    if( aUsrRec.getLength() )
                        return UrlRecord( aIter->first, aUsrRec );
                }
            }
        }
    }

    return UrlRecord();
}

//-------------------------------------------------------------------------

void PasswordContainer::getMasterPassword( const Reference< XInteractionHandler >& Handler ) throw(RuntimeException)
{
    if( !hasMasterPasswd && Handler.is() )
    {
        ::rtl::OUString encodedMP;
        PasswordRequestMode aRMode = PasswordRequestMode_PASSWORD_ENTER;
        sal_Bool aAskAgain;

        if( storageFile && !storageFile->getEncodedMP( encodedMP ) )
            aRMode = PasswordRequestMode_PASSWORD_CREATE;

        do {
            aAskAgain = sal_False;

            ::rtl::Reference< MasterPasswordRequest_Impl > xRequest = new MasterPasswordRequest_Impl( aRMode );

            Handler->handle( xRequest.get() );

            ::rtl::Reference< ucbhelper::InteractionContinuation > xSelection = xRequest->getSelection();

            if ( xSelection.is() )
            {
                Reference< XInteractionAbort > xAbort( xSelection.get(), UNO_QUERY );
                if ( xAbort.is() )
                {
                    masterPasswd = ::rtl::OUString();
                    hasMasterPasswd = sal_False;
                    throw NoMasterException( ::rtl::OUString::createFromAscii( "No master password!" ), Reference< XInterface >(), aRMode );
                }
                else
                {
                    const ::rtl::Reference< ucbhelper::InteractionSupplyAuthentication > & xSupp
                                = xRequest->getAuthenticationSupplier();

                    masterPasswd = xSupp->getPassword();
                    hasMasterPasswd = sal_True;

                    if( aRMode == PasswordRequestMode_PASSWORD_CREATE )
                    {
                        vector< ::rtl::OUString > aMaster( 1, masterPasswd );

                        // Handler is not needed here,
                        // masterpassword exist
                        if( storageFile )
                            storageFile->setEncodedMP( encodePasswords( aMaster, Reference< XInteractionHandler >() ) );
                    }
                    else
                    {
                        vector< ::rtl::OUString > aRM( decodePasswords( encodedMP, Handler ) );
                        if( !aRM.size() || !masterPasswd.equals( aRM[0] ) )
                        {
                            aAskAgain = sal_True;
                            aRMode = PasswordRequestMode_PASSWORD_REENTER;
                        }
                    }
                }
            }

        } while( aAskAgain );
    }
}

//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::remove( const ::rtl::OUString& url, const ::rtl::OUString& name ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    ::rtl::OUString aUrl( url );
    if( !container.empty() )
    {
        PassMap::iterator aIter = container.find( aUrl );

        if( aIter == container.end() )
        {
            sal_Int32 aInd = aUrl.lastIndexOf( sal_Unicode( '/' ) );
            if( aInd > 0 && aUrl.getLength()-1 == aInd )
                aUrl = aUrl.copy( 0, aUrl.getLength() - 1 );
            else
                aUrl += ::rtl::OUString::createFromAscii( "/" );

            aIter = container.find( aUrl );
        }

        if( aIter != container.end() )
        {
            for( vector< NamePassRecord >::iterator aVIter = aIter->second.begin(); aVIter != aIter->second.end(); aVIter++ )
                if( aVIter->mName.equals( name ) )
                {
                    if( aVIter->mStatus == PERSISTENT_RECORD && storageFile )
                        storageFile->remove( url, name ); // remove record ( url, name )

                    aIter->second.erase( aVIter );

                    if( aIter->second.begin() == aIter->second.end() )
                        container.erase( aIter );

                    return;
                }
        }
    }
}

//-------------------------------------------------------------------------


void SAL_CALL PasswordContainer::removePersistent( const ::rtl::OUString& url, const ::rtl::OUString& name ) throw(RuntimeException)
{
    // this feature still should not be used
    //throw RuntimeException( ::rtl::OUString::createFromAscii( "Not implememted!" ), Reference< XInterface >() );

    ::osl::MutexGuard aGuard( mMutex );

    ::rtl::OUString aUrl( url );
    if( !container.empty() )
    {
        PassMap::iterator aIter = container.find( aUrl );

        if( aIter == container.end() )
        {
            sal_Int32 aInd = aUrl.lastIndexOf( sal_Unicode( '/' ) );
            if( aInd > 0 && aUrl.getLength()-1 == aInd )
                aUrl = aUrl.copy( 0, aUrl.getLength() - 1 );
            else
                aUrl += ::rtl::OUString::createFromAscii( "/" );

            aIter = container.find( aUrl );
        }

        if( aIter != container.end() )
        {
            for( vector< NamePassRecord >::iterator aVIter = aIter->second.begin(); aVIter != aIter->second.end(); aVIter++ )
                if( aVIter->mName.equals( name ) )
                {
                    if( aVIter->mStatus == PERSISTENT_RECORD && storageFile )
                    {
                        aVIter->mStatus = SINGLE_RECORD;
                        storageFile->remove( url, name ); // remove record ( url, name )
                        return;
                    }
                }
        }
    }
}
//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::removeAllPersistent(  ) throw(RuntimeException)
{
    // this feature still should not be used
    //throw RuntimeException( ::rtl::OUString::createFromAscii( "Not implememted!" ), Reference< XInterface >() );

    ::osl::MutexGuard aGuard( mMutex );

    if( storageFile )
        storageFile->clear();

    for( PassMap::iterator aIter = container.begin(); aIter != container.end(); aIter++ )
    {
        for( vector< NamePassRecord >::iterator aVIter = aIter->second.begin(); aVIter != aIter->second.end(); aVIter++ )
            if( aVIter->mStatus == PERSISTENT_RECORD )
                aVIter->mStatus = SINGLE_RECORD;
    }
}
//-------------------------------------------------------------------------

Sequence< UrlRecord > SAL_CALL PasswordContainer::getAllPersistent( const Reference< XInteractionHandler >& ) throw(RuntimeException)
{
    // this feature still should not be used
    // throw RuntimeException( ::rtl::OUString::createFromAscii( "Not implememted!" ), Reference< XInterface >() );

    Sequence< UrlRecord > aResult;

    ::osl::MutexGuard aGuard( mMutex );
    for( PassMap::iterator aIter = container.begin(); aIter != container.end(); aIter++ )
    {
        Sequence< UserRecord > aUsers;
        for( vector< NamePassRecord >::iterator aVIter = aIter->second.begin(); aVIter != aIter->second.end(); aVIter++ )
            if( aVIter->mStatus == PERSISTENT_RECORD )
            {
                sal_Int32 oldLen = aUsers.getLength();
                aUsers.realloc( oldLen + 1 );
                aUsers[ oldLen ] = UserRecord( aVIter->mName, copyVectorToSequence( aVIter->mPass ) );
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

//-------------------------------------------------------------------------

void PasswordContainer::Notify()
{
    ::osl::MutexGuard aGuard( mMutex );

    PassMap::iterator aIter;

    for( aIter = container.begin(); aIter != container.end(); aIter++ )
    {
        for( vector< NamePassRecord >::iterator aVIter = aIter->second.begin(); aVIter != aIter->second.end(); aVIter++ )
            if( aVIter->mStatus == PERSISTENT_RECORD )
                aVIter->mStatus = SINGLE_RECORD;
    }

    PassMap addon;
    if( storageFile )
        addon = storageFile->getInfo();

    for( aIter = addon.begin(); aIter != addon.end(); aIter++ )
    {
        PassMap::iterator aSearchIter = container.find( aIter->first );
        if( aSearchIter != container.end() )
            for( vector< NamePassRecord >::iterator aVIter = aIter->second.begin(); aVIter != aIter->second.end(); aVIter++ )
                updateVector( aSearchIter->first, aSearchIter->second, *aVIter, sal_False, Reference< XInteractionHandler >() );
        else
            container.insert( PairUrlRecord( aIter->first, aIter->second ) );
    }
}

//-------------------------------------------------------------------------

::rtl::OUString SAL_CALL PasswordContainer::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}

//-------------------------------------------------------------------------

sal_Bool SAL_CALL PasswordContainer::supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( ServiceName.compareToAscii("com.sun.star.task.PasswordContainer") == 0 )
        return sal_True;
    else
        return sal_False;
}

//-------------------------------------------------------------------------

Sequence< ::rtl::OUString > SAL_CALL PasswordContainer::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}

//-------------------------------------------------------------------------

Sequence< ::rtl::OUString > SAL_CALL PasswordContainer::impl_getStaticSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aRet(1);
    *aRet.getArray() = ::rtl::OUString::createFromAscii("com.sun.star.task.PasswordContainer");
    return aRet;
}

//-------------------------------------------------------------------------

::rtl::OUString SAL_CALL PasswordContainer::impl_getStaticImplementationName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii("stardiv.svtools.PasswordContainer");
}

//-------------------------------------------------------------------------

Reference< XInterface > SAL_CALL PasswordContainer::impl_createInstance( const Reference< XMultiServiceFactory >& xServiceManager ) throw( RuntimeException )
{
    return Reference< XInterface >( *new PasswordContainer( xServiceManager ) );
}

//-------------------------------------------------------------------------

Reference< XSingleServiceFactory > SAL_CALL PasswordContainer::impl_createFactory( const Reference< XMultiServiceFactory >& ServiceManager ) throw(RuntimeException)
{
    Reference< XSingleServiceFactory > xReturn( ::cppu::createOneInstanceFactory( ServiceManager,
                                                        PasswordContainer::impl_getStaticImplementationName(),
                                                        PasswordContainer::impl_createInstance,
                                                        PasswordContainer::impl_getStaticSupportedServiceNames()));
    return xReturn ;

}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

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
                RememberAuthentication_NO // eDefaultRememberAccountMode
            );

    Sequence<
        Reference< XInteractionContinuation > > aContinuations( 3 );
    aContinuations[ 0 ] = new ::ucbhelper::InteractionAbort( this );
    aContinuations[ 1 ] = new ::ucbhelper::InteractionRetry( this );
    aContinuations[ 2 ] = m_xAuthSupplier.get();

    setContinuations( aContinuations );
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

extern "C"
{
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * /* pServiceManager */, void * pRegistryKey)
{
    if (pRegistryKey)
    {
        Reference< XRegistryKey > xRegistryKey (
            reinterpret_cast< XRegistryKey* >( pRegistryKey ));
        Reference< XRegistryKey > xNewKey;

        xNewKey = xRegistryKey->createKey(
            ::rtl::OUString::createFromAscii( "/stardiv.svtools.PasswordContainer/UNO/SERVICES" ));
        xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.task.PasswordContainer"));

        return sal_True;
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * /* pRegistryKey */)
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
