/*************************************************************************
 *
 *  $RCSfile: webdavcontent.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: kso $ $Date: 2001-06-18 08:22:51 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSETINFOCHANGE_HPP_
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSETINFOCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertySetInfoChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFRERURLEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#include <ucbhelper/propertyvalueset.hxx>
#endif

#ifndef _WEBDAV_UCP_CONTENT_HXX
#include "webdavcontent.hxx"
#endif
#ifndef _WEBDAV_UCP_PROVIDER_HXX
#include "webdavprovider.hxx"
#endif
#ifndef _WEBDAV_UCP_RESULTSET_HXX
#include "webdavresultset.hxx"
#endif
#ifndef _WEBDAV_UCP_CONTENTPROPERTIES_HXX
#include "ContentProperties.hxx"
#endif
#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif
#ifndef _UCBDEADPROPERTYVALUE_HXX_
#include "UCBDeadPropertyValue.hxx"
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace cppu;
using namespace rtl;

using namespace webdav_ucp;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
// ctr for content on an existing webdav resource
Content::Content( const Reference< XMultiServiceFactory >& rxSMgr,
                    ContentProvider* pProvider,
                    const Reference< XContentIdentifier >& Identifier,
                  DAVSessionFactory* pSessionFactory )
  throw ( ContentCreationException )
: ContentImplHelper( rxSMgr, pProvider, Identifier ),
  m_pProvider( pProvider ),
  m_bTransient( sal_False ),
  m_bCollection( sal_False )
{
    try
    {
        m_aResAccess = DAVResourceAccess(
            rxSMgr, pSessionFactory, Identifier->getContentIdentifier() );
    }
    catch ( DAVException const & )
    {
          throw ContentCreationException();
    }

    NeonUri aURI( Identifier->getContentIdentifier() );
       m_aEscapedTitle = aURI.GetPathBaseName();
}

//=========================================================================
// ctr for content on an non-existing webdav resource
Content::Content( const Reference< XMultiServiceFactory >& rxSMgr,
                    ContentProvider* pProvider,
                    const Reference< XContentIdentifier >& Identifier,
                  DAVSessionFactory* pSessionFactory,
                    sal_Bool isCollection )
  throw ( ContentCreationException )
: ContentImplHelper( rxSMgr, pProvider, Identifier, sal_False ),
  m_pProvider( pProvider ),
  m_bTransient( sal_True ),
  m_bCollection( isCollection )
{
    try
    {
        m_aResAccess = DAVResourceAccess(
            rxSMgr, pSessionFactory, Identifier->getContentIdentifier() );
    }
    catch ( DAVException const & )
    {
          throw ContentCreationException();
    }

    // Do not set m_aEscapedTitle here! Content::insert relays on this!!!
}

//=========================================================================
// virtual
Content::~Content()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::acquire()
    throw( RuntimeException )
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release()
    throw( RuntimeException )
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
Any SAL_CALL Content::queryInterface( const Type & rType )
    throw ( RuntimeException )
{
    // Note: isFolder may require network activities! So call it only
    //       if it is really necessary!!!
      Any aRet = cppu::queryInterface( rType,
                                      static_cast< XContentCreator * >( this ) );
    if ( aRet.hasValue() )
        return isFolder( Reference< XCommandEnvironment >() ) ? aRet : Any();

      return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( Content );

//=========================================================================
// virtual
Sequence< Type > SAL_CALL Content::getTypes()
    throw( RuntimeException )
{
      static OTypeCollection* pCollection = NULL;

      if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
        {
              if ( isFolder( Reference< XCommandEnvironment >() ) )
            {
                  static OTypeCollection aCollection(
                                 CPPU_TYPE_REF( XTypeProvider ),
                                 CPPU_TYPE_REF( XServiceInfo ),
                                 CPPU_TYPE_REF( XComponent ),
                                 CPPU_TYPE_REF( XContent ),
                                 CPPU_TYPE_REF( XCommandProcessor ),
                                 CPPU_TYPE_REF( XPropertiesChangeNotifier ),
                                 CPPU_TYPE_REF( XCommandInfoChangeNotifier ),
                                 CPPU_TYPE_REF( XPropertyContainer ),
                                 CPPU_TYPE_REF( XPropertySetInfoChangeNotifier ),
                                 CPPU_TYPE_REF( XChild ),
                                 CPPU_TYPE_REF( XContentCreator ) );    // !!
                  pCollection = &aCollection;
            }
              else
            {
                  static OTypeCollection aCollection(
                                 CPPU_TYPE_REF( XTypeProvider ),
                                 CPPU_TYPE_REF( XServiceInfo ),
                                 CPPU_TYPE_REF( XComponent ),
                                 CPPU_TYPE_REF( XContent ),
                                 CPPU_TYPE_REF( XCommandProcessor ),
                                 CPPU_TYPE_REF( XPropertiesChangeNotifier ),
                                 CPPU_TYPE_REF( XCommandInfoChangeNotifier ),
                                 CPPU_TYPE_REF( XPropertyContainer ),
                                 CPPU_TYPE_REF( XPropertySetInfoChangeNotifier ),
                                 CPPU_TYPE_REF( XChild ) );
                  pCollection = &aCollection;
            }
        }
    }
      return (*pCollection).getTypes();
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
OUString SAL_CALL Content::getImplementationName()
    throw( RuntimeException )
{
    return OUString::createFromAscii( "com.sun.star.comp.ucb.WebDAVContent" );
}

//=========================================================================
// virtual
Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
            = OUString::createFromAscii( WEBDAV_CONTENT_SERVICE_NAME );
    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
OUString SAL_CALL Content::getContentType()
    throw( RuntimeException )
{
    if ( isFolder( Reference< XCommandEnvironment >() ) )
        return OUString::createFromAscii( WEBDAV_COLLECTION_TYPE );

    return OUString::createFromAscii( WEBDAV_CONTENT_TYPE );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
Any SAL_CALL Content::execute( const Command& aCommand,
                   sal_Int32 CommandId,
                   const Reference<
                   XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{
      Any aRet;

    if ( aCommand.Name.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( "getPropertyValues" ) ) )
    {
          //////////////////////////////////////////////////////////////////
          // getPropertyValues
          //////////////////////////////////////////////////////////////////

          Sequence< Property > Properties;
          if ( !( aCommand.Argument >>= Properties ) )
        {
              OSL_ENSURE( sal_False, "Wrong argument type!" );
              return Any();
        }

          aRet <<= getPropertyValues( Properties, Environment );
    }
      else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "setPropertyValues" ) ) )
    {
          //////////////////////////////////////////////////////////////////
          // setPropertyValues
          //////////////////////////////////////////////////////////////////

          Sequence< PropertyValue > aProperties;
          if ( !( aCommand.Argument >>= aProperties ) )
        {
              OSL_ENSURE( sal_False, "Wrong argument type!" );
              return Any();
        }

          if ( !aProperties.getLength() )
        {
              OSL_ENSURE( sal_False, "No properties!" );
              return Any();
        }

          setPropertyValues( aProperties, Environment );
    }
      else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getPropertySetInfo" ) ) )
       {
          //////////////////////////////////////////////////////////////////
          // getPropertySetInfo
          //////////////////////////////////////////////////////////////////

          // Note: Implemented by base class.
          aRet <<= getPropertySetInfo( Environment,
                                     sal_False /* don't cache data */ );
       }
      else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ) )
       {
          //////////////////////////////////////////////////////////////////
          // getCommandInfo
          //////////////////////////////////////////////////////////////////

          // Note: Implemented by base class.
          aRet <<= getCommandInfo( Environment, sal_False );
       }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "open" ) ) )
    {
        //////////////////////////////////////////////////////////////////
          // open
          //////////////////////////////////////////////////////////////////

          OpenCommandArgument2 aOpenCommand;
          if ( aCommand.Argument >>= aOpenCommand )
        {
            sal_Bool bOpenFolder =
                        ( ( aOpenCommand.Mode == OpenMode::ALL ) ||
                            ( aOpenCommand.Mode == OpenMode::FOLDERS ) ||
                            ( aOpenCommand.Mode == OpenMode::DOCUMENTS ) );

              if ( bOpenFolder && isFolder( Environment ) )
            {
                // Open collection.

                Reference< XDynamicResultSet > xSet
                                = new DynamicResultSet( m_xSMgr,
                                                        this,
                                                        aOpenCommand,
                                                        Environment );
                aRet <<= xSet;
              }

            if ( aOpenCommand.Sink.is() )
            {
                // Open document.

                if ( ( aOpenCommand.Mode
                            == OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                     ( aOpenCommand.Mode
                             == OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
                {
                    // Currently(?) unsupported.
                      throw CommandAbortedException();
                }

                OUString aURL = m_xIdentifier->getContentIdentifier();
                Reference< XOutputStream > xOut
                      = Reference< XOutputStream >(aOpenCommand.Sink, UNO_QUERY );
                if ( xOut.is() )
                  {
                    // PUSH: write data
                    try
                    {
                          m_aResAccess.GET( xOut, Environment );
                    }
                    catch ( DAVException const & )
                    {
//                      OSL_ENSURE( sal_False, "GET : DAVException" );
                          throw CommandAbortedException();
                    }
                  }
                else
                  {
                    Reference< XActiveDataSink > xDataSink
                          = Reference< XActiveDataSink >(
                                                aOpenCommand.Sink, UNO_QUERY );
                      if ( xDataSink.is() )
                    {
                          // PULL: wait for client read
                          try
                        {
                            Reference< XInputStream > xIn
                                = m_aResAccess.GET( Environment );
                            xDataSink->setInputStream( xIn );
                          }
                        catch ( DAVException &)
                        {
//                          OSL_ENSURE( sal_False, "GET : DAVException" );
                            throw CommandAbortedException();
                          }
                    }
                      else
                    {
                        // Note: aOpenCommand.Sink may contain an XStream
                        //       implementation. Support for this type of
                        //       sink is optional...
                          throw CommandAbortedException();
                    }
                  }
              }
        }
          else
        {
              OSL_ENSURE( sal_False, "Content::execute - invalid parameter!" );
              throw CommandAbortedException();
        }
    }
      else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
        //////////////////////////////////////////////////////////////////
          // insert
          //////////////////////////////////////////////////////////////////

          InsertCommandArgument arg;
          if ( !( aCommand.Argument >>= arg ) )
        {
              OSL_ENSURE( sal_False, "Wrong argument type!" );
              return Any();
        }

          insert( arg.Data, arg.ReplaceExisting, Environment );
    }
      else if ( aCommand.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
    {
          //////////////////////////////////////////////////////////////////
          // delete
          //////////////////////////////////////////////////////////////////

          sal_Bool bDeletePhysical = sal_False;
          aCommand.Argument >>= bDeletePhysical;

//  KSO: Ignore parameter and destroy the content, if you don't support
//       putting objects into trashcan. ( Since we do not have a trash can
//       service yet (src603), you actually have no other choice. )
//      if ( bDeletePhysical )
//      {
            try
            {
                  m_aResAccess.DESTROY( Environment );
            }
            catch ( DAVException & )
            {
                  OSL_ENSURE( sal_False, "DESTROY : DAVException" );
                  throw CommandAbortedException();
            }
//      }

        // Propagate destruction.
        destroy( bDeletePhysical );

// DAV resources store all additional props on server!
//      // Remove own and all children's Additional Core Properties.
//      removeAdditionalPropertySet( sal_True );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "transfer" ) ) )
      {
        TransferInfo transferArgs;
        if ( !( aCommand.Argument >>= transferArgs ) )
            throw CommandAbortedException();

        if ( !isFolder( Environment ) )
            throw CommandAbortedException();

        if ( transferArgs.NameClash == NameClash::KEEP ||
             transferArgs.NameClash == NameClash::RENAME )
        {
            // @@@ RENAME and KEEP are not directly implemented
            // by WebDAV methods. KEEP is deprecated and it
            // is okay to abort in this case.

            throw CommandAbortedException();
        }

        try
        {
            NeonUri sourceURI( transferArgs.SourceURL );
            NeonUri targetURI( m_xIdentifier->getContentIdentifier() );

            // Check source's and target's URL scheme
            //
            const OUString aScheme = sourceURI.GetScheme().toAsciiLowerCase();
            if ( aScheme.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( WEBDAV_URL_SCHEME ) ) )
            {
                sourceURI.SetScheme(
                    OUString::createFromAscii( HTTP_URL_SCHEME ) );
            }
            else
            {
                if ( !aScheme.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( HTTP_URL_SCHEME ) ) &&
                     !aScheme.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( HTTPS_URL_SCHEME ) ) )
                    throw InteractiveBadTransferURLException();
            }

            if ( targetURI.GetScheme().toAsciiLowerCase().equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( WEBDAV_URL_SCHEME ) ) )
                targetURI.SetScheme(
                    OUString::createFromAscii( HTTP_URL_SCHEME ) );

            // @@@ This implementation of 'transfer' only works
            //     if the source and target are located at same host.
            //     (Neon does not support cross-server copy/move)

            // Check for same host
            //
            if ( sourceURI.GetHost().getLength() &&
                 sourceURI.GetHost() != targetURI.GetHost() )
                throw InteractiveBadTransferURLException();

            if ( !transferArgs.NewTitle.getLength() )
                transferArgs.NewTitle = sourceURI.GetPathBaseNameUnescaped();

            if ( transferArgs.NewTitle.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "/" ) ) )
                throw CommandAbortedException();

            targetURI.AppendPath( transferArgs.NewTitle );

            OUString aTargetURL = m_xIdentifier->getContentIdentifier();
            if ( ( aTargetURL.lastIndexOf( '/' ) + 1 ) != aTargetURL.getLength() )
                aTargetURL += OUString::createFromAscii( "/" );

            aTargetURL += transferArgs.NewTitle;

            Reference< XContentIdentifier > xTargetId
                = new ::ucb::ContentIdentifier( m_xSMgr, aTargetURL );

            Reference< XContentIdentifier > xId
                = new ::ucb::ContentIdentifier( m_xSMgr,
                                                transferArgs.SourceURL );

            DAVResourceAccess aSourceAccess( m_xSMgr,
                                             m_aResAccess.getSessionFactory(),
                                             sourceURI.GetURI() );

            if ( transferArgs.MoveData == sal_True )
            {
                // Note: The static cast is okay here, because its sure that
                //       m_xProvider is always the WebDAVContentProvider.
                vos::ORef< Content > xSource
                    = static_cast< Content * >(
                        m_xProvider->queryContent( xId ).get() );

                aSourceAccess.MOVE( sourceURI.GetPath(),
                                       targetURI.GetURI(),
                                       transferArgs.NameClash
                                           == NameClash::OVERWRITE,
                                       Environment );

                if ( xSource.isValid() )
                {
                    // Propagate destruction to listeners.
                    xSource->destroy( sal_True );
                }

// DAV resources store all additional props on server!
//              // Rename own and all children's Additional Core Properties.
//              renameAdditionalPropertySet( xId->getContentIdentifier(),
//                                           xTargetId->getContentIdentifier(),
//                                           sal_True );
            }
            else
            {
                aSourceAccess.COPY( sourceURI.GetPath(),
                                       targetURI.GetURI(),
                                       transferArgs.NameClash
                                           == NameClash::OVERWRITE,
                                       Environment );

// DAV resources store all additional props on server!
//              // Copy own and all children's Additional Core Properties.
//              copyAdditionalPropertySet( xId->getContentIdentifier(),
//                                         xTargetId->getContentIdentifier(),
//                                         sal_True );
            }

            // Note: The static cast is okay here, because its sure that
            //       m_xProvider is always the WebDAVContentProvider.
            vos::ORef< Content > xTarget
                = static_cast< Content * >(
                        m_xProvider->queryContent( xTargetId ).get() );

            // Announce transfered content in its new folder.
            xTarget->inserted();
        }
        catch ( DAVException const & )
        {
            throw CommandAbortedException();
        }
    }
      else
    {
          //////////////////////////////////////////////////////////////////
          // Unsupported command
          //////////////////////////////////////////////////////////////////

          OSL_ENSURE( sal_False, "Content::execute - unsupported command!" );
          throw CommandAbortedException();
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL Content::abort( sal_Int32 CommandId )
    throw( RuntimeException )
{
    // @@@ Implement logic to abort running commands, if this makes
    //     sense for your content.
}

//=========================================================================
//
// XPropertyContainer methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::addProperty(
        const OUString& Name, sal_Int16 Attributes, const Any& DefaultValue )
    throw( PropertyExistException,
           IllegalTypeException,
           IllegalArgumentException,
           RuntimeException )
{
//  if ( m_bTransient )
//      @@@ ???

//  osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !Name.getLength() )
        throw IllegalArgumentException();

    // Check property type.
    if ( !UCBDeadPropertyValue::supportsType( DefaultValue.getValueType() ) )
    {
        OSL_ENSURE( sal_False, "Content::addProperty - "
                               "Unsupported property type!" );
        throw IllegalTypeException();
    }

    //////////////////////////////////////////////////////////////////////
    // Make sure a property with the requested name does not already
    // exist in dynamic and static(!) properties.
    //////////////////////////////////////////////////////////////////////

    // @@@ Need real command environment here, but where to get it from?
    //     XPropertyContainer interface should be replaced by
    //     XCommandProcessor commands!
    Reference< XCommandEnvironment > xEnv;

    // Note: This requires network access!
    if ( getPropertySetInfo( xEnv, sal_False /* don't cache data */ )
            ->hasPropertyByName( Name ) )
    {
        // Property does already exist.
        throw PropertyExistException();
    }

    //////////////////////////////////////////////////////////////////////
    // Add a new dynamic property.
    //////////////////////////////////////////////////////////////////////

    ProppatchValue aValue( PROPSET, Name, DefaultValue );

    std::vector< ProppatchValue > aProppatchValues;
    aProppatchValues.push_back( aValue );

    try
    {
        // Set property value at server.
        m_aResAccess.PROPPATCH( aProppatchValues, xEnv );

        // Notify propertyset info change listeners.
        PropertySetInfoChangeEvent evt(
                            static_cast< OWeakObject * >( this ),
                            Name,
                            -1, // No handle available
                            PropertySetInfoChange::PROPERTY_INSERTED );
        notifyPropertySetInfoChange( evt );
    }
    catch ( DAVException const & )
    {
        try
        {
            DAVCapabilities caps;
               m_aResAccess.OPTIONS( caps, xEnv );

            if ( caps.class1 )
            {
                // DAV resource!
                throw IllegalArgumentException();
            }
            else
            {
                // HTTP resource!

                // Store property locally.
                ContentImplHelper::addProperty(
                                        Name, Attributes, DefaultValue );
            }
        }
        catch ( DAVException const & )
        {
            throw IllegalArgumentException();
          }
    }
}

