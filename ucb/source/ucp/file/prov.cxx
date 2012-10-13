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

#include <osl/security.hxx>
#include <osl/file.hxx>
#include <osl/socket.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ucb/FileSystemNotation.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include "filglob.hxx"
#include "filid.hxx"
#include "shell.hxx"
#include "bc.hxx"
#include "prov.hxx"


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::ucb;
using namespace com::sun::star::container;

//=========================================================================
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpfile_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = 0;

    Reference< XMultiServiceFactory > xSMgr(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // File Content Provider.
    //////////////////////////////////////////////////////////////////////

    if ( fileaccess::shell::getImplementationName_static().
            compareToAscii( pImplName ) == 0 )
    {
        xFactory = FileProvider::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        FileProvider                                      */
/*                                                                          */
/*                                                                          */
/****************************************************************************/



FileProvider::FileProvider( const Reference< XMultiServiceFactory >& xMultiServiceFactory )
    : m_xMultiServiceFactory( xMultiServiceFactory ),
      m_pMyShell( 0 )
{
}


FileProvider::~FileProvider()
{
    if( m_pMyShell )
        delete m_pMyShell;
}


//////////////////////////////////////////////////////////////////////////
// XInterface
//////////////////////////////////////////////////////////////////////////

void SAL_CALL
FileProvider::acquire(
    void )
    throw()
{
  OWeakObject::acquire();
}


void SAL_CALL
FileProvider::release(
    void )
  throw()
{
  OWeakObject::release();
}


Any SAL_CALL
FileProvider::queryInterface(
    const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface(
        rType,
        (static_cast< XContentProvider* >(this)),
        (static_cast< XInitialization* >(this)),
        (static_cast< XContentIdentifierFactory* >(this)),
        (static_cast< XServiceInfo* >(this)),
        (static_cast< XTypeProvider* >(this)),
        (static_cast< XFileIdentifierConverter* >(this)),
        (static_cast< XPropertySet* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

///////////////////////////////////////////////////////////////////////////////
// XInitialization

void SAL_CALL FileProvider::init()
{
    if( ! m_pMyShell )
        m_pMyShell = new shell( m_xMultiServiceFactory, this, sal_True );
}


void SAL_CALL
FileProvider::initialize(
    const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
{
    if( ! m_pMyShell ) {
        rtl::OUString config;
        if( aArguments.getLength() > 0 &&
            (aArguments[0] >>= config) &&
            config.compareToAscii("NoConfig") == 0 )
            m_pMyShell = new shell( m_xMultiServiceFactory, this, sal_False );
        else
            m_pMyShell = new shell( m_xMultiServiceFactory, this, sal_True );
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// XTypeProvider methods.


XTYPEPROVIDER_IMPL_7( FileProvider,
                         XTypeProvider,
                         XServiceInfo,
                      XInitialization,
                      XContentIdentifierFactory,
                      XPropertySet,
                        XFileIdentifierConverter,
                           XContentProvider )


////////////////////////////////////////////////////////////////////////////////
// XServiceInfo methods.

rtl::OUString SAL_CALL
FileProvider::getImplementationName()
    throw( RuntimeException )
{
    return fileaccess::shell::getImplementationName_static();
}


sal_Bool SAL_CALL
FileProvider::supportsService(
                  const rtl::OUString& ServiceName )
  throw( RuntimeException )
{
  return ServiceName == rtl::OUString("com.sun.star.ucb.FileContentProvider");
}


Sequence< rtl::OUString > SAL_CALL
FileProvider::getSupportedServiceNames(
                       void )
  throw( RuntimeException )
{
    return fileaccess::shell::getSupportedServiceNames_static();
}



Reference< XSingleServiceFactory > SAL_CALL
FileProvider::createServiceFactory(
                   const Reference< XMultiServiceFactory >& rxServiceMgr )
{
  /**
   * Create a single service factory.<BR>
   * Note: The function pointer ComponentInstantiation points to a function throws Exception.
   *
   * @param rServiceManager     the service manager used by the implementation.
   * @param rImplementationName the implementation name. An empty string is possible.
   * @param ComponentInstantiation the function pointer to create an object.
   * @param rServiceNames           the service supported by the implementation.
   * @return a factory that support the interfaces XServiceProvider, XServiceInfo
   *            XSingleServiceFactory and XComponent.
   *
   * @see createOneInstanceFactory
   */
  /*
   *  Reference< ::com::sun::star::XSingleServiceFactory > createSingleFactory
   *  (
   *  const ::com::sun::star::Reference< ::com::sun::star::XMultiServiceFactory > & rServiceManager,
   *  const ::rtl::OUString & rImplementationName,
   *  ComponentInstantiation pCreateFunction,

   *  const ::com::sun::star::Sequence< ::rtl::OUString > & rServiceNames
   *  );
   */

    return Reference< XSingleServiceFactory > ( cppu::createSingleFactory(
        rxServiceMgr,
        fileaccess::shell::getImplementationName_static(),
        FileProvider::CreateInstance,
        fileaccess::shell::getSupportedServiceNames_static() ) );
}

Reference< XInterface > SAL_CALL
FileProvider::CreateInstance(
    const Reference< XMultiServiceFactory >& xMultiServiceFactory )
{
    XServiceInfo* xP = (XServiceInfo*) new FileProvider( xMultiServiceFactory );
    return Reference< XInterface >::query( xP );
}



////////////////////////////////////////////////////////////////////////////////
// XContent
////////////////////////////////////////////////////////////////////////////////


Reference< XContent > SAL_CALL
FileProvider::queryContent(
    const Reference< XContentIdentifier >& xIdentifier )
    throw( IllegalIdentifierException,
           RuntimeException)
{
    init();
    rtl::OUString aUnc;
    sal_Bool err = m_pMyShell->getUnqFromUrl( xIdentifier->getContentIdentifier(),
                                              aUnc );

    if(  err )
        throw IllegalIdentifierException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );

    return Reference< XContent >( new BaseContent( m_pMyShell,xIdentifier,aUnc ) );
}



sal_Int32 SAL_CALL
FileProvider::compareContentIds(
                const Reference< XContentIdentifier >& Id1,
                const Reference< XContentIdentifier >& Id2 )
  throw( RuntimeException )
{
    init();
    rtl::OUString aUrl1 = Id1->getContentIdentifier();
    rtl::OUString aUrl2 = Id2->getContentIdentifier();

    sal_Int32   iComp = aUrl1.compareTo( aUrl2 );

    if ( 0 != iComp )
    {
        rtl::OUString aPath1, aPath2;

        m_pMyShell->getUnqFromUrl( aUrl1, aPath1 );
        m_pMyShell->getUnqFromUrl( aUrl2, aPath2 );

        osl::FileBase::RC   error;
        osl::DirectoryItem  aItem1, aItem2;

        error = osl::DirectoryItem::get( aPath1, aItem1 );
        if ( error == osl::FileBase::E_None )
            error = osl::DirectoryItem::get( aPath2, aItem2 );

        if ( error != osl::FileBase::E_None )
            return iComp;

        osl::FileStatus aStatus1( osl_FileStatus_Mask_FileURL );
        osl::FileStatus aStatus2( osl_FileStatus_Mask_FileURL );
        error = aItem1.getFileStatus( aStatus1 );
        if ( error == osl::FileBase::E_None )
            error = aItem2.getFileStatus( aStatus2 );

        if ( error == osl::FileBase::E_None )
        {
            iComp = aStatus1.getFileURL().compareTo( aStatus2.getFileURL() );

// Quick hack for Windows to threat all file systems as case insensitive
#ifdef  WNT
            if ( 0 != iComp )
            {
                error = osl::FileBase::getSystemPathFromFileURL( aStatus1.getFileURL(), aPath1 );
                if ( error == osl::FileBase::E_None )
                    error = osl::FileBase::getSystemPathFromFileURL( aStatus2.getFileURL(), aPath2 );

                if ( error == osl::FileBase::E_None )
                    iComp = rtl_ustr_compareIgnoreAsciiCase( aPath1.getStr(), aPath2.getStr() );
            }
#endif
        }
    }

    return iComp;
}



Reference< XContentIdentifier > SAL_CALL
FileProvider::createContentIdentifier(
                      const rtl::OUString& ContentId )
  throw( RuntimeException )
{
    init();
    FileContentIdentifier* p = new FileContentIdentifier( m_pMyShell,ContentId,false );
    return Reference< XContentIdentifier >( p );
}



//XPropertySetInfoImpl

class XPropertySetInfoImpl2
    : public cppu::OWeakObject,
      public XPropertySetInfo
{
public:
    XPropertySetInfoImpl2();
    ~XPropertySetInfoImpl2();

    // XInterface
    virtual Any SAL_CALL
    queryInterface(
        const Type& aType )
        throw( RuntimeException);

    virtual void SAL_CALL
    acquire(
        void )
        throw();

    virtual void SAL_CALL
    release(
        void )
        throw();


    virtual Sequence< Property > SAL_CALL
    getProperties(
        void )
        throw( RuntimeException );

    virtual Property SAL_CALL
    getPropertyByName(
        const rtl::OUString& aName )
        throw( UnknownPropertyException,
               RuntimeException);

    virtual sal_Bool SAL_CALL
    hasPropertyByName( const rtl::OUString& Name )
        throw( RuntimeException );


private:
    Sequence< Property > m_seq;
};


XPropertySetInfoImpl2::XPropertySetInfoImpl2()
    : m_seq( 3 )
{
    m_seq[0] = Property( rtl::OUString("HostName"),
                         -1,
                         getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                         PropertyAttribute::READONLY );

    m_seq[1] = Property( rtl::OUString("HomeDirectory"),
                         -1,
                         getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                         PropertyAttribute::READONLY );

    m_seq[2] = Property( rtl::OUString("FileSystemNotation"),
                         -1,
                         getCppuType( static_cast< sal_Int32* >( 0 ) ),
                         PropertyAttribute::READONLY );
}


XPropertySetInfoImpl2::~XPropertySetInfoImpl2()
{
    // nothing
}


void SAL_CALL
XPropertySetInfoImpl2::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XPropertySetInfoImpl2::release(
    void )
    throw()
{
    OWeakObject::release();
}


Any SAL_CALL
XPropertySetInfoImpl2::queryInterface(
    const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                          (static_cast< XPropertySetInfo* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


Property SAL_CALL
XPropertySetInfoImpl2::getPropertyByName(
    const rtl::OUString& aName )
    throw( UnknownPropertyException,
           RuntimeException)
{
    for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
        if( m_seq[i].Name == aName )
            return m_seq[i];

    throw UnknownPropertyException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
}



Sequence< Property > SAL_CALL
XPropertySetInfoImpl2::getProperties(
    void )
    throw( RuntimeException )
{
    return m_seq;
}


sal_Bool SAL_CALL
XPropertySetInfoImpl2::hasPropertyByName(
    const rtl::OUString& aName )
    throw( RuntimeException )
{
    for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
        if( m_seq[i].Name == aName )
            return true;
    return false;
}





void SAL_CALL FileProvider::initProperties( void )
{
    osl::MutexGuard aGuard( m_aMutex );
    if( ! m_xPropertySetInfo.is() )
    {
        osl_getLocalHostname( &m_HostName.pData );

#if defined ( UNX )
        m_FileSystemNotation = FileSystemNotation::UNIX_NOTATION;
#elif defined( WNT )
        m_FileSystemNotation = FileSystemNotation::DOS_NOTATION;
#else
        m_FileSystemNotation = FileSystemNotation::UNKNOWN_NOTATION;
#endif
        osl::Security aSecurity;
        aSecurity.getHomeDir( m_HomeDirectory );

        // static const sal_Int32 UNKNOWN_NOTATION = (sal_Int32)0;
        // static const sal_Int32 UNIX_NOTATION = (sal_Int32)1;
        // static const sal_Int32 DOS_NOTATION = (sal_Int32)2;
        // static const sal_Int32 MAC_NOTATION = (sal_Int32)3;

        XPropertySetInfoImpl2* p = new XPropertySetInfoImpl2();
        m_xPropertySetInfo = Reference< XPropertySetInfo >( p );
    }
}


// XPropertySet

Reference< XPropertySetInfo > SAL_CALL
FileProvider::getPropertySetInfo(  )
    throw( RuntimeException )
{
    initProperties();
    return m_xPropertySetInfo;
}


void SAL_CALL
FileProvider::setPropertyValue( const rtl::OUString& aPropertyName,
                                const Any& )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
{
    if( aPropertyName.compareToAscii( "FileSystemNotation" ) == 0 ||
        aPropertyName.compareToAscii( "HomeDirectory" ) == 0      ||
        aPropertyName.compareToAscii( "HostName" ) == 0 )
        return;
    else
        throw UnknownPropertyException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
}



Any SAL_CALL
FileProvider::getPropertyValue(
    const rtl::OUString& aPropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    initProperties();
    if( aPropertyName.compareToAscii( "FileSystemNotation" ) == 0 )
    {
        Any aAny;
        aAny <<= m_FileSystemNotation;
        return aAny;
    }
    else if( aPropertyName.compareToAscii( "HomeDirectory" ) == 0 )
    {
        Any aAny;
        aAny <<= m_HomeDirectory;
        return aAny;
    }
    else if( aPropertyName.compareToAscii( "HostName" ) == 0 )
    {
        Any aAny;
        aAny <<= m_HostName;
        return aAny;
    }
    else
        throw UnknownPropertyException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
}


void SAL_CALL
FileProvider::addPropertyChangeListener(
    const rtl::OUString&,
    const Reference< XPropertyChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException)
{
    return;
}


void SAL_CALL
FileProvider::removePropertyChangeListener(
    const rtl::OUString&,
    const Reference< XPropertyChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    return;
}

void SAL_CALL
FileProvider::addVetoableChangeListener(
    const rtl::OUString&,
    const Reference< XVetoableChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    return;
}


void SAL_CALL
FileProvider::removeVetoableChangeListener(
    const rtl::OUString&,
    const Reference< XVetoableChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException)
{
    return;
}



// XFileIdentifierConverter

sal_Int32 SAL_CALL
FileProvider::getFileProviderLocality( const rtl::OUString& BaseURL )
    throw( RuntimeException )
{
    // If the base URL is a 'file' URL, return 10 (very 'local'), otherwise
    // return -1 (missmatch).  What is missing is a fast comparison to ASCII,
    // ignoring case:
    return BaseURL.getLength() >= 5
           && (BaseURL[0] == 'F' || BaseURL[0] == 'f')
           && (BaseURL[1] == 'I' || BaseURL[1] == 'i')
           && (BaseURL[2] == 'L' || BaseURL[2] == 'l')
           && (BaseURL[3] == 'E' || BaseURL[3] == 'e')
           && BaseURL[4] == ':' ?
               10 : -1;
}

rtl::OUString SAL_CALL FileProvider::getFileURLFromSystemPath( const rtl::OUString&,
                                                               const rtl::OUString& SystemPath )
    throw( RuntimeException )
{
    rtl::OUString aNormalizedPath;
    if ( osl::FileBase::getFileURLFromSystemPath( SystemPath,aNormalizedPath ) != osl::FileBase::E_None )
        return rtl::OUString();

    return aNormalizedPath;
}

rtl::OUString SAL_CALL FileProvider::getSystemPathFromFileURL( const rtl::OUString& URL )
    throw( RuntimeException )
{
    rtl::OUString aSystemPath;
    if (osl::FileBase::getSystemPathFromFileURL( URL,aSystemPath ) != osl::FileBase::E_None )
        return rtl::OUString();

    return aSystemPath;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
