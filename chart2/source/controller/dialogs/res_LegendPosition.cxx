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
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "res_LegendPosition_IDs.hrc"
#include "ResId.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "LegendHelper.hxx"

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

LegendPositionResources::LegendPositionResources( Window* pWindow )
    : m_xCC() //unused in this scenario
    , m_aCbxShow( pWindow ) //unused in this scenario
    , m_aRbtLeft( pWindow, SchResId(RBT_LEFT) )
    , m_aRbtRight( pWindow, SchResId(RBT_RIGHT) )
    , m_aRbtTop( pWindow, SchResId(RBT_TOP) )
    , m_aRbtBottom( pWindow, SchResId(RBT_BOTTOM) )
{
    m_aCbxShow.Check();//legend is assumed to be visible in this scenario
    impl_setRadioButtonToggleHdl();
}

LegendPositionResources::LegendPositionResources( Window* pWindow, const uno::Reference< uno::XComponentContext >& xCC )
    : m_xCC( xCC )
    , m_aCbxShow( pWindow, SchResId(CBX_SHOWLEGEND) )
    , m_aRbtLeft( pWindow, SchResId(RBT_LEFT) )
    , m_aRbtRight( pWindow, SchResId(RBT_RIGHT) )
    , m_aRbtTop( pWindow, SchResId(RBT_TOP) )
    , m_aRbtBottom( pWindow, SchResId(RBT_BOTTOM) )
{
    m_aCbxShow.SetToggleHdl( LINK( this, LegendPositionResources, PositionEnableHdl ) );
    impl_setRadioButtonToggleHdl();
    m_aCbxShow.SetAccessibleRelationMemberOf(&m_aCbxShow);
    m_aRbtLeft.SetAccessibleRelationMemberOf(&m_aCbxShow);
    m_aRbtRight.SetAccessibleRelationMemberOf(&m_aCbxShow);
    m_aRbtTop.SetAccessibleRelationMemberOf(&m_aCbxShow);
    m_aRbtBottom.SetAccessibleRelationMemberOf(&m_aCbxShow);
}