//=========================================================================
// virtual
void SAL_CALL Content::removeProperty( const OUString& Name )
    throw( UnknownPropertyException,
           NotRemoveableException,
           RuntimeException )
{
//  osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // @@@ Need real command environment here, but where to get it from?
    //     XPropertyContainer interface should be replaced by
    //     XCommandProcessor commands!
    Reference< XCommandEnvironment > xEnv;

#if 0
    // @@@ REMOVEABLE z.Z. nicht richtig and der PropSetInfo gesetzt!!!
    try
    {
        Property aProp = getPropertySetInfo( xEnv,
                                             sal_False /* don't cache data */ )
                                ->getPropertyByName( Name );

        if ( !( aProp.Attributes & PropertyAttribute::REMOVEABLE ) )
        {
            // Not removeable!
            throw NotRemoveableException();
        }
    }
    catch ( UnknownPropertyException const & )
    {
//      OSL_ENSURE( sal_False, "removeProperty - Unknown property!" );
        throw;
    }
#endif

    //////////////////////////////////////////////////////////////////////
    // Try to remove property from server.
    //////////////////////////////////////////////////////////////////////

    try
    {
        std::vector< ProppatchValue > aProppatchValues;
        ProppatchValue aValue( PROPREMOVE, Name, Any() );
        aProppatchValues.push_back( aValue );

        // Remove property value from server.
        m_aResAccess.PROPPATCH( aProppatchValues, xEnv );

        // Notify propertyset info change listeners.
        PropertySetInfoChangeEvent evt(
                            static_cast< OWeakObject * >( this ),
                            Name,
                            -1, // No handle available
                            PropertySetInfoChange::PROPERTY_REMOVED );
        notifyPropertySetInfoChange( evt );
    }
    catch ( DAVException const & )
    {
        try
        {
            DAVCapabilities caps;
               m_aResAccess.OPTIONS( caps, xEnv );

            if ( caps.class1 )
            {
                // DAV resource!
                throw UnknownPropertyException();
            }
            else
            {
                // HTTP resource!

                // Try to remove property from local store.
                ContentImplHelper::removeProperty( Name );
            }
        }
        catch ( DAVException const & )
        {
            throw UnknownPropertyException();
          }
    }
}

