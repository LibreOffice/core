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

#include "basmethnode.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <osl/mutex.hxx>
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

#define BASPROV_PROPERTY_URI            ::rtl::OUString( "URI"  )
#define BASPROV_PROPERTY_EDITABLE       ::rtl::OUString( "Editable"  )

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
        ,m_bEditable( true )
    {
        if ( m_pMethod )
        {
            SbModule* pModule = m_pMethod->GetModule();
            if ( pModule )
            {
                StarBASIC* pBasic = static_cast< StarBASIC* >( pModule->GetParent() );
                if ( pBasic )
                {
                    m_sURI = ::rtl::OUString("vnd.sun.star.script:");
                    m_sURI += pBasic->GetName();
                    m_sURI += ::rtl::OUString(".");
                    m_sURI += pModule->GetName();
                    m_sURI += ::rtl::OUString(".");
                    m_sURI += m_pMethod->GetName();
                    m_sURI += ::rtl::OUString("?language=Basic&location=");
                    if ( m_bIsAppScript )
                        m_sURI += ::rtl::OUString("application");
                    else
                        m_sURI += ::rtl::OUString("document");
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
        SolarMutexGuard aGuard;

        ::rtl::OUString sMethodName;
        if ( m_pMethod )
            sMethodName = m_pMethod->GetName();

        return sMethodName;
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicMethodNodeImpl::getChildNodes(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;

        return Sequence< Reference< browse::XBrowseNode > >();
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicMethodNodeImpl::hasChildNodes(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;

        return sal_False;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 BasicMethodNodeImpl::getType(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;

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
                    if ( sDocURL.isEmpty() )
                    {
                        Sequence < PropertyValue > aProps = xModel->getArgs();
                        sal_Int32 nProps = aProps.getLength();
                        const PropertyValue* pProps = aProps.getConstArray();
                        for ( sal_Int32 i = 0; i < nProps; ++i )
                        {
                            // TODO: according to MBA the property 'Title' may change in future
                            if ( pProps[i].Name == "Title" )
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
                        ::rtl::OUString( "com.sun.star.frame.Desktop"  ), m_xContext ), UNO_QUERY );

                    if ( xDesktop.is() )
                    {
                        Reference < frame::XDispatchProvider > xProv( xDesktop->getCurrentFrame(), UNO_QUERY );

                        if ( xProv.is() )
                        {
                            Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create( m_xContext ) );

                            Sequence < PropertyValue > aArgs(7);
                            aArgs[0].Name = ::rtl::OUString("Document");
                            aArgs[0].Value <<= sDocURL;
                            aArgs[1].Name = ::rtl::OUString("LibName");
                            aArgs[1].Value <<= sLibName;
                            aArgs[2].Name = ::rtl::OUString("Name");
                            aArgs[2].Value <<= sModName;
                            aArgs[3].Name = ::rtl::OUString("Type");
                            aArgs[3].Value <<= ::rtl::OUString("Module");
                            aArgs[4].Name = ::rtl::OUString("Line");
                            aArgs[4].Value <<= static_cast< sal_uInt32 >( nLine1 );
                            xHelper->executeDispatch( xProv, ::rtl::OUString(".uno:BasicIDEAppear"), ::rtl::OUString(), 0, aArgs );
                        }
                    }
                }
            }
        }
        else
        {
            throw IllegalArgumentException(
                ::rtl::OUString( "BasicMethodNodeImpl::invoke: function name not supported!"  ),
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
            ::rtl::OUString( "BasicMethodNodeImpl::setValue: property name is unknown!"  ),
            Reference< XInterface >() );
    }

    // -----------------------------------------------------------------------------

    Any BasicMethodNodeImpl::getValue( const ::rtl::OUString& aPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        (void)aPropertyName;

        throw UnknownPropertyException(
            ::rtl::OUString( "BasicMethodNodeImpl::getValue: property name is unknown!"  ),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
