/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "tp_LegendPosition.hxx"
#include "ResId.hxx"
#include "TabPages.hrc"
#include "res_LegendPosition.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"
#include <svx/chrtitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

SchLegendPosTabPage::SchLegendPosTabPage(Window* pWindow,
                                         const SfxItemSet& rInAttrs)
    : SfxTabPage( pWindow, SchResId(TP_LEGEND_POS), rInAttrs )
    , aGrpLegend( this, SchResId(GRP_LEGEND) )
    , m_apLegendPositionResources( new LegendPositionResources(this) )
    , m_aFlTextOrient( this, SchResId( FL_LEGEND_TEXTORIENT ) )
    , m_aFtTextDirection( this, SchResId( FT_LEGEND_TEXTDIR ) )
    , m_aLbTextDirection( this, SchResId( LB_LEGEND_TEXTDIR ), &m_aFlTextOrient, &m_aFtTextDirection )
{
    m_apLegendPositionResources->SetAccessibleRelationMemberOf(&aGrpLegend);
    FreeResource();
}

SchLegendPosTabPage::~SchLegendPosTabPage()
{
}

SfxTabPage* SchLegendPosTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new SchLegendPosTabPage(pWindow, rOutAttrs);
}

sal_Bool SchLegendPosTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apLegendPositionResources->writeToItemSet(rOutAttrs);

    if( m_aLbTextDirection.GetSelectEntryCount() > 0 )
        rOutAttrs.Put( SfxInt32Item( EE_PARA_WRITINGDIR, m_aLbTextDirection.GetSelectEntryValue() ) );

    return sal_True;
}

void SchLegendPosTabPage::Reset(const SfxItemSet& rInAttrs)
{
    m_apLegendPositionResources->initFromItemSet(rInAttrs);

    const SfxPoolItem* pPoolItem = 0;
    if( rInAttrs.GetItemState( EE_PARA_WRITINGDIR, sal_True, &pPoolItem ) == SFX_ITEM_SET )
        m_aLbTextDirection.SelectEntryValue( SvxFrameDirection(((const SvxFrameDirectionItem*)pPoolItem)->GetValue()) );
}

//.............................................................................
} //namespace chart
//.............................................................................