//=========================================================================
//
// XContentCreator methods.
//
//=========================================================================

// virtual
Sequence< ContentInfo > SAL_CALL
Content::queryCreatableContentsInfo()
    throw( RuntimeException )
{
//  if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        Sequence< ContentInfo > aSeq( 2 );

        // document.
        aSeq.getArray()[ 0 ].Type
            = OUString::createFromAscii( WEBDAV_CONTENT_TYPE );
        aSeq.getArray()[ 0 ].Attributes
            = ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
              | ContentInfoAttribute::KIND_DOCUMENT;

        Property aProp;
        m_pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ), aProp );

        Sequence< Property > aDocProps( 1 );
        aDocProps.getArray()[ 0 ] = aProp;
        aSeq.getArray()[ 0 ].Properties = aDocProps;

        // folder.
        aSeq.getArray()[ 1 ].Type
            = OUString::createFromAscii( WEBDAV_COLLECTION_TYPE );
        aSeq.getArray()[ 1 ].Attributes = ContentInfoAttribute::KIND_FOLDER;

        Sequence< Property > aFolderProps( 1 );
        aFolderProps.getArray()[ 0 ] = aProp;
        aSeq.getArray()[ 1 ].Properties = aFolderProps;
        return aSeq;
    }
/*
    else
    {
        OSL_ENSURE( sal_False,
                    "queryCreatableContentsInfo called on non-folder object!" );

        return Sequence< ContentInfo >( 0 );
    }
*/
}

