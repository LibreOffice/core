/*************************************************************************
 *
 *  $RCSfile: xstorage.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:13:03 $
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

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif


#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif


#ifndef _COMPHELPER_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif


#include "xstorage.hxx"
#include "owriteablestream.hxx"
#include "disposelistener.hxx"

#define STOR_MESS_PRECOMMIT 1
#define STOR_MESS_COMMITED  2
#define STOR_MESS_PREREVERT 3
#define STOR_MESS_REVERTED  4

using namespace ::com::sun::star;

//=========================================================

typedef ::std::list< uno::WeakReference< lang::XComponent > > WeakComponentList;

struct StorInternalData_Impl
{
    SotMutexHolderRef m_rSharedMutexRef;
    ::cppu::OMultiTypeInterfaceContainerHelper m_aListenersContainer; // list of listeners
    ::cppu::OTypeCollection* m_pTypeCollection;
    sal_Bool m_bIsRoot;
    sal_Bool m_bReadOnlyWrap;

    OChildDispListener_Impl* m_pSubElDispListener;

    WeakComponentList m_aOpenSubComponentsList;

    // the mutex reference MUST NOT be empty
    StorInternalData_Impl( const SotMutexHolderRef& rMutexRef, sal_Bool bRoot, sal_Bool bReadOnlyWrap )
    : m_rSharedMutexRef( rMutexRef )
    , m_aListenersContainer( rMutexRef->GetMutex() )
    , m_pTypeCollection( NULL )
    , m_bIsRoot( bRoot )
    , m_bReadOnlyWrap( bReadOnlyWrap )
    , m_pSubElDispListener( NULL )
    {}

    ~StorInternalData_Impl();
};

//=========================================================

extern uno::Sequence< sal_Int8 > MakeKeyFromPass( ::rtl::OUString aPass, sal_Bool bUseUTF );
extern void copyInputToOutput_Impl( const uno::Reference< io::XInputStream >& aIn,
                                    const uno::Reference< io::XOutputStream >& aOut );

::rtl::OUString GetNewTempFileURL( const uno::Reference< lang::XMultiServiceFactory > xFactory );

void completeStorageStreamCopy_Impl( const uno::Reference< io::XStream >& xSource,
                              const uno::Reference< io::XStream >& xDest )
{
        uno::Reference< beans::XPropertySet > xSourceProps( xSource, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xDestProps( xDest, uno::UNO_QUERY );
        if ( !xSourceProps.is() || !xDestProps.is() )
            throw uno::RuntimeException(); //TODO

        uno::Reference< io::XOutputStream > xDestOutStream = xDest->getOutputStream();
        if ( !xDestOutStream.is() )
            throw io::IOException(); // TODO

        uno::Reference< io::XInputStream > xSourceInStream = xSource->getInputStream();
        if ( !xSourceInStream.is() )
            throw io::IOException(); // TODO

        // TODO: headers of encripted streams should be copied also
        copyInputToOutput_Impl( xSourceInStream, xDestOutStream );

        const char* pStrings[3] = { "MediaType", "Compressed", "UseCommonStoragePasswordEncryption" };
        for ( int ind = 0; ind < 3; ind++ )
        {
            ::rtl::OUString aPropName = ::rtl::OUString::createFromAscii( pStrings[ind] );
            xDestProps->setPropertyValue( aPropName, xSourceProps->getPropertyValue( aPropName ) );
        }
}

uno::Reference< io::XInputStream > GetSeekableTempCopy( uno::Reference< io::XInputStream > xInStream,
                                                        uno::Reference< lang::XMultiServiceFactory > xFactory )
{
    uno::Reference < io::XOutputStream > xTempOut(
                        xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                        uno::UNO_QUERY );
    uno::Reference < io::XInputStream > xTempIn( xTempOut, uno::UNO_QUERY );

    if ( !xTempOut.is() || !xTempIn.is() )
        throw io::IOException();

    copyInputToOutput_Impl( xInStream, xTempOut );
    xTempOut->closeOutput();

    return xTempIn;
}

StorInternalData_Impl::~StorInternalData_Impl()
{
    if ( m_pTypeCollection )
        delete m_pTypeCollection;
}


SotElement_Impl::SotElement_Impl( const ::rtl::OUString& rName, sal_Bool bStor, sal_Bool bNew )
: m_aName( rName )
, m_aOriginalName( rName )
, m_bIsStorage( bStor )
, m_bIsRemoved( sal_False )
, m_bIsInserted( bNew )
, m_pStorage( NULL )
, m_pStream( NULL )
{
}

SotElement_Impl::~SotElement_Impl()
{
    if ( m_pStorage )
        delete m_pStorage;

    if ( m_pStream )
        delete m_pStream;
}

//-----------------------------------------------
// most of properties are holt by the storage but are not used
OStorage_Impl::OStorage_Impl(   uno::Reference< io::XInputStream > xInputStream,
                                sal_Int32 nMode,
                                uno::Sequence< beans::PropertyValue > xProperties,
                                uno::Reference< lang::XMultiServiceFactory > xFactory )
: m_rMutexRef( new SotMutexHolder )
, m_pAntiImpl( NULL )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( sal_False )
, m_bCommited( sal_False )
, m_bIsRoot( sal_True )
, m_bListCreated( sal_False )
, m_xFactory( xFactory )
, m_xInputStream( xInputStream )
, m_xProperties( xProperties )
, m_bHasCommonPassword( sal_False )
, m_pParent( NULL )
, m_bControlMediaType( sal_False )
{
    // all the checks done below by assertion statements must be done by factory
    OSL_ENSURE( xInputStream.is(), "No input stream is provided!\n" );

    if ( m_nStorageMode & embed::ElementModes::WRITE )
    {
        // check that the stream allows to write
        OSL_ENSURE( sal_False, "No stream for writing is provided!\n" );
    }
}

//-----------------------------------------------
// most of properties are holt by the storage but are not used
OStorage_Impl::OStorage_Impl(   uno::Reference< io::XStream > xStream,
                                sal_Int32 nMode,
                                uno::Sequence< beans::PropertyValue > xProperties,
                                uno::Reference< lang::XMultiServiceFactory > xFactory )
: m_rMutexRef( new SotMutexHolder )
, m_pAntiImpl( NULL )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( sal_False )
, m_bCommited( sal_False )
, m_bIsRoot( sal_True )
, m_bListCreated( sal_False )
, m_xFactory( xFactory )
, m_xStream( xStream )
, m_xProperties( xProperties )
, m_pParent( NULL )
, m_bControlMediaType( sal_False )
{
    // all the checks done below by assertion statements must be done by factory
    OSL_ENSURE( xStream.is(), "No stream is provided!\n" );

    if ( m_nStorageMode & embed::ElementModes::WRITE )
    {
        // check that the stream allows to write
        OSL_ENSURE( xStream->getOutputStream().is(), "No stream for writing is provided!\n" );
    }
    else
    {
        m_xInputStream = xStream->getInputStream();
        OSL_ENSURE( m_xInputStream.is(), "No input stream is provided!\n" );
    }
}

//-----------------------------------------------
OStorage_Impl::OStorage_Impl(   OStorage_Impl* pParent,
                                sal_Int32 nMode,
                                uno::Reference< container::XNameContainer > xPackageFolder,
                                uno::Reference< lang::XSingleServiceFactory > xPackage,
                                uno::Reference< lang::XMultiServiceFactory > xFactory )
: m_rMutexRef( new SotMutexHolder )
, m_pAntiImpl( NULL )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( sal_False )
, m_bCommited( sal_False )
, m_bIsRoot( sal_False )
, m_bListCreated( sal_False )
, m_xPackageFolder( xPackageFolder )
, m_xPackage( xPackage )
, m_xFactory( xFactory )
, m_pParent( pParent ) // can be empty in case of temporary readonly substorages
, m_bControlMediaType( sal_False )
{
    OSL_ENSURE( xPackageFolder.is(), "No package folder!\n" );
}

//-----------------------------------------------
OStorage_Impl::~OStorage_Impl()
{
    {
        ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
        if ( m_pAntiImpl ) // root storage wrapper must set this member to NULL before destruction of object
        {
            OSL_ENSURE( !m_bIsRoot, "The root storage wrapper must be disposed already" );

            try {
                m_pAntiImpl->InternalDispose( sal_False );
            }
            catch ( uno::RuntimeException& )
            {}
            m_pAntiImpl = NULL;
        }
        else if ( !m_aReadOnlyWrapList.empty() )
        {
            for ( OStorageList_Impl::iterator pStorageIter = m_aReadOnlyWrapList.begin();
                  pStorageIter != m_aReadOnlyWrapList.end(); pStorageIter++ )
            {
                uno::Reference< embed::XStorage > xTmp = pStorageIter->m_xWeakRef;
                if ( xTmp.is() )
                    try {
                        pStorageIter->m_pPointer->InternalDispose( sal_False );
                    } catch( uno::Exception& ) {}
            }

            m_aReadOnlyWrapList.clear();
        }

        m_pParent = NULL;
    }

    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); pElementIter++ )
        delete *pElementIter;

    m_aChildrenList.clear();

    m_xPackageFolder = uno::Reference< container::XNameContainer >();
    m_xPackage = uno::Reference< lang::XSingleServiceFactory >();

    ::rtl::OUString aPropertyName = ::rtl::OUString::createFromAscii( "URL" );
    for ( sal_Int32 aInd = 0; aInd < m_xProperties.getLength(); aInd++ )
    {
        if ( m_xProperties[aInd].Name.equals( aPropertyName ) )
        {
            // the storage is URL based so all the streams are opened by factory and should be closed
            try
            {
                if ( m_xInputStream.is() )
                {
                    m_xInputStream->closeInput();
                    m_xInputStream = uno::Reference< io::XInputStream >();
                }

                if ( m_xStream.is() )
                {
                    uno::Reference< io::XInputStream > xInStr = m_xStream->getInputStream();
                    if ( xInStr.is() )
                        xInStr->closeInput();

                    uno::Reference< io::XOutputStream > xOutStr = m_xStream->getOutputStream();
                    if ( xOutStr.is() )
                        xOutStr->closeOutput();

                    m_xStream = uno::Reference< io::XStream >();
                }
            }
            catch( uno::Exception& )
            {}
        }
    }
}

//-----------------------------------------------
void OStorage_Impl::SetReadOnlyWrap( OStorage& aStorage )
{
    // Weak reference is used inside the holder so the refcount must not be zero at this point
    OSL_ENSURE( aStorage.GetRefCount_Impl(), "There must be a reference alive to use this method!\n" );
    m_aReadOnlyWrapList.push_back( StorageHolder_Impl( &aStorage ) );
}

//-----------------------------------------------
void OStorage_Impl::RemoveReadOnlyWrap( OStorage& aStorage )
{
    for ( OStorageList_Impl::iterator pStorageIter = m_aReadOnlyWrapList.begin();
      pStorageIter != m_aReadOnlyWrapList.end();)
    {
        uno::Reference< embed::XStorage > xTmp = pStorageIter->m_xWeakRef;
        if ( !xTmp.is() || pStorageIter->m_pPointer == &aStorage )
        {
            try {
                pStorageIter->m_pPointer->InternalDispose( sal_False );
            } catch( uno::Exception& ) {}

            OStorageList_Impl::iterator pIterToDelete( pStorageIter );
            pStorageIter++;
            m_aReadOnlyWrapList.erase( pIterToDelete );
        }
        else
            pStorageIter++;
    }
}

//-----------------------------------------------
void OStorage_Impl::OpenOwnPackage()
{
    OSL_ENSURE( m_bIsRoot, "Opening of the package has no sence!\n" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xPackageFolder.is() )
    {
        if ( !m_xPackage.is() )
        {
            sal_Bool bRepairPackage = sal_False;
            uno::Reference< ucb::XProgressHandler > xProgressHandler;

            uno::Sequence< uno::Any > aArguments( 1 );
            if ( m_nStorageMode & embed::ElementModes::WRITE )
                aArguments[ 0 ] <<= m_xStream;
            else
            {
                OSL_ENSURE( m_xInputStream.is(), "Input stream must be set for readonly access!\n" );
                aArguments[ 0 ] <<= m_xInputStream;
                // TODO: if input stream is not seekable or XSeekable interface is supported
                // on XStream object a wrapper must be used
            }

            sal_Int32 nArgNum = 1;
            for ( sal_Int32 aInd = 0; aInd < m_xProperties.getLength(); aInd++ )
            {
                if ( m_xProperties[aInd].Name.equalsAscii( "RepairPackage" ) )
                {
                    aArguments.realloc( ++nArgNum );
                    // TODO: implement in package component
                    // May be it should not be just a boolean value
                    // but a property value
                    aArguments[nArgNum] = m_xProperties[aInd].Value;
                }
                else if ( m_xProperties[aInd].Name.equalsAscii( "Password" ) )
                {
                    // TODO: implement password setting for documents
                    // the password entry must be removed after setting
                }
                /* TODO:
                else if ( m_xProperties[aInd].Name.equalsAscii( "ProgressHandler" ) )
                {
                    aArguments.realloc( ++nArgNum );
                    aArguments[nArgNum] = m_xProperties[aInd].Value;
                }
                */
            }

            m_xPackage = uno::Reference< lang::XSingleServiceFactory > (
                                        GetServiceFactory()->createInstanceWithArguments(
                                            ::rtl::OUString::createFromAscii( "com.sun.star.packages.comp.ZipPackage" ),
                                            aArguments ),
                                        uno::UNO_QUERY );
        }

        uno::Reference< container::XHierarchicalNameAccess > xHNameAccess( m_xPackage, uno::UNO_QUERY );
        OSL_ENSURE( xHNameAccess.is(), "The package could not be created!\n" );

        if ( xHNameAccess.is() )
        {
            uno::Any aFolder = xHNameAccess->getByHierarchicalName( ::rtl::OUString::createFromAscii( "/" ) );
            aFolder >>= m_xPackageFolder;
        }
    }

    OSL_ENSURE( m_xPackageFolder.is(), "The package root folder can not be opened!\n" );
    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException(); // TODO
}

