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
#include "dlg_InsertTitle.hxx"
#include "dlg_InsertTitle.hrc"
#include "res_Titles.hxx"
#include "ResId.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "ObjectNameProvider.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

SchTitleDlg::SchTitleDlg(Window* pWindow, const TitleDialogData& rInput )
    : ModalDialog(pWindow, SchResId(DLG_TITLE))
    , m_apTitleResources( new TitleResources(this,true) )
    , aBtnOK(this, SchResId(BTN_OK))
    , aBtnCancel(this, SchResId(BTN_CANCEL))
    , aBtnHelp(this, SchResId(BTN_HELP))
{
    FreeResource();

    this->SetText( ObjectNameProvider::getName(OBJECTTYPE_TITLE,true) );
    m_apTitleResources->writeToResources( rInput );
}

SchTitleDlg::~SchTitleDlg()
{
}

void SchTitleDlg::getResult( TitleDialogData& rOutput )
{
    m_apTitleResources->readFromResources( rOutput );
}

//.............................................................................
} //namespace chart
//.............................................................................

