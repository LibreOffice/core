/*************************************************************************
 *
 *  $RCSfile: passwordcontainer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mav $ $Date: 2001-05-18 14:46:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "passwordcontainer.hxx"

#include "pathoptions.hxx"

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

using namespace std;
using namespace osl;
using namespace rtl;
using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;

//-------------------------------------------------------------------------

OUString createIndex( vector< OUString > lines )
{
    OString aResult;
    const sal_Char* pLine;

    for( unsigned int i = 0; i < lines.size(); i++ )
    {
        aResult += OString( "__" );
        OString line = OUStringToOString( lines[i], RTL_TEXTENCODING_UTF8 );
        pLine = line.getStr();

        while( *pLine )
        {
            if( ( *pLine >= 'A' && *pLine <= 'Z' )
                || ( *pLine >= 'a' && *pLine <= 'z' )
                || ( *pLine >= '0' && *pLine <= '9' ) )
            {
                aResult += OString::valueOf( *pLine );
            }
            else
            {
                aResult += OString("_");
                aResult += OString::valueOf( (sal_Int32) *pLine, 16 );
            }

            pLine++;
        }
    }

    return OUString::createFromAscii( aResult.getStr() );
}

vector< OUString > getInfoFromInd( OUString aInd )
{
    vector< OUString > aResult;

    OString line = OUStringToOString( aInd, RTL_TEXTENCODING_ASCII_US );
    const sal_Char* pLine = line.getStr();
    while( pLine[0] == '_' && pLine[1] == '_' )
    {
        OUString newItem;
        pLine += 2;

        while( *pLine && !( pLine[0] == '_' && pLine[1] == '_' ))
            if( *pLine != '_' )
            {
                newItem += OUString::valueOf( (sal_Unicode) *pLine );
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
                        OSL_ENSURE( sal_False, "Wrong index syntax!\n" );
                        return aResult;
                    }

                    aNum += OUString::valueOf( (sal_Unicode) *pLine );
                }

                newItem += OUString::valueOf( (sal_Unicode) aNum.toInt32( 16 ) );
                pLine += 3;
            }

        aResult.push_back( newItem );
    }

    if( *pLine )
        OSL_ENSURE( sal_False, "Wrong index syntax!\n" );

    return aResult;
}

//-------------------------------------------------------------------------

vector< OUString > decodePasswords( Any pass )
{
    OUString aLine;
    pass >>= aLine;

    return getInfoFromInd( aLine );
}

//-------------------------------------------------------------------------

OUString encodePasswords( vector< OUString > lines )
{
    return createIndex( lines );
}

//-------------------------------------------------------------------------

sal_Bool shorterUrl( OUString& url )
{
    sal_Int32 aInd = url.lastIndexOf( sal_Unicode( '/' ) );
    if( aInd > 0  && url.indexOf( OUString::createFromAscii( "://" ) ) != aInd-2 )
    {
        url = url.copy( 0, aInd );
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

PassMap StorageItem::getInfo()
{
    PassMap aResult;

    Sequence< OUString > aNodeNames  = ConfigItem::GetNodeNames( OUString::createFromAscii("Store") );
    sal_Int32 aNodeCount = aNodeNames.getLength();
    Sequence< OUString > aPropNames( aNodeCount );
    sal_Int32 aNodeInd;

    for( aNodeInd = 0; aNodeInd < aNodeCount; ++aNodeInd )
    {
        aPropNames[aNodeInd]  = OUString::createFromAscii( "Store/" );
        aPropNames[aNodeInd] += aNodeNames[aNodeInd];
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

            vector< OUString > aPass = decodePasswords( aPropertyValues[aNodeInd] );

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

void StorageItem::remove( const OUString& url, const OUString& name )
{
    vector < OUString > forIndex;
    forIndex.push_back( url );
    forIndex.push_back( name );

    Sequence< OUString > sendSeq(1);

    sendSeq[0]  = OUString::createFromAscii( "Store/" );
    sendSeq[0] += createIndex( forIndex );

    ConfigItem::ClearNodeElements( OUString::createFromAscii( "Store" ), sendSeq );
}

//-------------------------------------------------------------------------

void StorageItem::clear()
{
    Sequence< OUString > sendSeq(1);

    ConfigItem::ClearNodeSet( OUString::createFromAscii( "Store" ) );
}

//-------------------------------------------------------------------------

void StorageItem::update( const OUString& url, const NamePassRecord& rec )
{
    vector < OUString > forIndex;
    forIndex.push_back( url );
    forIndex.push_back( rec.mName );

    Sequence< ::com::sun::star::beans::PropertyValue > sendSeq(1);

    sendSeq[0].Name  = OUString::createFromAscii( "Store/" );
    sendSeq[0].Name += createIndex( forIndex );

    sendSeq[0].Value <<= encodePasswords( rec.mPass );

    ConfigItem::SetModified();
    ConfigItem::SetSetProperties( OUString::createFromAscii( "Store" ), sendSeq );
}

//-------------------------------------------------------------------------

PasswordContainer::PasswordContainer( const Reference<XMultiServiceFactory>& xServiceFactory ):
storageFile( NULL )
{
    ::osl::MutexGuard aGuard( mMutex );

//  storageFile = new StorageItem( this, OUString::createFromAscii( "Office.Common/Passwords" ) );
//  container = storageFile->getInfo();
}

//-------------------------------------------------------------------------

PasswordContainer::~PasswordContainer()
{
    if( storageFile )
        delete storageFile;
}
//-------------------------------------------------------------------------

void PasswordContainer::updateVector( const OUString& url, vector< NamePassRecord >& toUpdate, NamePassRecord& rec, sal_Bool writeFile )
{
    sal_Int32 oldLen = toUpdate.size();

    for( int i = 0; i < oldLen; i++ )
        if( toUpdate[i].mName.equals( rec.mName ) )
        {
            if( toUpdate[i].mStatus == PERSISTENT_RECORD )
                rec.mStatus = PERSISTENT_RECORD;

            toUpdate[i] = rec;
            if( rec.mStatus == PERSISTENT_RECORD && writeFile && storageFile )
                storageFile->update( url, rec ); // change existing ( url, name ) record in the configfile
            return;
        }

    toUpdate.insert( toUpdate.begin(), rec );

    if( rec.mStatus == PERSISTENT_RECORD && writeFile && storageFile)
        storageFile->update( url, rec ); // add new name to the existing url
}

//-------------------------------------------------------------------------

Sequence< OUString > copyVectorToSequence( const vector< OUString >& original )
{
    Sequence< OUString > newOne ( original.size() );
    for( unsigned int i = 0; i < original.size() ; i++ )
        newOne[i] = original[i];

    return newOne;
}

vector< OUString > copySequenceToVector( const Sequence< OUString >& original )
{
    vector< OUString > newOne ( original.getLength() );
    for( int i = 0; i < original.getLength() ; i++ )
        newOne[i] = original[i];

    return newOne;
}

Sequence< UserRecord > copyToUserRecordSequence( const vector< NamePassRecord >& original )
{
    Sequence< UserRecord >     aResult( original.size() );
    for( unsigned int i = 0; i < original.size(); i++ )
        aResult[i] = UserRecord( original[i].mName, copyVectorToSequence( original[i].mPass ) );

    return aResult;
}

//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::add( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, const Reference< XInteractionHandler >& Handler ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    privateAdd( Url, UserName, Passwords, SINGLE_RECORD );
}

//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::addPersistent( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{
    // this feature should not be used in 1.02 version
    //throw RuntimeException( OUString::createFromAscii( "Not implememted!" ), Reference< XInterface >() );

    ::osl::MutexGuard aGuard( mMutex );

    privateAdd( Url, UserName, Passwords, PERSISTENT_RECORD );
}

//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::privateAdd( const OUString& Url, const OUString& UserName, const Sequence< OUString >& Passwords, char Mode )
{
    vector< OUString > storePass = copySequenceToVector( Passwords );

    if( !container.empty() )
    {
        PassMap::iterator aIter = container.find( Url );

        if( aIter != container.end() )
        {
            updateVector( aIter->first, aIter->second, NamePassRecord( UserName, storePass, Mode ), sal_True );
            return;
        }
    }

    NamePassRecord aNewRecord( UserName, storePass, Mode );
    vector< NamePassRecord > vectorToAdd( 1, aNewRecord );
    container.insert( PairUrlRecord( Url, vectorToAdd ) );

    if( Mode == PERSISTENT_RECORD && storageFile )
        storageFile->update( Url, aNewRecord );

}

//-------------------------------------------------------------------------


UrlRecord SAL_CALL PasswordContainer::find( const OUString& url, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    if( !container.empty() )
    {
        OUString aUrl( url );
        PassMap::iterator aIter = container.find( aUrl );

        if( aIter != container.end() )
            return UrlRecord( aIter->first, copyToUserRecordSequence( aIter->second ) );

        // each iteration remove last '/...' section from the aUrl
        // while it's possible, up to the most left '://'
        while( shorterUrl( aUrl ) )
        {
            // first look for <url>/somename and then look for <url>/somename/...
            aIter = container.find( aUrl );
            if( aIter != container.end() )
                return UrlRecord( aIter->first, copyToUserRecordSequence( aIter->second ) );
            else
            {
                OUString tmpUrl( aUrl );
                tmpUrl += OUString::createFromAscii( "/" );

                aIter = container.lower_bound( aUrl );
                if( aIter != container.end() )
                    return UrlRecord( aIter->first, copyToUserRecordSequence( aIter->second ) );
            }
        }
    }

    return UrlRecord();
}

//-------------------------------------------------------------------------

Sequence< UserRecord > findUsr( const vector< NamePassRecord >& userlist, const OUString& name )
{
    for( unsigned int i = 0; i < userlist.size(); i++ )
        if( userlist[i].mName.equals( name ) )
        {
            Sequence< UserRecord > aResult(1);
            *aResult.getArray() = UserRecord( name, copyVectorToSequence( userlist[i].mPass ) );
            return aResult;
        }

    return Sequence< UserRecord >();
}

//-------------------------------------------------------------------------

UrlRecord SAL_CALL PasswordContainer::findForName( const OUString& url, const OUString& name, const Reference< XInteractionHandler >& Handler  ) throw(RuntimeException)
{

    ::osl::MutexGuard aGuard( mMutex );
    if( !container.empty() )
    {
        OUString aUrl( url );
        PassMap::iterator aIter = container.find( aUrl );

        if( aIter != container.end() )
        {
            Sequence< UserRecord > aUsrRec = findUsr( aIter->second, name );
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
                Sequence< UserRecord > aUsrRec = findUsr( aIter->second, name );
                if( aUsrRec.getLength() )
                    return UrlRecord( aIter->first, aUsrRec );
            }
            else
            {
                OUString tmpUrl( aUrl );
                tmpUrl += OUString::createFromAscii( "/" );

                aIter = container.lower_bound( aUrl );
                if( aIter != container.end() )
                {
                    Sequence< UserRecord > aUsrRec = findUsr( aIter->second, name );
                    if( aUsrRec.getLength() )
                        return UrlRecord( aIter->first, aUsrRec );
                }
            }
        }
    }

    return UrlRecord();
}

//-------------------------------------------------------------------------

void SAL_CALL PasswordContainer::remove( const OUString& url, const OUString& name ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( mMutex );

    OUString aUrl( url );
    if( !container.empty() )
    {
        PassMap::iterator aIter = container.find( aUrl );

        if( aIter == container.end() )
        {
            sal_Int32 aInd = aUrl.lastIndexOf( sal_Unicode( '/' ) );
            if( aInd > 0 && aUrl.getLength()-1 == aInd )
                aUrl = aUrl.copy( 0, aUrl.getLength() - 1 );
            else
                aUrl += OUString::createFromAscii( "/" );

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


void SAL_CALL PasswordContainer::removePersistent( const OUString& url, const OUString& name ) throw(RuntimeException)
{
    // this feature still should not be used
    //throw RuntimeException( OUString::createFromAscii( "Not implememted!" ), Reference< XInterface >() );

    ::osl::MutexGuard aGuard( mMutex );

    OUString aUrl( url );
    if( !container.empty() )
    {
        PassMap::iterator aIter = container.find( aUrl );

        if( aIter == container.end() )
        {
            sal_Int32 aInd = aUrl.lastIndexOf( sal_Unicode( '/' ) );
            if( aInd > 0 && aUrl.getLength()-1 == aInd )
                aUrl = aUrl.copy( 0, aUrl.getLength() - 1 );
            else
                aUrl += OUString::createFromAscii( "/" );

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
    //throw RuntimeException( OUString::createFromAscii( "Not implememted!" ), Reference< XInterface >() );

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

Sequence< UrlRecord > SAL_CALL PasswordContainer::getAllPersistent( const Reference< XInteractionHandler >& Handler ) throw(RuntimeException)
{
    // this feature still should not be used
    // throw RuntimeException( OUString::createFromAscii( "Not implememted!" ), Reference< XInterface >() );

    Sequence< UrlRecord > aResult;
    sal_Int32 oldLen;

    ::osl::MutexGuard aGuard( mMutex );
    for( PassMap::iterator aIter = container.begin(); aIter != container.end(); aIter++ )
    {
        Sequence< UserRecord > aUsers;
        for( vector< NamePassRecord >::iterator aVIter = aIter->second.begin(); aVIter != aIter->second.end(); aVIter++ )
            if( aVIter->mStatus == PERSISTENT_RECORD )
            {
                oldLen = aUsers.getLength();
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
                updateVector( aSearchIter->first, aSearchIter->second, *aVIter, sal_False );
        else
            container.insert( PairUrlRecord( aIter->first, aIter->second ) );
    }
}

//-------------------------------------------------------------------------

OUString SAL_CALL PasswordContainer::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
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

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL PasswordContainer::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}

//-------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL PasswordContainer::impl_getStaticSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aRet(1);
    *aRet.getArray() = OUString::createFromAscii("com.sun.star.task.PasswordContainer");
    return aRet;
}

//-------------------------------------------------------------------------

::rtl::OUString SAL_CALL PasswordContainer::impl_getStaticImplementationName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii("stardiv.svtools.PasswordContainer");
}

//-------------------------------------------------------------------------

Reference< ::com::sun::star::uno::XInterface > SAL_CALL PasswordContainer::impl_createInstance( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( RuntimeException )
{
    return Reference< ::com::sun::star::uno::XInterface >( *new PasswordContainer( xServiceManager ) );
}

//-------------------------------------------------------------------------

Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL PasswordContainer::impl_createFactory( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& ServiceManager ) throw(RuntimeException)
{
    Reference< ::com::sun::star::lang::XSingleServiceFactory > xReturn( ::cppu::createOneInstanceFactory( ServiceManager,
                                                        PasswordContainer::impl_getStaticImplementationName(),
                                                        PasswordContainer::impl_createInstance,
                                                        PasswordContainer::impl_getStaticSupportedServiceNames()));
    return xReturn ;

}

//-------------------------------------------------------------------------

void StorageItem::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames )
{
    // this feature still should not be used
    if( mainCont )
        mainCont->Notify();
}

