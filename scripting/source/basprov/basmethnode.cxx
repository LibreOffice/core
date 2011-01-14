/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
#include "basmethnode.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>

#include <util/MiscUtils.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;
using namespace ::com::sun::star::script;
using namespace ::sf_misc;

#define BASPROV_PROPERTY_ID_URI         1
#define BASPROV_PROPERTY_ID_EDITABLE    2

#define BASPROV_PROPERTY_URI            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URI" ) )
#define BASPROV_PROPERTY_EDITABLE       ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Editable" ) )

#define BASPROV_DEFAULT_ATTRIBS()       PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY


//.........................................................................
namespace basprov
{
//.........................................................................

    // =============================================================================
    // BasicMethodNodeImpl
    // =============================================================================

    BasicMethodNodeImpl::BasicMethodNodeImpl( const Reference< XComponentContext >& rxContext,
        const ::rtl::OUString& sScriptingContext, SbMethod* pMethod, bool isAppScript )
        : ::scripting_helper::OBroadcastHelperHolder( m_aMutex )
        ,OPropertyContainer( GetBroadcastHelper() )
        ,m_xContext( rxContext )
        ,m_sScriptingContext( sScriptingContext )
        ,m_pMethod( pMethod )
        ,m_bIsAppScript( isAppScript )
        ,m_bEditable( sal_True )
    {
        if ( m_pMethod )
        {
            SbModule* pModule = m_pMethod->GetModule();
            if ( pModule )
            {
                StarBASIC* pBasic = static_cast< StarBASIC* >( pModule->GetParent() );
                if ( pBasic )
                {
                    m_sURI = ::rtl::OUString::createFromAscii( "vnd.sun.star.script:" );
                    m_sURI += pBasic->GetName();
                    m_sURI += ::rtl::OUString::createFromAscii( "." );
                    m_sURI += pModule->GetName();
                    m_sURI += ::rtl::OUString::createFromAscii( "." );
                    m_sURI += m_pMethod->GetName();
                    m_sURI += ::rtl::OUString::createFromAscii( "?language=Basic&location=" );
                    if ( m_bIsAppScript )
                        m_sURI += ::rtl::OUString::createFromAscii( "application" );
                    else
                        m_sURI += ::rtl::OUString::createFromAscii( "document" );
                }
            }
        }

        registerProperty( BASPROV_PROPERTY_URI,      BASPROV_PROPERTY_ID_URI,      BASPROV_DEFAULT_ATTRIBS(), &m_sURI,      ::getCppuType( &m_sURI ) );
        registerProperty( BASPROV_PROPERTY_EDITABLE, BASPROV_PROPERTY_ID_EDITABLE, BASPROV_DEFAULT_ATTRIBS(), &m_bEditable, ::getCppuType( &m_bEditable ) );
    }

    // -----------------------------------------------------------------------------

    BasicMethodNodeImpl::~BasicMethodNodeImpl()
    {
    }

    // -----------------------------------------------------------------------------
    // XInterface
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XINTERFACE2( BasicMethodNodeImpl, BasicMethodNodeImpl_BASE, OPropertyContainer )

    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( BasicMethodNodeImpl, BasicMethodNodeImpl_BASE, OPropertyContainer )

