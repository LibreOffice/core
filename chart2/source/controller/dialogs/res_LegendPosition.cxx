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

#include <res_LegendPosition.hxx>
#include <ChartModelHelper.hxx>
#include <LegendHelper.hxx>
#include <ChartModel.hxx>

#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>

//itemset stuff
#include <chartview/ChartSfxItemIds.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/weld.hxx>

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LegendPositionResources::LegendPositionResources(weld::Builder& rBuilder)
    : m_xRbtLeft(rBuilder.weld_radio_button("left"))
    , m_xRbtRight(rBuilder.weld_radio_button("right"))
    , m_xRbtTop(rBuilder.weld_radio_button("top"))
    , m_xRbtBottom(rBuilder.weld_radio_button("bottom"))
{
    impl_setRadioButtonToggleHdl();
}

LegendPositionResources::LegendPositionResources(weld::Builder& rBuilder,
    const uno::Reference< uno::XComponentContext >& xCC)
    : m_xCC(xCC)
    , m_xCbxShow(rBuilder.weld_check_button("show"))
    , m_xRbtLeft(rBuilder.weld_radio_button("left"))
    , m_xRbtRight(rBuilder.weld_radio_button("right"))
    , m_xRbtTop(rBuilder.weld_radio_button("top"))
    , m_xRbtBottom(rBuilder.weld_radio_button("bottom"))
{
    m_xCbxShow->connect_toggled( LINK( this, LegendPositionResources, PositionEnableHdl ) );
    impl_setRadioButtonToggleHdl();
}