//=========================================================================
// virtual
Reference< XContent > SAL_CALL Content::createNewContent( const ContentInfo& Info )
    throw( RuntimeException )
{
//  if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

          if ( !Info.Type.getLength() )
            return Reference< XContent >();

          if ( ( !Info.Type.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( WEBDAV_COLLECTION_TYPE ) ) )
                &&
                ( !Info.Type.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM( WEBDAV_CONTENT_TYPE ) ) ) )
            return Reference< XContent >();

          OUString aURL = m_xIdentifier->getContentIdentifier();

          OSL_ENSURE( aURL.getLength() > 0,
                      "WebdavContent::createNewContent - empty identifier!" );

        if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
              aURL += OUString::createFromAscii( "/" );

          sal_Bool isCollection;
          if ( Info.Type.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( WEBDAV_COLLECTION_TYPE ) ) )
        {
            aURL += OUString::createFromAscii( "[New_Collection]" );
            isCollection = sal_True;
          }
          else
        {
            aURL += OUString::createFromAscii( "[New_Content]" );
            isCollection = sal_False;
          }

          Reference< XContentIdentifier > xId(
                          new ::ucb::ContentIdentifier( m_xSMgr, aURL ) );

          // create the local content
          try
        {
            return new ::webdav_ucp::Content( m_xSMgr,
                                              m_pProvider,
                                                xId,
                                                m_aResAccess.getSessionFactory(),
                                                isCollection );
        }
        catch ( ContentCreationException & )
          {
              return Reference< XContent >();
          }
    }
/*
      else
    {
        OSL_ENSURE( sal_False, "createNewContent called on non-folder object!" );
          return Reference< XContent >();
    }
*/
}

//=========================================================================
// virtual
OUString Content::getParentURL()
{
    // <scheme>://              -> ""
    // <scheme>://foo           -> ""
    // <scheme>://foo/          -> ""
    // <scheme>://foo/bar       -> <scheme>://foo/
    // <scheme>://foo/bar/      -> <scheme>://foo/
    // <scheme>://foo/bar/abc   -> <scheme>://foo/bar/

    OUString aURL = m_xIdentifier->getContentIdentifier();

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos == ( aURL.getLength() - 1 ) )
    {
        // Trailing slash found. Skip.
        nPos = aURL.lastIndexOf( '/', nPos );
    }

    sal_Int32 nPos1 = aURL.lastIndexOf( '/', nPos );
    if ( nPos1 != -1 )
        nPos1 = aURL.lastIndexOf( '/', nPos1 );

    if ( nPos1 == -1 )
        return OUString();

    return OUString( aURL.copy( 0, nPos + 1 ) );
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

