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
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>

#include <comphelper/propertyvalue.hxx>
#include <utility>
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

constexpr OUStringLiteral BASPROV_PROPERTY_URI = u"URI";
constexpr OUString BASPROV_PROPERTY_EDITABLE = u"Editable"_ustr;

#define BASPROV_DEFAULT_ATTRIBS()       PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY


namespace basprov
{


    // BasicMethodNodeImpl


    BasicMethodNodeImpl::BasicMethodNodeImpl( const Reference< XComponentContext >& rxContext,
        OUString sScriptingContext, SbMethod* pMethod, bool isAppScript )
        :
        m_xContext( rxContext )
        ,m_sScriptingContext(std::move( sScriptingContext ))
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


    IMPLEMENT_FORWARD_XINTERFACE2( BasicMethodNodeImpl, BasicMethodNodeImpl_BASE, comphelper::OPropertyContainer2 )


    // XTypeProvider


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( BasicMethodNodeImpl, BasicMethodNodeImpl_BASE, comphelper::OPropertyContainer2 )


    // XBrowseNode


    OUString BasicMethodNodeImpl::getName(  )
    {
        SolarMutexGuard aGuard;

        OUString sMethodName;
        if ( m_pMethod )
            sMethodName = m_pMethod->GetName();

        return sMethodName;
    }


    Sequence< Reference< browse::XBrowseNode > > BasicMethodNodeImpl::getChildNodes(  )
    {
        return Sequence< Reference< browse::XBrowseNode > >();
    }


    sal_Bool BasicMethodNodeImpl::hasChildNodes(  )
    {
        return false;
    }


    sal_Int16 BasicMethodNodeImpl::getType(  )
    {
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


    Reference< XPropertySetInfo > BasicMethodNodeImpl::getPropertySetInfo(  )
    {
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    // XInvocation


    Reference< XIntrospectionAccess > BasicMethodNodeImpl::getIntrospection(  )
    {
        return Reference< XIntrospectionAccess >();
    }


    Any BasicMethodNodeImpl::invoke( const OUString& aFunctionName, const Sequence< Any >&,
        Sequence< sal_Int16 >&, Sequence< Any >& )
    {
        if ( aFunctionName != BASPROV_PROPERTY_EDITABLE )
        {
            throw IllegalArgumentException(
                u"BasicMethodNodeImpl::invoke: function name not supported!"_ustr,
                Reference< XInterface >(), 1 );
        }

        OUString sDocURL, sLibName, sModName;
        sal_uInt16 nLine1 = 0;

        if ( !m_bIsAppScript )
        {
            Reference< frame::XModel > xModel = MiscUtils::tDocUrlToModel( m_sScriptingContext );

            if ( xModel.is() )
            {
                sDocURL = xModel->getURL();
                if ( sDocURL.isEmpty() )
                {
                    const Sequence < PropertyValue > aProps = xModel->getArgs();
                    // TODO: according to MBA the property 'Title' may change in future
                    const PropertyValue* pProp = std::find_if(aProps.begin(), aProps.end(),
                        [](const PropertyValue& rProp) { return rProp.Name == "Title"; });
                    if (pProp != aProps.end())
                        pProp->Value >>= sDocURL;
                }
            }
        }

        if ( m_pMethod )
        {
            sal_uInt16 nLine2;
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

                Sequence < PropertyValue > aArgs{
                    comphelper::makePropertyValue(u"Document"_ustr, sDocURL),
                    comphelper::makePropertyValue(u"LibName"_ustr, sLibName),
                    comphelper::makePropertyValue(u"Name"_ustr, sModName),
                    comphelper::makePropertyValue(u"Type"_ustr, u"Module"_ustr),
                    comphelper::makePropertyValue(u"Line"_ustr, static_cast< sal_uInt32 >( nLine1 ))
                };
                xHelper->executeDispatch( xProv, u".uno:BasicIDEAppear"_ustr, OUString(), 0, aArgs );
            }
        }


        return Any();
    }


    void BasicMethodNodeImpl::setValue( const OUString&, const Any& )
    {
        throw UnknownPropertyException(
            u"BasicMethodNodeImpl::setValue: property name is unknown!"_ustr );
    }


    Any BasicMethodNodeImpl::getValue( const OUString& )
    {
        throw UnknownPropertyException(
            u"BasicMethodNodeImpl::getValue: property name is unknown!"_ustr );
    }


    sal_Bool BasicMethodNodeImpl::hasMethod( const OUString& aName )
    {
        bool bReturn = false;
        if ( aName == BASPROV_PROPERTY_EDITABLE )
            bReturn = true;

        return bReturn;
    }


    sal_Bool BasicMethodNodeImpl::hasProperty( const OUString& )
    {
        return false;
    }


}   // namespace basprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