//-----------------------------------------------
uno::Reference< lang::XMultiServiceFactory > OStorage_Impl::GetServiceFactory()
{
    if ( m_xFactory.is() )
        return m_xFactory;

    return ::comphelper::getProcessServiceFactory();
}

//-----------------------------------------------
SotElementList_Impl& OStorage_Impl::GetChildrenList()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    ReadContents();
    return m_aChildrenList;
}

//-----------------------------------------------
void OStorage_Impl::GetStorageProperties()
{
    if ( !m_bControlMediaType )
    {
        uno::Reference< beans::XPropertySet > xProps( m_xPackageFolder, uno::UNO_QUERY );
        if ( !xProps.is() )
            throw uno::RuntimeException(); // TODO:

        xProps->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) ) >>= m_aMediaType;
        m_bControlMediaType = sal_True;
    }
}

//-----------------------------------------------
void OStorage_Impl::ReadContents()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bListCreated )
        return;

    if ( m_bIsRoot )
        OpenOwnPackage();

    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xPackageFolder, uno::UNO_QUERY );
    if ( !xEnumAccess.is() )
        throw uno::RuntimeException(); // TODO:

    uno::Reference< container::XEnumeration > xEnum = xEnumAccess->createEnumeration();
    if ( !xEnum.is() )
        throw uno::RuntimeException(); // TODO:

    m_bListCreated = sal_True;

    while( xEnum->hasMoreElements() )
    {
        try {
            uno::Reference< container::XNamed > xNamed;
            xEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                OSL_ENSURE( sal_False, "XNamed is not supported!\n" );
                throw uno::RuntimeException(); // TODO:
            }

            ::rtl::OUString aName = xNamed->getName();
            OSL_ENSURE( aName.getLength(), "Empty name!\n" );

            sal_Bool bIsStorage = sal_False;
            uno::Reference< container::XNameContainer > xNameContainer( xNamed, uno::UNO_QUERY );

            SotElement_Impl* pNewElement = new SotElement_Impl( aName, xNameContainer.is(), sal_False );
            if ( ( m_nStorageMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
            {
                // if a storage is truncated all of it elements are marked as deleted
                pNewElement->m_bIsRemoved = sal_True;
            }

            m_aChildrenList.push_back( pNewElement );
        }
        catch( container::NoSuchElementException& )
        {
            OSL_ENSURE( sal_False, "hasMoreElements() implementation has problems!\n" );
            break;
        }
    }

    // cache changeable folder properties
    GetStorageProperties();
}

//-----------------------------------------------
void OStorage_Impl::CopyToStorage( const uno::Reference< embed::XStorage >& xDest )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    uno::Reference< beans::XPropertySet > xPropSet( xDest, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        throw lang::IllegalArgumentException(); // TODO:

    sal_Int32 nDestMode = embed::ElementModes::READ;
    xPropSet->getPropertyValue( ::rtl::OUString::createFromAscii( "OpenMode" ) ) >>= nDestMode;

    if ( !( nDestMode & embed::ElementModes::WRITE ) )
        throw io::IOException(); // TODO: access_denied

    ReadContents();

    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException(); // TODO:

    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); pElementIter++ )
    {
        if ( !(*pElementIter)->m_bIsRemoved )
            CopyStorageElement( *pElementIter, xDest, (*pElementIter)->m_aName );
    }

    // move storage properties to the destination one ( means changeable properties )
    ::rtl::OUString aMediaTypeString = ::rtl::OUString::createFromAscii( "MediaType" );
    xPropSet->setPropertyValue( aMediaTypeString, uno::makeAny( m_aMediaType ) );

    // if this is a root storage, the common key from current one should be moved there
    sal_Bool bIsRoot = sal_False;
    ::rtl::OUString aRootString = ::rtl::OUString::createFromAscii( "IsRoot" );
    if ( ( xPropSet->getPropertyValue( aRootString ) >>= bIsRoot ) && bIsRoot )
    {
        try
        {
            ::rtl::OUString aCommonPass = GetCommonRootPass();
            uno::Reference< embed::XEncryptionProtectedSource > xEncr( xDest, uno::UNO_QUERY );
            if ( xEncr.is() )
                xEncr->setEncryptionPassword( aCommonPass );
        }
        catch( packages::NoEncryptionException& )
        {}
    }

    // if possible the destination storage should be commited after successful copying
    uno::Reference< embed::XTransactedObject > xObjToCommit( xDest, uno::UNO_QUERY );
    if ( xObjToCommit.is() )
        xObjToCommit->commit();
}

