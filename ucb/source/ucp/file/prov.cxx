/*************************************************************************
 *
 *  $RCSfile: prov.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: abi $ $Date: 2000-10-17 12:39:23 $
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
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_SOCKET_H_
#include <osl/socket.h>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_FILESYSTEMNOTATION_HPP_
#include <com/sun/star/ucb/FileSystemNotation.hpp>
#endif
#ifndef _FILID_HXX_
#include "filid.hxx"
#endif
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif
#ifndef _BC_HXX_
#include "bc.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;

//=========================================================================
static sal_Bool writeInfo( void * pRegistryKey,
                           const rtl::OUString & rImplementationName,
                              uno::Sequence< rtl::OUString > const & rServiceNames )
{
    rtl::OUString aKeyName( rtl::OUString::createFromAscii( "/" ) );
    aKeyName += rImplementationName;
    aKeyName += rtl::OUString::createFromAscii( "/UNO/SERVICES" );

    uno::Reference< registry::XRegistryKey > xKey;
    try
    {
        xKey = static_cast< registry::XRegistryKey * >(
                                    pRegistryKey )->createKey( aKeyName );
    }
    catch ( registry::InvalidRegistryException const & )
    {
    }

    if ( !xKey.is() )
        return sal_False;

    sal_Bool bSuccess = sal_True;

    for ( sal_Int32 n = 0; n < rServiceNames.getLength(); ++n )
    {
        try
        {
            xKey->createKey( rServiceNames[ n ] );
        }
        catch ( registry::InvalidRegistryException const & )
        {
            bSuccess = sal_False;
            break;
        }
    }
    return bSuccess;
}

//=========================================================================
extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//=========================================================================
extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return pRegistryKey &&

    //////////////////////////////////////////////////////////////////////
    // File Content Provider.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               fileaccess::shell::getImplementationName_static(),
               fileaccess::shell::getSupportedServiceNames_static() );
}

//=========================================================================
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    uno::Reference< lang::XMultiServiceFactory > xSMgr(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

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


#ifdef UNX
extern "C" oslFileError osl_getRealPath(rtl_uString* strPath, rtl_uString** strRealPath);
#endif

FileProvider::FileProvider( const uno::Reference< lang::XMultiServiceFactory >& xMultiServiceFactory )
    : m_xMultiServiceFactory( xMultiServiceFactory ),
      m_pMyShell( 0 )
{

    if( ! m_pMyShell )
        m_pMyShell = new shell( m_xMultiServiceFactory, this );

    try
    {
        rtl::OUString sProviderService =
            rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" );

        uno::Reference< lang::XMultiServiceFactory >
            sProvider(
                m_xMultiServiceFactory->createInstance( sProviderService ),
                uno::UNO_QUERY );

        rtl::OUString sReaderService =
            rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationAccess" );

        uno::Sequence< uno::Any > aArguments( 1 );
#if SUPD > 604
        aArguments[0] <<=
            rtl::OUString::createFromAscii( "org.openoffice.Security" );
#else
        aArguments[0] <<=
            rtl::OUString::createFromAscii( "com.sun.star.Security" );
#endif
        uno::Reference< container::XHierarchicalNameAccess > xHierAccess(
            sProvider->createInstanceWithArguments( sReaderService,aArguments ),
            uno::UNO_QUERY );

        uno::Reference< container::XNameAccess > xSubNode( xHierAccess,uno::UNO_QUERY );

        rtl::OUString d = rtl::OUString::createFromAscii( "Directory" );
        rtl::OUString a = rtl::OUString::createFromAscii( "AliasName" );

        rtl::OUString aRootDirectory;
        if( xSubNode.is() )
        {
            uno::Reference< frame::XConfigManager > xCfgMgr(
                m_xMultiServiceFactory->createInstance(
                    rtl::OUString::createFromAscii(
                               "com.sun.star.config.SpecialConfigManager" ) ),
                uno::UNO_QUERY );

            uno::Any aAny = xSubNode->getByName( rtl::OUString::createFromAscii("MountPoints" ) );
            uno::Reference< container::XNameAccess > xSubSubNode;
            aAny >>= xSubSubNode;

            uno::Sequence< rtl::OUString > seqNames =
                xSubSubNode->getElementNames();
            for( sal_Int32 k = 0; k < seqNames.getLength(); ++k )
            {
                uno::Any nocheinany = xSubSubNode->getByName( seqNames[k] );
                uno::Reference< container::XNameAccess > xAuaAuaAuaNode;
                nocheinany >>= xAuaAuaAuaNode;

                uno::Any vorletztesany = xAuaAuaAuaNode->getByName( d );
                rtl::OUString aDirectory;
                vorletztesany >>= aDirectory;

                uno::Any letztesany    = xAuaAuaAuaNode->getByName( a );
                rtl::OUString aAliasName;
                letztesany >>= aAliasName;

                VOS_ENSURE( xCfgMgr.is(),
                        "FileProvider::FileProvider - No Config Manager!" );

                rtl::OUString aUnqDir;
                rtl::OUString aUnqAl;

                if ( xCfgMgr.is() )
                {
                    // Substitute path variables, like "$(user)".

                    rtl::OUString aDir
                        = xCfgMgr->substituteVariables( aDirectory );
                    osl::FileBase::getNormalizedPathFromFileURL( aDir, aUnqDir );

                    rtl::OUString aAlias
                        = xCfgMgr->substituteVariables( aAliasName );
                    osl::FileBase::getNormalizedPathFromFileURL( aAlias, aUnqAl );
                }

                if ( !aUnqDir.getLength() )
                    m_pMyShell->getUnqFromUrl( aDirectory,aUnqDir );

                if ( !aUnqAl.getLength() )
                    m_pMyShell->getUnqFromUrl( aAliasName,aUnqAl );

                if ( aUnqDir.getLength() && aUnqAl.getLength() )
                {
                    m_pMyShell->m_vecMountPoint.push_back(
                        shell::MountPoint( aUnqAl, aUnqDir ) );
                    m_pMyShell->m_bFaked = true;
                }

#ifdef UNX
                rtl::OUString aRealUnqDir;
                rtl::OUString aRealUnqAlias;

                osl_getRealPath( aUnqDir.pData, &aRealUnqDir.pData );
                osl_getRealPath( aUnqAl.pData, &aRealUnqAlias.pData );

                if ( !aRealUnqAlias.getLength() )
                    aRealUnqAlias = aUnqAl;

                if ( aRealUnqDir.getLength() && aRealUnqAlias.getLength() )
                {
                    m_pMyShell->m_vecMountPoint.push_back(
                        shell::MountPoint( aRealUnqAlias, aRealUnqDir ) );
                    m_pMyShell->m_bFaked = true;
                }
#endif

            }
        }
    }
    catch( ... )
    {

    }
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
    throw( uno::RuntimeException )
{
  OWeakObject::acquire();
}


void SAL_CALL
FileProvider::release(
              void )
  throw( uno::RuntimeException )
{
  OWeakObject::release();
}


uno::Any SAL_CALL
FileProvider::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( XContentProvider*, this ),
                                          SAL_STATIC_CAST( XContentIdentifierFactory*, this ),
                                          SAL_STATIC_CAST( lang::XServiceInfo*,     this ),
                                          SAL_STATIC_CAST( beans::XPropertySet*, this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



////////////////////////////////////////////////////////////////////////////////
// XServiceInfo methods.

rtl::OUString SAL_CALL
FileProvider::getImplementationName()
    throw( uno::RuntimeException )
{
    return fileaccess::shell::getImplementationName_static();
}


sal_Bool SAL_CALL
FileProvider::supportsService(
                  const rtl::OUString& ServiceName )
  throw( uno::RuntimeException )
{
  return ServiceName == rtl::OUString::createFromAscii( "com.sun.star.ucb.FileProvider" );
}


uno::Sequence< rtl::OUString > SAL_CALL
FileProvider::getSupportedServiceNames(
                       void )
  throw( uno::RuntimeException )
{
    return fileaccess::shell::getSupportedServiceNames_static();
}



uno::Reference< lang::XSingleServiceFactory > SAL_CALL
FileProvider::createServiceFactory(
                   const uno::Reference< lang::XMultiServiceFactory >& rxServiceMgr )
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
   *  Reference< ::com::sun::star::lang::XSingleServiceFactory > createSingleFactory
   *  (
   *  const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rServiceManager,
   *  const ::rtl::OUString & rImplementationName,
   *  ComponentInstantiation pCreateFunction,

   *  const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rServiceNames
   *  );
   */

    return uno::Reference< lang::XSingleServiceFactory > ( cppu::createSingleFactory(
        rxServiceMgr,
        fileaccess::shell::getImplementationName_static(),
        FileProvider::CreateInstance,
        fileaccess::shell::getSupportedServiceNames_static() ) );
}

