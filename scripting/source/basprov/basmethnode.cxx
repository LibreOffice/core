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
#include <com/sun/star/frame/Desktop.hpp>
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

#define BASPROV_PROPERTY_URI            "URI"
#define BASPROV_PROPERTY_EDITABLE       "Editable"

#define BASPROV_DEFAULT_ATTRIBS()       PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY



namespace basprov
{



    // BasicMethodNodeImpl


    BasicMethodNodeImpl::BasicMethodNodeImpl( const Reference< XComponentContext >& rxContext,
        const OUString& sScriptingContext, SbMethod* pMethod, bool isAppScript )
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
                    m_sURI = "vnd.sun.star.script:";
                    m_sURI += pBasic->GetName();
                    m_sURI += ".";
                    m_sURI += pModule->GetName();
                    m_sURI += ".";
                    m_sURI += m_pMethod->GetName();
                    m_sURI += "?language=Basic&location=";
                    if ( m_bIsAppScript )
                        m_sURI += "application";
                    else
                        m_sURI += "document";
                }
            }
        }

        registerProperty( BASPROV_PROPERTY_URI,      BASPROV_PROPERTY_ID_URI,      BASPROV_DEFAULT_ATTRIBS(), &m_sURI,      cppu::UnoType<decltype(m_sURI)>::get() );
        registerProperty( BASPROV_PROPERTY_EDITABLE, BASPROV_PROPERTY_ID_EDITABLE, BASPROV_DEFAULT_ATTRIBS(), &m_bEditable, cppu::UnoType<decltype(m_bEditable)>::get() );
    }



    BasicMethodNodeImpl::~BasicMethodNodeImpl()
    {
    }


    // XInterface


    IMPLEMENT_FORWARD_XINTERFACE2( BasicMethodNodeImpl, BasicMethodNodeImpl_BASE, OPropertyContainer )


    // XTypeProvider


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( BasicMethodNodeImpl, BasicMethodNodeImpl_BASE, OPropertyContainer )


    // XBrowseNode


    OUString BasicMethodNodeImpl::getName(  ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aGuard;

        OUString sMethodName;
        if ( m_pMethod )
            sMethodName = m_pMethod->GetName();

        return sMethodName;
    }



    Sequence< Reference< browse::XBrowseNode > > BasicMethodNodeImpl::getChildNodes(  ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aGuard;

        return Sequence< Reference< browse::XBrowseNode > >();
    }



    sal_Bool BasicMethodNodeImpl::hasChildNodes(  ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aGuard;

        return sal_False;
    }



    sal_Int16 BasicMethodNodeImpl::getType(  ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aGuard;

        return browse::BrowseNodeTypes::SCRIPT;
    }


    // OPropertySetHelper


    ::cppu::IPropertyArrayHelper& BasicMethodNodeImpl::getInfoHelper(  )
    {
        return *getArrayHelper();
    }


    // OPropertyArrayUsageHelper


    ::cppu::IPropertyArrayHelper* BasicMethodNodeImpl::createArrayHelper(  ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }


    // XPropertySet


    Reference< XPropertySetInfo > BasicMethodNodeImpl::getPropertySetInfo(  ) throw (RuntimeException, std::exception)
    {
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    // XInvocation


    Reference< XIntrospectionAccess > BasicMethodNodeImpl::getIntrospection(  ) throw (RuntimeException, std::exception)
    {
        return Reference< XIntrospectionAccess >();
    }



    Any BasicMethodNodeImpl::invoke( const OUString& aFunctionName, const Sequence< Any >& aParams,
        Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
        throw (IllegalArgumentException, script::CannotConvertException,
               reflection::InvocationTargetException, RuntimeException, std::exception)
    {
        (void)aParams;
        (void)aOutParamIndex;
        (void)aOutParam;

        if ( aFunctionName == BASPROV_PROPERTY_EDITABLE )
        {
            OUString sDocURL, sLibName, sModName;
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
                Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( m_xContext );

                Reference < frame::XDispatchProvider > xProv( xDesktop->getCurrentFrame(), UNO_QUERY );

                if ( xProv.is() )
                {
                    Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create( m_xContext ) );

                    Sequence < PropertyValue > aArgs(7);
                    aArgs[0].Name = "Document";
                    aArgs[0].Value <<= sDocURL;
                    aArgs[1].Name = "LibName";
                    aArgs[1].Value <<= sLibName;
                    aArgs[2].Name = "Name";
                    aArgs[2].Value <<= sModName;
                    aArgs[3].Name = "Type";
                    aArgs[3].Value <<= OUString("Module");
                    aArgs[4].Name = "Line";
                    aArgs[4].Value <<= static_cast< sal_uInt32 >( nLine1 );
                    xHelper->executeDispatch( xProv, ".uno:BasicIDEAppear", OUString(), 0, aArgs );
                }
            }
        }
        else
        {
            throw IllegalArgumentException(
                "BasicMethodNodeImpl::invoke: function name not supported!",
                Reference< XInterface >(), 1 );
        }

        return Any();
    }



    void BasicMethodNodeImpl::setValue( const OUString& aPropertyName, const Any& aValue )
        throw (UnknownPropertyException, script::CannotConvertException,
               reflection::InvocationTargetException, RuntimeException, std::exception)
    {
        (void)aPropertyName;
        (void)aValue;

        throw UnknownPropertyException(
            "BasicMethodNodeImpl::setValue: property name is unknown!" );
    }



    Any BasicMethodNodeImpl::getValue( const OUString& aPropertyName ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        (void)aPropertyName;

        throw UnknownPropertyException(
            "BasicMethodNodeImpl::getValue: property name is unknown!" );
    }



    sal_Bool BasicMethodNodeImpl::hasMethod( const OUString& aName ) throw (RuntimeException, std::exception)
    {
        bool bReturn = false;
        if ( aName == BASPROV_PROPERTY_EDITABLE )
            bReturn = true;

        return bReturn;
    }



    sal_Bool BasicMethodNodeImpl::hasProperty( const OUString& aName ) throw (RuntimeException, std::exception)
    {
        (void)aName;

        return sal_False;
    }




}   // namespace basprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