//-----------------------------------------------
void OStorage_Impl::CopyStorageElement( SotElement_Impl* pElement,
                                        uno::Reference< embed::XStorage > xDest,
                                        ::rtl::OUString aName )
{
    OSL_ENSURE( xDest.is(), "No destination storage!\n" );
    OSL_ENSURE( aName.getLength(), "Empty element name!\n" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    uno::Reference< container::XNameAccess > xDestAccess( xDest, uno::UNO_QUERY );
    if ( !xDestAccess.is() )
        throw uno::RuntimeException();

    if ( xDestAccess->hasByName( aName )
      && !( pElement->m_bIsStorage && xDest->isStorageElement( aName ) ) )
        xDest->removeElement( aName );

    if ( pElement->m_bIsStorage )
    {
        uno::Reference< embed::XStorage > xSubDest =
                                    xDest->openStorageElement(  aName,
                                                                embed::ElementModes::WRITE );

        OSL_ENSURE( xSubDest.is(), "No destination substorage!\n" );

        if ( !pElement->m_pStorage )
        {
            OpenSubStorage( pElement, embed::ElementModes::READ );
            if ( !pElement->m_pStorage )
                throw io::IOException(); // TODO
        }

        pElement->m_pStorage->CopyToStorage( xSubDest );
    }
    else
    {
        if ( !pElement->m_pStream )
        {
            OpenSubStream( pElement );
            if ( !pElement->m_pStream )
                throw io::IOException(); // TODO
        }

        // TODO: copy encrypted element
        if ( !pElement->m_pStream->IsEncrypted() )
        {
            uno::Reference< io::XStream > xSubStr =
                                        xDest->openStreamElement( aName,
                                        embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
            OSL_ENSURE( xSubStr.is(), "No destination substream!\n" );

            pElement->m_pStream->CopyInternallyTo_Impl( xSubStr );
        }
        else if ( pElement->m_pStream->HasCachedPassword() && pElement->m_pStream->IsModified() )
        {
            uno::Reference< io::XStream > xSubStr =
                                        xDest->openEncryptedStreamElement( aName,
                                            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE,
                                            pElement->m_pStream->GetCachedPassword() );
            OSL_ENSURE( xSubStr.is(), "No destination substream!\n" );

            pElement->m_pStream->CopyInternallyTo_Impl( xSubStr, pElement->m_pStream->GetCachedPassword() );
        }
        else
        {
            try {
                uno::Reference< io::XInputStream > xRawInStream = pElement->m_pStream->GetRawInStream();
                xDest->insertRawEncrStreamElement( aName, xRawInStream );
            }
            catch( uno::RuntimeException& )
            {
                throw;
            }
            catch( uno::Exception& e )
            {
                throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Can't copy raw stream" ),
                                                 uno::Reference< io::XInputStream >(),
                                                 uno::makeAny( e ) );
            }
        }
    }
}

//-----------------------------------------------
void OStorage_Impl::CopyLastCommitTo( const uno::Reference< embed::XStorage >& xNewStor )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_xPackageFolder.is(), "A commited storage is incomplete!\n" );
    if ( !m_xPackageFolder.is() )
        throw uno::RuntimeException();

    OStorage_Impl aTempRepresent( NULL,
                                embed::ElementModes::READ,
                                m_xPackageFolder,
                                m_xPackage,
                                m_xFactory );

    aTempRepresent.CopyToStorage( xNewStor );
}

//-----------------------------------------------
void OStorage_Impl::InsertIntoPackageFolder( const ::rtl::OUString& aName,
                                             const uno::Reference< container::XNameContainer >& xParentPackageFolder )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_xPackageFolder.is(), "An inserted storage is incomplete!\n" );
    uno::Reference< lang::XUnoTunnel > xTunnel( m_xPackageFolder, uno::UNO_QUERY );
    if ( !xTunnel.is() )
        throw uno::RuntimeException(); // TODO

    xParentPackageFolder->insertByName( aName, uno::makeAny( xTunnel ) );

    m_bCommited = sal_False;
}

//-----------------------------------------------
void OStorage_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    // if storage is commited it should have a valid Package representation
    OSL_ENSURE( m_xPackageFolder.is(), "The package representation should exist!\n" );
    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException(); // TODO:

    OSL_ENSURE( m_nStorageMode & embed::ElementModes::WRITE,
                "Commit of readonly storage, should be detected before!\n" );

    if ( !m_bIsModified )
        return;

    uno::Reference< container::XNameContainer > xNewPackageFolder;

    // here the storage will switch to the temporary package folder
    // if the storage was already commited and the parent was not commited after that
    // the switch should not be done since the package folder in use is a temporary one;
    // it can be detected by m_bCommited flag ( root storage doesn't need temporary representation )
    if ( !m_bCommited && !m_bIsRoot )
    {
        uno::Sequence< uno::Any > aSeq( 1 );
        aSeq[0] <<= sal_True;

        xNewPackageFolder = uno::Reference< container::XNameContainer >(
                                                    m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );
    }
    else
        xNewPackageFolder = m_xPackageFolder;

    // remove replaced removed elements
    for ( SotElementList_Impl::iterator pDeletedIter = m_aDeletedList.begin();
          pDeletedIter != m_aDeletedList.end();
          pDeletedIter++ )
    {
        // the removed elements are not in new temporary storage
        if ( m_bCommited || m_bIsRoot )
            xNewPackageFolder->removeByName( (*pDeletedIter)->m_aOriginalName );
        delete *pDeletedIter;
        *pDeletedIter = NULL;
    }
    m_aDeletedList.clear();

    // remove removed elements
    SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
    while (  pElementIter != m_aChildrenList.end() )
    {
        // renamed and inserted elements must be really inserted to package later
        // since thay can conflict with removed elements

        if ( (*pElementIter)->m_bIsRemoved )
        {
            // the removed elements are not in new temporary storage
            if ( m_bCommited || m_bIsRoot )
                xNewPackageFolder->removeByName( (*pElementIter)->m_aOriginalName );

            SotElement_Impl* pToDelete = *pElementIter;

            pElementIter++; // to let the iterator be valid it should be increased before removing

            m_aChildrenList.remove( pToDelete );
            delete pToDelete;
        }
        else
            pElementIter++;
    }

    // there should be no more deleted elements
    for ( pElementIter = m_aChildrenList.begin(); pElementIter != m_aChildrenList.end(); pElementIter++ )
    {
        // if it is a 'duplicate commit' inserted elements must be really inserted to package later
        // since thay can conflict with renamed elements

        if ( !(*pElementIter)->m_bIsInserted )
        {
            // for now stream is opened in direct mode that means that in case
            // storage is commited all the streams from it are commited in current state.
            // following two steps are separated to allow easily implement transacted mode
            // for streams if we need it in future.
            if ( !(*pElementIter)->m_bIsStorage && (*pElementIter)->m_pStream )
                (*pElementIter)->m_pStream->Commit();

            // if the storage was not open, there is no need to commit it ???
            // the storage should be checked that it is commited
            if ( (*pElementIter)->m_bIsStorage && (*pElementIter)->m_pStorage && (*pElementIter)->m_pStorage->m_bCommited )
            {
                // it's temporary PackageFolder should be inserted instead of current one
                // also the new copy of PackageFolder should be used by the children storages

                // the renamed elements are not in new temporary storage
                if ( m_bCommited || m_bIsRoot )
                    xNewPackageFolder->removeByName( (*pElementIter)->m_aOriginalName );

                (*pElementIter)->m_pStorage->InsertIntoPackageFolder( (*pElementIter)->m_aName, xNewPackageFolder );
            }
            else if ( !(*pElementIter)->m_bIsStorage && (*pElementIter)->m_pStream && (*pElementIter)->m_pStream->m_bFlushed )
            {
                // the renamed elements are not in new temporary storage
                if ( m_bCommited || m_bIsRoot )
                    xNewPackageFolder->removeByName( (*pElementIter)->m_aOriginalName );

                (*pElementIter)->m_pStream->InsertIntoPackageFolder( (*pElementIter)->m_aName, xNewPackageFolder );
            }
            else if ( !m_bCommited && !m_bIsRoot )
            {
                // the element must be just copied to the new temporary package folder
                uno::Any aPackageElement = m_xPackageFolder->getByName( (*pElementIter)->m_aOriginalName );
                xNewPackageFolder->insertByName( (*pElementIter)->m_aName, aPackageElement );
            }
            else if ( (*pElementIter)->m_aName.compareTo( (*pElementIter)->m_aOriginalName ) )
            {
                // in case the name was changed and it is not a changed storage - rename the element
                uno::Reference< container::XNamed > xNamed;
                uno::Any aPackageElement = m_xPackageFolder->getByName( (*pElementIter)->m_aOriginalName );

                aPackageElement >>= xNamed;
                OSL_ENSURE( xNamed.is(), "PackageFolder/PackageStream MUST support XNamed interface" );

                xNamed->setName( (*pElementIter)->m_aName );
            }

            (*pElementIter)->m_aOriginalName = (*pElementIter)->m_aName;
        }
    }

    for ( pElementIter = m_aChildrenList.begin(); pElementIter != m_aChildrenList.end(); pElementIter++ )
    {
        // now inserted elements can be inserted to the package
        if ( (*pElementIter)->m_bIsInserted )
        {
            (*pElementIter)->m_aOriginalName = (*pElementIter)->m_aName;
            uno::Reference< lang::XUnoTunnel > xNewElement;

            if ( (*pElementIter)->m_bIsStorage )
            {
                if ( (*pElementIter)->m_pStorage->m_bCommited )
                {
                    OSL_ENSURE( (*pElementIter)->m_pStorage, "An inserted storage is incomplete!\n" );
                    if ( !(*pElementIter)->m_pStorage )
                        throw uno::RuntimeException(); // TODO

                    (*pElementIter)->m_pStorage->InsertIntoPackageFolder( (*pElementIter)->m_aName, xNewPackageFolder );

                    (*pElementIter)->m_bIsInserted = sal_False;
                }
            }
            else
            {
                OSL_ENSURE( (*pElementIter)->m_pStream, "An inserted stream is incomplete!\n" );
                if ( !(*pElementIter)->m_pStream )
                    throw uno::RuntimeException(); // TODO

                (*pElementIter)->m_pStream->Commit(); // direct mode
                (*pElementIter)->m_pStream->InsertIntoPackageFolder( (*pElementIter)->m_aName, xNewPackageFolder );

                (*pElementIter)->m_bIsInserted = sal_False;
            }
        }
    }

    // move properties to the destination package folder
    uno::Reference< beans::XPropertySet > xProps( xNewPackageFolder, uno::UNO_QUERY );
    if ( !xProps.is() )
        throw uno::RuntimeException(); // TODO:

    xProps->setPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ), uno::makeAny( m_aMediaType ) );

    if ( m_bIsRoot )
    {
        uno::Reference< util::XChangesBatch > xChangesBatch( m_xPackage, uno::UNO_QUERY );

        OSL_ENSURE( xChangesBatch.is(), "Impossible to commit package!\n" );
        if ( !xChangesBatch.is() )
            throw uno::RuntimeException(); // TODO

        xChangesBatch->commitChanges();
    }
    else if ( !m_bCommited )
    {
        m_xPackageFolder = xNewPackageFolder;
        m_bCommited = sal_True;
    }

    SetModifiedInternally( sal_False );

    // when the storage is commited the parent is modified
    if ( m_pParent )
        m_pParent->SetModifiedInternally( sal_True );
}

