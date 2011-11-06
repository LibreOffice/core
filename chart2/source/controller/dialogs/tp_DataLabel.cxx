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
#include "tp_DataLabel.hxx"

#include "TabPages.hrc"
#include "ResId.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

DataLabelsTabPage::DataLabelsTabPage(Window* pWindow, const SfxItemSet& rInAttrs) :
        SfxTabPage(pWindow, SchResId(TP_DATA_DESCR), rInAttrs),
        m_aDataLabelResources(this, rInAttrs)
{
    FreeResource();
}

DataLabelsTabPage::~DataLabelsTabPage()
{
}

SfxTabPage* DataLabelsTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new DataLabelsTabPage(pWindow, rOutAttrs);
}

sal_Bool DataLabelsTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    return m_aDataLabelResources.FillItemSet(rOutAttrs);
}

void DataLabelsTabPage::Reset(const SfxItemSet& rInAttrs)
{
    m_aDataLabelResources.Reset(rInAttrs);
}

void DataLabelsTabPage::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    m_aDataLabelResources.SetNumberFormatter( pFormatter );
}

//.............................................................................
} //namespace chart
//.............................................................................
