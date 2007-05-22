/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_LegendPosition.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:39:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "res_LegendPosition.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "res_LegendPosition_IDs.hrc"
#include "ResId.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "LegendHelper.hxx"

#ifndef _SVT_CONTROLDIMS_HRC_
#include <svtools/controldims.hrc>
#endif

#ifndef _COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <com/sun/star/chart2/LegendPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_LEGENDEXPANSION_HPP_
#include <com/sun/star/chart2/LegendExpansion.hpp>
#endif

//itemset stuff
#include "chartview/ChartSfxItemIds.hxx"
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif
// header for class SfxItemPool
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LegendPositionResources::LegendPositionResources( Window* pWindow )
    : m_xCC() //unused in this scenario
    , m_aCbxShow( pWindow ) //unused in this scenario
    , m_aRbtLeft( pWindow, SchResId(RBT_LEFT) )
    , m_aRbtTop( pWindow, SchResId(RBT_TOP) )
    , m_aRbtRight( pWindow, SchResId(RBT_RIGHT) )
    , m_aRbtBottom( pWindow, SchResId(RBT_BOTTOM) )
{
    m_aCbxShow.Check();//legend is assumed to be visible in this scenario
    impl_setRadioButtonToggleHdl();
}

LegendPositionResources::LegendPositionResources( Window* pWindow, const uno::Reference< uno::XComponentContext >& xCC )
    : m_xCC( xCC )
    , m_aCbxShow( pWindow, SchResId(CBX_SHOWLEGEND) )
    , m_aRbtLeft( pWindow, SchResId(RBT_LEFT) )
    , m_aRbtTop( pWindow, SchResId(RBT_TOP) )
    , m_aRbtRight( pWindow, SchResId(RBT_RIGHT) )
    , m_aRbtBottom( pWindow, SchResId(RBT_BOTTOM) )
{
    m_aCbxShow.SetToggleHdl( LINK( this, LegendPositionResources, PositionEnableHdl ) );
    impl_setRadioButtonToggleHdl();
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
            xProp->getPropertyValue( C2U("Show") ) >>= bShowLegend;
            m_aCbxShow.Check( bShowLegend );
            PositionEnableHdl(0);

            //position
            chart2::LegendPosition ePos;
            xProp->getPropertyValue( C2U( "AnchorPosition" )) >>= ePos;
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
    catch( uno::Exception & ex )
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
            xProp->setPropertyValue( C2U("Show"), uno::makeAny( bShowLegend ));

            //position
            chart2::LegendPosition eNewPos;
            chart2::LegendExpansion eExp = chart2::LegendExpansion_HIGH;

            if( m_aRbtLeft.IsChecked() )
                eNewPos = chart2::LegendPosition_LINE_START;
            else if( m_aRbtRight.IsChecked() )
            {
                eNewPos = chart2::LegendPosition_LINE_END;
            }
            else if( m_aRbtTop.IsChecked() )
            {
                eNewPos = chart2::LegendPosition_PAGE_START;
                eExp = chart2::LegendExpansion_WIDE;
            }
            else if( m_aRbtBottom.IsChecked() )
            {
                eNewPos = chart2::LegendPosition_PAGE_END;
                eExp = chart2::LegendExpansion_WIDE;
            }

            xProp->setPropertyValue( C2U( "AnchorPosition" ), uno::makeAny( eNewPos ));
            xProp->setPropertyValue( C2U( "Expansion" ), uno::makeAny( eExp ));
            xProp->setPropertyValue( C2U( "RelativePosition" ), uno::Any());
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

IMPL_LINK( LegendPositionResources, PositionEnableHdl, void*, EMPTYARG )
{
    BOOL bEnable = m_aCbxShow.IsChecked();

    m_aRbtLeft.Enable( bEnable );
    m_aRbtTop.Enable( bEnable );
    m_aRbtRight.Enable( bEnable );
    m_aRbtBottom.Enable( bEnable );

    m_aChangeLink.Call(NULL);

    return 0;
}

void LegendPositionResources::initFromItemSet( const SfxItemSet& rInAttrs )
{
    SvxChartLegendPos ePos = CHLEGEND_NONE;

    const SfxPoolItem* pPoolItem = NULL;
    if( rInAttrs.GetItemState( SCHATTR_LEGEND_POS,
                               TRUE, &pPoolItem ) != SFX_ITEM_SET )
        pPoolItem = &(rInAttrs.GetPool()->GetDefaultItem( SCHATTR_LEGEND_POS ));

    if( pPoolItem )
        ePos = ((const SvxChartLegendPosItem*)pPoolItem)->GetValue();

    switch( ePos )
    {
        case CHLEGEND_LEFT:
            m_aRbtLeft.Check(TRUE);
            break;
        case CHLEGEND_TOP:
            m_aRbtTop.Check(TRUE);
            break;
        case CHLEGEND_RIGHT:
            m_aRbtRight.Check(TRUE);
            break;
        case CHLEGEND_BOTTOM:
            m_aRbtBottom.Check(TRUE);
            break;
        default:
            break;
    }
}

void LegendPositionResources::writeToItemSet( SfxItemSet& rOutAttrs ) const
{
    SvxChartLegendPos ePos;

    if( m_aRbtLeft.IsChecked() )
        ePos = CHLEGEND_LEFT;
    else if( m_aRbtTop.IsChecked() )
        ePos = CHLEGEND_TOP;
    else if( m_aRbtRight.IsChecked() )
        ePos = CHLEGEND_RIGHT;
    else if( m_aRbtBottom.IsChecked() )
        ePos = CHLEGEND_BOTTOM;
    else
        ePos = CHLEGEND_NONE;

    rOutAttrs.Put(SvxChartLegendPosItem( ePos, SCHATTR_LEGEND_POS ));
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

//.............................................................................
} //namespace chart
//.............................................................................