//-----------------------------------------------
void OStorage_Impl::Revert()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !( m_nStorageMode & embed::ElementModes::WRITE ) )
        return; // nothing to do

    // all the children must be removed
    // they will be created later on demand

    SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
    while (  pElementIter != m_aChildrenList.end() )
    {
        if ( (*pElementIter)->m_bIsInserted )
        {
            SotElement_Impl* pToDelete = *pElementIter;

            pElementIter++; // to let the iterator be valid it should be increased before removing

            m_aChildrenList.remove( pToDelete );
            delete pToDelete;
        }
        else
        {
            ClearElement( *pElementIter );

            (*pElementIter)->m_aName = (*pElementIter)->m_aOriginalName;
            (*pElementIter)->m_bIsRemoved = sal_False;

            pElementIter++;
        }
    }

    // return replaced removed elements
    for ( SotElementList_Impl::iterator pDeletedIter = m_aDeletedList.begin();
          pDeletedIter != m_aDeletedList.end();
          pDeletedIter++ )
    {
        m_aChildrenList.push_back( (*pDeletedIter) );

        ClearElement( *pDeletedIter );

        (*pDeletedIter)->m_aName = (*pDeletedIter)->m_aOriginalName;
        (*pDeletedIter)->m_bIsRemoved = sal_False;
    }
    m_aDeletedList.clear();

    GetStorageProperties();

    SetModifiedInternally( sal_False );
}

//-----------------------------------------------
void OStorage_Impl::SetModifiedInternally( sal_Bool bModified )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_aReadOnlyWrapList.empty(), "Opened for reading storage changes own modified state!\n" );

    if ( m_pAntiImpl )
        m_pAntiImpl->setModified( bModified );
    else
        m_bIsModified = bModified;
}

::rtl::OUString OStorage_Impl::GetCommonRootPass()
    throw ( ::com::sun::star::packages::NoEncryptionException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( m_bIsRoot )
    {
        if ( !m_bHasCommonPassword )
            throw packages::NoEncryptionException();

        return m_aCommonPassword;
    }
    else
    {
        if ( !m_pParent )
            throw packages::NoEncryptionException();

        return m_pParent->GetCommonRootPass();
    }
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::FindElement( const ::rtl::OUString& rName )
{
    OSL_ENSURE( rName.getLength(), "Name is empty!" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    ReadContents();

    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); pElementIter++ )
    {
        if ( (*pElementIter)->m_aName == rName && !(*pElementIter)->m_bIsRemoved )
            return *pElementIter;
    }

    return NULL;
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::InsertStream( ::rtl::OUString aName, sal_Bool bEncr )
{
    OSL_ENSURE( m_xPackage.is(), "Not possible to refer to package as to factory!\n" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException(); // TODO

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= sal_False;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    OSL_ENSURE( xNewElement.is(), "Not possible to create a new stream!\n" );
    if ( !xNewElement.is() )
        throw io::IOException(); // TODO:

    uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xNewElement, uno::UNO_QUERY );
    if ( !xPackageSubStream.is() )
        throw uno::RuntimeException(); // TODO

    // the mode is not needed for storage stream internal implementation
    SotElement_Impl* pNewElement = InsertElement( aName, sal_False );
    pNewElement->m_pStream = new OWriteStream_Impl( this, xPackageSubStream, m_xPackage, m_xFactory, bEncr );

    m_aChildrenList.push_back( pNewElement );
    SetModifiedInternally( sal_True );

    return pNewElement;
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::InsertRawStream( ::rtl::OUString aName, const uno::Reference< io::XInputStream >& xInStream )
{
    // insert of raw stream means insert and commit
    OSL_ENSURE( m_xPackage.is(), "Not possible to refer to package as to factory!\n" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException(); // TODO

    uno::Reference< io::XSeekable > xSeek( xInStream, uno::UNO_QUERY );
    uno::Reference< io::XInputStream > xInStrToInsert = xSeek.is() ? xInStream :
                                                                     GetSeekableTempCopy( xInStream, GetServiceFactory() );

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= sal_False;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    OSL_ENSURE( xNewElement.is(), "Not possible to create a new stream!\n" );
    if ( !xNewElement.is() )
        throw io::IOException(); // TODO:

    uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xNewElement, uno::UNO_QUERY );
    if ( !xPackageSubStream.is() )
        throw uno::RuntimeException(); // TODO

    xPackageSubStream->setRawStream( xInStrToInsert );

    // the mode is not needed for storage stream internal implementation
    SotElement_Impl* pNewElement = InsertElement( aName, sal_False );
    pNewElement->m_pStream = new OWriteStream_Impl( this, xPackageSubStream, m_xPackage, m_xFactory, sal_True );
    // the stream is inserted and must be treated as a commited one
    pNewElement->m_pStream->SetToBeCommited();

    m_aChildrenList.push_back( pNewElement );
    SetModifiedInternally( sal_True );

    return pNewElement;
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::InsertStorage( ::rtl::OUString aName, sal_Int32 nStorageMode )
{
    SotElement_Impl* pNewElement = InsertElement( aName, sal_True );

    OSL_ENSURE( m_xPackage.is(), "Not possible to refer to package as to factory!\n" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException(); // TODO

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= sal_True;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    OSL_ENSURE( xNewElement.is(), "Not possible to create a new storage!\n" );
    if ( !xNewElement.is() )
        throw io::IOException(); // TODO:

    uno::Reference< container::XNameContainer > xPackageSubFolder( xNewElement, uno::UNO_QUERY );
    if ( !xPackageSubFolder.is() )
        throw uno::RuntimeException(); // TODO:

    pNewElement->m_pStorage = new OStorage_Impl( this, nStorageMode, xPackageSubFolder, m_xPackage, m_xFactory );

    m_aChildrenList.push_back( pNewElement );
    SetModifiedInternally( sal_True );

    return pNewElement;
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::InsertElement( ::rtl::OUString aName, sal_Bool bIsStorage )
{
    OSL_ENSURE( FindElement( aName ) == NULL, "Should not try to insert existing element" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    SotElement_Impl* pDeletedElm = NULL;

    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); pElementIter++ )
    {
        if ( (*pElementIter)->m_aName == aName )
        {
            OSL_ENSURE( (*pElementIter)->m_bIsRemoved, "Try to insert an element instead of existing one!\n" );
            if ( (*pElementIter)->m_bIsRemoved )
            {
                OSL_ENSURE( !(*pElementIter)->m_bIsInserted, "Inserted elements must be deleted immediatelly!\n" );
                pDeletedElm = *pElementIter;
                break;
            }
        }
    }

    if ( pDeletedElm )
    {
        if ( pDeletedElm->m_bIsStorage )
            OpenSubStorage( pDeletedElm, embed::ElementModes::READWRITE );
        else
            OpenSubStream( pDeletedElm );

        m_aChildrenList.remove( pDeletedElm );  // correct usage of list ???
        m_aDeletedList.push_back( pDeletedElm );
    }

    // create new element
    return new SotElement_Impl( aName, bIsStorage, sal_True );
}

//-----------------------------------------------
void OStorage_Impl::OpenSubStorage( SotElement_Impl* pElement, sal_Int32 nStorageMode )
{
    OSL_ENSURE( pElement, "pElement is not set!\n" );
    OSL_ENSURE( pElement->m_bIsStorage, "Storage flag is not set!\n" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !pElement->m_pStorage )
    {
        OSL_ENSURE( !pElement->m_bIsInserted, "Inserted element must be created already!\n" );

        uno::Reference< lang::XUnoTunnel > xTunnel;
        m_xPackageFolder->getByName( pElement->m_aOriginalName ) >>= xTunnel;
        if ( !xTunnel.is() )
            throw container::NoSuchElementException(); // TODO:

        uno::Reference< container::XNameContainer > xPackageSubFolder( xTunnel, uno::UNO_QUERY );

        OSL_ENSURE( xPackageSubFolder.is(), "Can not get XNameContainer interface from folder!\n" );

        if ( !xPackageSubFolder.is() )
            throw uno::RuntimeException(); // TODO:

        pElement->m_pStorage = new OStorage_Impl( this, nStorageMode, xPackageSubFolder, m_xPackage, m_xFactory );
    }
}

//-----------------------------------------------
void OStorage_Impl::OpenSubStream( SotElement_Impl* pElement )
{
    OSL_ENSURE( pElement, "pElement is not set!\n" );
    OSL_ENSURE( !pElement->m_bIsStorage, "Storage flag is set!\n" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !pElement->m_pStream )
    {
        OSL_ENSURE( !pElement->m_bIsInserted, "Inserted element must be created already!\n" );

        uno::Reference< lang::XUnoTunnel > xTunnel;
        m_xPackageFolder->getByName( pElement->m_aOriginalName ) >>= xTunnel;
        if ( !xTunnel.is() )
            throw container::NoSuchElementException(); // TODO:

        uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xTunnel, uno::UNO_QUERY );
        if ( !xPackageSubStream.is() )
            throw uno::RuntimeException(); // TODO

        pElement->m_pStream = new OWriteStream_Impl( this, xPackageSubStream, m_xPackage, m_xFactory, sal_False );
    }
}

//-----------------------------------------------
uno::Sequence< ::rtl::OUString > OStorage_Impl::GetElementNames()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    ReadContents();

    sal_uInt32 nSize = m_aChildrenList.size();
    uno::Sequence< ::rtl::OUString > aElementNames( nSize );

    sal_uInt32 nInd = 0;
    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); pElementIter++ )
    {
        if ( !(*pElementIter)->m_bIsRemoved )
            aElementNames[nInd++] = (*pElementIter)->m_aName;
    }

    aElementNames.realloc( nInd );
    return aElementNames;
}