// static
Reference< XRow > Content::getPropertyValues(
                const Reference< XMultiServiceFactory >& rSMgr,
                const Sequence< Property >& rProperties,
                const ContentProperties& rData,
                const vos::ORef< ucb::ContentProviderImplHelper >& rProvider,
                const OUString& rContentId )
{
      // Note: Empty sequence means "get values of all supported properties".

      vos::ORef< ::ucb::PropertyValueSet > xRow
        = new ::ucb::PropertyValueSet( rSMgr );

      sal_Int32 nCount = rProperties.getLength();
      if ( nCount )
    {
        Reference< XPropertySet > xAdditionalPropSet;
          sal_Bool bTriedToGetAdditonalPropSet = sal_False;

          const Property* pProps = rProperties.getConstArray();
          for ( sal_Int32 n = 0; n < nCount; ++n )
        {
              const Property& rProp = pProps[ n ];

              // Process Core properties.

              if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
               {
                if ( rData.pIsFolder && *rData.pIsFolder )
                      xRow->appendString( rProp, OUString::createFromAscii(
                                                      WEBDAV_COLLECTION_TYPE ) );
                else
                      xRow->appendString( rProp, OUString::createFromAscii(
                                                      WEBDAV_CONTENT_TYPE ) );
            }
              else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
            {
                  xRow->appendString ( rProp, rData.aTitle );
            }
              else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
            {
                if ( rData.pIsDocument )
                      xRow->appendBoolean( rProp, *rData.pIsDocument );
                else
                      xRow->appendBoolean( rProp, sal_True );
            }
              else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
            {
                if ( rData.pIsFolder )
                      xRow->appendBoolean( rProp, *rData.pIsFolder );
                else
                      xRow->appendBoolean( rProp, sal_False );
            }
              else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
              {
                if ( rData.pSize )
                      xRow->appendLong( rProp, *rData.pSize );
                else
                    xRow->appendVoid( rProp );
            }
              else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) ) )
              {
                if ( rData.pDateCreated )
                      xRow->appendTimestamp( rProp, *rData.pDateCreated );
                else
                    xRow->appendVoid( rProp );
            }
              else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) )
              {
                if ( rData.pDateModified )
                      xRow->appendTimestamp( rProp, *rData.pDateModified );
                else
                    xRow->appendVoid( rProp );

            }
              else if ( rProp.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
            {
                if ( rData.pgetcontenttype )
                      xRow->appendString( rProp, *rData.pgetcontenttype );
                else
                    xRow->appendVoid( rProp );
            }
              else if ( rProp.Name.equals( DAVProperties::CREATIONDATE ) )
            {
                if ( rData.pcreationdate )
                    xRow->appendString( rProp, *rData.pcreationdate );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::DISPLAYNAME ) )
            {
                if ( rData.pdisplayname )
                    xRow->appendString( rProp, *rData.pdisplayname );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::GETCONTENTLANGUAGE ) )
            {
                if ( rData.pgetcontentlanguage )
                    xRow->appendString( rProp, *rData.pgetcontentlanguage );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::GETCONTENTLENGTH ) )
            {
                if ( rData.pgetcontentlength )
                    xRow->appendString( rProp, *rData.pgetcontentlength );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::GETCONTENTTYPE ) )
            {
                if ( rData.pgetcontenttype )
                    xRow->appendString( rProp, *rData.pgetcontenttype );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::GETETAG ) )
            {
                if ( rData.pgetetag )
                    xRow->appendString( rProp, *rData.pgetetag );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::GETLASTMODIFIED ) )
            {
                if ( rData.pgetlastmodified )
                    xRow->appendString( rProp, *rData.pgetlastmodified );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::LOCKDISCOVERY ) )
            {
                if ( rData.plockdiscovery )
                    xRow->appendObject( rProp,
                                        makeAny( *rData.plockdiscovery ) );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::RESOURCETYPE ) )
            {
                if ( rData.presourcetype )
                    xRow->appendObject( rProp,
                                        makeAny( *rData.presourcetype ) );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::SOURCE ) )
            {
                if ( rData.psource )
                    xRow->appendObject( rProp, makeAny( *rData.psource ) );
                else
                    xRow->appendVoid( rProp );
              }
              else if ( rProp.Name.equals( DAVProperties::SUPPORTEDLOCK ) )
            {
                if ( rData.psupportedlock )
                    xRow->appendObject( rProp,
                                        makeAny( *rData.psupportedlock ) );
                else
                    xRow->appendVoid( rProp );
              }
            else
            {
                sal_Bool bAppened = sal_False;

                if ( rData.pOtherProps )
                {
                    // Process additional properties (DAV "dead" properties).
                    const PropertyValueMap::const_iterator it
                        = rData.pOtherProps->find( rProp.Name );
                    if ( it != rData.pOtherProps->end() )
                    {
                        xRow->appendObject( rProp, (*it).second );
                        bAppened = sal_True;
                    }
                }

                if ( !bAppened )
                {
                    // Process local additional properties.
                      if ( !bTriedToGetAdditonalPropSet
                           && !xAdditionalPropSet.is() )
                    {
                          xAdditionalPropSet
                            = Reference< XPropertySet >(
                                rProvider->getAdditionalPropertySet(
                                    rContentId, sal_False ),
                                UNO_QUERY );
                          bTriedToGetAdditonalPropSet = sal_True;
                    }

                      if ( xAdditionalPropSet.is() )
                    {
                          if ( xRow->appendPropertySetValue(
                                         xAdditionalPropSet, rProp ) )
                            bAppened = sal_True;
                    }
                }

                if ( !bAppened )
                {
                      // Append empty entry.
                      xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        // Append all Core Properties.
        ContentProvider * pProvider
            = static_cast< ContentProvider * >( rProvider.getBodyPtr() );
        Property aProp;

        pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ), aProp );
        if ( rData.pIsFolder && *rData.pIsFolder )
              xRow->appendString(   aProp, OUString::createFromAscii(
                                                    WEBDAV_COLLECTION_TYPE ) );
        else
              xRow->appendString(   aProp, OUString::createFromAscii(
                                                      WEBDAV_CONTENT_TYPE ) );
        pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ), aProp );
          xRow->appendString( aProp, rData.aTitle );

        pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ), aProp );
        if ( rData.pIsDocument )
              xRow->appendBoolean( aProp, *rData.pIsDocument );
        else
              xRow->appendBoolean( aProp, sal_True );

        pProvider->getProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ), aProp );
        if ( rData.pIsFolder )
              xRow->appendBoolean( aProp,   *rData.pIsFolder );
        else
              xRow->appendBoolean( aProp, sal_False );

        if ( rData.pSize )
        {
            pProvider->getProperty(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ), aProp );
              xRow->appendLong( aProp, *rData.pSize );
        }

        if ( rData.pDateCreated )
        {
            pProvider->getProperty(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ),
                aProp );
              xRow->appendTimestamp( aProp, *rData.pDateCreated );
        }

        if ( rData.pDateModified )
        {
            pProvider->getProperty(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ),
                aProp );
              xRow->appendTimestamp( aProp, *rData.pDateModified );
        }

        if ( rData.pgetcontenttype )
        {
            pProvider->getProperty(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ), aProp );
              xRow->appendString( aProp, *rData.pgetcontenttype );
        }

        if ( rData.pcreationdate )
        {
            pProvider->getProperty( DAVProperties::CREATIONDATE, aProp );
              xRow->appendString( aProp, *rData.pcreationdate );
        }

        if ( rData.pdisplayname )
        {
            pProvider->getProperty( DAVProperties::DISPLAYNAME, aProp );
              xRow->appendString( aProp, *rData.pdisplayname );
        }

        if ( rData.pgetcontentlanguage )
        {
            pProvider->getProperty( DAVProperties::GETCONTENTLANGUAGE, aProp );
              xRow->appendString( aProp, *rData.pgetcontentlanguage );
        }

        if ( rData.pgetcontentlength )
        {
            pProvider->getProperty( DAVProperties::GETCONTENTLENGTH, aProp );
              xRow->appendString( aProp, *rData.pgetcontentlength );
        }

        if ( rData.pgetcontenttype )
        {
            pProvider->getProperty( DAVProperties::GETCONTENTTYPE, aProp );
              xRow->appendString( aProp, *rData.pgetcontenttype );
        }

        if ( rData.pgetetag )
        {
            pProvider->getProperty( DAVProperties::GETETAG, aProp );
              xRow->appendString( aProp, *rData.pgetetag );
        }

        if ( rData.pgetlastmodified )
        {
            pProvider->getProperty( DAVProperties::GETLASTMODIFIED, aProp );
              xRow->appendString( aProp, *rData.pgetlastmodified );
        }

        if ( rData.plockdiscovery )
        {
            pProvider->getProperty( DAVProperties::LOCKDISCOVERY, aProp );
              xRow->appendObject( aProp, makeAny( *rData.plockdiscovery ) );
        }

        if ( rData.presourcetype )
        {
            pProvider->getProperty( DAVProperties::RESOURCETYPE, aProp );
              xRow->appendString( aProp, *rData.presourcetype );
        }

        if ( rData.psource )
        {
            pProvider->getProperty( DAVProperties::SOURCE, aProp );
              xRow->appendObject( aProp, makeAny( *rData.psource ) );
        }

        if ( rData.psupportedlock )
        {
            pProvider->getProperty( DAVProperties::SUPPORTEDLOCK, aProp );
              xRow->appendObject( aProp, makeAny( *rData.psupportedlock ) );
        }

        // Process additional properties (DAV "dead" properties).
        if ( rData.pOtherProps )
        {
            PropertyValueMap::const_iterator it  = rData.pOtherProps->begin();
            PropertyValueMap::const_iterator end = rData.pOtherProps->end();

            Property aProp;
            while ( it != end )
            {
                pProvider->getProperty( (*it).first, aProp );
                xRow->appendObject( aProp, (*it).second );
                it++;
            }
        }

        // Append all local Additional Core Properties.
          Reference< XPropertySet > xSet(
                 rProvider->getAdditionalPropertySet( rContentId, sal_False ),
                 UNO_QUERY );
          xRow->appendPropertySet( xSet );
    }

      return Reference< XRow >( xRow.getBodyPtr() );
}