#if SUPD > 583
uno::Reference< uno::XInterface > SAL_CALL
#else
uno::Reference< uno::XInterface >
#endif
FileProvider::CreateInstance(
    const uno::Reference< lang::XMultiServiceFactory >& xMultiServiceFactory )
{
    lang::XServiceInfo* xP = (lang::XServiceInfo*) new FileProvider( xMultiServiceFactory );
    return uno::Reference< uno::XInterface >::query( xP );
}



////////////////////////////////////////////////////////////////////////////////
// XContent
////////////////////////////////////////////////////////////////////////////////


uno::Reference< XContent > SAL_CALL
FileProvider::queryContent(
    const uno::Reference< XContentIdentifier >& xIdentifier )
    throw( IllegalIdentifierException,
           uno::RuntimeException)
{

    rtl::OUString aUnc;
    sal_Bool err = m_pMyShell->getUnqFromUrl( xIdentifier->getContentIdentifier(),
                                              aUnc );

    if(  err )
        throw IllegalIdentifierException();


    rtl::OUString aRedirectedPath;
    sal_Bool mounted = m_pMyShell->checkMountPoint( aUnc,aRedirectedPath );

    BaseContent* pBaseContent = 0;
    if( mounted )
        pBaseContent = new BaseContent( m_pMyShell,xIdentifier,aRedirectedPath );

    return uno::Reference< XContent >( pBaseContent );
}



