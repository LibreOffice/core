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

#include "PageBackground.hxx"
#include <comphelper/lok.hxx>
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include <PropertyHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <svtools/colorcfg.hxx>
#include <sfx2/viewsh.hxx>

#include <com/sun/star/drawing/LineStyle.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <vector>
#include <algorithm>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace
{

const ::chart::tPropertyValueMap& StaticPageBackgroundDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults = []()
        {
            ::chart::tPropertyValueMap aTmp;
            ::chart::LinePropertiesHelper::AddDefaultsToMap( aTmp );
            ::chart::FillProperties::AddDefaultsToMap( aTmp );

            // override other defaults
            Color aDocColor = COL_WHITE;
            if (comphelper::LibreOfficeKit::isActive()) {
                aDocColor = COL_AUTO;
            } else {
                if (SfxViewShell* pCurrentSh = SfxViewShell::Current()) {
                    aDocColor = pCurrentSh->GetColorConfigColor(svtools::DOCCOLOR);
                } else {
                    aDocColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
                }
            }
            ::chart::PropertyHelper::setPropertyValue( aTmp, ::chart::FillProperties::PROP_FILL_COLOR, aDocColor );
            ::chart::PropertyHelper::setPropertyValue( aTmp, ::chart::LinePropertiesHelper::PROP_LINE_STYLE, drawing::LineStyle_NONE );
            return aTmp;
        }();
    return aStaticDefaults;
};

::cppu::OPropertyArrayHelper& StaticPageBackgroundInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []()
        {
            std::vector< css::beans::Property > aProperties;
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::FillProperties::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{

PageBackground::PageBackground() :
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{}

PageBackground::PageBackground( const PageBackground & rOther ) :
        impl::PageBackground_Base(rOther),
        ::property::OPropertySet( rOther ),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{}

PageBackground::~PageBackground()
{}

// ____ XTypeProvider ____
uno::Sequence< css::uno::Type > SAL_CALL PageBackground::getTypes()
{
    return ::comphelper::concatSequences(
        impl::PageBackground_Base::getTypes(),
        ::property::OPropertySet::getTypes());
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PageBackground::createClone()
{
    return uno::Reference< util::XCloneable >( new PageBackground( *this ));
}

// ____ OPropertySet ____
void PageBackground::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticPageBackgroundDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL PageBackground::getInfoHelper()
{
    return StaticPageBackgroundInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL PageBackground::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticPageBackgroundInfoHelper() ) );
    return xPropertySetInfo;
}

// ____ XModifyBroadcaster ____
void SAL_CALL PageBackground::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL PageBackground::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
}

// ____ XModifyListener ____
void SAL_CALL PageBackground::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL PageBackground::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void PageBackground::firePropertyChangeEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

OUString SAL_CALL PageBackground::getImplementationName()
{
    return "com.sun.star.comp.chart2.PageBackground";
}

sal_Bool SAL_CALL PageBackground::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PageBackground::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart2.PageBackground",
        "com.sun.star.beans.PropertySet" };
}

using impl::PageBackground_Base;

IMPLEMENT_FORWARD_XINTERFACE2( PageBackground, PageBackground_Base, ::property::OPropertySet )

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_PageBackground_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PageBackground );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