//-----------------------------------------------
void OStorage_Impl::RemoveElement( SotElement_Impl* pElement )
{
    OSL_ENSURE( pElement, "Element must be provided!" );

    if ( !pElement )
        return;

    if ( pElement->m_pStorage && ( pElement->m_pStorage->m_pAntiImpl || !pElement->m_pStorage->m_aReadOnlyWrapList.empty() )
      || pElement->m_pStream && ( pElement->m_pStream->m_pAntiImpl || !pElement->m_pStream->m_aInputStreamsList.empty() ) )
        throw io::IOException(); // TODO: Access denied

    if ( pElement->m_bIsInserted )
    {
        m_aChildrenList.remove( pElement );
        delete pElement; // ???
    }
    else
    {
        pElement->m_bIsRemoved = sal_True;
        ClearElement( pElement );
    }
}

//-----------------------------------------------
void OStorage_Impl::ClearElement( SotElement_Impl* pElement )
{
    if ( pElement->m_pStorage )
    {
        delete pElement->m_pStorage;
        pElement->m_pStorage = NULL;
    }

    if ( pElement->m_pStream )
    {
        delete pElement->m_pStream;
        pElement->m_pStream = NULL;
    }
}


//=====================================================
// OStorage implementation
//=====================================================

//-----------------------------------------------
OStorage::OStorage( uno::Reference< io::XInputStream > xInputStream,
                    sal_Int32 nMode,
                    uno::Sequence< beans::PropertyValue > xProperties,
                    uno::Reference< lang::XMultiServiceFactory > xFactory )
: m_pImpl( new OStorage_Impl( xInputStream, nMode, xProperties, xFactory ) )
{
    m_pImpl->m_pAntiImpl = this;
    m_pData = new StorInternalData_Impl( m_pImpl->m_rMutexRef, m_pImpl->m_bIsRoot, sal_False );
}

//-----------------------------------------------
OStorage::OStorage( uno::Reference< io::XStream > xStream,
                    sal_Int32 nMode,
                    uno::Sequence< beans::PropertyValue > xProperties,
                    uno::Reference< lang::XMultiServiceFactory > xFactory )
: m_pImpl( new OStorage_Impl( xStream, nMode, xProperties, xFactory ) )
{
    m_pImpl->m_pAntiImpl = this;
    m_pData = new StorInternalData_Impl( m_pImpl->m_rMutexRef, m_pImpl->m_bIsRoot, sal_False );
}

//-----------------------------------------------
OStorage::OStorage( OStorage_Impl* pImpl, sal_Bool bReadOnlyWrap )
: m_pImpl( pImpl )
{
    // this call can be done only from OStorage_Impl implementation to create child storage
    OSL_ENSURE( m_pImpl && m_pImpl->m_rMutexRef.Is(), "The provided pointer & mutex MUST NOT be empty!\n" );

    m_pData = new StorInternalData_Impl( m_pImpl->m_rMutexRef, m_pImpl->m_bIsRoot, bReadOnlyWrap );

    OSL_ENSURE( ( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) == embed::ElementModes::WRITE ||
                    m_pData->m_bReadOnlyWrap,
                "The wrapper can not allow writing in case implementation does not!\n" );

    if ( !bReadOnlyWrap )
        m_pImpl->m_pAntiImpl = this;
}

//-----------------------------------------------
OStorage::~OStorage()
{
    {
        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );
        if ( m_pImpl )
        {
            m_refCount++; // to call dispose
            try {
                dispose();
            }
            catch( uno::RuntimeException& )
            {}
        }
    }

    if ( m_pData )
    {
        if ( m_pData->m_pSubElDispListener )
        {
            m_pData->m_pSubElDispListener->release();
            m_pData->m_pSubElDispListener = NULL;
        }

        delete m_pData;
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::InternalDispose( sal_Bool bNotifyImpl )
{
    if ( !m_pImpl )
        throw lang::DisposedException();

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    m_pData->m_aListenersContainer.disposeAndClear( aSource );

    if ( m_pData->m_bReadOnlyWrap )
    {
        OSL_ENSURE( !m_pData->m_aOpenSubComponentsList.size() || m_pData->m_pSubElDispListener,
                    "If any subelements are open the listener must exist!\n" );

        if ( m_pData->m_pSubElDispListener )
        {
            m_pData->m_pSubElDispListener->OwnerIsDisposed();

            // iterate through m_pData->m_aOpenSubComponentsList
            // deregister m_pData->m_pSubElDispListener and dispose all of them
            if ( !m_pData->m_aOpenSubComponentsList.empty() )
            {
                for ( WeakComponentList::iterator pCompIter = m_pData->m_aOpenSubComponentsList.begin();
                      pCompIter != m_pData->m_aOpenSubComponentsList.end(); pCompIter++ )
                {
                    uno::Reference< lang::XComponent > xTmp = (*pCompIter);
                    if ( xTmp.is() )
                    {
                        xTmp->removeEventListener( uno::Reference< lang::XEventListener >(
                                    static_cast< lang::XEventListener* >( m_pData->m_pSubElDispListener ) ) );

                        try {
                            xTmp->dispose();
                        } catch( uno::Exception& ) {}
                    }
                }

                m_pData->m_aOpenSubComponentsList.clear();
            }
        }

        if ( bNotifyImpl )
            m_pImpl->RemoveReadOnlyWrap( *this );
    }
    else
    {
        m_pImpl->m_pAntiImpl = NULL;

        if ( bNotifyImpl )
        {
            if ( m_pImpl->m_bIsRoot )
                delete m_pImpl;
            else
            {
                // the noncommited changes for the storage must be removed
                m_pImpl->Revert();
            }
        }
    }

    m_pImpl = NULL;
}

//-----------------------------------------------
void OStorage::ChildIsDisposed( const uno::Reference< uno::XInterface >& xChild )
{
    // this method can only be called by child disposing listener

    // this method must not contain any locking
    // the locking is done in the listener

    if ( !m_pData->m_aOpenSubComponentsList.empty() )
    {
        for ( WeakComponentList::iterator pCompIter = m_pData->m_aOpenSubComponentsList.begin();
              pCompIter != m_pData->m_aOpenSubComponentsList.end(); )
        {
            uno::Reference< lang::XComponent > xTmp = (*pCompIter);
            if ( !xTmp.is() || xTmp == xChild )
            {
                WeakComponentList::iterator pIterToRemove = pCompIter;
                pCompIter++;
                m_pData->m_aOpenSubComponentsList.erase( pIterToRemove );
            }
            else
                pCompIter++;
        }
    }
}

//-----------------------------------------------
void OStorage::BroadcastModified()
{
    if ( !m_pImpl )
        throw lang::DisposedException();

    OSL_ENSURE( !m_pData->m_bReadOnlyWrap, "The storage can not be modified at all!\n" );

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );

       ::cppu::OInterfaceContainerHelper* pContainer =
            m_pData->m_aListenersContainer.getContainer(
                ::getCppuType( ( const uno::Reference< util::XModifyListener >*) NULL ) );
       if ( pContainer )
    {
           ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
           while ( pIterator.hasMoreElements( ) )
           {
               ( ( util::XModifyListener* )pIterator.next( ) )->modified( aSource );
           }
    }
}

//-----------------------------------------------
void OStorage::BroadcastTransaction( sal_Int8 nMessage )
/*
    1 - preCommit
    2 - commited
    3 - preRevert
    4 - reverted
*/
{
    if ( !m_pImpl )
        throw lang::DisposedException();

    OSL_ENSURE( !m_pData->m_bReadOnlyWrap, "The storage can not be modified at all!\n" );

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );

       ::cppu::OInterfaceContainerHelper* pContainer =
            m_pData->m_aListenersContainer.getContainer(
                ::getCppuType( ( const uno::Reference< embed::XTransactionListener >*) NULL ) );
       if ( pContainer )
    {
           ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
           while ( pIterator.hasMoreElements( ) )
           {
            OSL_ENSURE( nMessage >= 1 && nMessage <= 4, "Wrong internal notification code is used!\n" );

            switch( nMessage )
            {
                case STOR_MESS_PRECOMMIT:
                       ( ( embed::XTransactionListener* )pIterator.next( ) )->preCommit( aSource );
                    break;
                case STOR_MESS_COMMITED:
                       ( ( embed::XTransactionListener* )pIterator.next( ) )->commited( aSource );
                    break;
                case STOR_MESS_PREREVERT:
                       ( ( embed::XTransactionListener* )pIterator.next( ) )->preRevert( aSource );
                    break;
                case STOR_MESS_REVERTED:
                       ( ( embed::XTransactionListener* )pIterator.next( ) )->reverted( aSource );
                    break;
            }
           }
    }
}

