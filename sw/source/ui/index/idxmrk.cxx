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


#include <hintids.hxx>
#ifndef _HELPID_H
#include <helpid.h>
#endif
#define _SVSTDARR_STRINGSSORT
#include <svl/svstdarr.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <svl/stritem.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <svtools/txtcmp.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svl/itemset.hxx>
#include <editeng/langitem.hxx>
#include <swtypes.hxx>
#include <idxmrk.hxx>
#include <txttxmrk.hxx>
#include <wrtsh.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <multmrk.hxx>
#include <swundo.hxx>                   // fuer Undo-Ids
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _INDEX_HRC
#include <index.hrc>
#endif
#ifndef _IDXMRK_HRC
#include <idxmrk.hrc>
#endif
#include <swmodule.hxx>
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include <utlui.hrc>
#include <swcont.hxx>
#include <svl/cjkoptions.hxx>
#include <ndtxt.hxx>
#include <breakit.hxx>


/* -----------------07.09.99 08:15-------------------

 --------------------------------------------------*/
SFX_IMPL_CHILDWINDOW(SwInsertIdxMarkWrapper, FN_INSERT_IDX_ENTRY_DLG)

SwInsertIdxMarkWrapper::SwInsertIdxMarkWrapper( Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");
    pAbstDlg = pFact->CreateIndexMarkFloatDlg( DLG_INSIDXMARK , pBindings, this, pParentWindow, pInfo );
    DBG_ASSERT(pAbstDlg, "Dialogdiet fail!");
    pWindow = pAbstDlg->GetWindow();
    pWindow->Show();    // at this point,because before pSh has to be initialized in ReInitDlg()
                        // -> Show() will invoke StateChanged() and save pos
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}
/* -----------------07.09.99 09:14-------------------

 --------------------------------------------------*/
SfxChildWinInfo SwInsertIdxMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();

    return aInfo;
}

void    SwInsertIdxMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    pAbstDlg->ReInitDlg(rWrtShell);
}


/* -----------------07.09.99 08:15-------------------

 --------------------------------------------------*/
SFX_IMPL_CHILDWINDOW(SwInsertAuthMarkWrapper, FN_INSERT_AUTH_ENTRY_DLG)

SwInsertAuthMarkWrapper::SwInsertAuthMarkWrapper(   Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");
    pAbstDlg = pFact->CreateAuthMarkFloatDlg( DLG_INSAUTHMARK, pBindings, this, pParentWindow, pInfo );
    DBG_ASSERT(pAbstDlg, "Dialogdiet fail!");
    pWindow = pAbstDlg->GetWindow();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}
/* -----------------07.09.99 09:14-------------------

 --------------------------------------------------*/
SfxChildWinInfo SwInsertAuthMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}
/* -----------------19.10.99 11:16-------------------

 --------------------------------------------------*/
void    SwInsertAuthMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    pAbstDlg->ReInitDlg(rWrtShell);
}