void LegendPositionResources::impl_setRadioButtonToggleHdl()
{
    m_aRbtLeft.SetToggleHdl( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_aRbtTop.SetToggleHdl( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_aRbtRight.SetToggleHdl( LINK( this, LegendPositionResources, PositionChangeHdl ) );
    m_aRbtBottom.SetToggleHdl( LINK( this, LegendPositionResources, PositionChangeHdl ) );
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
            sal_Bool bShowLegend = sal_False;
            xProp->getPropertyValue( "Show" ) >>= bShowLegend;
            m_aCbxShow.Check( bShowLegend );
            PositionEnableHdl(0);

            //position
            chart2::LegendPosition ePos;
            xProp->getPropertyValue( "AnchorPosition" )  >>= ePos;
            switch( ePos )
            {
                case chart2::LegendPosition_LINE_START:
                    m_aRbtLeft.Check();
                    break;
                case chart2::LegendPosition_LINE_END:
                    m_aRbtRight.Check();
                    break;
                case chart2::LegendPosition_PAGE_START:
                    m_aRbtTop.Check();
                    break;
                case chart2::LegendPosition_PAGE_END:
                    m_aRbtBottom.Check();
                    break;

                case chart2::LegendPosition_CUSTOM:
                default:
                    m_aRbtRight.Check();
                    break;
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void LegendPositionResources::writeToModel( const ::com::sun::star::uno::Reference< frame::XModel >& xChartModel ) const
{
    try
    {
        sal_Bool bShowLegend = static_cast<sal_Bool>(m_aCbxShow.IsChecked());
        uno::Reference< beans::XPropertySet > xProp( LegendHelper::getLegend( xChartModel,m_xCC,bShowLegend ), uno::UNO_QUERY );
        if( xProp.is() )
        {
            //show
            xProp->setPropertyValue( "Show" , uno::makeAny( bShowLegend ));

            //position
            chart2::LegendPosition eNewPos;
            ::com::sun::star::chart::ChartLegendExpansion eExp = ::com::sun::star::chart::ChartLegendExpansion_HIGH;

            if( m_aRbtLeft.IsChecked() )
                eNewPos = chart2::LegendPosition_LINE_START;
            else if( m_aRbtRight.IsChecked() )
            {
                eNewPos = chart2::LegendPosition_LINE_END;
            }
            else if( m_aRbtTop.IsChecked() )
            {
                eNewPos = chart2::LegendPosition_PAGE_START;
                eExp = ::com::sun::star::chart::ChartLegendExpansion_WIDE;
            }
            else if( m_aRbtBottom.IsChecked() )
            {
                eNewPos = chart2::LegendPosition_PAGE_END;
                eExp = ::com::sun::star::chart::ChartLegendExpansion_WIDE;
            }

            xProp->setPropertyValue( "AnchorPosition" , uno::makeAny( eNewPos ));
            xProp->setPropertyValue( "Expansion" , uno::makeAny( eExp ));
            xProp->setPropertyValue( "RelativePosition" , uno::Any());
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

IMPL_LINK_NOARG(LegendPositionResources, PositionEnableHdl)
{
    sal_Bool bEnable = m_aCbxShow.IsChecked();

    m_aRbtLeft.Enable( bEnable );
    m_aRbtTop.Enable( bEnable );
    m_aRbtRight.Enable( bEnable );
    m_aRbtBottom.Enable( bEnable );

    m_aChangeLink.Call(NULL);

    return 0;
}

void LegendPositionResources::initFromItemSet( const SfxItemSet& rInAttrs )
{
    const SfxPoolItem* pPoolItem = NULL;
    if( rInAttrs.GetItemState( SCHATTR_LEGEND_POS, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_Int32 nLegendPosition = ((const SfxInt32Item*)pPoolItem)->GetValue();
        switch( nLegendPosition )
        {
            case chart2::LegendPosition_LINE_START:
                m_aRbtLeft.Check(sal_True);
                break;
            case chart2::LegendPosition_PAGE_START:
                m_aRbtTop.Check(sal_True);
                break;
            case chart2::LegendPosition_LINE_END:
                m_aRbtRight.Check(sal_True);
                break;
            case chart2::LegendPosition_PAGE_END:
                m_aRbtBottom.Check(sal_True);
                break;
            default:
                break;
        }
    }

    if( rInAttrs.GetItemState( SCHATTR_LEGEND_SHOW, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        bool bShow = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_aCbxShow.Check(bShow);
    }
}

void LegendPositionResources::writeToItemSet( SfxItemSet& rOutAttrs ) const
{
    sal_Int32 nLegendPosition = chart2::LegendPosition_CUSTOM;
    if( m_aRbtLeft.IsChecked() )
        nLegendPosition = chart2::LegendPosition_LINE_START;
    else if( m_aRbtTop.IsChecked() )
        nLegendPosition = chart2::LegendPosition_PAGE_START;
    else if( m_aRbtRight.IsChecked() )
        nLegendPosition = chart2::LegendPosition_LINE_END;
    else if( m_aRbtBottom.IsChecked() )
        nLegendPosition = chart2::LegendPosition_PAGE_END;
    rOutAttrs.Put(SfxInt32Item(SCHATTR_LEGEND_POS, nLegendPosition ));

    rOutAttrs.Put( SfxBoolItem(SCHATTR_LEGEND_SHOW, m_aCbxShow.IsChecked()) );
}

IMPL_LINK( LegendPositionResources, PositionChangeHdl, RadioButton*, pRadio )
{
    //for each radio click ther are coming two change events
    //first uncheck of previous button -> ignore that call
    //the second call gives the check of the new button
    if( pRadio && pRadio->IsChecked() )
        m_aChangeLink.Call(NULL);
    return 0;
}

void LegendPositionResources::SetChangeHdl( const Link& rLink )
{
    m_aChangeLink = rLink;
}

void LegendPositionResources::SetAccessibleRelationMemberOf(Window* pMemberOf)
{
    m_aRbtLeft.SetAccessibleRelationMemberOf(pMemberOf);
    m_aRbtRight.SetAccessibleRelationMemberOf(pMemberOf);
    m_aRbtTop.SetAccessibleRelationMemberOf(pMemberOf);
    m_aRbtBottom.SetAccessibleRelationMemberOf(pMemberOf);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