//-----------------------------------------------
SotElement_Impl* OStorage::OpenStreamElement_Impl( const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, sal_Bool bEncr )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    OSL_ENSURE( !m_pData->m_bReadOnlyWrap || ( nOpenMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE,
                "An element can not be opened for writing in readonly storage!\n" );

    SotElement_Impl *pElement = m_pImpl->FindElement( aStreamName );
    if ( !pElement )
    {
        // element does not exist, check if creation is allowed
        if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
          || (( nOpenMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE )
          || ( nOpenMode & embed::ElementModes::NOCREATE ) == embed::ElementModes::NOCREATE )
            throw io::IOException(); // TODO: access_denied

        // create a new StreamElement and insert it into the list
        pElement = m_pImpl->InsertStream( aStreamName, bEncr );
    }
    else if ( pElement->m_bIsStorage )
    {
        throw io::IOException(); // TODO:
    }

    OSL_ENSURE( pElement, "In case element can not be created an exception must be thrown!" );

    if ( !pElement->m_pStream )
        m_pImpl->OpenSubStream( pElement );

    if ( !pElement->m_pStream )
        throw io::IOException(); // TODO:

    return pElement;
}

//-----------------------------------------------
void OStorage::MakeLinkToSubComponent_Impl( const uno::Reference< lang::XComponent >& xComponent )
{
    if ( !xComponent.is() )
        throw uno::RuntimeException();

    if ( !m_pData->m_pSubElDispListener )
    {
        m_pData->m_pSubElDispListener = new OChildDispListener_Impl( *this );
        m_pData->m_pSubElDispListener->acquire();
    }

    xComponent->addEventListener( uno::Reference< lang::XEventListener >(
        static_cast< ::cppu::OWeakObject* >( m_pData->m_pSubElDispListener ), uno::UNO_QUERY ) );

    m_pData->m_aOpenSubComponentsList.push_back( xComponent );
}

//____________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Any SAL_CALL OStorage::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;
    if ( m_pData->m_bIsRoot )
    {
        aReturn <<= ::cppu::queryInterface
                    (   rType
                    ,   static_cast<lang::XTypeProvider*> ( this )
                    ,   static_cast<embed::XStorage*> ( this )
                    ,   static_cast<embed::XTransactedObject*> ( this )
                    ,   static_cast<embed::XTransactionBroadcaster*> ( this )
                    ,   static_cast<util::XModifiable*> ( this )
                    ,   static_cast<container::XNameAccess*> ( this )
                    ,   static_cast<container::XElementAccess*> ( this )
                    ,   static_cast<lang::XComponent*> ( this )
                    ,   static_cast<embed::XEncryptionProtectedSource*> ( this )
                    ,   static_cast<beans::XPropertySet*> ( this ) );
    }
    else
    {
        aReturn <<= ::cppu::queryInterface
                    (   rType
                    ,   static_cast<lang::XTypeProvider*> ( this )
                    ,   static_cast<embed::XStorage*> ( this )
                    ,   static_cast<embed::XTransactedObject*> ( this )
                    ,   static_cast<util::XModifiable*> ( this )
                    ,   static_cast<container::XNameAccess*> ( this )
                    ,   static_cast<container::XElementAccess*> ( this )
                    ,   static_cast<lang::XComponent*> ( this )
                    ,   static_cast<beans::XPropertySet*> ( this ) );
    }

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    return OWeakObject::queryInterface( rType );
}

//-----------------------------------------------
void SAL_CALL OStorage::acquire() throw()
{
    OWeakObject::acquire();
}

//-----------------------------------------------
void SAL_CALL OStorage::release() throw()
{
    OWeakObject::release();
}

//____________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OStorage::getTypes()
        throw( uno::RuntimeException )
{
    if ( m_pData->m_pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

        if ( m_pData->m_pTypeCollection == NULL )
        {
            if ( m_pData->m_bIsRoot )
            {
                m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XStorage >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactionBroadcaster >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< util::XModifiable >* )NULL )
                                    // ,    ::getCppuType( ( const uno::Reference< container::XNameAccess >* )NULL )
                                    // ,    ::getCppuType( ( const uno::Reference< lang::XComponent >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XEncryptionProtectedSource >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
            }
            else
            {
                m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XStorage >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< util::XModifiable >* )NULL )
                                    // ,    ::getCppuType( ( const uno::Reference< container::XNameAccess >* )NULL )
                                    // ,    ::getCppuType( ( const uno::Reference< lang::XComponent >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
            }
        }
    }

    return m_pData->m_pTypeCollection->getTypes() ;
}

//-----------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OStorage::getImplementationId()
        throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pID == NULL )
        {
            static ::cppu::OImplementationId aID( sal_False ) ;
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;

}

//-----------------------------------------------
uno::Reference< io::XStream > OStorage_Impl::CloneStreamElement( const ::rtl::OUString& aStreamName,
                                                                sal_Bool bPassProvided,
                                                                const ::rtl::OUString& aPass )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    SotElement_Impl *pElement = FindElement( aStreamName );
    if ( !pElement )
    {
        // element does not exist, throw exception
        throw io::IOException(); // TODO: access_denied
    }
    else if ( pElement->m_bIsStorage )
        throw io::IOException(); // TODO:

    if ( !pElement->m_pStream )
        OpenSubStream( pElement );

    uno::Reference< io::XStream > xResult;
    if ( pElement->m_pStream && pElement->m_pStream->m_xPackageStream.is() )
    {
        // the existence of m_pAntiImpl of the child is not interesting,
        // the copy will be created internally

        // usual copying is not applicable here, only last flushed version of the
        // child stream should be used for copiing. Probably the childs m_xPackageStream
        // can be used as a base of a new stream, that would be copied to result
        // storage. The only problem is that some package streams can be accessed from outside
        // at the same time ( now solwed by wrappers that remember own position ).

        if ( bPassProvided )
            xResult = pElement->m_pStream->GetCopyOfLastCommit( aPass );
        else
            xResult = pElement->m_pStream->GetCopyOfLastCommit();
    }
    else
        throw io::IOException(); // TODO: general_error

    return xResult;
}

//____________________________________________________________________________________________________
//  XStorage
//____________________________________________________________________________________________________


