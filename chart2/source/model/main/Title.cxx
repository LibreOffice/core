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

#include "Title.hxx"
#include "macros.hxx"
#include "FormattedString.hxx"
#include "LinePropertiesHelper.hxx"
#include "FillProperties.hxx"
#include "ContainerHelper.hxx"
#include "CloneHelper.hxx"
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <rtl/uuid.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

static const char lcl_aServiceName[] = "com.sun.star.comp.chart2.Title";

enum
{
    PROP_TITLE_PARA_ADJUST,
    PROP_TITLE_PARA_LAST_LINE_ADJUST,
    PROP_TITLE_PARA_LEFT_MARGIN,
    PROP_TITLE_PARA_RIGHT_MARGIN,
    PROP_TITLE_PARA_TOP_MARGIN,
    PROP_TITLE_PARA_BOTTOM_MARGIN,
    PROP_TITLE_PARA_IS_HYPHENATION,

    PROP_TITLE_TEXT_ROTATION,
    PROP_TITLE_TEXT_STACKED,
    PROP_TITLE_REL_POS,

    PROP_TITLE_REF_PAGE_SIZE
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "ParaAdjust",
                  PROP_TITLE_PARA_ADJUST,
                  cppu::UnoType<com::sun::star::style::ParagraphAdjust>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ParaLastLineAdjust",
                  PROP_TITLE_PARA_LAST_LINE_ADJUST,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ParaLeftMargin",
                  PROP_TITLE_PARA_LEFT_MARGIN,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ParaRightMargin",
                  PROP_TITLE_PARA_RIGHT_MARGIN,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ParaTopMargin",
                  PROP_TITLE_PARA_TOP_MARGIN,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ParaBottomMargin",
                  PROP_TITLE_PARA_BOTTOM_MARGIN,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ParaIsHyphenation",
                  PROP_TITLE_PARA_IS_HYPHENATION,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "TextRotation",
                  PROP_TITLE_TEXT_ROTATION,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "StackCharacters",
                  PROP_TITLE_TEXT_STACKED,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "RelativePosition",
                  PROP_TITLE_REL_POS,
                  cppu::UnoType<chart2::RelativePosition>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "ReferencePageSize",
                  PROP_TITLE_REF_PAGE_SIZE,
                  cppu::UnoType<awt::Size>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
}

struct StaticTitleDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::LinePropertiesHelper::AddDefaultsToMap( rOutMap );
        ::chart::FillProperties::AddDefaultsToMap( rOutMap );

        // ParagraphProperties
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_TITLE_PARA_ADJUST,
                                                          ::com::sun::star::style::ParagraphAdjust_CENTER );
        // PROP_TITLE_PARA_LAST_LINE_ADJUST

        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_TITLE_PARA_LEFT_MARGIN, 0 );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_TITLE_PARA_RIGHT_MARGIN, 0 );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_TITLE_PARA_TOP_MARGIN, 0 );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_TITLE_PARA_BOTTOM_MARGIN, 0 );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_TITLE_PARA_IS_HYPHENATION, true );

        // own properties
        ::chart::PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_TITLE_TEXT_ROTATION, 0.0 );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_TITLE_TEXT_STACKED, false );

        // override other defaults
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::FillProperties::PROP_FILL_STYLE, drawing::FillStyle_NONE );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::LinePropertiesHelper::PROP_LINE_STYLE, drawing::LineStyle_NONE );
    }
};

struct StaticTitleDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticTitleDefaults_Initializer >
{
};

struct StaticTitleInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    static uno::Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }

};

struct StaticTitleInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticTitleInfoHelper_Initializer >
{
};

struct StaticTitleInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticTitleInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticTitleInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticTitleInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

Title::Title( uno::Reference< uno::XComponentContext > const & /* xContext */ ) :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

Title::Title( const Title & rOther ) :
        MutexContainer(),
        impl::Title_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    CloneHelper::CloneRefSequence< uno::Reference< chart2::XFormattedString > >(
        rOther.m_aStrings, m_aStrings );
    ModifyListenerHelper::addListenerToAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
}

Title::~Title()
{
    ModifyListenerHelper::removeListenerFromAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL Title::createClone()
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< util::XCloneable >( new Title( *this ));
}

// ____ XTitle ____
uno::Sequence< uno::Reference< chart2::XFormattedString > > SAL_CALL Title::getText()
    throw (uno::RuntimeException, std::exception)
{
    MutexGuard aGuard( GetMutex() );
    return m_aStrings;
}

void SAL_CALL Title::setText( const uno::Sequence< uno::Reference< chart2::XFormattedString > >& rNewStrings )
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Reference< chart2::XFormattedString > > aOldStrings;
    {
        MutexGuard aGuard( GetMutex() );
        std::swap( m_aStrings, aOldStrings );
        m_aStrings = rNewStrings;
    }
    //don't keep the mutex locked while calling out
    ModifyListenerHelper::removeListenerFromAllElements(
        ContainerHelper::SequenceToVector( aOldStrings ), m_xModifyEventForwarder );
    ModifyListenerHelper::addListenerToAllElements(
        ContainerHelper::SequenceToVector( rNewStrings ), m_xModifyEventForwarder );
    fireModifyEvent();
}

// ____ OPropertySet ____
uno::Any Title::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticTitleDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL Title::getInfoHelper()
{
    return *StaticTitleInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL Title::getPropertySetInfo()
    throw (uno::RuntimeException, std::exception)
{
    return *StaticTitleInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL Title::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException, std::exception)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL Title::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException, std::exception)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL Title::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException, std::exception)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Title::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException, std::exception)
{
    // nothing
}

// ____ OPropertySet ____
void Title::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void Title::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

uno::Sequence< OUString > Title::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = "com.sun.star.chart2.Title";
    aServices[ 1 ] = "com.sun.star.style.ParagraphProperties";
    aServices[ 2 ] = "com.sun.star.beans.PropertySet";
    aServices[ 3 ] = "com.sun.star.layout.LayoutElement";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL Title::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString Title::getImplementationName_Static()
{
    return OUString(lcl_aServiceName);
}

sal_Bool SAL_CALL Title::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL Title::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

// needed by MSC compiler
using impl::Title_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Title, Title_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Title, Title_Base, ::property::OPropertySet )

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_Title_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::Title(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
