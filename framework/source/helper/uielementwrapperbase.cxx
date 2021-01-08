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

#include <helper/uielementwrapperbase.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/svapp.hxx>
#include <comphelper/sequence.hxx>

const int UIELEMENT_PROPHANDLE_RESOURCEURL  = 1;
const int UIELEMENT_PROPHANDLE_TYPE         = 2;
const int UIELEMENT_PROPHANDLE_FRAME        = 3;
constexpr OUStringLiteral UIELEMENT_PROPNAME_RESOURCEURL = u"ResourceURL";
constexpr OUStringLiteral UIELEMENT_PROPNAME_TYPE = u"Type";
constexpr OUStringLiteral UIELEMENT_PROPNAME_FRAME = u"Frame";

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace framework
{

UIElementWrapperBase::UIElementWrapperBase( sal_Int16 nType )
    :   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aMutex )
    ,   ::cppu::OPropertySetHelper  ( *static_cast< ::cppu::OBroadcastHelper* >(this) )
    ,   m_aListenerContainer        ( m_aMutex )
    ,   m_nType                     ( nType                                             )
    ,   m_bInitialized              ( false                                         )
    ,   m_bDisposed                 ( false                                         )
{
}

UIElementWrapperBase::~UIElementWrapperBase()
{
}

Any SAL_CALL UIElementWrapperBase::queryInterface( const Type& _rType )
{
    Any aRet = UIElementWrapperBase_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    return aRet;
}

Sequence< Type > SAL_CALL UIElementWrapperBase::getTypes(  )
{
    return comphelper::concatSequences(
        UIElementWrapperBase_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}

void SAL_CALL UIElementWrapperBase::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    m_aListenerContainer.addInterface( cppu::UnoType<css::lang::XEventListener>::get(), xListener );
}

void SAL_CALL UIElementWrapperBase::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    m_aListenerContainer.removeInterface( cppu::UnoType<css::lang::XEventListener>::get(), xListener );
}

void SAL_CALL UIElementWrapperBase::initialize( const Sequence< Any >& aArguments )
{
    SolarMutexGuard g;

    if ( m_bInitialized )
        return;

    for ( const Any& rArg : aArguments )
    {
        PropertyValue aPropValue;
        if ( rArg >>= aPropValue )
        {
            if ( aPropValue.Name == "ResourceURL" )
                aPropValue.Value >>= m_aResourceURL;
            else if ( aPropValue.Name == "Frame" )
            {
                Reference< XFrame > xFrame;
                aPropValue.Value >>= xFrame;
                m_xWeakFrame = xFrame;
            }
        }
    }

    m_bInitialized = true;
}

// XUIElement
css::uno::Reference< css::frame::XFrame > SAL_CALL UIElementWrapperBase::getFrame()
{
    css::uno::Reference< css::frame::XFrame > xFrame( m_xWeakFrame );
    return xFrame;
}

OUString SAL_CALL UIElementWrapperBase::getResourceURL()
{
    return m_aResourceURL;
}

::sal_Int16 SAL_CALL UIElementWrapperBase::getType()
{
    return m_nType;
}

// XUpdatable
void SAL_CALL UIElementWrapperBase::update()
{
    // can be implemented by derived class
}

// XPropertySet helper
sal_Bool SAL_CALL UIElementWrapperBase::convertFastPropertyValue( Any&       /*aConvertedValue*/ ,
                                                                  Any&       /*aOldValue*/       ,
                                                                  sal_Int32  /*nHandle*/         ,
                                                                  const Any& /*aValue*/             )
{
    //  Initialize state with sal_False !!!
    //  (Handle can be invalid)
    return false;
}

void SAL_CALL UIElementWrapperBase::setFastPropertyValue_NoBroadcast(   sal_Int32               /*nHandle*/ ,
                                                                        const css::uno::Any&    /*aValue*/  )
{
}

void SAL_CALL UIElementWrapperBase::getFastPropertyValue( css::uno::Any& aValue  ,
                                                          sal_Int32      nHandle                ) const
{
    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue <<= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue <<= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
            Reference< XFrame > xFrame( m_xWeakFrame );
            aValue <<= xFrame;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL UIElementWrapperBase::getInfoHelper()
{
    // Define static member to give structure of properties to baseclass "OPropertySetHelper".
    // "impl_getStaticPropertyDescriptor" is a non exported and static function, who will define a static propertytable.
    // "true" say: Table is sorted by name.
    static ::cppu::OPropertyArrayHelper ourInfoHelper( impl_getStaticPropertyDescriptor(), true );

    return ourInfoHelper;
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL UIElementWrapperBase::getPropertySetInfo()
{
    // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
    // (Use method "getInfoHelper()".)
    static css::uno::Reference< css::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );

    return xInfo;
}

css::uno::Sequence< css::beans::Property > UIElementWrapperBase::impl_getStaticPropertyDescriptor()
{
    // Create a property array to initialize sequence!
    // Table of all predefined properties of this class. It's used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    return
    {
        css::beans::Property( UIELEMENT_PROPNAME_FRAME, UIELEMENT_PROPHANDLE_FRAME          , cppu::UnoType<XFrame>::get(), css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( UIELEMENT_PROPNAME_RESOURCEURL, UIELEMENT_PROPHANDLE_RESOURCEURL    , cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( UIELEMENT_PROPNAME_TYPE, UIELEMENT_PROPHANDLE_TYPE           , cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY )
    };
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
