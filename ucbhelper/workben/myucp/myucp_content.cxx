/*************************************************************************
 *
 *  $RCSfile: myucp_content.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-11-17 15:38:10 $
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
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
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

// @@@ Adjust multi-include-protection-ifdef and header file name.
#ifndef _MYUCP_CONTENT_HXX
#include "myucp_content.hxx"
#endif
// @@@ Adjust multi-include-protection-ifdef and header file name.
#ifndef _MYUCP_PROVIDER_HXX
#include "myucp_provider.hxx"
#endif
// @@@ Adjust multi-include-protection-ifdef and header file name.
//#ifndef _MYUCP_RESULTSET_HXX
//#include "myucp_resultset.hxx"
//#endif

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

// @@@ Adjust namespace name.
using namespace myucp;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content( const Reference< XMultiServiceFactory >& rxSMgr,
                  ::ucb::ContentProviderImplHelper* pProvider,
                  const Reference< XContentIdentifier >& Identifier )
: ContentImplHelper( rxSMgr, pProvider, Identifier )
{
    // @@@ Fill m_aProps here or implement lazy evaluation logic for this.
    // m_aProps.aTitle       =
    // m_aprops.aContentType =
    // m_aProps.bIsDocument  =
    // m_aProps.bIsFolder    =
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
    Any aRet;

    // @@@ Add support for additional interfaces.
#if 0
      aRet = cppu::queryInterface( rType,
                                 static_cast< Xxxxxxxxx * >( this ) );
#endif

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
    // @@@ Add own interfaces.

    static OTypeCollection* pCollection = NULL;

    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
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
    // @@@ Adjust implementation name.
    return OUString::createFromAscii( "myucp_Content" );
}

//=========================================================================
// virtual
Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( RuntimeException )
{
    // @@@ Adjust macro name.
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
            = OUString::createFromAscii( MYUCP_CONTENT_SERVICE_NAME );
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
    // @@@ Adjust macro name ( def in myucp_provider.hxx ).
    return OUString::createFromAscii( MYUCP_CONTENT_TYPE );
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

        aRet <<= getPropertyValues( Properties );
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

        setPropertyValues( aProperties );
    }
    else if ( aCommand.Name.compareToAscii( "getPropertySetInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo();
    }
    else if ( aCommand.Name.compareToAscii( "getCommandInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getCommandInfo();
    }
#if 0
    else if ( aCommand.Name.compareToAscii( "open" ) == 0 )
    {
          OpenCommandArgument2 aOpenCommand;
          if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            VOS_ENSURE( sal_False,
                        "Content::execute - invalid parameter!" );
            throw CommandAbortedException();
        }

          if ( isFolder() )
        {
            //////////////////////////////////////////////////////////////
            // open command for a folder content
            //////////////////////////////////////////////////////////////

            Reference< XDynamicResultSet > xSet
                            = new DynamicResultSet( m_xSMgr,
                                                    this,
                                                    aOpenCommand,
                                                    Environment );
            aRet <<= xSet;
          }
          else
        {
            //////////////////////////////////////////////////////////////
            // open command for a document content
            //////////////////////////////////////////////////////////////

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
                  = Reference< XOutputStream >( aOpenCommand.Sink, UNO_QUERY );
            if ( xOut.is() )
              {
                // @@@ PUSH: write data into xOut
              }
            else
              {
                Reference< XActiveDataSink > xDataSink
                      = Reference< XActiveDataSink >(
                                            aOpenCommand.Sink, UNO_QUERY );
                  if ( xDataSink.is() )
                {
                      // @@@ PULL: wait for client read

                    Reference< XInputStream > xIn
                        = new // @@@ your XInputStream + XSeekable impl. object
                    xDataSink->setInputStream( xIn );
                }
                  else
                {
                      VOS_ENSURE( sal_False,
                                  "Content::execute - invalid parameter!" );
                      throw CommandAbortedException();
                }
              }
        }
    }
    else if ( aCommand.Name.compareToAscii( "insert" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //////////////////////////////////////////////////////////////////

        insert();
    }
    else if ( aCommand.Name.compareToAscii( "delete" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //////////////////////////////////////////////////////////////////

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;
        destroy( bDeletePhysical );

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet( sal_True );

        // Remove own and all childrens(!) persistent data.
//      removeData();
    }
#endif
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unknown command
        //////////////////////////////////////////////////////////////////

        VOS_ENSURE( sal_False,
                    "Content::execute - unknown command!" );
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
// Non-interface methods.
//
//=========================================================================

// virtual
OUString Content::getParentURL()
{
    OUString aURL = m_xIdentifier->getContentIdentifier();

    // @@@ Assemble URL of parent...

    return OUString();
}

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
                xRow->appendString ( rProp, rData.aContentType );
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

            // @@@ Process other properties supported directly.
#if 0
            else if ( rProp.Name.compareToAscii( "xxxxxx" ) == 0 )
            {
            }
#endif
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
            rData.aContentType );
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
                                const Sequence< Property >& rProperties )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return getPropertyValues( m_xSMgr,
                              rProperties,
                              m_aProps,
                              m_xProvider,
                              m_xIdentifier->getContentIdentifier() );
}

//=========================================================================
void Content::setPropertyValues( const Sequence< PropertyValue >& rValues )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Sequence< PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< OWeakObject * >( this );
    aEvent.Further        = sal_False;
//  aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//  aEvent.OldValue       =
//  aEvent.NewValue       =

    const PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    Reference< XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditonalPropSet = sal_False;

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
                if ( aNewValue != m_aProps.aTitle )
                {
                    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );
                    m_aProps.aTitle = aNewValue;

                    aGuard.clear();

                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue     = makeAny( m_aProps.aTitle );
                    aEvent.NewValue     = makeAny( aNewValue );

                    aChanges.getArray()[ nChanged ] = aEvent;
                    nChanged++;
                }
            }
        }

        // @@@ Process other properties supported directly.
#if 0
        else if ( rValue.Name.compareToAscii( "xxxxx" ) == 0 )
        {
        }
#endif
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
                xAdditionalPropSet = getAdditionalPropertySet( sal_False );
                bTriedToGetAdditonalPropSet = sal_True;
            }

            if ( xAdditionalPropSet.is() )
            {
                try
                {
                    Any aOldValue = xAdditionalPropSet->getPropertyValue(
                                                                rValue.Name );
                    xAdditionalPropSet->setPropertyValue(
                                                rValue.Name, rValue.Value );

                    if ( aOldValue != rValue.Value )
                    {
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

    if ( nChanged > 0 )
    {
        // @@@ Save changes.
//      storeData();

        aGuard.clear();
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }
}

#if 0
//=========================================================================
void Content::queryChildren( ContentRefList& rChildren )
{
    // @@@ Adapt method to your URL scheme...

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
                    ContentRef(
                        static_cast< Content * >( xChild.getBodyPtr() ) ) );
            }
        }
        ++it;
    }
}

//=========================================================================
void Content::insert()
    throw( CommandAbortedException )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    if ( m_aProps.xxxx == yyyyy )
    {
        VOS_ENSURE( sal_False, "Content::insert - property value missing!" );
        throw CommandAbortedException();
    }

    // Assemble new content identifier...

    Reference< XContentIdentifier > xId = ...;
    if ( !xId.is() )
        throw CommandAbortedException();

    // Fail, if a content with given id already exists.
    if ( hasData( xId ) )
        throw CommandAbortedException();

    m_xIdentifier = xId;

//  @@@
//  storeData();

    aGuard.clear();
    inserted();
}

//=========================================================================
void Content::destroy( sal_Bool bDeletePhysical )
    throw( CommandAbortedException )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    Reference< XContent > xThis = this;

    deleted();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Process instanciated children...

    ContentRefList aChildren;
    queryChildren( aChildren );

    ContentRefList::const_iterator it  = aChildren.begin();
    ContentRefList::const_iterator end = aChildren.end();

    while ( it != end )
    {
        (*it)->destroy( bDeletePhysical );
        ++it;
    }
}
#endif