sal_Int32 SAL_CALL
FileProvider::compareContentIds(
                const uno::Reference< XContentIdentifier >& Id1,
                const uno::Reference< XContentIdentifier >& Id2 )
  throw( uno::RuntimeException )
{
    rtl::OUString aUrl1 = Id1->getContentIdentifier();
    rtl::OUString aUrl2 = Id2->getContentIdentifier();

    return aUrl1.compareTo( aUrl2 );
}



uno::Reference< XContentIdentifier > SAL_CALL
FileProvider::createContentIdentifier(
                      const rtl::OUString& ContentId )
  throw( uno::RuntimeException )
{
    FileContentIdentifier* p = new FileContentIdentifier( m_pMyShell,ContentId,false );
    return uno::Reference< XContentIdentifier >( p );
}



//XPropertySetInfoImpl

class XPropertySetInfoImpl2
    : public cppu::OWeakObject,
      public beans::XPropertySetInfo
{
public:
    XPropertySetInfoImpl2();
    ~XPropertySetInfoImpl2();

    // XInterface
    virtual uno::Any SAL_CALL
    queryInterface(
        const uno::Type& aType )
        throw( uno::RuntimeException);

    virtual void SAL_CALL
    acquire(
        void )
        throw( uno::RuntimeException);

    virtual void SAL_CALL
    release(
        void )
        throw( uno::RuntimeException );


    virtual uno::Sequence< beans::Property > SAL_CALL
    getProperties(
        void )
        throw( uno::RuntimeException );

    virtual beans::Property SAL_CALL
    getPropertyByName(
        const rtl::OUString& aName )
        throw( beans::UnknownPropertyException,
               uno::RuntimeException);

    virtual sal_Bool SAL_CALL
    hasPropertyByName( const rtl::OUString& Name )
        throw( uno::RuntimeException );


private:
    uno::Sequence< beans::Property > m_seq;
};


XPropertySetInfoImpl2::XPropertySetInfoImpl2()
    : m_seq( 2 )
{
    m_seq[0] = beans::Property( rtl::OUString::createFromAscii( "HostName" ),
                                -1,
                                getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                                beans::PropertyAttribute::READONLY );

    m_seq[1] = beans::Property( rtl::OUString::createFromAscii( "FileSystemNotation" ),
                                -1,
                                getCppuType( static_cast< sal_Int32* >( 0 ) ),
                                beans::PropertyAttribute::READONLY );
}


XPropertySetInfoImpl2::~XPropertySetInfoImpl2()
{
    // nothing
}