void LegendPositionResources::impl_setRadioButtonToggleHdl()
{
    m_xRbtLeft->connect_toggled( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_xRbtTop->connect_toggled( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_xRbtRight->connect_toggled( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_xRbtBottom->connect_toggled( LINK( this, LegendPositionResources, PositionChangeHdl ) );
}

LegendPositionResources::~LegendPositionResources()
{
}

void LegendPositionResources::writeToResources( const uno::Reference< frame::XModel >& xChartModel )
{
    try
    {
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( xChartModel );
        uno::Reference< beans::XPropertySet > xProp( xDiagram->getLegend(), uno::UNO_QUERY );
        if( xProp.is() )
        {
            //show
            bool bShowLegend = false;
            xProp->getPropertyValue( "Show" ) >>= bShowLegend;
            if (m_xCbxShow)
                m_xCbxShow->set_active( bShowLegend );
            PositionEnableHdl(*m_xCbxShow);

            //position
            chart2::LegendPosition ePos;
            xProp->getPropertyValue( "AnchorPosition" )  >>= ePos;
            switch( ePos )
            {
                case chart2::LegendPosition_LINE_START:
                    m_xRbtLeft->set_active(true);
                    break;
                case chart2::LegendPosition_LINE_END:
                    m_xRbtRight->set_active(true);
                    break;
                case chart2::LegendPosition_PAGE_START:
                    m_xRbtTop->set_active(true);
                    break;
                case chart2::LegendPosition_PAGE_END:
                    m_xRbtBottom->set_active(true);
                    break;
                case chart2::LegendPosition_CUSTOM:
                default:
                    m_xRbtRight->set_active(true);
                    break;
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void LegendPositionResources::writeToModel( const css::uno::Reference< frame::XModel >& xChartModel ) const
{
    try
    {
        bool bShowLegend = m_xCbxShow && m_xCbxShow->get_active();
        ChartModel& rModel = dynamic_cast<ChartModel&>(*xChartModel);
        uno::Reference< beans::XPropertySet > xProp(LegendHelper::getLegend(rModel, m_xCC, bShowLegend), uno::UNO_QUERY);
        if( xProp.is() )
        {
            //show
            xProp->setPropertyValue( "Show" , uno::Any( bShowLegend ));

            //position
            chart2::LegendPosition eNewPos;
            css::chart::ChartLegendExpansion eExp = css::chart::ChartLegendExpansion_HIGH;

            if( m_xRbtLeft->get_active() )
                eNewPos = chart2::LegendPosition_LINE_START;
            else if( m_xRbtRight->get_active() )
            {
                eNewPos = chart2::LegendPosition_LINE_END;
            }
            else if( m_xRbtTop->get_active() )
            {
                eNewPos = chart2::LegendPosition_PAGE_START;
                eExp = css::chart::ChartLegendExpansion_WIDE;
            }
            else if( m_xRbtBottom->get_active() )
            {
                eNewPos = chart2::LegendPosition_PAGE_END;
                eExp = css::chart::ChartLegendExpansion_WIDE;
            }

            xProp->setPropertyValue( "AnchorPosition" , uno::Any( eNewPos ));
            xProp->setPropertyValue( "Expansion" , uno::Any( eExp ));
            xProp->setPropertyValue( "RelativePosition" , uno::Any());
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2" );
    }
}

IMPL_LINK_NOARG(LegendPositionResources, PositionEnableHdl, weld::ToggleButton&, void)
{
    bool bEnable = !m_xCbxShow || m_xCbxShow->get_active();

    m_xRbtLeft->set_sensitive( bEnable );
    m_xRbtTop->set_sensitive( bEnable );
    m_xRbtRight->set_sensitive( bEnable );
    m_xRbtBottom->set_sensitive( bEnable );

    m_aChangeLink.Call(nullptr);
}

void LegendPositionResources::initFromItemSet( const SfxItemSet& rInAttrs )
{
    const SfxPoolItem* pPoolItem = nullptr;
    if( rInAttrs.GetItemState( SCHATTR_LEGEND_POS, true, &pPoolItem ) == SfxItemState::SET )
    {
        chart2::LegendPosition nLegendPosition = static_cast<chart2::LegendPosition>(static_cast<const SfxInt32Item*>(pPoolItem)->GetValue());
        switch( nLegendPosition )
        {
            case chart2::LegendPosition_LINE_START:
                m_xRbtLeft->set_active(true);
                break;
            case chart2::LegendPosition_PAGE_START:
                m_xRbtTop->set_active(true);
                break;
            case chart2::LegendPosition_LINE_END:
                m_xRbtRight->set_active(true);
                break;
            case chart2::LegendPosition_PAGE_END:
                m_xRbtBottom->set_active(true);
                break;
            default:
                break;
        }
    }

    if( m_xCbxShow && rInAttrs.GetItemState( SCHATTR_LEGEND_SHOW, true, &pPoolItem ) == SfxItemState::SET )
    {
        bool bShow = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_xCbxShow->set_active(bShow);
    }
}

void LegendPositionResources::writeToItemSet( SfxItemSet& rOutAttrs ) const
{
    chart2::LegendPosition nLegendPosition = chart2::LegendPosition_CUSTOM;
    if( m_xRbtLeft->get_active() )
        nLegendPosition = chart2::LegendPosition_LINE_START;
    else if( m_xRbtTop->get_active() )
        nLegendPosition = chart2::LegendPosition_PAGE_START;
    else if( m_xRbtRight->get_active() )
        nLegendPosition = chart2::LegendPosition_LINE_END;
    else if( m_xRbtBottom->get_active() )
        nLegendPosition = chart2::LegendPosition_PAGE_END;
    rOutAttrs.Put( SfxInt32Item(SCHATTR_LEGEND_POS, static_cast<sal_Int32>(nLegendPosition) ) );

    rOutAttrs.Put( SfxBoolItem(SCHATTR_LEGEND_SHOW, !m_xCbxShow || m_xCbxShow->get_active()) );
}

IMPL_LINK (LegendPositionResources, PositionChangeHdl, weld::ToggleButton&, rRadio, void)
{
    //for each radio click there are coming two change events
    //first uncheck of previous button -> ignore that call
    //the second call gives the check of the new button
    if( rRadio.get_active() )
        m_aChangeLink.Call(nullptr);
}

void LegendPositionResources::SetChangeHdl( const Link<LinkParamNone*,void>& rLink )
{
    m_aChangeLink = rLink;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