//=========================================================================
Reference< XRow > Content::getPropertyValues(
                        const Sequence< Property >& rProperties,
                        const Reference< XCommandEnvironment >& xEnv )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_bTransient )
    {
        // No PROPFIND, but minimal local props.
        return getPropertyValues( m_xSMgr,
                                      rProperties,
                                        ContentProperties(
                                      NeonUri::unescape( m_aEscapedTitle ),
                                         m_bCollection ),
                                        m_xProvider,
                                        m_xIdentifier->getContentIdentifier() );
    }

    // Only title requested? No PROPFIND necessary.
    if ( ( rProperties.getLength() == 1 )
         &&
         rProperties[ 0 ].Name.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
    {
        return getPropertyValues( m_xSMgr,
                                      rProperties,
                                        ContentProperties(
                                      NeonUri::unescape( m_aEscapedTitle ),
                                         m_bCollection ),
                                        m_xProvider,
                                        m_xIdentifier->getContentIdentifier() );
    }

    bool bSuccess = true;
    std::vector< OUString > aPropNames;
    ContentProperties::UCBNamesToDAVNames( rProperties, aPropNames );

    std::vector< DAVResource > resources;
    if ( aPropNames.size() > 0 )
    {
        try
        {
            m_aResAccess.PROPFIND( ZERO, aPropNames, resources, xEnv );
        }
        catch ( DAVException & )
        {
//          OSL_ENSURE( sal_False, "PROPFIND : DAVException" );
              bSuccess = false;
        }
    }

    bSuccess &= ( resources.size() == 1 );

    if ( bSuccess )
    {
        return getPropertyValues( m_xSMgr,
                                      rProperties,
                                        ContentProperties( resources[ 0 ] ),
                                        m_xProvider,
                                        m_xIdentifier->getContentIdentifier() );
    }
    else
    {
        // PROPFIND failed, but minimal local props "available".
        return getPropertyValues( m_xSMgr,
                                      rProperties,
                                        ContentProperties(
                                      NeonUri::unescape( m_aEscapedTitle ),
                                         sal_False /* no collection */ ),
                                        m_xProvider,
                                        m_xIdentifier->getContentIdentifier() );
    }
}