//-----------------------------------------------
void SAL_CALL OStorage::copyToStorage( const uno::Reference< embed::XStorage >& xDest )
        throw ( embed::InvalidStorageException,
                io::IOException,
                lang::IllegalArgumentException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject*> ( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException(); // TODO:

    m_pImpl->CopyToStorage( xDest );
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::openStreamElement(
    const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( ( nOpenMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException(); // TODO: access denied

    SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamName, nOpenMode, sal_False );
    OSL_ENSURE( pElement && pElement->m_pStream, "In case element can not be created an exception must be thrown!" );

    uno::Reference< io::XStream > xResult = pElement->m_pStream->GetStream( nOpenMode );
    OSL_ENSURE( xResult.is(), "The method must throw exception instead of removing empty result!\n" );

    if ( m_pData->m_bReadOnlyWrap )
    {
        // before the storage disposes the stream it must deregister itself as listener
        uno::Reference< lang::XComponent > xStreamComponent( xResult, uno::UNO_QUERY );
        if ( !xStreamComponent.is() )
            throw uno::RuntimeException(); // TODO

        MakeLinkToSubComponent_Impl( xStreamComponent );
    }

    return xResult;
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::openEncryptedStreamElement(
    const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, const ::rtl::OUString& aPass )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( ( nOpenMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException(); // TODO: access denied

    if ( !aPass.getLength() )
        throw lang::IllegalArgumentException();

    SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamName, nOpenMode, sal_True );
    OSL_ENSURE( pElement && pElement->m_pStream, "In case element can not be created an exception must be thrown!" );

    uno::Reference< io::XStream > xResult = pElement->m_pStream->GetStream( nOpenMode, aPass );
    OSL_ENSURE( xResult.is(), "The method must throw exception instead of removing empty result!\n" );

    if ( m_pData->m_bReadOnlyWrap )
    {
        // before the storage disposes the stream it must deregister itself as listener
        uno::Reference< lang::XComponent > xStreamComponent( xResult, uno::UNO_QUERY );
        if ( !xStreamComponent.is() )
            throw uno::RuntimeException(); // TODO

        MakeLinkToSubComponent_Impl( xStreamComponent );
    }

    return xResult;
}

//-----------------------------------------------
uno::Reference< embed::XStorage > SAL_CALL OStorage::openStorageElement(
            const ::rtl::OUString& aStorName, sal_Int32 nStorageMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( ( nStorageMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException(); // TODO: access denied

    if ( ( nStorageMode & embed::ElementModes::TRUNCATE )
      && !( nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException(); // TODO: access denied

    // it's allways possible to read written storage in this implementation
    nStorageMode |= embed::ElementModes::READ;

    SotElement_Impl *pElement = m_pImpl->FindElement( aStorName );
    if ( !pElement )
    {
        // element does not exist, check if creation is allowed
        if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
          || (( nStorageMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE )
          || ( nStorageMode & embed::ElementModes::NOCREATE ) == embed::ElementModes::NOCREATE )
            throw io::IOException(); // TODO: access_denied

        // create a new StorageElement and insert it into the list
        pElement = m_pImpl->InsertStorage( aStorName, nStorageMode );
    }
    else if ( !pElement->m_bIsStorage )
    {
        throw io::IOException(); // TODO:
    }
    else if ( pElement->m_pStorage )
    {
        // storage has already been opened; it may be opened another time, if it the mode allows to do so
        if ( pElement->m_pStorage->m_pAntiImpl )
        {
            throw io::IOException(); // TODO: access_denied
        }
        else if ( !pElement->m_pStorage->m_aReadOnlyWrapList.empty()
                && ( nStorageMode & embed::ElementModes::WRITE ) )
        {
            throw io::IOException(); // TODO: access_denied
        }
        else
        {
            if ( ( nStorageMode & embed::ElementModes::WRITE )
              && !( pElement->m_pStorage->m_nStorageMode & embed::ElementModes::WRITE ) )
            {
                delete pElement->m_pStorage;
                pElement->m_pStorage = NULL;
            }
            else
            {
                // in case parent storage allows writing the readonly mode of the child storage is
                // virtual, that means that it is just enough to change the flag to let it be writable
                // and since there is no AntiImpl nobody should be notified about it
                pElement->m_pStorage->m_nStorageMode = nStorageMode | embed::ElementModes::READ;

                if ( ( nStorageMode & embed::ElementModes::TRUNCATE ) )
                {
                    for ( SotElementList_Impl::iterator pElementIter = pElement->m_pStorage->m_aChildrenList.begin();
                           pElementIter != pElement->m_pStorage->m_aChildrenList.end(); )
                    {
                        SotElement_Impl* pElementToDel = (*pElementIter);
                        pElementIter++;

                        m_pImpl->RemoveElement( pElementToDel );
                    }
                }
            }
        }
    }

    if ( !pElement->m_pStorage )
        m_pImpl->OpenSubStorage( pElement, nStorageMode );

    if ( !pElement->m_pStorage )
        throw io::IOException(); // TODO: general_error

    sal_Bool bReadOnlyWrap = ( ( nStorageMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE );
    OStorage* pResultStorage = new OStorage( pElement->m_pStorage, bReadOnlyWrap );
    uno::Reference< embed::XStorage > xResult( (embed::XStorage*) pResultStorage );

    if ( bReadOnlyWrap )
    {
        // Before this call is done the object must be refcounted already
        pElement->m_pStorage->SetReadOnlyWrap( *pResultStorage );

        // before the storage disposes the stream it must deregister itself as listener
        uno::Reference< lang::XComponent > xStorageComponent( xResult, uno::UNO_QUERY );
        if ( !xStorageComponent.is() )
            throw uno::RuntimeException(); // TODO

        MakeLinkToSubComponent_Impl( xStorageComponent );
    }

    return xResult;
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::cloneStreamElement( const ::rtl::OUString& aStreamName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    return m_pImpl->CloneStreamElement( aStreamName, sal_False, ::rtl::OUString() );
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::cloneEncryptedStreamElement(
    const ::rtl::OUString& aStreamName,
    const ::rtl::OUString& aPass )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aPass.getLength() )
        throw lang::IllegalArgumentException();

    return m_pImpl->CloneStreamElement( aStreamName, sal_True, aPass );
}

//-----------------------------------------------
void SAL_CALL OStorage::copyLastCommitTo(
            const uno::Reference< embed::XStorage >& xTargetStorage )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pImpl->CopyLastCommitTo( xTargetStorage );
}

//-----------------------------------------------
void SAL_CALL OStorage::copyStorageElementLastCommitTo(
            const ::rtl::OUString& aStorName,
            const uno::Reference< embed::XStorage >& xTargetStorage )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    // it's allways possible to read written storage in this implementation
    sal_Int32 nStorageMode = embed::ElementModes::READ;

    SotElement_Impl *pElement = m_pImpl->FindElement( aStorName );
    if ( !pElement )
    {
        // element does not exist, throw exception
        throw io::IOException(); // TODO: access_denied
    }
    else if ( !pElement->m_bIsStorage )
    {
        throw io::IOException(); // TODO:
    }

    if ( !pElement->m_pStorage )
        m_pImpl->OpenSubStorage( pElement, nStorageMode );

    uno::Reference< embed::XStorage > xResult;
    if ( pElement->m_pStorage )
    {
        // the existence of m_pAntiImpl of the child is not interesting,
        // the copy will be created internally

        pElement->m_pStorage->CopyLastCommitTo( xTargetStorage );
    }
    else
        throw io::IOException(); // TODO: general_error
}

//-----------------------------------------------
sal_Bool SAL_CALL OStorage::isStreamElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aElementName.getLength() )
        throw lang::IllegalArgumentException();

    SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );

    if ( !pElement )
        throw container::NoSuchElementException(); //???

    return !pElement->m_bIsStorage;
}

//-----------------------------------------------
sal_Bool SAL_CALL OStorage::isStorageElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aElementName.getLength() )
        throw lang::IllegalArgumentException();

    SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );

    if ( !pElement )
        throw container::NoSuchElementException(); //???

    return pElement->m_bIsStorage;
}

//-----------------------------------------------
void SAL_CALL OStorage::removeElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aElementName.getLength() )
        throw lang::IllegalArgumentException();

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException(); // TODO: access denied

    SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );

    if ( !pElement )
        throw container::NoSuchElementException(); //???

    m_pImpl->RemoveElement( pElement );
}

//-----------------------------------------------
void SAL_CALL OStorage::renameElement( const ::rtl::OUString& aElementName, const ::rtl::OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aElementName.getLength() || !aNewName.getLength() )
        throw lang::IllegalArgumentException();

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException(); // TODO: access denied

    SotElement_Impl* pRefElement = m_pImpl->FindElement( aNewName );
    if ( pRefElement )
        throw container::ElementExistException(); //???

    SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
    if ( !pElement )
        throw container::NoSuchElementException(); //???

    pElement->m_aName = aNewName;
}

//-----------------------------------------------
void SAL_CALL OStorage::copyElementTo(  const ::rtl::OUString& aElementName,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aElementName.getLength() || !aNewName.getLength()
      || !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException();

    SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
    if ( !pElement )
        throw container::NoSuchElementException(); //TODO

    uno::Reference< XNameAccess > xNameAccess( xDest, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); // TODO

    if ( xNameAccess->hasByName( aNewName ) )
        throw container::ElementExistException();

    m_pImpl->CopyStorageElement( pElement, xDest, aNewName );
}


//-----------------------------------------------
void SAL_CALL OStorage::moveElementTo(  const ::rtl::OUString& aElementName,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aElementName.getLength() || !aNewName.getLength()
      || !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException();

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException(); // TODO: access denied

    SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
    if ( !pElement )
        throw container::NoSuchElementException(); //???

    uno::Reference< XNameAccess > xNameAccess( xDest, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); // TODO

    if ( xNameAccess->hasByName( aNewName ) )
        throw container::ElementExistException();

    m_pImpl->CopyStorageElement( pElement, xDest, aNewName );

    m_pImpl->RemoveElement( pElement );
}

void SAL_CALL OStorage::insertRawEncrStreamElement( const ::rtl::OUString& aStreamName,
                                const uno::Reference< io::XInputStream >& xInStream )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoRawFormatException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aStreamName.getLength() || !xInStream.is() )
        throw lang::IllegalArgumentException(); // TODO

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException(); // TODO: access denied

    SotElement_Impl* pElement = m_pImpl->FindElement( aStreamName );
    if ( pElement )
        throw container::ElementExistException(); //TODO

    m_pImpl->InsertRawStream( aStreamName, xInStream );
}

//____________________________________________________________________________________________________
//  XTransactedObject
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL OStorage::commit()
        throw ( io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_pData->m_bReadOnlyWrap )
        throw io::IOException(); // TODO: access_denied

    if ( !m_pImpl->m_bListCreated )
        return; // nothing was changed

    try {
        BroadcastTransaction( STOR_MESS_PRECOMMIT );
        m_pImpl->Commit(); // the root storage initiates the storing to source
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& e )
    {
        throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Problems on commit!" ),
                                  uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                                  uno::makeAny( e ) );
    }

    BroadcastTransaction( STOR_MESS_COMMITED );
}

//-----------------------------------------------
void SAL_CALL OStorage::revert()
        throw ( io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    // the method removes all the changes done after last commit

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    for ( SotElementList_Impl::iterator pElementIter = m_pImpl->m_aChildrenList.begin();
          pElementIter != m_pImpl->m_aChildrenList.end(); pElementIter++ )
        if ( (*pElementIter)->m_pStorage
                && ( (*pElementIter)->m_pStorage->m_pAntiImpl || !(*pElementIter)->m_pStorage->m_aReadOnlyWrapList.empty() )
          || (*pElementIter)->m_pStream
                  && ( (*pElementIter)->m_pStream->m_pAntiImpl || !(*pElementIter)->m_pStream->m_aInputStreamsList.empty() ) )
            throw io::IOException(); // TODO: access denied

    if ( m_pData->m_bReadOnlyWrap || !m_pImpl->m_bListCreated )
        return; // nothing to do

    try {
        BroadcastTransaction( STOR_MESS_PREREVERT );
        m_pImpl->Revert();
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& e )
    {
        throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Problems on revert!" ),
                                  uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                                  uno::makeAny( e ) );
    }

    BroadcastTransaction( STOR_MESS_REVERTED );
}

//____________________________________________________________________________________________________
//  XTransactionBroadcaster
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL OStorage::addTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.addInterface( ::getCppuType((const uno::Reference< embed::XTransactionListener >*)0),
                                                aListener );
}

//-----------------------------------------------
void SAL_CALL OStorage::removeTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.removeInterface( ::getCppuType((const uno::Reference< embed::XTransactionListener >*)0),
                                                    aListener );
}