void SAL_CALL
XPropertySetInfoImpl2::acquire(
    void )
    throw( uno::RuntimeException )
{
    OWeakObject::acquire();
}


void SAL_CALL
XPropertySetInfoImpl2::release(
    void )
    throw( uno::RuntimeException )
{
    OWeakObject::release();
}


uno::Any SAL_CALL
XPropertySetInfoImpl2::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( beans::XPropertySetInfo*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


beans::Property SAL_CALL
XPropertySetInfoImpl2::getPropertyByName(
    const rtl::OUString& aName )
    throw( beans::UnknownPropertyException,
           uno::RuntimeException)
{
    for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
        if( m_seq[i].Name == aName )
            return m_seq[i];

    throw beans::UnknownPropertyException();
}



uno::Sequence< beans::Property > SAL_CALL
XPropertySetInfoImpl2::getProperties(
    void )
    throw( uno::RuntimeException )
{
    return m_seq;
}


sal_Bool SAL_CALL
XPropertySetInfoImpl2::hasPropertyByName(
    const rtl::OUString& aName )
    throw( uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
        if( m_seq[i].Name == aName )
            return true;
    return false;
}





void SAL_CALL FileProvider::initProperties( void )
{
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

        // static const sal_Int32 UNKNOWN_NOTATION = (sal_Int32)0;
        // static const sal_Int32 UNIX_NOTATION = (sal_Int32)1;
        // static const sal_Int32 DOS_NOTATION = (sal_Int32)2;
        // static const sal_Int32 MAC_NOTATION = (sal_Int32)3;

        XPropertySetInfoImpl2* p = new XPropertySetInfoImpl2();
        m_xPropertySetInfo = uno::Reference< beans::XPropertySetInfo >( p );
    }
}


// XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL
FileProvider::getPropertySetInfo(  )
    throw( uno::RuntimeException )
{
    initProperties();
    return m_xPropertySetInfo;
}


void SAL_CALL
FileProvider::setPropertyValue( const rtl::OUString& aPropertyName,
                                const uno::Any& aValue )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    if( aPropertyName.compareToAscii( "FileSystemNotation" ) == 0 ||
        aPropertyName.compareToAscii( "HostName" ) == 0 )
        return;
    else
        throw beans::UnknownPropertyException();
}



uno::Any SAL_CALL
FileProvider::getPropertyValue(
    const rtl::OUString& aPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    initProperties();
    if( aPropertyName.compareToAscii( "FileSystemNotation" ) == 0 )
    {
        uno::Any aAny;
        aAny <<= m_FileSystemNotation;
        return aAny;
    }
    else if( aPropertyName.compareToAscii( "HostName" ) == 0 )
    {
        uno::Any aAny;
        aAny <<= m_HostName;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException();
}


void SAL_CALL
FileProvider::addPropertyChangeListener(
    const rtl::OUString& aPropertyName,
    const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    return;
}


void SAL_CALL
FileProvider::removePropertyChangeListener(
    const rtl::OUString& aPropertyName,
    const uno::Reference< beans::XPropertyChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    return;
}

void SAL_CALL
FileProvider::addVetoableChangeListener(
    const rtl::OUString& PropertyName,
    const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    return;
}


void SAL_CALL
FileProvider::removeVetoableChangeListener(
    const rtl::OUString& PropertyName,
    const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    return;
}



// XFileIdentifierConverter

rtl::OUString SAL_CALL FileProvider::getHostName()
    throw( uno::RuntimeException )
{
    initProperties();
    return m_HostName;
}

rtl::OUString SAL_CALL FileProvider::getFileURLFromNormalizedPath( const rtl::OUString& NormalizedPath )
    throw( IllegalIdentifierException,
           uno::RuntimeException )
{
    rtl::OUString aUrl;
    sal_Bool err = m_pMyShell->getUrlFromUnq( NormalizedPath,aUrl );
    if( err )
        throw IllegalIdentifierException();

    return aUrl;
}

rtl::OUString SAL_CALL FileProvider::getNormalizedPathFromFileURL( const rtl::OUString& FileURL )
    throw( IllegalIdentifierException,
           uno::RuntimeException )
{
    rtl::OUString aUnq;
    sal_Bool err = m_pMyShell->getUnqFromUrl( FileURL,aUnq );
    if( err )
        throw IllegalIdentifierException();

    return aUnq;
}
