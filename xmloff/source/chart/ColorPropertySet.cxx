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

#include "ColorPropertySet.hxx"

#include <cppuhelper/implbase.hxx>

#include <osl/diagnose.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
class lcl_ColorPropertySetInfo : public ::cppu::WeakImplHelper<
        XPropertySetInfo  >
{
public:
    explicit lcl_ColorPropertySetInfo();

protected:
    // ____ XPropertySetInfo ____
    virtual Sequence< Property > SAL_CALL getProperties() override;
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;

private:
    static constexpr OUStringLiteral g_aColorPropName = u"FillColor";
    Property m_aColorProp;
};

lcl_ColorPropertySetInfo::lcl_ColorPropertySetInfo() :
        m_aColorProp( g_aColorPropName, -1,
                      cppu::UnoType<sal_Int32>::get(), 0)
{}

Sequence< Property > SAL_CALL lcl_ColorPropertySetInfo::getProperties()
{

    return Sequence< Property >( & m_aColorProp, 1 );
}

Property SAL_CALL lcl_ColorPropertySetInfo::getPropertyByName( const OUString& aName )
{
    if( aName == g_aColorPropName )
        return m_aColorProp;
    throw UnknownPropertyException( g_aColorPropName, static_cast< uno::XWeak * >( this ));
}

sal_Bool SAL_CALL lcl_ColorPropertySetInfo::hasPropertyByName( const OUString& Name )
{
    return Name == g_aColorPropName;
}

} // anonymous namespace

namespace xmloff::chart
{

ColorPropertySet::ColorPropertySet( ::Color nColor ) :
        m_nColor( nColor ),
        m_nDefaultColor( 0x0099ccff )  // blue 8
{}

ColorPropertySet::~ColorPropertySet()
{}

// ____ XPropertySet ____

Reference< XPropertySetInfo > SAL_CALL ColorPropertySet::getPropertySetInfo()
{
    if( ! m_xInfo.is())
        m_xInfo.set( new lcl_ColorPropertySetInfo );

    return m_xInfo;
}

void SAL_CALL ColorPropertySet::setPropertyValue( const OUString& /* aPropertyName */, const uno::Any& aValue )
{
    aValue >>= m_nColor;
}

uno::Any SAL_CALL ColorPropertySet::getPropertyValue( const OUString& /* PropertyName */ )
{
    return uno::makeAny( m_nColor );
}

void SAL_CALL ColorPropertySet::addPropertyChangeListener( const OUString& /* aPropertyName */, const Reference< XPropertyChangeListener >& /* xListener */ )
{
    OSL_FAIL( "Not Implemented" );
}

void SAL_CALL ColorPropertySet::removePropertyChangeListener( const OUString& /* aPropertyName */, const Reference< XPropertyChangeListener >& /* aListener */ )
{
    OSL_FAIL( "Not Implemented" );
}

void SAL_CALL ColorPropertySet::addVetoableChangeListener( const OUString& /* PropertyName */, const Reference< XVetoableChangeListener >& /* aListener */ )
{
    OSL_FAIL( "Not Implemented" );
}

void SAL_CALL ColorPropertySet::removeVetoableChangeListener( const OUString& /* PropertyName */, const Reference< XVetoableChangeListener >& /* aListener */ )
{
    OSL_FAIL( "Not Implemented" );
}

// ____ XPropertyState ____

PropertyState SAL_CALL ColorPropertySet::getPropertyState( const OUString& /* PropertyName */ )
{
    return PropertyState_DIRECT_VALUE;
}

Sequence< PropertyState > SAL_CALL ColorPropertySet::getPropertyStates( const Sequence< OUString >& /* aPropertyName */ )
{
    PropertyState aState = PropertyState_DIRECT_VALUE;
    // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
    return Sequence<PropertyState>(&aState, 1);
}

void SAL_CALL ColorPropertySet::setPropertyToDefault( const OUString& PropertyName )
{
    if( PropertyName == g_aColorPropName )
        m_nColor = m_nDefaultColor;
}

uno::Any SAL_CALL ColorPropertySet::getPropertyDefault( const OUString& aPropertyName )
{
    if( aPropertyName == g_aColorPropName )
        return uno::makeAny( m_nDefaultColor );
    return uno::Any();
}

} //  namespace xmloff::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