//=========================================================================
void Content::setPropertyValues( const Sequence< PropertyValue >& rValues,
                                    const Reference< XCommandEnvironment >& xEnv )
{
      osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

      Sequence< PropertyChangeEvent > aChanges( rValues.getLength() );
      sal_Int32 nChanged = 0;

      PropertyChangeEvent aEvent;
      aEvent.Source           = static_cast< OWeakObject * >( this );
      aEvent.Further          = sal_False;
      // aEvent.PropertyName =
      aEvent.PropertyHandle = -1;
      // aEvent.OldValue     =
      // aEvent.NewValue   =

    sal_Bool bCheckedForDAV = sal_False;
    sal_Bool bDAV = sal_False;

    std::vector< ProppatchValue > aProppatchValues;

    Reference< XPersistentPropertySet > xAdditionalPropSet;
      sal_Bool bTriedToGetAdditonalPropSet = sal_False;

      sal_Bool bExchange = sal_False;
    OUString aNewTitle;
    OUString aOldTitle;

    Reference< XPropertySetInfo > xInfo;

      const PropertyValue* pValues = rValues.getConstArray();
      sal_Int32 nCount = rValues.getLength();
      for ( sal_Int32 n = 0; n < nCount; ++n )
      {
        const PropertyValue& rValue = pValues[ n ];

        Property aTmpProp;
        m_pProvider->getProperty( rValue.Name, aTmpProp );

        if ( aTmpProp.Attributes & PropertyAttribute::READONLY )
        {
              // Read-only property!
            continue;
        }

        if ( !xInfo.is() )
            xInfo
                = getPropertySetInfo( xEnv, sal_False /* don't cache data */ );

        if ( !xInfo->hasPropertyByName( rValue.Name ) )
        {
            // Check, whether property exists. Abort otherwise.
            // PROPPATCH::set would add the property automatically, which
            // is not allowed for "setPropertyValues" command!
            continue;
        }

//      if ( rValue.Name.equalsAsciiL(
//                      RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
//      {
//          // Read-only property!
//      }
//      else if ( rValue.Name.equalsAsciiL(
//                      RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
//      {
//          // Read-only property!
//      }
//      else if ( rValue.Name.equalsAsciiL(
//                      RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
//      {
//          // Read-only property!
//      }
          /*else*/ if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
              {
                // No empty titles!
                if ( aNewValue.getLength() > 0 )
                {
                    NeonUri aURI( m_xIdentifier->getContentIdentifier() );
                       aOldTitle = aURI.GetPathBaseNameUnescaped();

                    if ( aNewValue != aOldTitle )
                      {
                        // modified title -> modified URL -> exchange !
                        if ( !m_bTransient )
                            bExchange = sal_True;

                        // new value will be set later...
                        aNewTitle = aNewValue;
                      }
                }
            }
        }
//      else if ( rValue.Name.equalsAsciiL(
//                      RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
//      {
//          // Read-only property!
//      }
//      else if ( rValue.Name.equalsAsciiL(
//                      RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) ) )
//      {
//          // Read-only property!
//      }
//      else if ( rValue.Name.equalsAsciiL(
//                      RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) )
//      {
//          // Read-only property!
//      }
//      else if ( rValue.Name.equalsAsciiL(
//                      RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
//      {
//          // Read-only property!
//          // (but could be writable, if 'getcontenttype' would be)
//      }
        else
        {
            if ( !bCheckedForDAV )
            {
                bCheckedForDAV = sal_True;

                try
                {
                    DAVCapabilities caps;
                       m_aResAccess.OPTIONS( caps, xEnv );
                    bDAV = caps.class1;
                }
                catch ( DAVException const & )
                {
                    bDAV = sal_False;
                  }
            }

            if ( bDAV )
            {
                // Property value will be set on server.
                ProppatchValue aValue( PROPSET, rValue.Name, rValue.Value );
                aProppatchValues.push_back( aValue );
            }
            else
            {
                // Property value will be stored in local property store.
                if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet = getAdditionalPropertySet( sal_False );
                    bTriedToGetAdditonalPropSet = sal_True;
                }

                if ( xAdditionalPropSet.is() )
                {
                    try
                    {
                        Any aOldValue
                                = xAdditionalPropSet->getPropertyValue(
                                                                rValue.Name );
                        if ( aOldValue != rValue.Value )
                        {
                            xAdditionalPropSet->setPropertyValue(
                                                    rValue.Name, rValue.Value );

                            aEvent.PropertyName = rValue.Name;
                            aEvent.OldValue     = aOldValue;
                            aEvent.NewValue     = rValue.Value;

                            aChanges.getArray()[ nChanged ] = aEvent;
                            nChanged++;
                        }
                    }
                    catch ( UnknownPropertyException )
                    {
                    }
                    catch ( WrappedTargetException )
                    {
                    }
                    catch ( PropertyVetoException )
                    {
                    }
                    catch ( IllegalArgumentException )
                    {
                    }
                }
            }
        }
      }

    if ( !m_bTransient && aProppatchValues.size() )
    {
        try
        {
            // Set property values at server.
            m_aResAccess.PROPPATCH( aProppatchValues, xEnv );

            std::vector< ProppatchValue >::const_iterator it
                = aProppatchValues.begin();
            std::vector< ProppatchValue >::const_iterator end
                = aProppatchValues.end();

            while ( it != end )
            {
                aEvent.PropertyName = (*it).name;
                aEvent.OldValue     = Any(); // @@@ to expensive to obtain...!
                aEvent.NewValue     = (*it).value;

                aChanges.getArray()[ nChanged ] = aEvent;
                nChanged++;

                ++it;
            }
        }
        catch ( DAVException const & )
        {
              OSL_ENSURE( sal_False,
                        "Content::setPropertyValues - PROPPATCH failed!" );
        }
    }

      if ( bExchange )
      {
        // Assemble new content identifier...

          OUString aNewURL = getParentURL();
          if ( aNewURL.lastIndexOf( '/' ) != ( aNewURL.getLength() - 1 ) )
            aNewURL += OUString::createFromAscii( "/" );

          aNewURL += NeonUri::escapeSegment( aNewTitle );

        Reference< XContentIdentifier > xNewId
                            = new ::ucb::ContentIdentifier( m_xSMgr, aNewURL );
          Reference< XContentIdentifier > xOldId = m_xIdentifier;

        try
        {
            NeonUri sourceURI( xOldId->getContentIdentifier() );
            NeonUri targetURI( xNewId->getContentIdentifier() );
            targetURI.SetScheme( OUString::createFromAscii( "http" ) );

            m_aResAccess.MOVE(
                sourceURI.GetPath(), targetURI.GetURI(), sal_False, xEnv );
            // @@@ Should check for resources that could not be moved
            //     (due to source access or target overwrite) and send
            //     this information through the interaction handler.

            // @@@ Existing content should be checked to see if it needs
            //     to be deleted at the source

            // @@@ Existing content should be checked to see if it has
            //     been overwritten at the target

            aGuard.clear();
            if ( exchangeIdentity( xNewId ) )
            {
                m_aResAccess.setURL( aNewURL );

// DAV resources store all additional props on server!
//              // Adapt Additional Core Properties.
//              renameAdditionalPropertySet( xOldId->getContentIdentifier(),
//                                           xNewId->getContentIdentifier(),
//                                           sal_True );
            }
            else
            {
                // Do not set new title!
                aNewTitle = OUString();
            }
        }
        catch ( DAVException const & )
        {
            // Do not set new title!
            aNewTitle = OUString();
        }
      }

    if ( aNewTitle.getLength() )
    {
        aEvent.PropertyName = OUString::createFromAscii( "Title" );
        aEvent.OldValue     = makeAny( aOldTitle );
        aEvent.NewValue     = makeAny( aNewTitle );

        m_aEscapedTitle = NeonUri::escapeSegment( aNewTitle );

        aChanges.getArray()[ nChanged ] = aEvent;
        nChanged++;
    }

    if ( nChanged > 0 )
    {
        aGuard.clear();
          aChanges.realloc( nChanged );
          notifyPropertiesChange( aChanges );
    }
}

