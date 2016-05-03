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
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ucb/FileSystemNotation.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
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

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif


extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpfile_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = nullptr;

    Reference< XMultiServiceFactory > xSMgr(
            static_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;


    // File Content Provider.


    if ( fileaccess::shell::getImplementationName_static().
            equalsAscii( pImplName ) )
    {
        xFactory = FileProvider::createServiceFactory( xSMgr );
    }


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
FileProvider::FileProvider( const Reference< XComponentContext >& rxContext )
    : m_xContext(rxContext)
    , m_FileSystemNotation(FileSystemNotation::UNKNOWN_NOTATION)
    , m_pMyShell(nullptr)
{
}

FileProvider::~FileProvider()
{
    delete m_pMyShell;
}

// XInitialization
void SAL_CALL FileProvider::init()
{
    if( ! m_pMyShell )
        m_pMyShell = new shell( m_xContext, this, true );
}


void SAL_CALL
FileProvider::initialize(
    const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException, std::exception)
{
    if( ! m_pMyShell ) {
        OUString config;
        if( aArguments.getLength() > 0 &&
            (aArguments[0] >>= config) &&
            config == "NoConfig" )
            m_pMyShell = new shell( m_xContext, this, false );
        else
            m_pMyShell = new shell( m_xContext, this, true );
    }
}

// XServiceInfo methods.
OUString SAL_CALL
FileProvider::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return fileaccess::shell::getImplementationName_static();
}

sal_Bool SAL_CALL FileProvider::supportsService(const OUString& ServiceName )
  throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL
FileProvider::getSupportedServiceNames(
                       void )
  throw( RuntimeException, std::exception )
{
    return fileaccess::shell::getSupportedServiceNames_static();
}

Reference< XSingleServiceFactory > SAL_CALL
FileProvider::createServiceFactory(
                   const Reference< XMultiServiceFactory >& rxServiceMgr )
{
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
    XServiceInfo* xP = static_cast<XServiceInfo*>(new FileProvider( comphelper::getComponentContext(xMultiServiceFactory) ));
    return Reference< XInterface >::query( xP );
}


// XContent


Reference< XContent > SAL_CALL
FileProvider::queryContent(
    const Reference< XContentIdentifier >& xIdentifier )
    throw( IllegalIdentifierException,
           RuntimeException, std::exception)
{
    init();
    OUString aUnc;
    bool err = fileaccess::shell::getUnqFromUrl( xIdentifier->getContentIdentifier(),
                                              aUnc );

    if(  err )
        throw IllegalIdentifierException( THROW_WHERE );

    return Reference< XContent >( new BaseContent( m_pMyShell,xIdentifier,aUnc ) );
}


sal_Int32 SAL_CALL
FileProvider::compareContentIds(
                const Reference< XContentIdentifier >& Id1,
                const Reference< XContentIdentifier >& Id2 )
  throw( RuntimeException, std::exception )
{
    init();
    OUString aUrl1 = Id1->getContentIdentifier();
    OUString aUrl2 = Id2->getContentIdentifier();

    sal_Int32   iComp = aUrl1.compareTo( aUrl2 );

    if ( 0 != iComp )
    {
        OUString aPath1, aPath2;

        fileaccess::shell::getUnqFromUrl( aUrl1, aPath1 );
        fileaccess::shell::getUnqFromUrl( aUrl2, aPath2 );

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
                    iComp = aPath1.compareToIgnoreAsciiCase( aPath2 );
            }
#endif
        }
    }

    return iComp;
}


Reference< XContentIdentifier > SAL_CALL
FileProvider::createContentIdentifier(
                      const OUString& ContentId )
  throw( RuntimeException, std::exception )
{
    init();
    FileContentIdentifier* p = new FileContentIdentifier( ContentId,false );
    return Reference< XContentIdentifier >( p );
}


//XPropertySetInfoImpl

