/*************************************************************************
 *
 *  $RCSfile: prov.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: as $ $Date: 2001-07-30 12:47:28 $
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _FILGLOB_HXX_
#include "filglob.hxx"
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

#include <unotools/configmgr.hxx>

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::ucb;


//=========================================================================
static sal_Bool writeInfo( void * pRegistryKey,
                           const rtl::OUString & rImplementationName,
                              Sequence< rtl::OUString > const & rServiceNames )
{
    rtl::OUString aKeyName( rtl::OUString::createFromAscii( "/" ) );
    aKeyName += rImplementationName;
    aKeyName += rtl::OUString::createFromAscii( "/UNO/SERVICES" );

    Reference< registry::XRegistryKey > xKey;
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



static bool moreLength( const shell::MountPoint& m1, const shell::MountPoint& m2 )
{
    return m1.m_aDirectory.getLength() > m2.m_aDirectory.getLength();
}

FileProvider::FileProvider( const Reference< XMultiServiceFactory >& xMultiServiceFactory )
    : m_xMultiServiceFactory( xMultiServiceFactory ),
      m_pMyShell( 0 )
{

    if( ! m_pMyShell )
        m_pMyShell = new shell( m_xMultiServiceFactory, this );

    try
    {
        rtl::OUString sProviderService =
            rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" );


        // New access to configuration with locally cached components
        rtl::OUString plugin = rtl::OUString::createFromAscii( "plugin" );
        Any aAny;
        aAny <<= plugin;
        PropertyValue aProp( rtl::OUString::createFromAscii( "servertype" ),
                                    -1,
                                    aAny,
                                    PropertyState_DIRECT_VALUE );

        Sequence< Any > seq(1);
        seq[0] <<= aProp;

        Reference< XMultiServiceFactory >
            sProvider(
                m_xMultiServiceFactory->createInstanceWithArguments( sProviderService,seq ),
                UNO_QUERY );


        /*
          // Old access to configuration without locally cached components
          Reference< XMultiServiceFactory >
          sProvider(
          m_xMultiServiceFactory->createInstance( sProviderService ),
          UNO_QUERY );
        */

        rtl::OUString sReaderService =
            rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationAccess" );

        Sequence< Any > aArguments( 1 );

#ifdef TF_CFGDATA
        aArguments[0] <<=
            rtl::OUString::createFromAscii( "org.openoffice.Webtop.Security" );
#elif SUPD > 604
        aArguments[0] <<=
            rtl::OUString::createFromAscii( "org.openoffice.Security" );
#else
        aArguments[0] <<=
            rtl::OUString::createFromAscii( "com.sun.star.Security" );
#endif

        Reference< container::XHierarchicalNameAccess > xHierAccess;
        if( sProvider.is() )
            xHierAccess =
                Reference< container::XHierarchicalNameAccess >(
                    sProvider->createInstanceWithArguments( sReaderService,aArguments ),
                    UNO_QUERY );

        Reference< container::XNameAccess > xSubNode;
        if( xHierAccess.is() )
            xSubNode = Reference< container::XNameAccess >( xHierAccess,UNO_QUERY );

#ifdef TF_CFGDATA
        rtl::OUString d = rtl::OUString::createFromAscii( "oowSecDir" );
        rtl::OUString a = rtl::OUString::createFromAscii( "oowSecAlias" );
#else
        rtl::OUString d = rtl::OUString::createFromAscii( "Directory" );
        rtl::OUString a = rtl::OUString::createFromAscii( "AliasName" );
