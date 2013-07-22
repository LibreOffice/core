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


#include "ConfigColorScheme.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"

#include <unotools/configitem.hxx>
#include <sal/macros.h>

#include <set>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

static const OUString aSeriesPropName( "Series" );

} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{

uno::Reference< chart2::XColorScheme > createConfigColorScheme( const uno::Reference< uno::XComponentContext > & xContext )
{
    return new ConfigColorScheme( xContext );
}

namespace impl
{
class ChartConfigItem : public ::utl::ConfigItem
{
public:
    explicit ChartConfigItem( ConfigItemListener & rListener );
    virtual ~ChartConfigItem();

    void addPropertyNotification( const OUString & rPropertyName );

    uno::Any getProperty( const OUString & aPropertyName );

protected:
    // ____ ::utl::ConfigItem ____
    virtual void                    Commit();
    virtual void Notify( const Sequence< OUString > & aPropertyNames );

private:
    ConfigItemListener & m_rListener;
    ::std::set< OUString >        m_aPropertiesToNotify;
};

ChartConfigItem::ChartConfigItem( ConfigItemListener & rListener ) :
        ::utl::ConfigItem( "Office.Chart/DefaultColor" ),
    m_rListener( rListener )
{}

ChartConfigItem::~ChartConfigItem()
{}

void ChartConfigItem::Notify( const Sequence< OUString > & aPropertyNames )
{
    for( sal_Int32 nIdx=0; nIdx<aPropertyNames.getLength(); ++nIdx )
    {
        if( m_aPropertiesToNotify.find( aPropertyNames[nIdx] ) != m_aPropertiesToNotify.end())
            m_rListener.notify( aPropertyNames[nIdx] );
    }
}

void ChartConfigItem::Commit()
{}

void ChartConfigItem::addPropertyNotification( const OUString & rPropertyName )
{
    m_aPropertiesToNotify.insert( rPropertyName );
    EnableNotification( ContainerHelper::ContainerToSequence( m_aPropertiesToNotify ));
}

uno::Any ChartConfigItem::getProperty( const OUString & aPropertyName )
{
    Sequence< uno::Any > aValues(
        GetProperties( Sequence< OUString >( &aPropertyName, 1 )));
    if( ! aValues.getLength())
        return uno::Any();
    return aValues[0];
}

} // namespace impl

// --------------------------------------------------------------------------------

// explicit
ConfigColorScheme::ConfigColorScheme(
    const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext  ),
        m_nNumberOfColors( 0 ),
        m_bNeedsUpdate( true )
{
}

ConfigColorScheme::~ConfigColorScheme()
{}

void ConfigColorScheme::retrieveConfigColors()
{
    if( ! m_xContext.is())
        return;

    // create config item if necessary
    if( ! m_apChartConfigItem.get())
    {
        m_apChartConfigItem.reset(
            new impl::ChartConfigItem( *this ));
        m_apChartConfigItem->addPropertyNotification( aSeriesPropName );
    }
    OSL_ASSERT( m_apChartConfigItem.get());
    if( ! m_apChartConfigItem.get())
        return;

    // retrieve colors
    uno::Any aValue(
        m_apChartConfigItem->getProperty( aSeriesPropName ));
    if( aValue >>= m_aColorSequence )
        m_nNumberOfColors = m_aColorSequence.getLength();
    m_bNeedsUpdate = false;
}

// ____ XColorScheme ____
::sal_Int32 SAL_CALL ConfigColorScheme::getColorByIndex( ::sal_Int32 nIndex )
    throw (uno::RuntimeException)
{
    if( m_bNeedsUpdate )
        retrieveConfigColors();

    if( m_nNumberOfColors > 0 )
        return static_cast< sal_Int32 >( m_aColorSequence[ nIndex % m_nNumberOfColors ] );

    // fall-back: hard-coded standard colors
    static const sal_Int32 nDefaultColors[] =  {
        0x9999ff, 0x993366, 0xffffcc,
        0xccffff, 0x660066, 0xff8080,
        0x0066cc, 0xccccff, 0x000080,
        0xff00ff, 0x00ffff, 0xffff00
    };

    static const sal_Int32 nMaxDefaultColors = SAL_N_ELEMENTS( nDefaultColors );
    return nDefaultColors[ nIndex % nMaxDefaultColors ];
}

void ConfigColorScheme::notify( const OUString & rPropertyName )
{
    if( rPropertyName.equals( aSeriesPropName ))
        m_bNeedsUpdate = true;
}

// ================================================================================

Sequence< OUString > ConfigColorScheme::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = "com.sun.star.chart2.ColorScheme";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ConfigColorScheme,
                             OUString("com.sun.star.comp.chart2.ConfigDefaultColorScheme") )

// ================================================================================

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
