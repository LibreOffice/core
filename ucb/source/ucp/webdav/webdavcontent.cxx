/*************************************************************************
 *
 *  $RCSfile: webdavcontent.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kso $ $Date: 2001-04-04 15:26:12 $
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

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFRERURLEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
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

#ifndef _WEBDAV_SESSION_HXX
#include "DAVSession.hxx"
#endif

#include "DateTimeHelper.hxx"

#include "NeonUri.hxx"

#ifndef _WEBDAV_UCP_AUTHINTERACTION_HXX
#include "authinteraction.hxx"
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace cppu;
using namespace rtl;

using namespace webdav_ucp;

//=========================================================================

class AuthListener : public DAVAuthListener
{
  virtual int authenticate(const ::rtl::OUString & inRealm,
               const ::rtl::OUString & inHostName,
               ::rtl::OUStringBuffer & inUserName,
               ::rtl::OUStringBuffer & inPassWord,
               const com::sun::star::uno::Reference<
                   com::sun::star::ucb::XCommandEnvironment >& Environment);
};

//=========================================================================
// virtual
int AuthListener::authenticate( const ::rtl::OUString & inRealm,
                                   const ::rtl::OUString & inHostName,
                                   ::rtl::OUStringBuffer & inUserName,
                                   ::rtl::OUStringBuffer & inPassWord,
                                   const com::sun::star::uno::Reference<
                                       com::sun::star::ucb::XCommandEnvironment >&
                                        Environment )
{
    if ( Environment.is() )
    {
        Reference< com::sun::star::task::XInteractionHandler > xIH
              = Environment->getInteractionHandler();
        if ( xIH.is() )
        {
            vos::ORef< InteractionRequest_Impl > xRequest
                  = new InteractionRequest_Impl( inHostName,
                                               inRealm,
                                               inUserName,
                                               inPassWord );

              xIH->handle( xRequest.getBodyPtr() );

            vos::ORef< InteractionContinuation_Impl > xSelection
                = xRequest->getSelection();

            if ( xSelection.isValid() )
            {
                Reference< XInteractionAbort > xAbort(
                                        xSelection.getBodyPtr(), UNO_QUERY );
                if ( !xAbort.is() )
                {
                    // okay.
                    return 0;
                }
            }
        }
    }
    // abort.
    return -1;
}

AuthListener webdavAuthListener;

//=========================================================================

void ContentProperties::setValues(DAVResource& resource)
{
  // title
  NeonUri aURI( resource.uri );
  aTitle = aURI.GetPathBaseNameUnescaped();
  aEscapedTitle = aURI.GetPathBaseName();

  // other required properties (default values)
  bIsDocument  =  sal_True;
  bIsFolder  =  sal_False;

  std::vector< com::sun::star::beans::PropertyValue >::iterator it;
  for ( it=resource.properties.begin(); it!=resource.properties.end(); ++it) {
//    if ( (*it) == 0 ) {
//      continue;
//    }

    if ( (*it).Name.equals(DAVProperties::CREATIONDATE) )  {
      (*it).Value >>= creationdate;

      // Map the DAV:creationdate to UCP:DateCreated
      //
      DateTimeHelper::convert (creationdate, dateCreated);
    }
    else if ( (*it).Name.equals(DAVProperties::DISPLAYNAME) ) {
      (*it).Value >>= displayname;
    }
    else if ( (*it).Name.equals(DAVProperties::GETCONTENTLANGUAGE) ) {
      (*it).Value >>= getcontentlanguage;
    }
    else if ( (*it).Name.equals(DAVProperties::GETCONTENTLENGTH) ) {
      (*it).Value >>= getcontentlength;

      // Map the DAV:getcontentlength to UCP:Size
      //
      size = getcontentlength.toInt64 ();
    }
    else if ( (*it).Name.equals(DAVProperties::GETCONTENTTYPE) ) {
      (*it).Value >>= getcontenttype;
    }
    else if ( (*it).Name.equals(DAVProperties::GETETAG) ) {
      (*it).Value >>= getetag;
    }
    else if ( (*it).Name.equals(DAVProperties::GETLASTMODIFIED) ) {
      (*it).Value >>= getlastmodified;

      // Map the DAV:getlastmodified to UCP:DateModified
      //
      DateTimeHelper::convert (getlastmodified, dateModified);
    }
    else if ( (*it).Name.equals(DAVProperties::LOCKDISCOVERY) ) {
      (*it).Value >>= lockdiscovery;
    }
    else if ( (*it).Name.equals(DAVProperties::RESOURCETYPE) ) {
      (*it).Value >>= resourcetype;
      if (resourcetype.getLength()>0) {
        bIsDocument =  sal_False;
        bIsFolder   =  sal_True;
      }
    }
    else if ( (*it).Name.equals(DAVProperties::SOURCE) ) {
      (*it).Value >>= source;
    }
    else if ( (*it).Name.equals(DAVProperties::SUPPORTEDLOCK) ) {
      (*it).Value >>= supportedlock;
    }
  }
}


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
                    ::ucb::ContentProviderImplHelper* pProvider,
                    const Reference< XContentIdentifier >& Identifier,
                  DAVSessionFactory* pSessionFactory )
  throw ( ContentCreationException )
: ContentImplHelper( rxSMgr, pProvider, Identifier ),
  _transient( sal_False ),
  _upToDate( sal_False ),
#ifdef HTTP_SUPPORTED
  _davResource( sal_False )
#else
  _davResource( sal_True )
#endif
{
      if ( !initpath() )
    {
        VOS_ENSURE( sal_False, "invalid URL" );
          throw ContentCreationException();
      }

    try
      {
        // set the webdav session
          _pWebdavSession = pSessionFactory->createDAVSession(
                                Identifier->getContentIdentifier(), rxSMgr );
          _pWebdavSession->setServerAuthListener( &webdavAuthListener );
    }
    catch ( DAVException & )
    {
        VOS_ENSURE( sal_False, "createDAVSession : DAVException" );
          throw ContentCreationException();
      }
}


//=========================================================================
// ctr for content on an non-existing webdav resource
Content::Content( const Reference< XMultiServiceFactory >& rxSMgr,
                    ::ucb::ContentProviderImplHelper* pProvider,
                    const Reference< XContentIdentifier >& Identifier,
                    ::vos::ORef< DAVSession > pSession,
                    sal_Bool isCollection )
  throw ( ContentCreationException )
: ContentImplHelper( rxSMgr, pProvider, Identifier, sal_False ),
  _transient( sal_True ),
  _upToDate( sal_False ),
#ifdef HTTP_SUPPORTED
  _davResource( sal_False ),
#else
  _davResource( sal_True ),
#endif
  _pWebdavSession( pSession )
{
      if ( !initpath() )
    {
        VOS_ENSURE( sal_False, "invalid URL" );
          throw ContentCreationException();
      }

    // this is not an existing Content, let's set some props anyway ...

    if ( isCollection )
    {
        m_aProps.bIsDocument = sal_False;
        m_aProps.bIsFolder   = sal_True;
      }
      else
    {
        m_aProps.bIsDocument = sal_True;
        m_aProps.bIsFolder   = sal_False;
      }
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
    // Note: isFolder ma yrequire network activities! So call it only
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
    return OUString::createFromAscii( "WebDAV_ucp_Content" );
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
//  if ( isFolder( Reference< XCommandEnvironment >() ) )
    if ( m_aProps.bIsFolder )
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

    if ( aCommand.Name.compareToAscii( "getPropertyValues" ) == 0 )
    {
          //////////////////////////////////////////////////////////////////
          // getPropertyValues
          //////////////////////////////////////////////////////////////////

          Sequence< Property > Properties;
          if ( !( aCommand.Argument >>= Properties ) )
        {
              VOS_ENSURE( sal_False, "Wrong argument type!" );
              return Any();
        }

          aRet <<= getPropertyValues( Properties, Environment );
    }
      else if ( aCommand.Name.compareToAscii( "setPropertyValues" ) == 0 )
    {
          //////////////////////////////////////////////////////////////////
          // setPropertyValues
          //////////////////////////////////////////////////////////////////

          Sequence< PropertyValue > aProperties;
          if ( !( aCommand.Argument >>= aProperties ) )
        {
              VOS_ENSURE( sal_False, "Wrong argument type!" );
              return Any();
        }

          if ( !aProperties.getLength() )
        {
              VOS_ENSURE( sal_False, "No properties!" );
              return Any();
        }

          setPropertyValues( aProperties, Environment );
    }
      else if ( aCommand.Name.compareToAscii( "getPropertySetInfo" ) == 0 )
       {
          //////////////////////////////////////////////////////////////////
          // getPropertySetInfo
          //////////////////////////////////////////////////////////////////

          // Note: Implemented by base class.
          aRet <<= getPropertySetInfo( Environment, sal_False );
       }
      else if ( aCommand.Name.compareToAscii( "getCommandInfo" ) == 0 )
       {
          //////////////////////////////////////////////////////////////////
          // getCommandInfo
          //////////////////////////////////////////////////////////////////

          // Note: Implemented by base class.
          aRet <<= getCommandInfo( Environment, sal_False );
       }
    else if ( aCommand.Name.compareToAscii( "open" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
          // open command
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
                          _pWebdavSession->GET( _path, xOut, Environment );
                    }
                    catch ( DAVException & )
                    {
                          VOS_ENSURE( sal_False, "GET : DAVException" );
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
                                = _pWebdavSession->GET( _path, Environment );
                            xDataSink->setInputStream( xIn );
                          }
                        catch ( DAVException &)
                        {
                            VOS_ENSURE( sal_False, "GET : DAVException" );
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
              VOS_ENSURE( sal_False, "Content::execute - invalid parameter!" );
              throw CommandAbortedException();
        }
    }
  else if ( aCommand.Name.compareToAscii( "insert" ) == 0 )
    {
      //////////////////////////////////////////////////////////////////
      // insert
      //////////////////////////////////////////////////////////////////

      InsertCommandArgument arg;
      aCommand.Argument >>= arg;
      insert(arg.Data,arg.ReplaceExisting, Environment);
    }
  else if (
       ( aCommand.Name.compareToAscii( "delete" ) == 0 )
       || ( aCommand.Name.compareToAscii( "DELETE" ) == 0 )
       )
    {
      //////////////////////////////////////////////////////////////////
      // delete
      //////////////////////////////////////////////////////////////////

      sal_Bool bDeletePhysical = sal_False;
      aCommand.Argument >>= bDeletePhysical;

//  KSO: Ignore parameter and destroy the content, if you don't support
//       putting objects into trashcan. ( Since we do not have a trash can
//       service yet (src603), you actually have no other coice. )
//      if (bDeletePhysical){
    try {
      _pWebdavSession->DESTROY (_path, Environment);
    } catch (DAVException e) {
      VOS_ENSURE( sal_False, "DELETE : DAVException" );
      throw CommandAbortedException();
    }
//      }
      destroy( bDeletePhysical );

      // Remove own and all children's Additional Core Properties.
      removeAdditionalPropertySet( sal_True );
    }
  else if (aCommand.Name.compareToAscii( "transfer" ) == 0)
      {
        // This command transfers (copies/moves) an object from one location
        // to another. It must be executed at the folder content representing
        // the destination of the transfer operation.

        // This is opposite of the WebDAV COPY/MOVE methods
        // They operate on the source not the target
        //
        //  COPY /container/index.html HTTP/1.1
        //  Host: www.foo.bar
        //  Desitnation: http://www.foo.bar/othercontainer
        //  Depth: Infinity
        //
        // @@@@ Does the TransferInfo::SourceURL have
        //      the same scheme as this content
        //
        // @@@@ Does the TransferInfo::SourceURL have
        //      the same host
        //
        // If the latter is true we can use the same
        // DAV session. If not we either have to create
        // a temporary new DAV session for the host source
        // in question or obtain content for the source
        //

        // DELETE, COPY and MOVE are not atomic operations,
        // unless the 'overwrite' option is set to false.
        // Thus a WebDAV server will return a multi-status
        // for the method in question saying what resource
        // failed to be deleted, copied or moved.
        // It is assumed that the operation is completely
        // successful since the DAVSession interface does
        // not yet return multi-status responses to these
        // commands
        //

        if ( !isFolder( Environment ) )
        {
            throw CommandAbortedException();
        }

        TransferInfo transferArgs;
        aCommand.Argument >>= transferArgs;

        if (transferArgs.NameClash == NameClash::KEEP ||
            transferArgs.NameClash == NameClash::RENAME )
        {
            throw CommandAbortedException();

            // @@@ RENAME and KEEP are not directly implemented
            // by WebDAV methods
        }

        // @@@ This implementation of 'transfer' only works
        //     if the source and target are the same scheme
        //     and the same host
        //
        //     In addition just like 'delete' it assumes that
        //     if the DAV method succeeded then all resources
        //     were deleted, which is not the case with a
        //     multi-status response
        //
        //     Overwrite is always set to false regardless of
        //     NameClass value


        try
        {
            NeonUri sourceURI (transferArgs.SourceURL);
            NeonUri targetURI (m_xIdentifier->getContentIdentifier());

#ifdef HTTP_SUPPORTED
            // Check scheme
            //
            const OUString aScheme = sourceURI.GetScheme();
            if ( !aScheme.equalsIgnoreCase( OUString::createFromAscii(
                                                WEBDAV_URL_SCHEME ) ) &&
                  !aScheme.equalsIgnoreCase( OUString::createFromAscii(
                                                HTTP_URL_SCHEME ) ) &&
                  !aScheme.equalsIgnoreCase( OUString::createFromAscii(
                                                HTTPS_URL_SCHEME ) ) )
                throw InteractiveBadTransferURLException();
#else
            // Check scheme
            //
            const OUString aScheme = sourceURI.GetScheme();
            if ( !aScheme.equalsIgnoreCase( OUString::createFromAscii(
                                                WEBDAV_URL_SCHEME ) ) )
                throw InteractiveBadTransferURLException();
#endif
            sourceURI.SetScheme (OUString::createFromAscii ("http"));
            targetURI.SetScheme (OUString::createFromAscii ("http"));

            // Check for same host
            //
            if (sourceURI.GetHost ().getLength () &&
                sourceURI.GetHost () != targetURI.GetHost ())
            {
                throw InteractiveBadTransferURLException();
            }

            if (!transferArgs.NewTitle.getLength ())
            {
                transferArgs.NewTitle = sourceURI.GetPathBaseNameUnescaped();
            }

            if (transferArgs.NewTitle.compareToAscii ("/") == 0)
            {
                throw CommandAbortedException();
            }

            targetURI.AppendPath (transferArgs.NewTitle);

            OUString aTargetURL = m_xIdentifier->getContentIdentifier();
            if ( ( aTargetURL.lastIndexOf( '/' ) + 1 ) != aTargetURL.getLength() )
                aTargetURL += OUString::createFromAscii( "/" );

            aTargetURL += transferArgs.NewTitle;

            Reference< XContentIdentifier > xTargetId
                = new ::ucb::ContentIdentifier( m_xSMgr, aTargetURL );

            Reference< XContentIdentifier > xId
                = new ::ucb::ContentIdentifier( m_xSMgr,
                                                transferArgs.SourceURL );

            if (transferArgs.MoveData == sal_True)
            {
                    // Note: The static cast is okay here, because its sure that
                //       m_xProvider is always the WebDAVContentProvider.
                vos::ORef< Content > xSource
                    = static_cast< Content * >(
                        m_xProvider->queryContent( xId ).get() );

                _pWebdavSession->MOVE (sourceURI.GetPath (), targetURI.GetURI (), Environment);
                // @@@ Should check for resources that could not be moved
                //     (due to source access or target overwrite) and send
                //     this information through the interaction handler.

                // @@@ Existing content should be checked to see if it needs
                //     to be deleted at the source

                // @@@ Existing content should be checked to see if it has
                //     been overwritten at the target

                if ( xSource.isValid() )
                {
                    // Destroy source content
                    xSource->destroy( sal_True );
                }

                // Rename own and all children's Additional Core Properties.
                renameAdditionalPropertySet( xId->getContentIdentifier(),
                                             xTargetId->getContentIdentifier(),
                                             sal_True );
            }
            else
            {
                _pWebdavSession->COPY (sourceURI.GetPath (), targetURI.GetURI (), Environment);
                // @@@ Should check for resources that could not be copied
                //     (due to source access or target overwrite) and send
                //     this information through the interaction handler

                // @@@ Existing content should be checked to see if it has
                //     been overwritten at the target

                // Copy own and all children's Additional Core Properties.
                copyAdditionalPropertySet( xId->getContentIdentifier(),
                                           xTargetId->getContentIdentifier(),
                                           sal_True );
            }

            // Note: The static cast is okay here, because its sure that
            //       m_xProvider is always the WebDAVContentProvider.
            vos::ORef< Content > xTarget
                = static_cast< Content * >(
                        m_xProvider->queryContent( xTargetId ).get() );

            if ( xTarget.isValid() )
            {
                // Announce transfered content in its new folder.
                xTarget->inserted();
            }
        }
        catch (DAVException&)
        {
            throw CommandAbortedException();
        }
    }
    /*
  else if ( aCommand.Name.compareToAscii( "COPY" ) == 0 )
    {
      //////////////////////////////////////////////////////////////////
      // COPY
      //////////////////////////////////////////////////////////////////

      OUString destination;
      aCommand.Argument >>= destination;
      try {
    _pWebdavSession->COPY (_path, destination, Environment);
      } catch (DAVException e) {
    VOS_ENSURE( sal_False, "COPY : DAVException" );
    throw CommandAbortedException();
      }
      // synchronize destination ?
    }
  else if ( aCommand.Name.compareToAscii( "MOVE" ) == 0 )
    {
      //////////////////////////////////////////////////////////////////
      // MOVE
      //////////////////////////////////////////////////////////////////

      OUString destination;
      aCommand.Argument >>= destination;
      try {
    _pWebdavSession->MOVE (_path, destination, Environment);
      } catch (DAVException e) {
    VOS_ENSURE( sal_False, "MOVE : DAVException" );
    throw CommandAbortedException();
      }
      // synchronize destination ?
    }
    */
  else
    {
      //////////////////////////////////////////////////////////////////
      // Unsupported command
      //////////////////////////////////////////////////////////////////

      VOS_ENSURE( sal_False, "Content::execute - unsupported command!" );
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

        Sequence< Property > aDocProps( 1 );
        aDocProps.getArray()[ 0 ] = Property(
                            OUString::createFromAscii( "Title" ),
                            -1,
                            getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                            PropertyAttribute::MAYBEVOID
                            | PropertyAttribute::BOUND );
        aSeq.getArray()[ 0 ].Properties = aDocProps;

        // folder.
        aSeq.getArray()[ 1 ].Type
            = OUString::createFromAscii( WEBDAV_COLLECTION_TYPE );
        aSeq.getArray()[ 1 ].Attributes = ContentInfoAttribute::KIND_FOLDER;

        Sequence< Property > aFolderProps( 1 );
        aFolderProps.getArray()[ 0 ] = Property(
                            OUString::createFromAscii( "Title" ),
                            -1,
                            getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                            PropertyAttribute::MAYBEVOID
                            | PropertyAttribute::BOUND );
        aSeq.getArray()[ 1 ].Properties = aFolderProps;
        return aSeq;
    }
