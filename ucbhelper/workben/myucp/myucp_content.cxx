/*************************************************************************
 *
 *  $RCSfile: myucp_content.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:24:21 $
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALACCESSEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalAccessException.hpp>
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
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#include <ucbhelper/propertyvalueset.hxx>
#endif
#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#include <ucbhelper/cancelcommandexecution.hxx>
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

using namespace com::sun;
using namespace com::sun::star;

// @@@ Adjust namespace name.
using namespace myucp;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
                  ::ucb::ContentProviderImplHelper* pProvider,
                  const uno::Reference< star::ucb::XContentIdentifier >& Identifier )
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
    throw( uno::RuntimeException )
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release()
    throw( uno::RuntimeException )
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet;

    // @@@ Add support for additional interfaces.
#if 0
      aRet = cppu::queryInterface( rType,
                                 static_cast< yyy::Xxxxxxxxx * >( this ) );
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
uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException )
{
    // @@@ Add own interfaces.

    static cppu::OTypeCollection* pCollection = 0;

    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
          {
            static cppu::OTypeCollection aCollection(
                CPPU_TYPE_REF( lang::XTypeProvider ),
                CPPU_TYPE_REF( lang::XServiceInfo ),
                CPPU_TYPE_REF( lang::XComponent ),
                CPPU_TYPE_REF( star::ucb::XContent ),
                CPPU_TYPE_REF( star::ucb::XCommandProcessor ),
                CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                CPPU_TYPE_REF( star::ucb::XCommandInfoChangeNotifier ),
                CPPU_TYPE_REF( beans::XPropertyContainer ),
                CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                CPPU_TYPE_REF( container::XChild ) );
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
rtl::OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException )
{
    // @@@ Adjust implementation name. Keep the prefix "com.sun.star.comp."!
    return rtl::OUString::createFromAscii( "com.sun.star.comp.myucp.Content" );
}

//=========================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    // @@@ Adjust macro name.
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
            = rtl::OUString::createFromAscii( MYUCP_CONTENT_SERVICE_NAME );
    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getContentType()
    throw( uno::RuntimeException )
{
    // @@@ Adjust macro name ( def in myucp_provider.hxx ).
    return rtl::OUString::createFromAscii( MYUCP_CONTENT_TYPE );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
uno::Any SAL_CALL Content::execute(
        const star::ucb::Command& aCommand,
        sal_Int32 CommandId,
        const uno::Reference< star::ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           star::ucb::CommandAbortedException,
           uno::RuntimeException )
{
    uno::Any aRet;

    if ( aCommand.Name.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( "getPropertyValues" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertyValues
        //////////////////////////////////////////////////////////////////

        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= getPropertyValues( Properties, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "setPropertyValues" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // setPropertyValues
        //////////////////////////////////////////////////////////////////

        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            OSL_ENSURE( sal_False, "No properties!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getPropertySetInfo" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment );
    }
#if 0
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "open" ) ) )
    {
        star::ucb::OpenCommandArgument2 aOpenCommand;
          if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        sal_Bool bOpenFolder =
            ( ( aOpenCommand.Mode == star::ucb::OpenMode::ALL ) ||
              ( aOpenCommand.Mode == star::ucb::OpenMode::FOLDERS ) ||
              ( aOpenCommand.Mode == star::ucb::OpenMode::DOCUMENTS ) );

        if ( bOpenFolder && isFolder( Environment ) )
        {
            // open as folder - return result set

            uno::Reference< star::ucb::XDynamicResultSet > xSet
                            = new DynamicResultSet( m_xSMgr,
                                                    this,
                                                    aOpenCommand,
                                                    Environment );
            aRet <<= xSet;
          }

        if ( aOpenCommand.Sink.is() )
        {
            // Open document - supply document data stream.

            // Check open mode
            if ( ( aOpenCommand.Mode
                    == star::ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                 ( aOpenCommand.Mode
                    == star::ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
            {
                // Unsupported.
                ucbhelper::cancelCommandExecution(
                    uno::makeAny( star::ucb::UnsupportedOpenModeException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    sal_Int16( aOpenCommand.Mode ) ) ),
                    Environment );
                // Unreachable
            }


            rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
            uno::Reference< io::XOutputStream > xOut
                = uno::Reference< io::XOutputStream >(
                    aOpenCommand.Sink, uno::UNO_QUERY );
            if ( xOut.is() )
              {
                // @@@ PUSH: write data into xOut
              }
            else
              {
                uno::Reference< io::XActiveDataSink > xDataSink
                    = uno::Reference< io::XActiveDataSink >(
                        aOpenCommand.Sink, uno::UNO_QUERY );
                  if ( xDataSink.is() )
                {
                      // @@@ PULL: wait for client read

                    uno::Reference< io::XInputStream > xIn
                        = new // @@@ your XInputStream + XSeekable impl. object
                    xDataSink->setInputStream( xIn );
                }
                  else
                {
                    // Note: aOpenCommand.Sink may contain an XStream
                    //       implementation. Support for this type of
                    //       sink is optional...
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny( ucb::UnsupportedDataSinkException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aOpenCommand.Sink ) ),
                        Environment );
                    // Unreachable
                }
              }
        }
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //////////////////////////////////////////////////////////////////

        star::ucb::InsertCommandArgument arg;
          if ( !( aCommand.Argument >>= arg ) )
        {
              OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
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
        // Unsupported command
        //////////////////////////////////////////////////////////////////

        OSL_ENSURE( sal_False, "Content::execute - unsupported command!" );

        ucbhelper::cancelCommandExecution(
            uno::makeAny( star::ucb::UnsupportedCommandException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL Content::abort( sal_Int32 CommandId )
    throw( uno::RuntimeException )
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
rtl::OUString Content::getParentURL()
{
    rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

    // @@@ Extract URL of parent from aURL and return it...

    return rtl::OUString();
}

//=========================================================================
// static
uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Reference< lang::XMultiServiceFactory >& rSMgr,
            const uno::Sequence< beans::Property >& rProperties,
            const ContentProperties& rData,
            const rtl::Reference< ::ucb::ContentProviderImplHelper >& rProvider,
            const rtl::OUString& rContentId )
{
    // Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucb::PropertyValueSet > xRow
                                = new ::ucb::PropertyValueSet( rSMgr );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
        sal_Bool bTriedToGetAdditonalPropSet = sal_False;

        const beans::Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::Property& rProp = pProps[ n ];

            // Process Core properties.

            if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
            {
                xRow->appendString ( rProp, rData.aContentType );
            }
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
            {
                xRow->appendString ( rProp, rData.aTitle );
            }
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
            {
                xRow->appendBoolean( rProp, rData.bIsDocument );
            }
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
            {
                xRow->appendBoolean( rProp, rData.bIsFolder );
            }

            // @@@ Process other properties supported directly.
#if 0
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "xxxxxx" ) ) )
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
                        = uno::Reference< beans::XPropertySet >(
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 sal_False ),
                            uno::UNO_QUERY );
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
            beans::Property( rtl::OUString::createFromAscii( "ContentType" ),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.aContentType );
        xRow->appendString (
            beans::Property( rtl::OUString::createFromAscii( "Title" ),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
            rData.aTitle );
        xRow->appendBoolean(
            beans::Property( rtl::OUString::createFromAscii( "IsDocument" ),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.bIsDocument );
        xRow->appendBoolean(
            beans::Property( rtl::OUString::createFromAscii( "IsFolder" ),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.bIsFolder );

        // @@@ Append other properties supported directly.

        // @@@ Note: If your data source supports adding/removing
        //     properties, you should implement the interface
        //     XPropertyContainer by yourself and supply your own
        //     logic here. The base class uses the service
        //     "com.sun.star.ucb.Store" to maintain Additional Core
        //     properties. But using server functionality is preferred!

        // Append all Additional Core Properties.

        uno::Reference< beans::XPropertySet > xSet(
            rProvider->getAdditionalPropertySet( rContentId, sal_False ),
            uno::UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

//=========================================================================
uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Sequence< beans::Property >& rProperties,
            const uno::Reference< star::ucb::XCommandEnvironment >& xEnv )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return getPropertyValues( m_xSMgr,
                              rProperties,
                              m_aProps,
                              rtl::Reference<
                                ::ucb::ContentProviderImplHelper >(
                                    m_xProvider.getBodyPtr() ),
                              m_xIdentifier->getContentIdentifier() );
}

//=========================================================================
uno::Sequence< uno::Any > Content::setPropertyValues(
            const uno::Sequence< beans::PropertyValue >& rValues,
            const uno::Reference< star::ucb::XCommandEnvironment >& xEnv )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = sal_False;
//  aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//  aEvent.OldValue       =
//  aEvent.NewValue       =

    const beans::PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    uno::Reference< star::ucb::XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditonalPropSet = sal_False;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            rtl::OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                if ( aNewValue != m_aProps.aTitle )
                {
                    osl::Guard< osl::Mutex > aGuard( m_aMutex );

                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue     = uno::makeAny( m_aProps.aTitle );
                    aEvent.NewValue     = uno::makeAny( aNewValue );

                    aChanges.getArray()[ nChanged ] = aEvent;

                    m_aProps.aTitle = aNewValue;
                    nChanged++;
                }
                else
                {
                    // Old value equals new value. No error!
                }
            }
            else
            {
                aRet[ n ] <<= beans::IllegalTypeException(
                                rtl::OUString::createFromAscii(
                                    "Property value has wrong type!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }

        // @@@ Process other properties supported directly.
#if 0
        else if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "xxxxxx" ) ) )
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
                    uno::Any aOldValue
                        = xAdditionalPropSet->getPropertyValue( rValue.Name );
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
                    else
                    {
                        // Old value equals new value. No error!
                    }
                }
                catch ( beans::UnknownPropertyException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::WrappedTargetException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( beans::PropertyVetoException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::IllegalArgumentException const & e )
                {
                    aRet[ n ] <<= e;
                }
            }
            else
            {
                aRet[ n ] <<= uno::Exception(
                                rtl::OUString::createFromAscii(
                                    "No property set for storing the value!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
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

    return aRet;
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
void Content::insert(
        const uno::Reference< io::XInputStream > & xInputStream,
        sal_Bool bReplaceExisting,
        const uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    if ( m_aProps.xxxx == yyyyy )
    {
        OSL_ENSURE( sal_False, "Content::insert - property value missing!" );

        uno::Sequence< rtl::OUString > aProps( 1 );
        aProps[ 0 ] = rtl::OUString::createFromAscii( "zzzz" );
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingPropertiesException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aProps ) ),
            Environment );
        // Unreachable
    }

    if ( bNeedInputStream && !xInputStream.is() )
    {
        OSL_ENSURE( sal_False, "Content::insert - No data stream!" );

        ucbhelper::cancelCommandExecution(
            uno::makeAny( star::ucb::MissingInputStreamException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    // Assemble new content identifier...

    uno::Reference< star::ucb::XContentIdentifier > xId = ...;

    // Fail, if a resource with given id already exists.
    if ( !bReplaceExisting && hasData( xId ) )
    {
        ucbhelper::cancelCommandExecution(
                        static_cast< cppu::OWeakObject * >( this ),
                        star::ucb::IOErrorCode_ALREADY_EXISTING,
                        Environment );
        // Unreachable
    }

    m_xIdentifier = xId;

//  @@@
//  storeData();

    aGuard.clear();
    inserted();
}

//=========================================================================
void Content::destroy( sal_Bool bDeletePhysical )
    throw( uno::Exception )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    uno::Reference< star::ucb::XContent > xThis = this;

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