    // -----------------------------------------------------------------------------
    // XBrowseNode
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicMethodNodeImpl::getName(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        ::rtl::OUString sMethodName;
        if ( m_pMethod )
            sMethodName = m_pMethod->GetName();

        return sMethodName;
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicMethodNodeImpl::getChildNodes(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return Sequence< Reference< browse::XBrowseNode > >();
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicMethodNodeImpl::hasChildNodes(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return sal_False;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 BasicMethodNodeImpl::getType(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return browse::BrowseNodeTypes::SCRIPT;
    }

    // -----------------------------------------------------------------------------
    // OPropertySetHelper
    // -----------------------------------------------------------------------------

    ::cppu::IPropertyArrayHelper& BasicMethodNodeImpl::getInfoHelper(  )
    {
        return *getArrayHelper();
    }

    // -----------------------------------------------------------------------------
    // OPropertyArrayUsageHelper
    // -----------------------------------------------------------------------------

    ::cppu::IPropertyArrayHelper* BasicMethodNodeImpl::createArrayHelper(  ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    // -----------------------------------------------------------------------------
    // XPropertySet
    // -----------------------------------------------------------------------------

    Reference< XPropertySetInfo > BasicMethodNodeImpl::getPropertySetInfo(  ) throw (RuntimeException)
    {
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    // -----------------------------------------------------------------------------
    // XInvocation
    // -----------------------------------------------------------------------------

    Reference< XIntrospectionAccess > BasicMethodNodeImpl::getIntrospection(  ) throw (RuntimeException)
    {
        return Reference< XIntrospectionAccess >();
    }

    // -----------------------------------------------------------------------------

    Any BasicMethodNodeImpl::invoke( const ::rtl::OUString& aFunctionName, const Sequence< Any >& aParams,
        Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
        throw (IllegalArgumentException, script::CannotConvertException,
               reflection::InvocationTargetException, RuntimeException)
    {
        (void)aParams;
        (void)aOutParamIndex;
        (void)aOutParam;

        if ( aFunctionName == BASPROV_PROPERTY_EDITABLE )
        {
            ::rtl::OUString sDocURL, sLibName, sModName;
            sal_uInt16 nLine1 = 0, nLine2;

            if ( !m_bIsAppScript )
            {
                Reference< frame::XModel > xModel = MiscUtils::tDocUrlToModel( m_sScriptingContext );

                if ( xModel.is() )
                {
                    sDocURL = xModel->getURL();
                    if ( sDocURL.getLength() == 0 )
                    {
                        Sequence < PropertyValue > aProps = xModel->getArgs();
                        sal_Int32 nProps = aProps.getLength();
                        const PropertyValue* pProps = aProps.getConstArray();
                        for ( sal_Int32 i = 0; i < nProps; ++i )
                        {
                            // TODO: according to MBA the property 'Title' may change in future
                            if ( pProps[i].Name == ::rtl::OUString::createFromAscii( "Title" ) )
                            {
                                pProps[i].Value >>= sDocURL;
                                break;
                            }
                        }
                    }
                }
            }

            if ( m_pMethod )
            {
                m_pMethod->GetLineRange( nLine1, nLine2 );
                SbModule* pModule = m_pMethod->GetModule();
                if ( pModule )
                {
                    sModName = pModule->GetName();
                    StarBASIC* pBasic = static_cast< StarBASIC* >( pModule->GetParent() );
                    if ( pBasic )
                        sLibName = pBasic->GetName();
                }
            }

            if ( m_xContext.is() )
            {
                Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );

                if ( xSMgr.is() )
                {
                    Reference< frame::XDesktop > xDesktop( xSMgr->createInstanceWithContext(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ), m_xContext ), UNO_QUERY );

                    if ( xDesktop.is() )
                    {
                        Reference < frame::XDispatchProvider > xProv( xDesktop->getCurrentFrame(), UNO_QUERY );

                        if ( xProv.is() )
                        {
                            Reference< frame::XDispatchHelper > xHelper( xSMgr->createInstanceWithContext(
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" ) ), m_xContext ), UNO_QUERY );

                            if ( xHelper.is() )
                            {
                                Sequence < PropertyValue > aArgs(7);
                                aArgs[0].Name = ::rtl::OUString::createFromAscii( "Document" );
                                aArgs[0].Value <<= sDocURL;
                                aArgs[1].Name = ::rtl::OUString::createFromAscii( "LibName" );
                                aArgs[1].Value <<= sLibName;
                                aArgs[2].Name = ::rtl::OUString::createFromAscii( "Name" );
                                aArgs[2].Value <<= sModName;
                                aArgs[3].Name = ::rtl::OUString::createFromAscii( "Type" );
                                aArgs[3].Value <<= ::rtl::OUString::createFromAscii( "Module" );
                                aArgs[4].Name = ::rtl::OUString::createFromAscii( "Line" );
                                aArgs[4].Value <<= static_cast< sal_uInt32 >( nLine1 );
                                xHelper->executeDispatch( xProv, ::rtl::OUString::createFromAscii( ".uno:BasicIDEAppear" ), ::rtl::OUString(), 0, aArgs );
                            }
                        }
                    }
                }
            }
        }
        else
        {
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicMethodNodeImpl::invoke: function name not supported!" ) ),
                Reference< XInterface >(), 1 );
        }

        return Any();
    }

    // -----------------------------------------------------------------------------

    void BasicMethodNodeImpl::setValue( const ::rtl::OUString& aPropertyName, const Any& aValue )
        throw (UnknownPropertyException, script::CannotConvertException,
               reflection::InvocationTargetException, RuntimeException)
    {
        (void)aPropertyName;
        (void)aValue;

        throw UnknownPropertyException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicMethodNodeImpl::setValue: property name is unknown!" ) ),
            Reference< XInterface >() );
    }

    // -----------------------------------------------------------------------------

    Any BasicMethodNodeImpl::getValue( const ::rtl::OUString& aPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        (void)aPropertyName;

        throw UnknownPropertyException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicMethodNodeImpl::getValue: property name is unknown!" ) ),
            Reference< XInterface >() );
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicMethodNodeImpl::hasMethod( const ::rtl::OUString& aName ) throw (RuntimeException)
    {
        sal_Bool bReturn = sal_False;
        if ( aName == BASPROV_PROPERTY_EDITABLE )
            bReturn = sal_True;

        return bReturn;
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicMethodNodeImpl::hasProperty( const ::rtl::OUString& aName ) throw (RuntimeException)
    {
        (void)aName;

        return sal_False;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................
