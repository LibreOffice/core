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
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif
#include <mmmergepage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <swtypes.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <dbui.hrc>
#include <mmmergepage.hrc>
#include <svl/srchitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <swabstdlg.hxx>

/*-- 02.04.2004 16:38:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeMergePage::SwMailMergeMergePage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage(_pParent, SW_RES(DLG_MM_MERGE_PAGE)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(this,           SW_RES(  FI_HEADER           ) ),
    m_aEditFI(this, SW_RES(           FI_EDIT )),
    m_aEditPB(this, SW_RES(           PB_EDIT )),
    m_aFindFL(this, SW_RES(           FL_FIND )),
    m_aFineFT(this, SW_RES(           FT_FIND )),
    m_aFindED(this, SW_RES(           ED_FIND )),
    m_aFindPB(this, SW_RES(           PB_FIND )),
    m_aWholeWordsCB(this, SW_RES(     CB_WHOLEWORDS)),
    m_aBackwardsCB(this, SW_RES(      CB_BACKWARDS )),
    m_aMatchCaseCB(this, SW_RES(      CB_MATCHCASE )),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_pWizard(_pParent)
{
    FreeResource();
    String sTemp(m_aEditFI.GetText());
    sTemp.SearchAndReplace(String::CreateFromAscii("%1"), m_aEditPB.GetText());
    m_aEditFI.SetText(sTemp);
    m_aEditPB.SetClickHdl( LINK( this, SwMailMergeMergePage, EditDocumentHdl_Impl));
    m_aFindPB.SetClickHdl( LINK( this, SwMailMergeMergePage, FindHdl_Impl ));

    m_aFindED.SetReturnActionLink( LINK(this, SwMailMergeMergePage, EnteredFindStringHdl_Impl ));

}
/*-- 02.04.2004 16:38:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeMergePage::~SwMailMergeMergePage()
{
}
/*-- 25.05.2004 16:14:49---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergeMergePage, EditDocumentHdl_Impl, PushButton*, EMPTYARG)
{
    m_pWizard->SetRestartPage(MM_MERGEPAGE);
    m_pWizard->EndDialog(RET_EDIT_RESULT_DOC);
    return 0;
}
/*-- 25.05.2004 16:14:49---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergeMergePage, FindHdl_Impl, PushButton*, EMPTYARG)
{
    SvxSearchItem aSearchItem( SID_SEARCH_ITEM );

    SfxBoolItem aQuiet( SID_SEARCH_QUIET, sal_False );
    aSearchItem.SetSearchString(m_aFindED.GetText());

    aSearchItem.SetWordOnly(m_aWholeWordsCB.IsChecked());
    aSearchItem.SetExact(m_aMatchCaseCB.IsChecked());
    aSearchItem.SetBackward(m_aBackwardsCB.IsChecked());

    SwView* pTargetView = m_pWizard->GetConfigItem().GetTargetView();
    DBG_ASSERT(pTargetView, "no target view exists");
    if(pTargetView)
    {
        pTargetView->GetViewFrame()->GetDispatcher()->Execute(
            FID_SEARCH_NOW, SFX_CALLMODE_SYNCHRON, &aSearchItem, &aQuiet, 0L );
    }

    return 0;
}
IMPL_LINK( SwMailMergeMergePage, EnteredFindStringHdl_Impl, void*, EMPTYARG )
{
    m_aFindPB.GetClickHdl().Call( &m_aFindPB );
    return 0;
}
