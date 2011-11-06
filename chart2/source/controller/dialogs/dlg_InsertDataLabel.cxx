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

#include "dlg_InsertDataLabel.hxx"
#include "dlg_InsertDataLabel.hrc"
#include "ResId.hxx"
#include "ObjectNameProvider.hxx"
#include "res_DataLabel.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

DataLabelsDialog::DataLabelsDialog(Window* pWindow, const SfxItemSet& rInAttrs, SvNumberFormatter* pFormatter) :
    ModalDialog(pWindow, SchResId(DLG_DATA_DESCR)),
    m_aBtnOK(this, SchResId(BTN_OK)),
    m_aBtnCancel(this, SchResId(BTN_CANCEL)),
    m_aBtnHelp(this, SchResId(BTN_HELP)),
    m_apDataLabelResources( new DataLabelResources(this,rInAttrs) ),
    m_rInAttrs(rInAttrs)
{
    FreeResource();
    this->SetText( ObjectNameProvider::getName_ObjectForAllSeries( OBJECTTYPE_DATA_LABELS ) );

    m_apDataLabelResources->SetNumberFormatter( pFormatter );
    Reset();
}

DataLabelsDialog::~DataLabelsDialog()
{
}

void DataLabelsDialog::Reset()
{
    m_apDataLabelResources->Reset(m_rInAttrs);
}

void DataLabelsDialog::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apDataLabelResources->FillItemSet(rOutAttrs);
}

//.............................................................................
} //namespace chart
//.............................................................................

