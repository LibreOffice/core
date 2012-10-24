/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <hintids.hxx>
#include <helpid.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
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
#include <view.hxx>
#include <multmrk.hxx>
#include <swundo.hxx>                   // for Undo-Ids
#include <cmdid.h>
#include <index.hrc>
#include <idxmrk.hrc>
#include <swmodule.hxx>
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include <utlui.hrc>
#include <swcont.hxx>
#include <svl/cjkoptions.hxx>
#include <ndtxt.hxx>
#include <breakit.hxx>

SFX_IMPL_CHILDWINDOW_WITHID(SwInsertIdxMarkWrapper, FN_INSERT_IDX_ENTRY_DLG)

SwInsertIdxMarkWrapper::SwInsertIdxMarkWrapper( Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
    pAbstDlg = pFact->CreateIndexMarkFloatDlg(pBindings, this, pParentWindow, pInfo);
    OSL_ENSURE(pAbstDlg, "Dialogdiet fail!");
    pWindow = pAbstDlg->GetWindow();
    pWindow->Show();    // at this point,because before pSh has to be initialized in ReInitDlg()
                        // -> Show() will invoke StateChanged() and save pos
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}

SfxChildWinInfo SwInsertIdxMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();

    return aInfo;
}

void    SwInsertIdxMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    pAbstDlg->ReInitDlg(rWrtShell);
}

SFX_IMPL_CHILDWINDOW_WITHID(SwInsertAuthMarkWrapper, FN_INSERT_AUTH_ENTRY_DLG)

SwInsertAuthMarkWrapper::SwInsertAuthMarkWrapper(   Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
    pAbstDlg = pFact->CreateAuthMarkFloatDlg(pBindings, this, pParentWindow, pInfo);
    OSL_ENSURE(pAbstDlg, "Dialogdiet fail!");
    pWindow = pAbstDlg->GetWindow();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}

SfxChildWinInfo SwInsertAuthMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

void    SwInsertAuthMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    pAbstDlg->ReInitDlg(rWrtShell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
