/*************************************************************************
 *
 *  $RCSfile: provider.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: abi $ $Date: 2001-07-06 13:32:50 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
#endif
#ifndef _PROVIDER_HXX
#include <provider/provider.hxx>
#endif
#ifndef _CONTENT_HXX
#include <provider/content.hxx>
#endif
#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

using namespace chelp;

//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

ContentProvider::ContentProvider( const Reference< XMultiServiceFactory >& rSMgr )
  : ::ucb::ContentProviderImplHelper( rSMgr ),
                   isInitialized( false ),
                   m_aScheme( OUString::createFromAscii( MYUCP_URL_SCHEME ) ),
                   m_pDatabases( 0 )
{
}

//=========================================================================
// virtual
ContentProvider::~ContentProvider()
{
    delete m_pDatabases;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( ContentProvider,
                   XTypeProvider,
                   XServiceInfo,
                   XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ContentProvider,
                         XTypeProvider,
                         XServiceInfo,
                         XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( ContentProvider,
                     OUString::createFromAscii(
                             "CHelpContentProvider" ),
                     OUString::createFromAscii(
                             MYUCP_CONTENT_PROVIDER_SERVICE_NAME ) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
Reference< XContent > SAL_CALL ContentProvider::queryContent( const Reference< XContentIdentifier >& xCanonicId )
  throw( IllegalIdentifierException, RuntimeException )
{
  if ( ! xCanonicId->getContentProviderScheme().equalsIgnoreAsciiCase( m_aScheme ) )
    {   // Wrong URL-scheme
      throw IllegalIdentifierException();
    }

  {
    osl::MutexGuard aGuard( m_aMutex );
    if( ! isInitialized )
      init();
  }

  if( ! m_pDatabases )
    throw RuntimeException();


  // Check, if a content with given id already exists...
  Reference< XContent > xContent
    = queryExistingContent( xCanonicId ).getBodyPtr();
  if ( xContent.is() )
    return xContent;

  xContent = new Content( m_xSMgr,this,xCanonicId,m_pDatabases );

  // Further checks

  if ( !xContent->getIdentifier().is() )
    throw IllegalIdentifierException();

  return xContent;
}



void ContentProvider::init()
{
  isInitialized = true;

  rtl::OUString sProviderService =
    rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" );


  Any aAny;
  aAny <<= rtl::OUString::createFromAscii( "local" );
  PropertyValue aProp( rtl::OUString::createFromAscii( "servertype" ),
               -1,
               aAny,
               PropertyState_DIRECT_VALUE );

  Sequence< Any > seq(1);
  seq[0] <<= aProp;

  Reference< XMultiServiceFactory > sProvider;
  try
    {
      sProvider =
    Reference< XMultiServiceFactory >(
                      m_xSMgr->createInstanceWithArguments( sProviderService,seq ),
                      UNO_QUERY );
    }
  catch( const com::sun::star::uno::Exception& e )
    {
      VOS_ENSHURE( sProvider.is()," cant instantiate the multiservicefactory " );
    }


  if( ! sProvider.is() )
    {
      return;
    }

  rtl::OUString sReaderService =
    rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationAccess" );

  seq[0] <<= rtl::OUString::createFromAscii( "org.openoffice.Office.Common" );


  Reference< XHierarchicalNameAccess > xHierAccess;
  try
    {
      xHierAccess =
    Reference< XHierarchicalNameAccess >
    ( sProvider->createInstanceWithArguments( sReaderService,seq ),
      UNO_QUERY );
    }
  catch( const com::sun::star::uno::Exception& e )
    {
      VOS_ENSHURE( xHierAccess.is()," cant instantiate the reader service " );
    }

  if( ! xHierAccess.is() )
    return;

  try
    {
      aAny =
    xHierAccess->getByHierarchicalName( rtl::OUString::createFromAscii("Path/Current/Help") );
    }
  catch( const com::sun::star::container::NoSuchElementException& e )
    {
      VOS_ENSHURE( false," path to help files could not be determined " );
      return;
    }


  rtl::OUString instPath;
  bool err = ! ( aAny >>= instPath );

  if( err )
    {
      VOS_ENSHURE( false," path to help files could not be determined " );
      return;
    }

  instPath = rtl::OUString::createFromAscii( "$(instpath)/help" );

  Reference< XConfigManager >  xCfgMgr;
  try
    {
      xCfgMgr =
    Reference< XConfigManager >(
                    m_xSMgr->createInstance( rtl::OUString::createFromAscii( "com.sun.star.config.SpecialConfigManager" ) ),
                    UNO_QUERY );
    }
  catch( const com::sun::star::uno::Exception& e )
    {
      VOS_ENSHURE( xCfgMgr.is()," cant instantiate the special config manager " );
    }


  if( ! xCfgMgr.is() )
    return;

  instPath = xCfgMgr->substituteVariables( instPath );
  m_pDatabases = new Databases( instPath,m_xSMgr );

}
