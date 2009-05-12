/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterPagesPanel.cxx,v $
 * $Revision: 1.14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "MasterPagesPanel.hxx"
#include "taskpane/ScrollPanel.hxx"
#include "CurrentMasterPagesSelector.hxx"
#include "RecentMasterPagesSelector.hxx"
#include "AllMasterPagesSelector.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"
#include "taskpane/TitledControl.hxx"
#include "../TaskPaneShellManager.hxx"

#include "DrawViewShell.hxx"
#include "ViewShellBase.hxx"

#include "strings.hrc"
#include "sdresid.hxx"
#include "helpids.h"
#include <svtools/valueset.hxx>

namespace sd { namespace toolpanel { namespace controls {


MasterPagesPanel::MasterPagesPanel (TreeNode* pParent, ViewShellBase& rBase)
    : ScrollPanel (pParent)
{
    SdDrawDocument* pDocument = rBase.GetDocument();
    ::std::auto_ptr<controls::MasterPagesSelector> pSelector;
    TitledControl* pTitledControl;

    ::boost::shared_ptr<MasterPageContainer> pContainer (new MasterPageContainer());

    // Create a panel with the master pages that are in use by the currently
    // edited document.
    DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(rBase.GetMainViewShell().get());
    pSelector.reset(new controls::CurrentMasterPagesSelector (
        this,
        *pDocument,
        rBase,
        pContainer));
    pSelector->LateInit();
    pSelector->SetSmartHelpId( SmartId(HID_SD_TASK_PANE_PREVIEW_CURRENT) );
    GetShellManager()->AddSubShell(
        HID_SD_TASK_PANE_PREVIEW_CURRENT,
        pSelector.get(),
        pSelector->GetWindow());
    pTitledControl = AddControl (
        ::std::auto_ptr<TreeNode>(pSelector.release()),
        SdResId(STR_TASKPANEL_CURRENT_MASTER_PAGES_TITLE),
        HID_SD_CURRENT_MASTERS);

    // Create a panel with the most recently used master pages.
    pSelector.reset(new controls::RecentMasterPagesSelector (
        this,
        *pDocument,
        rBase,
        pContainer));
    pSelector->LateInit();
    pSelector->SetSmartHelpId( SmartId(HID_SD_TASK_PANE_PREVIEW_RECENT) );
    GetShellManager()->AddSubShell(
        HID_SD_TASK_PANE_PREVIEW_RECENT,
        pSelector.get(),
        pSelector->GetWindow());
    pTitledControl = AddControl (
        ::std::auto_ptr<TreeNode>(pSelector.release()),
        SdResId(STR_TASKPANEL_RECENT_MASTER_PAGES_TITLE),
        HID_SD_RECENT_MASTERS);

    // Create a panel with all available master pages.
    pSelector.reset(new controls::AllMasterPagesSelector (
        this,
        *pDocument,
        rBase,
        *pDrawViewShell,
        pContainer));
    pSelector->LateInit();
    pSelector->SetSmartHelpId( SmartId(HID_SD_TASK_PANE_PREVIEW_ALL) );
    GetShellManager()->AddSubShell(
        HID_SD_TASK_PANE_PREVIEW_ALL,
        pSelector.get(),
        pSelector->GetWindow());
    pTitledControl = AddControl (
        ::std::auto_ptr<TreeNode>(pSelector.release()),
        SdResId(STR_TASKPANEL_ALL_MASTER_PAGES_TITLE),
        HID_SD_ALL_MASTERS);
}





MasterPagesPanel::~MasterPagesPanel (void)
{
}




std::auto_ptr<ControlFactory> MasterPagesPanel::CreateControlFactory (ViewShellBase& rBase)
{
    return std::auto_ptr<ControlFactory>(
        new ControlFactoryWithArgs1<MasterPagesPanel,ViewShellBase>(rBase));
}




} } } // end of namespace ::sd::toolpanel::controls