//=========================================================================
void Content::queryChildren( ContentRefList& rChildren )
{
    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucb::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aURL.lastIndexOf( '/' );

    if ( nPos != ( aURL.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aURL += OUString::createFromAscii( "/" );
    }

    sal_Int32 nLen = aURL.getLength();

    ::ucb::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucb::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ::ucb::ContentImplHelperRef xChild = (*it);
        OUString aChildURL = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes / only a final slash. It's a child!
                rChildren.push_back(
                    ::webdav_ucp::Content::ContentRef(
                        static_cast< ::webdav_ucp::Content * >(
                            xChild.getBodyPtr() ) ) );
            }
        }
        ++it;
    }
}

//=========================================================================
void Content::insert( const Reference< XInputStream > & xInputStream,
                       sal_Bool bReplaceExisting,
                       const Reference< XCommandEnvironment >& Environment )
    throw( CommandAbortedException )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    if ( m_bTransient )
    {
        // Check, if all required properties are present.

        if ( m_aEscapedTitle.getLength() == 0 )
        {
               OSL_ENSURE( sal_False, "Content::insert - Title missing!" );
               throw CommandAbortedException();
        }

        // Assemble new content identifier...
        OUString aURL = getParentURL();
        if ( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ) )
            aURL += OUString::createFromAscii( "/" );

        aURL += m_aEscapedTitle;

        try
        {
            m_aResAccess.setURL( aURL );

            if ( m_bCollection )
                m_aResAccess.MKCOL( Environment );
            else
                  m_aResAccess.PUT( xInputStream, Environment );
          }
        catch ( DAVException const & )
        {
//          OSL_ENSURE( sal_False, "MKCOL/PUT: DAVException" );
            throw CommandAbortedException();
          }

        m_xIdentifier = new ::ucb::ContentIdentifier( m_xSMgr, aURL );

        aGuard.clear();
          inserted();
          m_bTransient = sal_False;
    }
    else
    {
        if ( !xInputStream.is() )
        {
               OSL_ENSURE( sal_False,
                "Content::insert - Persistent, but no new data stream!" );
            throw CommandAbortedException();
          }
/*
    Just try a PUT! (isFolder requires network access)

        if ( isFolder( Environment ) )
        {
               OSL_ENSURE( sal_False,
                        "Content::insert - Data stream + folder!" );
            throw CommandAbortedException();
        }
*/
        try
        {
            m_aResAccess.PUT( xInputStream, Environment );
          }
        catch ( DAVException const & )
        {
//          OSL_ENSURE( sal_False, "Content::insert - PUT: DAVException" );
            throw CommandAbortedException();
          }

    }
}

//=========================================================================
void Content::destroy( sal_Bool bDeletePhysical )
    throw( CommandAbortedException )
{
    // @@@ take care about bDeletePhysical -> trashcan support
    OUString aURL = m_xIdentifier->getContentIdentifier();

    Reference< XContent > xThis = this;

    deleted();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Process instanciated children...

    ::webdav_ucp::Content::ContentRefList aChildren;
    queryChildren( aChildren );

    ContentRefList::const_iterator it  = aChildren.begin();
    ContentRefList::const_iterator end = aChildren.end();

    while ( it != end )
    {
        (*it)->destroy( bDeletePhysical );
        ++it;
    }
}

//=========================================================================
sal_Bool Content::exchangeIdentity(
                        const Reference< XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Reference< XContent > xThis = this;

    // Already persistent?
    if ( m_bTransient )
    {
        OSL_ENSURE( sal_False, "Content::exchangeIdentity - Not persistent!" );
        return sal_False;
    }

    // Exchange own identitity.

    // Fail, if a content with given id already exists.
//  if ( !hasData( xNewId ) )
    {
        OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            // Process instanciated children...

            ContentRefList aChildren;
            queryChildren( aChildren );

            ContentRefList::const_iterator it  = aChildren.begin();
            ContentRefList::const_iterator end = aChildren.end();

            while ( it != end )
            {
                ContentRef xChild = (*it);

                // Create new content identifier for the child...
                Reference< XContentIdentifier > xOldChildId
                                                    = xChild->getIdentifier();
                OUString aOldChildURL = xOldChildId->getContentIdentifier();
                OUString aNewChildURL
                        = aOldChildURL.replaceAt(
                                        0,
                                        aOldURL.getLength(),
                                        xNewId->getContentIdentifier() );
                Reference< XContentIdentifier > xNewChildId
                    = new ::ucb::ContentIdentifier( m_xSMgr, aNewChildURL );

                if ( !xChild->exchangeIdentity( xNewChildId ) )
                    return sal_False;

                ++it;
            }
            return sal_True;
        }
    }

    OSL_ENSURE( sal_False,
                "Content::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return sal_False;
}

//=========================================================================
sal_Bool Content::isFolder( const Reference< XCommandEnvironment >& xEnv )
{
    if ( m_bTransient )
    {
        return m_bCollection;
    }
    else
    {
        std::vector< OUString > aPropNames;
        aPropNames.push_back( DAVProperties::RESOURCETYPE );

        std::vector< DAVResource > resources;
        try
        {
            m_aResAccess.PROPFIND( ZERO, aPropNames, resources, xEnv );
        }
        catch ( DAVException & )
        {
//          OSL_ENSURE( sal_False, "PROPFIND : DAVException" );
            return sal_False;
          }

        if ( resources.size() != 1 )
            return sal_False;

        ContentProperties aContentProperties( resources[ 0 ] );
        return ( aContentProperties.pIsFolder
                    && *aContentProperties.pIsFolder );
    }
}