/*
    else
    {
        VOS_ENSURE( sal_False,
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

      if ( ( Info.Type.compareToAscii( WEBDAV_COLLECTION_TYPE ) != 0 )
       &&
       ( Info.Type.compareToAscii( WEBDAV_CONTENT_TYPE ) != 0 ) )
        return Reference< XContent >();


      OUString aURL = m_xIdentifier->getContentIdentifier();

      VOS_ENSURE( aURL.getLength() > 0,
          "WebdavContent::createNewContent - empty identifier!" );

      if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
        aURL += OUString::createFromAscii( "/" );

      sal_Bool isCollection;
      if ( Info.Type.compareToAscii( WEBDAV_COLLECTION_TYPE ) == 0 ) {
        aURL += OUString::createFromAscii( "New_Collection" );
        isCollection = sal_True;
      }
      else {
        aURL += OUString::createFromAscii( "New_Content" );
        isCollection = sal_False;
      }

      Reference< XContentIdentifier > xId( new ::ucb::ContentIdentifier( m_xSMgr, aURL ) );

      // create the local content
      try {
        return new ::webdav_ucp::Content( m_xSMgr,
                                          m_xProvider.getBodyPtr(),
                                            xId,
                                            _pWebdavSession,
                                            isCollection);
      }
      catch (ContentCreationException e ) {
        return Reference< XContent >();
      }
    }
/*
  else
    {
      VOS_ENSURE( sal_False, "createNewContent called on non-folder object!" );
      return Reference< XContent >();
    }
*/
}