class XPropertySetInfoImpl2
    : public cppu::OWeakObject,
      public XPropertySetInfo
{
public:
    XPropertySetInfoImpl2();
    virtual ~XPropertySetInfoImpl2();

    // XInterface
    virtual Any SAL_CALL
    queryInterface(
        const Type& aType )
        throw( RuntimeException, std::exception) override;

    virtual void SAL_CALL
    acquire(
        void )
        throw() override;

    virtual void SAL_CALL
    release(
        void )
        throw() override;


    virtual Sequence< Property > SAL_CALL
    getProperties(
        void )
        throw( RuntimeException, std::exception ) override;

    virtual Property SAL_CALL
    getPropertyByName(
        const OUString& aName )
        throw( UnknownPropertyException,
               RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
    hasPropertyByName( const OUString& Name )
        throw( RuntimeException, std::exception ) override;


private:
    Sequence< Property > m_seq;
};


XPropertySetInfoImpl2::XPropertySetInfoImpl2()
    : m_seq( 3 )
{
    m_seq[0] = Property( OUString("HostName"),
                         -1,
                         cppu::UnoType<OUString>::get(),
                         PropertyAttribute::READONLY );

    m_seq[1] = Property( OUString("HomeDirectory"),
                         -1,
                         cppu::UnoType<OUString>::get(),
                         PropertyAttribute::READONLY );

    m_seq[2] = Property( OUString("FileSystemNotation"),
                         -1,
                         cppu::UnoType<sal_Int32>::get(),
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
    throw( RuntimeException, std::exception )
{
    Any aRet = cppu::queryInterface( rType,
                                          (static_cast< XPropertySetInfo* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


Property SAL_CALL
XPropertySetInfoImpl2::getPropertyByName(
    const OUString& aName )
    throw( UnknownPropertyException,
           RuntimeException, std::exception)
{
    for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
        if( m_seq[i].Name == aName )
            return m_seq[i];

    throw UnknownPropertyException( THROW_WHERE );
}


Sequence< Property > SAL_CALL
XPropertySetInfoImpl2::getProperties(
    void )
    throw( RuntimeException, std::exception )
{
    return m_seq;
}


sal_Bool SAL_CALL
XPropertySetInfoImpl2::hasPropertyByName(
    const OUString& aName )
    throw( RuntimeException, std::exception )
{
    for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
        if( m_seq[i].Name == aName )
            return true;
    return false;
}


void SAL_CALL FileProvider::initProperties()
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
        m_xPropertySetInfo.set( p );
    }
}


// XPropertySet

Reference< XPropertySetInfo > SAL_CALL
FileProvider::getPropertySetInfo(  )
    throw( RuntimeException, std::exception )
{
    initProperties();
    return m_xPropertySetInfo;
}


void SAL_CALL
FileProvider::setPropertyValue( const OUString& aPropertyName,
                                const Any& )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    if( aPropertyName == "FileSystemNotation" ||
        aPropertyName == "HomeDirectory"      ||
        aPropertyName == "HostName" )
        return;
    else
        throw UnknownPropertyException( THROW_WHERE );
}


Any SAL_CALL
FileProvider::getPropertyValue(
    const OUString& aPropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    initProperties();
    if( aPropertyName == "FileSystemNotation" )
    {
        return Any(m_FileSystemNotation);
    }
    else if( aPropertyName == "HomeDirectory" )
    {
        return Any(m_HomeDirectory);
    }
    else if( aPropertyName == "HostName" )
    {
        return Any(m_HostName);
    }
    else
        throw UnknownPropertyException( THROW_WHERE );
}


void SAL_CALL
FileProvider::addPropertyChangeListener(
    const OUString&,
    const Reference< XPropertyChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception)
{
    return;
}


void SAL_CALL
FileProvider::removePropertyChangeListener(
    const OUString&,
    const Reference< XPropertyChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    return;
}

void SAL_CALL
FileProvider::addVetoableChangeListener(
    const OUString&,
    const Reference< XVetoableChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    return;
}


void SAL_CALL
FileProvider::removeVetoableChangeListener(
    const OUString&,
    const Reference< XVetoableChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException, std::exception)
{
    return;
}


// XFileIdentifierConverter

sal_Int32 SAL_CALL
FileProvider::getFileProviderLocality( const OUString& BaseURL )
    throw( RuntimeException, std::exception )
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

OUString SAL_CALL FileProvider::getFileURLFromSystemPath( const OUString&,
                                                               const OUString& SystemPath )
    throw( RuntimeException, std::exception )
{
    OUString aNormalizedPath;
    if ( osl::FileBase::getFileURLFromSystemPath( SystemPath,aNormalizedPath ) != osl::FileBase::E_None )
        return OUString();

    return aNormalizedPath;
}

OUString SAL_CALL FileProvider::getSystemPathFromFileURL( const OUString& URL )
    throw( RuntimeException, std::exception )
{
    OUString aSystemPath;
    if (osl::FileBase::getSystemPathFromFileURL( URL,aSystemPath ) != osl::FileBase::E_None )
        return OUString();

    return aSystemPath;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