#endif

        rtl::OUString aRootDirectory;
        if( xSubNode.is() )
        {
            Any aAny = xSubNode->getByName( rtl::OUString::createFromAscii("MountPoints" ) );
            Reference< container::XNameAccess > xSubSubNode;
            aAny >>= xSubSubNode;

            Sequence< rtl::OUString > seqNames =
                xSubSubNode->getElementNames();
            for( sal_Int32 k = 0; k < seqNames.getLength(); ++k )
            {
                Any nocheinany = xSubSubNode->getByName( seqNames[k] );
                Reference< container::XNameAccess > xAuaAuaAuaNode;
                nocheinany >>= xAuaAuaAuaNode;

                Any vorletztesany = xAuaAuaAuaNode->getByName( d );
                rtl::OUString aDirectory;
                vorletztesany >>= aDirectory;

                Any letztesany    = xAuaAuaAuaNode->getByName( a );
                rtl::OUString aAliasName;
                letztesany >>= aAliasName;

                rtl::OUString aUnqDir;
                rtl::OUString aUnqAl;

                initSubstVars();

                rtl::OUString aDir = subst( aDirectory );
                // old,assuming URL: osl::FileBase::getNormalizedPathFromFileURL( aDir, aUnqDir );
                // new, assuming system path:
                osl::FileBase::getFileURLFromSystemPath( aDir,aUnqDir );

                rtl::OUString aAlias = subst( aAliasName );
                // old, assuming URL: osl::FileBase::getNormalizedPathFromFileURL( aAlias, aUnqAl );
                // new, assuming system path:
                osl::FileBase::getFileURLFromSystemPath( aAlias,aUnqAl );

                if ( !aUnqDir.getLength() )
                    osl::FileBase::getFileURLFromSystemPath( aDirectory,aUnqDir );
                // m_pMyShell->getUnqFromUrl( aDirectory,aUnqDir );

                if ( !aUnqAl.getLength() )
                    osl::FileBase::getFileURLFromSystemPath( aAliasName,aUnqAl );
                // m_pMyShell->getUnqFromUrl( aAliasName,aUnqAl );

                rtl::OUString aRealUnqDir;
                rtl::OUString aRealUnqAlias;

                getResolvedURL( aUnqDir.pData, &aRealUnqDir.pData );
                aRealUnqAlias = aUnqAl;

                if ( !aRealUnqAlias.getLength() )
                    aRealUnqAlias = aUnqAl;

                if ( aRealUnqDir.getLength() && aRealUnqAlias.getLength() )
                {
                    m_pMyShell->m_vecMountPoint.push_back(
                        shell::MountPoint( aRealUnqAlias, aRealUnqDir ) );
                    m_pMyShell->m_bFaked = true;
                }

            }

            // Cut trailing slashes

            for ( sal_uInt32 j = 0; j < m_pMyShell->m_vecMountPoint.size(); j++ )
            {
                sal_Int32   nLen = m_pMyShell->m_vecMountPoint[j].m_aDirectory.getLength();

                if ( m_pMyShell->m_vecMountPoint[j].m_aDirectory.lastIndexOf( '/' ) == nLen - 1 )
                    m_pMyShell->m_vecMountPoint[j].m_aDirectory = m_pMyShell->m_vecMountPoint[j].m_aDirectory.copy( 0, nLen - 1 );
            }

            // Now sort the mount point entries according to the length

            std::stable_sort( m_pMyShell->m_vecMountPoint.begin(), m_pMyShell->m_vecMountPoint.end(), moreLength );
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
    throw( RuntimeException )
{
  OWeakObject::acquire();
}


void SAL_CALL
FileProvider::release(
    void )
  throw( RuntimeException )
{
  OWeakObject::release();
}


Any SAL_CALL
FileProvider::queryInterface(
    const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( XContentProvider*, this ),
                                     SAL_STATIC_CAST( XContentIdentifierFactory*, this ),
                                     SAL_STATIC_CAST( XServiceInfo*,     this ),
                                     SAL_STATIC_CAST( XTypeProvider*,    this ),
                                     SAL_STATIC_CAST( XFileIdentifierConverter*,this ),
                                     SAL_STATIC_CAST( XPropertySet*, this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


////////////////////////////////////////////////////////////////////////////////
//
// XTypeProvider methods.


XTYPEPROVIDER_IMPL_6( FileProvider,
                         XTypeProvider,
                         XServiceInfo,
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
  return ServiceName == rtl::OUString::createFromAscii( "com.sun.star.ucb.FileContentProvider" );
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

    return Reference< XContent >( pBaseContent );
}



sal_Int32 SAL_CALL
FileProvider::compareContentIds(
                const Reference< XContentIdentifier >& Id1,
                const Reference< XContentIdentifier >& Id2 )
  throw( RuntimeException )
{
    rtl::OUString aUrl1 = Id1->getContentIdentifier();
    rtl::OUString aUrl2 = Id2->getContentIdentifier();

    sal_Int32   iComp = aUrl1.compareTo( aUrl2 );

    if ( 0 != iComp )
    {
        rtl::OUString aPath1, aPath2, aPath;

        m_pMyShell->getUnqFromUrl( aUrl1, aPath );
        m_pMyShell->checkMountPoint( aPath, aPath1 );

        m_pMyShell->getUnqFromUrl( aUrl2, aPath );
        m_pMyShell->checkMountPoint( aPath, aPath2 );

        osl::FileBase::RC   error;
        osl::DirectoryItem  aItem1, aItem2;

        error = osl::DirectoryItem::get( aPath1, aItem1 );
        if ( error == osl::FileBase::E_None )
            error = osl::DirectoryItem::get( aPath2, aItem2 );

        if ( error != osl::FileBase::E_None )
            return iComp;

        osl::FileStatus aStatus1( FileStatusMask_FileURL );
        osl::FileStatus aStatus2( FileStatusMask_FileURL );
        error = aItem1.getFileStatus( aStatus1 );
        if ( error == osl::FileBase::E_None )
            error = aItem2.getFileStatus( aStatus2 );

        if ( error == osl::FileBase::E_None )
            iComp = aStatus1.getFileURL().compareTo( aStatus2.getFileURL() );
    }

    return iComp;
}



Reference< XContentIdentifier > SAL_CALL
FileProvider::createContentIdentifier(
                      const rtl::OUString& ContentId )
  throw( RuntimeException )
{
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
        throw( RuntimeException);

    virtual void SAL_CALL
    release(
        void )
        throw( RuntimeException );


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
    : m_seq( 2 )
{
    m_seq[0] = Property( rtl::OUString::createFromAscii( "HostName" ),
                                -1,
                                getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                                PropertyAttribute::READONLY );

    m_seq[1] = Property( rtl::OUString::createFromAscii( "FileSystemNotation" ),
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
    throw( RuntimeException )
{
    OWeakObject::acquire();
}


void SAL_CALL
XPropertySetInfoImpl2::release(
    void )
    throw( RuntimeException )
{
    OWeakObject::release();
}


Any SAL_CALL
XPropertySetInfoImpl2::queryInterface(
    const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( XPropertySetInfo*,this) );
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

    throw UnknownPropertyException();
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
                                const Any& aValue )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
{
    if( aPropertyName.compareToAscii( "FileSystemNotation" ) == 0 ||
        aPropertyName.compareToAscii( "HostName" ) == 0 )
        return;
    else
        throw UnknownPropertyException();
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
    else if( aPropertyName.compareToAscii( "HostName" ) == 0 )
    {
        Any aAny;
        aAny <<= m_HostName;
        return aAny;
    }
    else
        throw UnknownPropertyException();
}


void SAL_CALL
FileProvider::addPropertyChangeListener(
    const rtl::OUString& aPropertyName,
    const Reference< XPropertyChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException)
{
    return;
}


void SAL_CALL
FileProvider::removePropertyChangeListener(
    const rtl::OUString& aPropertyName,
    const Reference< XPropertyChangeListener >& aListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    return;
}

void SAL_CALL
FileProvider::addVetoableChangeListener(
    const rtl::OUString& PropertyName,
    const Reference< XVetoableChangeListener >& aListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    return;
}


void SAL_CALL
FileProvider::removeVetoableChangeListener(
    const rtl::OUString& PropertyName,
    const Reference< XVetoableChangeListener >& aListener )
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

rtl::OUString SAL_CALL FileProvider::getFileURLFromSystemPath( const rtl::OUString& BaseURL,
                                                               const rtl::OUString& SystemPath )
    throw( RuntimeException )
{
    rtl::OUString aNormalizedPath;
    if ( osl::FileBase::getFileURLFromSystemPath( SystemPath,aNormalizedPath ) != osl::FileBase::E_None )
        return rtl::OUString();

    rtl::OUString aRed;
    sal_Bool success = m_pMyShell->uncheckMountPoint( aNormalizedPath,aRed );
    if( ! success )
        return rtl::OUString();

    rtl::OUString aUrl;

    aUrl = aRed;

    return aUrl;
}

rtl::OUString SAL_CALL FileProvider::getSystemPathFromFileURL( const rtl::OUString& URL )
    throw( RuntimeException )
{
    rtl::OUString aUnq;

    aUnq = URL;

    rtl::OUString aRed;
    sal_Bool success = m_pMyShell->checkMountPoint( aUnq,aRed );
    if( ! success )
        return rtl::OUString();

    rtl::OUString aSystemPath;
    if (osl::FileBase::getSystemPathFromFileURL( aRed,aSystemPath ) != osl::FileBase::E_None )
        return rtl::OUString();

    return aSystemPath;
}

////////////////////////
// private
////////////////////////
void SAL_CALL FileProvider::initSubstVars()
{
    m_sUserPath = rtl::OUString();
    m_sInstPath = rtl::OUString();

    utl::ConfigManager* pCfgMgr = utl::ConfigManager::GetConfigManager();
    if( pCfgMgr != NULL )
    {
        com::sun::star::uno::Any aAny = pCfgMgr->GetDirectConfigProperty( utl::ConfigManager::OFFICEINSTALL );
        aAny >>= m_sInstPath;

        aAny = pCfgMgr->GetDirectConfigProperty( utl::ConfigManager::INSTALLPATH );
        aAny >>= m_sUserPath;
    }
}

rtl::OUString SAL_CALL FileProvider::subst( const rtl::OUString& sValue )
{
    rtl::OUString sReturn = sValue;

    if( sValue.compareToAscii( "$(userpath)" ) == 0 )
        sReturn = m_sUserPath;
    else
    if( sValue.compareToAscii( "$(instpath)" ) == 0 )
        sReturn = m_sInstPath;

    return sReturn;
}
