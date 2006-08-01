/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterPagesPanel.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:23:34 $
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
    DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(
        rBase.GetMainViewShell());
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
