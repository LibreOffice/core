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

#include "res_LegendPosition.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "LegendHelper.hxx"
#include "ChartModel.hxx"

#include <svtools/controldims.hrc>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>

//itemset stuff
#include "chartview/ChartSfxItemIds.hxx"
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LegendPositionResources::LegendPositionResources(VclBuilderContainer& rParent)
    : m_xCC() //unused in this scenario
    , m_pCbxShow( nullptr ) //unused in this scenario, assumed to be visible
{
    rParent.get(m_pRbtLeft, "left");
    rParent.get(m_pRbtRight, "right");
    rParent.get(m_pRbtTop, "top");
    rParent.get(m_pRbtBottom, "bottom");
    impl_setRadioButtonToggleHdl();
}

LegendPositionResources::LegendPositionResources(VclBuilderContainer& rParent,
    const uno::Reference< uno::XComponentContext >& xCC)
    : m_xCC(xCC)
{
    rParent.get(m_pCbxShow, "show");
    rParent.get(m_pRbtLeft, "left");
    rParent.get(m_pRbtRight, "right");
    rParent.get(m_pRbtTop, "top");
    rParent.get(m_pRbtBottom, "bottom");

    m_pCbxShow->SetToggleHdl( LINK( this, LegendPositionResources, PositionEnableHdl ) );
    impl_setRadioButtonToggleHdl();
}

void LegendPositionResources::impl_setRadioButtonToggleHdl()
{
    m_pRbtLeft->SetToggleHdl( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_pRbtTop->SetToggleHdl( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_pRbtRight->SetToggleHdl( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_pRbtBottom->SetToggleHdl( LINK( this, LegendPositionResources, PositionChangeHdl ) );
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
            if (m_pCbxShow)
                m_pCbxShow->Check( bShowLegend );
            PositionEnableHdl(*m_pCbxShow);

            //position
            chart2::LegendPosition ePos;
            xProp->getPropertyValue( "AnchorPosition" )  >>= ePos;
            switch( ePos )
            {
                case chart2::LegendPosition_LINE_START:
                    m_pRbtLeft->Check();
                    break;
                case chart2::LegendPosition_LINE_END:
                    m_pRbtRight->Check();
                    break;
                case chart2::LegendPosition_PAGE_START:
                    m_pRbtTop->Check();
                    break;
                case chart2::LegendPosition_PAGE_END:
                    m_pRbtBottom->Check();
                    break;

                case chart2::LegendPosition_CUSTOM:
                default:
                    m_pRbtRight->Check();
                    break;
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void LegendPositionResources::writeToModel( const css::uno::Reference< frame::XModel >& xChartModel ) const
{
    try
    {
        bool bShowLegend = m_pCbxShow && m_pCbxShow->IsChecked();
        ChartModel& rModel = dynamic_cast<ChartModel&>(*xChartModel.get());
        uno::Reference< beans::XPropertySet > xProp(LegendHelper::getLegend(rModel, m_xCC, bShowLegend), uno::UNO_QUERY);
        if( xProp.is() )
        {
            //show
            xProp->setPropertyValue( "Show" , uno::Any( bShowLegend ));

            //position
            chart2::LegendPosition eNewPos;
            css::chart::ChartLegendExpansion eExp = css::chart::ChartLegendExpansion_HIGH;

            if( m_pRbtLeft->IsChecked() )
                eNewPos = chart2::LegendPosition_LINE_START;
            else if( m_pRbtRight->IsChecked() )
            {
                eNewPos = chart2::LegendPosition_LINE_END;
            }
            else if( m_pRbtTop->IsChecked() )
            {
                eNewPos = chart2::LegendPosition_PAGE_START;
                eExp = css::chart::ChartLegendExpansion_WIDE;
            }
            else if( m_pRbtBottom->IsChecked() )
            {
                eNewPos = chart2::LegendPosition_PAGE_END;
                eExp = css::chart::ChartLegendExpansion_WIDE;
            }

            xProp->setPropertyValue( "AnchorPosition" , uno::Any( eNewPos ));
            xProp->setPropertyValue( "Expansion" , uno::Any( eExp ));
            xProp->setPropertyValue( "RelativePosition" , uno::Any());
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

IMPL_LINK_NOARG(LegendPositionResources, PositionEnableHdl, CheckBox&, void)
{
    bool bEnable = m_pCbxShow == nullptr || m_pCbxShow->IsChecked();

    m_pRbtLeft->Enable( bEnable );
    m_pRbtTop->Enable( bEnable );
    m_pRbtRight->Enable( bEnable );
    m_pRbtBottom->Enable( bEnable );

    m_aChangeLink.Call(nullptr);
}

void LegendPositionResources::initFromItemSet( const SfxItemSet& rInAttrs )
{
    const SfxPoolItem* pPoolItem = nullptr;
    if( rInAttrs.GetItemState( SCHATTR_LEGEND_POS, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nLegendPosition = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        switch( nLegendPosition )
        {
            case chart2::LegendPosition_LINE_START:
                m_pRbtLeft->Check();
                break;
            case chart2::LegendPosition_PAGE_START:
                m_pRbtTop->Check();
                break;
            case chart2::LegendPosition_LINE_END:
                m_pRbtRight->Check();
                break;
            case chart2::LegendPosition_PAGE_END:
                m_pRbtBottom->Check();
                break;
            default:
                break;
        }
    }

    if( m_pCbxShow && rInAttrs.GetItemState( SCHATTR_LEGEND_SHOW, true, &pPoolItem ) == SfxItemState::SET )
    {
        bool bShow = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_pCbxShow->Check(bShow);
    }
}

void LegendPositionResources::writeToItemSet( SfxItemSet& rOutAttrs ) const
{
    sal_Int32 nLegendPosition = chart2::LegendPosition_CUSTOM;
    if( m_pRbtLeft->IsChecked() )
        nLegendPosition = chart2::LegendPosition_LINE_START;
    else if( m_pRbtTop->IsChecked() )
        nLegendPosition = chart2::LegendPosition_PAGE_START;
    else if( m_pRbtRight->IsChecked() )
        nLegendPosition = chart2::LegendPosition_LINE_END;
    else if( m_pRbtBottom->IsChecked() )
        nLegendPosition = chart2::LegendPosition_PAGE_END;
    rOutAttrs.Put(SfxInt32Item(SCHATTR_LEGEND_POS, nLegendPosition ));

    rOutAttrs.Put( SfxBoolItem(SCHATTR_LEGEND_SHOW, m_pCbxShow == nullptr || m_pCbxShow->IsChecked()) );
}

IMPL_LINK( LegendPositionResources, PositionChangeHdl, RadioButton&, rRadio, void )
{
    //for each radio click there are coming two change events
    //first uncheck of previous button -> ignore that call
    //the second call gives the check of the new button
    if( rRadio.IsChecked() )
        m_aChangeLink.Call(nullptr);
}

void LegendPositionResources::SetChangeHdl( const Link<LinkParamNone*,void>& rLink )
{
    m_aChangeLink = rLink;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