//____________________________________________________________________________________________________
//  XModifiable
//  TODO: if there will be no demand on this interface it will be removed from implementation,
//        I do not want to remove it now since it is still possible that it will be inserted
//        to the service back.
//____________________________________________________________________________________________________

//-----------------------------------------------
sal_Bool SAL_CALL OStorage::isModified()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    return m_pImpl->m_bIsModified;
}


//-----------------------------------------------
void SAL_CALL OStorage::setModified( sal_Bool bModified )
        throw ( beans::PropertyVetoException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_pData->m_bReadOnlyWrap )
        throw beans::PropertyVetoException(); // TODO: access denied

    if ( m_pImpl->m_bIsModified != bModified )
    {
        m_pImpl->m_bIsModified = bModified;
        BroadcastModified();
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::addModifyListener(
            const uno::Reference< util::XModifyListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.addInterface(
                                ::getCppuType( ( const uno::Reference< util::XModifyListener >* )0 ), aListener );
}


//-----------------------------------------------
void SAL_CALL OStorage::removeModifyListener(
            const uno::Reference< util::XModifyListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.removeInterface(
                                ::getCppuType( ( const uno::Reference< util::XModifyListener >* )0 ), aListener );
}

//____________________________________________________________________________________________________
//  XNameAccess
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Any SAL_CALL OStorage::getByName( const ::rtl::OUString& aName )
        throw ( container::NoSuchElementException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aName.getLength() )
        throw lang::IllegalArgumentException();

    SotElement_Impl* pElement = m_pImpl->FindElement( aName );
    if ( !pElement )
        throw container::NoSuchElementException(); //TODO:

    uno::Any aResult;

    if ( pElement->m_bIsStorage )
    {
        try {
            aResult <<= openStorageElement( aName, embed::ElementModes::READ );
        }
        catch ( uno::Exception& e )
        {
            throw lang::WrappedTargetException( ::rtl::OUString::createFromAscii( "Can not open storage!\n" ),
                                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                    uno::UNO_QUERY ),
                                                uno::makeAny( e ) );
        }
    }
    else
    {
        try {
            aResult <<= openStreamElement( aName, embed::ElementModes::READ );
        }
        catch ( uno::Exception& e )
        {
            throw lang::WrappedTargetException( ::rtl::OUString::createFromAscii( "Can not open storage!\n" ),
                                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                    uno::UNO_QUERY ),
                                                uno::makeAny( e ) );
        }
    }

    return aResult;
}


//-----------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OStorage::getElementNames()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    return m_pImpl->GetElementNames();
}


//-----------------------------------------------
sal_Bool SAL_CALL OStorage::hasByName( const ::rtl::OUString& aName )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !aName.getLength() )
        return sal_False;

    SotElement_Impl* pElement = m_pImpl->FindElement( aName );
    return ( pElement != NULL );
}


//-----------------------------------------------
uno::Type SAL_CALL OStorage::getElementType()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    // it is a multitype container
    return uno::Type();
}


//-----------------------------------------------
sal_Bool SAL_CALL OStorage::hasElements()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    return ( m_pImpl->GetChildrenList().size() != 0 );
}


//____________________________________________________________________________________________________
//  XDisposable
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL OStorage::dispose()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    InternalDispose( sal_True );
}

//-----------------------------------------------
void SAL_CALL OStorage::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.addInterface(
                                ::getCppuType( ( const uno::Reference< lang::XEventListener >* )0 ), xListener );
}

//-----------------------------------------------
void SAL_CALL OStorage::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.removeInterface(
                                ::getCppuType( ( const uno::Reference< lang::XEventListener >* )0 ), xListener );
}

//____________________________________________________________________________________________________
//  XEncryptionProtectedSource
//____________________________________________________________________________________________________

void SAL_CALL OStorage::setEncryptionPassword( const ::rtl::OUString& aPass )
    throw ( uno::RuntimeException,
            io::IOException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    OSL_ENSURE( m_pImpl->m_bIsRoot, "setEncryptionPassword() method is not available for nonroot storages!\n" );

    if ( m_pImpl->m_bIsRoot )
    {
        try {
            m_pImpl->ReadContents();
        }
        catch ( uno::Exception& e )
        {
            throw lang::WrappedTargetException( ::rtl::OUString::createFromAscii( "Can not open package!\n" ),
                                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                    uno::UNO_QUERY ),
                                                uno::makeAny( e ) );
        }

        uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY );
        if ( !xPackPropSet.is() )
            throw uno::RuntimeException(); // TODO

        try
        {
            xPackPropSet->setPropertyValue( ::rtl::OUString::createFromAscii("EncryptionKey"),
                                            uno::makeAny( MakeKeyFromPass( aPass, sal_True ) ) );

            m_pImpl->m_bHasCommonPassword = sal_True;
            m_pImpl->m_aCommonPassword = aPass;
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "The call must not fail, it is pretty simple!" );
            throw io::IOException(); // TODO:
        }
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::removeEncryption()
    throw ( uno::RuntimeException,
            io::IOException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    OSL_ENSURE( m_pImpl->m_bIsRoot, "removeEncryption() method is not available for nonroot storages!\n" );

    if ( m_pImpl->m_bIsRoot )
    {
        try {
            m_pImpl->ReadContents();
        }
        catch ( uno::Exception& e )
        {
            throw lang::WrappedTargetException( ::rtl::OUString::createFromAscii( "Can not open package!\n" ),
                                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                    uno::UNO_QUERY ),
                                                uno::makeAny( e ) );
        }

        // TODO: check if the password is valid
        // update all streams that was encrypted with old password

        uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY );
        if ( !xPackPropSet.is() )
            throw uno::RuntimeException(); // TODO

        try
        {
            xPackPropSet->setPropertyValue( ::rtl::OUString::createFromAscii("EncryptionKey"),
                                            uno::makeAny( uno::Sequence< sal_Int8 >() ) );

            m_pImpl->m_bHasCommonPassword = sal_False;
            m_pImpl->m_aCommonPassword = ::rtl::OUString();
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "The call must not fail, it is pretty simple!" );
            throw io::IOException(); // TODO:
        }
    }
}

//____________________________________________________________________________________________________
//  XPropertySet
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OStorage::getPropertySetInfo()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}


//-----------------------------------------------
void SAL_CALL OStorage::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw ( beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO: think about interaction handler

    if ( m_pData->m_bReadOnlyWrap )
        throw io::IOException(); // TODO: Access denied

    if ( aPropertyName.equalsAscii( "MediaType" ) )
    {
        aValue >>= m_pImpl->m_aMediaType;
        m_pImpl->m_bControlMediaType = sal_True;
    }
    else if ( m_pImpl->m_bIsRoot && ( aPropertyName.equalsAscii( "HasEncryptedEntries" )
                                    || aPropertyName.equalsAscii( "URL" )
                                    || aPropertyName.equalsAscii( "RepairPackage" ) )
           || aPropertyName.equalsAscii( "IsRoot" ) )
        throw beans::PropertyVetoException(); // TODO
    else
        throw beans::UnknownPropertyException(); // TODO
}


//-----------------------------------------------
uno::Any SAL_CALL OStorage::getPropertyValue( const ::rtl::OUString& aPropertyName )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aPropertyName.equalsAscii( "MediaType" ) )
    {
        try
        {
            m_pImpl->ReadContents();
        }
        catch ( uno::Exception& e )
        {
            throw lang::WrappedTargetException(
                                        ::rtl::OUString::createFromAscii( "Can't read contents!" ),
                                        uno::Reference< XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ),
                                        uno::makeAny( e ) ); // TODO:
        }

        return uno::makeAny( m_pImpl->m_aMediaType );
    }
    else if ( aPropertyName.equalsAscii( "IsRoot" ) )
    {
        return uno::makeAny( m_pImpl->m_bIsRoot );
    }
    else if ( aPropertyName.equalsAscii( "OpenMode" ) )
    {
        return uno::makeAny( m_pImpl->m_nStorageMode );
    }
    else if ( m_pImpl->m_bIsRoot )
    {
        if ( aPropertyName.equalsAscii( "URL" )
          || aPropertyName.equalsAscii( "RepairPackage" ) )
        {
            for ( sal_Int32 aInd = 0; aInd < m_pImpl->m_xProperties.getLength(); aInd++ )
            {
                if ( m_pImpl->m_xProperties[aInd].Name.equals( aPropertyName ) )
                    return m_pImpl->m_xProperties[aInd].Value;
            }

            if ( aPropertyName.equalsAscii( "URL" ) )
                return uno::makeAny( ::rtl::OUString() );

            return uno::makeAny( sal_False ); // RepairPackage
        }
        else if ( aPropertyName.equalsAscii( "HasEncriptedEntries" ) )
        {
            try {
                m_pImpl->ReadContents();
            }
            catch ( uno::Exception& e )
            {
                throw lang::WrappedTargetException( ::rtl::OUString::createFromAscii( "Can not open package!\n" ),
                                                    uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                        uno::UNO_QUERY ),
                                                    uno::makeAny( e ) );
            }

            uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY );
            if ( !xPackPropSet.is() )
                throw uno::RuntimeException(); // TODO

            return xPackPropSet->getPropertyValue( aPropertyName );
        }
    }

    throw beans::UnknownPropertyException(); // TODO

    // not reachable
    return uno::Any();
}


//-----------------------------------------------
void SAL_CALL OStorage::addPropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const uno::Reference< beans::XPropertyChangeListener >& xListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OStorage::removePropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const uno::Reference< beans::XPropertyChangeListener >& aListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OStorage::addVetoableChangeListener(
            const ::rtl::OUString& PropertyName,
            const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OStorage::removeVetoableChangeListener(
            const ::rtl::OUString& PropertyName,
            const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


