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

#include "tp_Trendline.hxx"
#include "ResId.hxx"
#include "TabPages.hrc"

//.............................................................................
namespace chart
{
//.............................................................................

TrendlineTabPage::TrendlineTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage( pParent, SchResId( TP_TRENDLINE ), rInAttrs ),
        m_aTrendlineResources( this, rInAttrs, false )
{
    FreeResource();
}

TrendlineTabPage::~TrendlineTabPage()
{
}

SfxTabPage* TrendlineTabPage::Create(
    Window* pParent, const SfxItemSet& rOutAttrs )
{
    return new TrendlineTabPage( pParent, rOutAttrs );
}

sal_Bool TrendlineTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    return m_aTrendlineResources.FillItemSet( rOutAttrs );
}

void TrendlineTabPage::Reset( const SfxItemSet& rInAttrs )
{
    m_aTrendlineResources.Reset( rInAttrs );
}

void TrendlineTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        m_aTrendlineResources.FillValueSets();
}

//.............................................................................
} //namespace chart
//.............................................................................