//=========================================================================
// virtual
OUString Content::getParentURL()
{
    OUString aURL = m_xIdentifier->getContentIdentifier();

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos == ( aURL.getLength() - 1 ) )
    {
        // Trailing slash found. Skip.
        nPos = aURL.lastIndexOf( '/', nPos );
    }

    if ( nPos != -1 )
    {
        OUString aParentURL = aURL.copy( 0, nPos );
        return aParentURL;
    }

    return OUString();
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

      if ( rProp.Name.compareToAscii( "ContentType" ) == 0 )
       {
            if ( rData.bIsFolder )
                  xRow->appendString( rProp, OUString::createFromAscii(
                                                      WEBDAV_COLLECTION_TYPE ) );
            else
                  xRow->appendString( rProp, OUString::createFromAscii(
                                                      WEBDAV_CONTENT_TYPE ) );
        }
      else if ( rProp.Name.compareToAscii( "Title" ) == 0 )
        {
          xRow->appendString ( rProp, rData.aTitle );
        }
      else if ( rProp.Name.compareToAscii( "IsDocument" ) == 0 )
        {
          xRow->appendBoolean( rProp, rData.bIsDocument );
        }
      else if ( rProp.Name.compareToAscii( "IsFolder" ) == 0 )
        {
          xRow->appendBoolean( rProp, rData.bIsFolder );
        }
      else if ( rProp.Name.compareToAscii( "Size" ) == 0 )
          {
          xRow->appendLong( rProp, rData.size );
        }
      else if ( rProp.Name.compareToAscii( "DateCreated" ) == 0 )
          {
          xRow->appendTimestamp( rProp, rData.dateCreated );
        }
      else if ( rProp.Name.compareToAscii( "DateModified" ) == 0 )
          {
          xRow->appendTimestamp( rProp, rData.dateModified );
        }
      else if ( rProp.Name.compareToAscii( "MediaType" ) == 0 ) {
          xRow->appendString( rProp, rData.getcontenttype );
        }
      else if ( rProp.Name.equals(DAVProperties::CREATIONDATE) ) {
        xRow->appendString( rProp, rData.creationdate );
      }
      else if ( rProp.Name.equals(DAVProperties::DISPLAYNAME) ) {
        xRow->appendString( rProp, rData.displayname );
      }
      else if ( rProp.Name.equals(DAVProperties::GETCONTENTLANGUAGE) ) {
        xRow->appendString( rProp, rData.getcontentlanguage );
      }
      else if ( rProp.Name.equals(DAVProperties::GETCONTENTLENGTH) ) {
        xRow->appendString( rProp, rData.getcontentlength );
      }
      else if ( rProp.Name.equals(DAVProperties::GETCONTENTTYPE) ) {
        xRow->appendString( rProp, rData.getcontenttype );
      }
      else if ( rProp.Name.equals(DAVProperties::GETETAG) ) {
        xRow->appendString( rProp, rData.getetag );
      }
      else if ( rProp.Name.equals(DAVProperties::GETLASTMODIFIED) ) {
        xRow->appendString( rProp, rData.getlastmodified );
      }
      else if ( rProp.Name.equals(DAVProperties::LOCKDISCOVERY) ) {
        xRow->appendString( rProp, rData.lockdiscovery );
      }
      else if ( rProp.Name.equals(DAVProperties::RESOURCETYPE) ) {
        xRow->appendString( rProp, rData.resourcetype );
      }
      else if ( rProp.Name.equals(DAVProperties::SOURCE) ) {
        xRow->appendString( rProp, rData.source );
      }
      else if ( rProp.Name.equals(DAVProperties::SUPPORTEDLOCK) ) {
        xRow->appendString( rProp, rData.supportedlock );
      }
      else
        {
                // @@@ Note: If your data source supports adding/removing
                //     properties, you should implement the interface
                //     XPropertyContainer by yourself and supply your own
                //     logic here. The base class uses the service
                //     "com.sun.star.ucb.Store" to maintain Additional Core
                //     properties. But using server functionality is preferred!

                // Not a Core Property! Maybe it's an Additional Core Property?!

          if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
        {
          xAdditionalPropSet
            = Reference< XPropertySet >(
                        rProvider->getAdditionalPropertySet( rContentId,
                                             sal_False ),
                        UNO_QUERY );
          bTriedToGetAdditonalPropSet = sal_True;
        }

          if ( xAdditionalPropSet.is() )
        {
          if ( !xRow->appendPropertySetValue(
                             xAdditionalPropSet,
                             rProp ) )
            {
              // Append empty entry.
              xRow->appendVoid( rProp );
            }
        }
          else
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
      xRow->appendString (
              Property( OUString::createFromAscii( "ContentType" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.bIsFolder ? OUString::createFromAscii(
                                                WEBDAV_COLLECTION_TYPE )
                              : OUString::createFromAscii(
                                                  WEBDAV_CONTENT_TYPE ) );
      xRow->appendString (
              Property( OUString::createFromAscii( "Title" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND ),
              rData.aTitle );
      xRow->appendBoolean(
              Property( OUString::createFromAscii( "IsDocument" ),
                    -1,
                    getCppuBooleanType(),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.bIsDocument );
      xRow->appendBoolean(
              Property( OUString::createFromAscii( "IsFolder" ),
                    -1,
                    getCppuBooleanType(),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.bIsFolder );
      xRow->appendLong(
              Property( OUString::createFromAscii( "Size" ),
                    -1,
                    getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.size );
      xRow->appendTimestamp(
              Property( OUString::createFromAscii( "DateCreated" ),
                    -1,
                    getCppuType( static_cast< const DateTime * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.dateCreated );
      xRow->appendTimestamp(
              Property( OUString::createFromAscii( "DateModified" ),
                    -1,
                    getCppuType( static_cast< const DateTime * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.dateModified );
      xRow->appendString (
              Property( OUString::createFromAscii( "MediaType" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.getcontenttype );
      xRow->appendString (
              Property(  DAVProperties::CREATIONDATE ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.creationdate );
      xRow->appendString (
              Property(  DAVProperties::DISPLAYNAME ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.displayname );
      xRow->appendString (
              Property(  DAVProperties::GETCONTENTLANGUAGE ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.getcontentlanguage );
      xRow->appendString (
              Property(  DAVProperties::GETCONTENTLENGTH ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.getcontentlength );
      xRow->appendString (
              Property(  DAVProperties::GETCONTENTTYPE ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.getcontenttype );
      xRow->appendString (
              Property(  DAVProperties::GETETAG ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.getetag );
      xRow->appendString (
              Property(  DAVProperties::GETLASTMODIFIED ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.getlastmodified );
      xRow->appendString (
              Property(  DAVProperties::LOCKDISCOVERY ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.lockdiscovery );
      xRow->appendString (
              Property(  DAVProperties::RESOURCETYPE ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.resourcetype );
      xRow->appendString (
              Property(  DAVProperties::SOURCE ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.source );
      xRow->appendString (
              Property(  DAVProperties::SUPPORTEDLOCK ,
                     -1,
                     getCppuType( static_cast< const OUString * >( 0 ) ),
                     PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
              rData.supportedlock );




      // @@@ Append other properties supported directly.

      // @@@ Note: If your data source supports adding/removing
      //     properties, you should implement the interface
      //     XPropertyContainer by yourself and supply your own
      //     logic here. The base class uses the service
      //     "com.sun.star.ucb.Store" to maintain Additional Core
      //     properties. But using server functionality is preferred!

      // Append all Additional Core Properties.

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

    if ( update( rProperties, xEnv ) )
        return getPropertyValues( m_xSMgr,
                                      rProperties,
                                      m_aProps,
                                      m_xProvider,
                                      m_xIdentifier->getContentIdentifier() );
    return Reference< XRow >();
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
  //    aEvent.PropertyName   =
  aEvent.PropertyHandle = -1;
  //    aEvent.OldValue       =
  //    aEvent.NewValue       =

  const PropertyValue* pValues = rValues.getConstArray();
  sal_Int32 nCount = rValues.getLength();

  Reference< XPersistentPropertySet > xAdditionalPropSet;
  sal_Bool bTriedToGetAdditonalPropSet = sal_False;

  sal_Bool bExchange = sal_False;

  for ( sal_Int32 n = 0; n < nCount; ++n )
  {
      const PropertyValue& rValue = pValues[ n ];

      if ( rValue.Name.compareToAscii( "ContentType" ) == 0 )
    {
      // Read-only property!
    }
      else if ( rValue.Name.compareToAscii( "IsDocument" ) == 0 )
    {
      // Read-only property!
    }
      else if ( rValue.Name.compareToAscii( "IsFolder" ) == 0 )
    {
      // Read-only property!
    }
      else if ( rValue.Name.compareToAscii( "Title" ) == 0 )
    {
        OUString aNewValue;
        if ( rValue.Value >>= aNewValue )
          {
            // No empty titles!
            if ( aNewValue.getLength() > 0 )
            {
                if ( aNewValue != m_aProps.aTitle )
                  {
                    osl::Guard< osl::Mutex > aGuard( m_aMutex );

                    // modified title -> modified URL -> exchange !
                    if ( !_transient )
                        bExchange = sal_True;

                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue     = makeAny( m_aProps.aTitle );
                    aEvent.NewValue     = makeAny( aNewValue );

                    aChanges.getArray()[ nChanged ] = aEvent;

                    m_aProps.aTitle = aNewValue;
                    m_aProps.aEscapedTitle
                        = NeonUri::escapeSegment( aNewValue );
                    nChanged++;
                  }
            }
        }
    }
      else if ( rValue.Name.compareToAscii( "MediaType" ) == 0 )
    {
      // Read-only property!
    }
    else
    {
        // Not a Core Property! Maybe it's an Additional Core Property?!

        if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
        {
            xAdditionalPropSet = getAdditionalPropertySet( sal_False );
            bTriedToGetAdditonalPropSet = sal_True;
        }

        if ( xAdditionalPropSet.is() )
        {
            try
            {
                Any aOldValue = xAdditionalPropSet->getPropertyValue(
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

  // @@@ What, if exchange fails??? Rollback of Title prop? Old title is
  //     contained in aChanges...
  if ( bExchange )
  {
    // Assemble new content identifier...

      OUString aNewURL = getParentURL();
      if ( aNewURL.lastIndexOf( '/' ) != ( aNewURL.getLength() - 1 ) )
        aNewURL += OUString::createFromAscii( "/" );

      aNewURL += m_aProps.aEscapedTitle;

    Reference< XContentIdentifier > xNewId
                        = new ::ucb::ContentIdentifier( m_xSMgr, aNewURL );
      Reference< XContentIdentifier > xOldId = m_xIdentifier;

    try
    {
        NeonUri sourceURI (xOldId->getContentIdentifier());
        NeonUri targetURI (xNewId->getContentIdentifier());
        targetURI.SetScheme (OUString::createFromAscii ("http"));

        _pWebdavSession->MOVE (sourceURI.GetPath (), targetURI.GetURI (), xEnv);
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
            // Adapt Additional Core Properties.
            renameAdditionalPropertySet( xOldId->getContentIdentifier(),
                                         xNewId->getContentIdentifier(),
                                         sal_True );
            initpath();
        }
    }
    catch (DAVException&)
    {
      VOS_ENSURE( sal_False, "Content::setPropertyValues - MOVE failed!" );
    }
  }

  if ( nChanged > 0 )
    {
      aGuard.clear();
      aChanges.realloc( nChanged );
      notifyPropertiesChange( aChanges );
    }
}

//=========================================================================
void Content::queryChildren(ContentRefList& rChildren )
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
void Content::insert(Reference<XInputStream> xInputStream,
             sal_Bool bReplaceExisting,
             const Reference< XCommandEnvironment >& Environment)
    throw( CommandAbortedException )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    // aTitle
    // IsFolder

    if ( m_aProps.aTitle.getLength() == 0 )
       {
        VOS_ENSURE( sal_False, "Content::insert - property value missing!" );
          throw CommandAbortedException();
       }

      // Assemble new content identifier...

      OUString aURL = getParentURL();
      if ( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ) )
        aURL += OUString::createFromAscii( "/" );

      aURL += m_aProps.aEscapedTitle;
      m_xIdentifier = new ::ucb::ContentIdentifier( m_xSMgr, aURL );

    if ( !initpath() )
          throw CommandAbortedException();

      try
    {
        if ( isFolder( Environment ) )
          {
            _pWebdavSession->MKCOL(_path, Environment);
          }
        else
          {
              _pWebdavSession->PUT(_path, xInputStream, Environment);
          }

      }
    catch ( DAVException& )
    {
        if ( isFolder( Environment ) )
              VOS_ENSURE( sal_False, "MKCOL : DAVException\n" );
        else
              VOS_ENSURE( sal_False, "PUT : DAVException\n" );
        throw CommandAbortedException();
      }

//  if ( ! update(Environment) )
//    throw CommandAbortedException();

    aGuard.clear();
      inserted();
      _transient = sal_False;
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
    if ( _transient )
    {
        VOS_ENSURE( sal_False, "Content::exchangeIdentity - Not persistent!" );
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

    VOS_ENSURE( sal_False,
                "Content::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return sal_False;
}

//=========================================================================
// init dav server and path
sal_Bool Content::initpath()
{
    const OUString aURL = m_xIdentifier->getContentIdentifier();

      // scheme://user@host:port/path
      sal_Int32 nPos = aURL.indexOf( '/' );
      if ( nPos == -1 )
      {
        VOS_ENSURE( sal_False, "No '/' in URL " );
        return sal_False;
      }

      nPos = aURL.indexOf( '/', nPos + 1 );
      if ( nPos == -1 )
      {
        VOS_ENSURE( sal_False, "No second '/' in URL " );
        return sal_False;
      }

      nPos = aURL.indexOf( '/', nPos + 1 );
      if ( nPos == -1 )
        _path = OUString::createFromAscii( "/" );
    else
        _path = aURL.copy( nPos );

    nPos = aURL.lastIndexOf( '/' );
      sal_Int32 nEnd = aURL.getLength();

      if ( nPos == nEnd - 1 )
    {
        // Trailing slash found. Skip.
        nPos = aURL.lastIndexOf( '/', nPos );
        nEnd--;
      }

      if ( nPos == -1 )
    {
        VOS_ENSURE( sal_False, "Invalid URL " );
        return sal_False;
    }

    NeonUri aURI( aURL );
       m_aProps.aTitle = aURI.GetPathBaseNameUnescaped();
       m_aProps.aEscapedTitle = aURI.GetPathBaseName();

    return sal_True;
}

//=========================================================================
sal_Bool Content::update( const Sequence< Property >& rProperties,
                           const Reference< XCommandEnvironment >& Environment )
{
    if ( !_upToDate && !_transient )
    {
        // Require requested resources network access???

        sal_Bool bDoIt = sal_False;
        sal_Int32 nCount = rProperties.getLength();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const OUString& rPropName = rProperties[ n ].Name;

            if ( ( rPropName == DAVProperties::CREATIONDATE ) ||
                 ( rPropName == DAVProperties::DISPLAYNAME ) ||
                 ( rPropName == DAVProperties::GETCONTENTLANGUAGE ) ||
                 ( rPropName == DAVProperties::GETCONTENTLENGTH ) ||
                 ( rPropName == DAVProperties::GETCONTENTTYPE ) ||
                 ( rPropName == DAVProperties::GETETAG ) ||
                 ( rPropName == DAVProperties::GETLASTMODIFIED ) ||
                 ( rPropName == DAVProperties::LOCKDISCOVERY ) ||
                 ( rPropName == DAVProperties::RESOURCETYPE ) ||
                 ( rPropName == DAVProperties::SOURCE ) ||
//               ( rPropName == DAVProperties::SUPPORTEDLOCK ) ||
                   ( rPropName.compareToAscii( "ContentType" ) == 0 ) ||
                   ( rPropName.compareToAscii( "IsDocument" ) == 0 ) ||
                   ( rPropName.compareToAscii( "IsFolder" ) == 0 ) ||
                   ( rPropName.compareToAscii( "Size" ) == 0 ) ||
                   ( rPropName.compareToAscii( "DateCreated" ) == 0 ) ||
                   ( rPropName.compareToAscii( "DateModified" ) == 0 ) ||
                   ( rPropName.compareToAscii( "MediaType" ) == 0 ) )
            {
                bDoIt = sal_True;
                break;
            }
        }

        if ( bDoIt )
        {
            try
            {
                DAVCapabilities caps;
                   _pWebdavSession->OPTIONS( _path, caps, Environment );
                _davResource = caps.class1;
            }
            catch ( DAVException & )
            {
//              VOS_ENSURE( sal_False, "OPTIONS : DAVException" );
                _davResource = sal_False;
              }

            if ( _davResource )
            {
                std::vector< DAVResource > resources;
                try
                {
                       // properties initialization
                       std::vector< OUString > propertyNames;
                       propertyNames.push_back( DAVProperties::CREATIONDATE );
                       propertyNames.push_back( DAVProperties::DISPLAYNAME );
                       propertyNames.push_back( DAVProperties::GETCONTENTLANGUAGE );
                       propertyNames.push_back( DAVProperties::GETCONTENTLENGTH );
                       propertyNames.push_back( DAVProperties::GETCONTENTTYPE );
                       propertyNames.push_back( DAVProperties::GETETAG );
                       propertyNames.push_back( DAVProperties::GETLASTMODIFIED );
                       propertyNames.push_back( DAVProperties::LOCKDISCOVERY );
                       propertyNames.push_back( DAVProperties::RESOURCETYPE );
                       propertyNames.push_back( DAVProperties::SOURCE );
                       //   propertyNames.push_back( DAVProperties::SUPPORTEDLOCK );

                       _pWebdavSession->PROPFIND(
                            _path, ZERO, propertyNames, resources, Environment );
                }
                catch ( DAVException & )
                {
    //              VOS_ENSURE( sal_False, "PROPFIND : DAVException" );
                       return sal_False;
                }

                if ( resources.size() != 1 )
                       return sal_False;

                m_aProps.setValues( resources[ 0 ] );
            }
            _upToDate = sal_True;
        }
    }
      return sal_True;
}

//=========================================================================
sal_Bool Content::isFolder(
                const Reference< XCommandEnvironment >& Environment )
{
    if ( !_upToDate && !_transient )
    {
        // We need network access to obtain whether we are a folder.
        Sequence< Property > aProps( 1 );
        aProps[ 0 ].Name = OUString::createFromAscii( "IsFolder" );
        update( aProps, Environment );
    }
    return m_aProps.bIsFolder;
}
