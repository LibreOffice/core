/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <toolbarmodedlg.hxx>

#include <dialmgr.hxx>
#include <vcl/weld.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graphicfilter.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>

#include <sal/log.hxx>

ToolbarmodeDialog::ToolbarmodeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/toolbarmodedialog.ui", "ToolbarmodeDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pApply(m_xBuilder->weld_button("btnApply"))
    , m_pStandard(m_xBuilder->weld_radio_button("rbButton1"))
    , m_pSingle(m_xBuilder->weld_radio_button("rbButton2"))
    , m_pSidebar(m_xBuilder->weld_radio_button("rbButton3"))
    , m_pNotebookBarTabbed(m_xBuilder->weld_radio_button("rbButton4"))
    , m_pNotebookBarTabbedCompact(m_xBuilder->weld_radio_button("rbButton5"))
    , m_pNotebookBarGroupedBarCompact(m_xBuilder->weld_radio_button("rbButton6"))
    , m_pNotebookBarGroupedBarFull(m_xBuilder->weld_radio_button("rbButton7"))
    , m_pNotebookBarSingle(m_xBuilder->weld_radio_button("rbButton8"))
    , m_pNotebookBarGroups(m_xBuilder->weld_radio_button("rbButton9"))

{
    // todo: use the actual toolbarmodes
    Link<weld::ToggleButton&,void> aLink = LINK(this, ToolbarmodeDialog, SelectToolbarmode);
    m_pStandard->connect_toggled(aLink);
    m_pSingle->connect_toggled(aLink);
    m_pSidebar->connect_toggled(aLink);
    m_pNotebookBarTabbed->connect_toggled(aLink);
    m_pNotebookBarTabbedCompact->connect_toggled(aLink);
    m_pNotebookBarGroupedBarCompact->connect_toggled(aLink);
    m_pNotebookBarGroupedBarFull->connect_toggled(aLink);
    m_pNotebookBarSingle->connect_toggled(aLink);
    m_pNotebookBarGroups->connect_toggled(aLink);

    m_pApply->connect_clicked(LINK(this, ToolbarmodeDialog, OnApplyClick));
}

ToolbarmodeDialog::~ToolbarmodeDialog()
{
}

static bool file_exists(const OUString& fileName)
{
    ::osl::File aFile(fileName);
    return aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None;
}

void ToolbarmodeDialog::UpdateImage(OUString aFileName)
{
    // load image
    OUString aURL("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/toolbarmode/");
    rtl::Bootstrap::expandMacros(aURL);
    if (aFileName.isEmpty() || !file_exists(aURL + aFileName))
        return;
    // draw image
    Graphic aGraphic;
    if (GraphicFilter::LoadGraphic(aURL + aFileName, OUString(), aGraphic) == ERRCODE_NONE)
    {
        ScopedVclPtr<VirtualDevice> m_pVirDev = m_pImage->create_virtual_device();
        m_pVirDev->SetOutputSizePixel(aGraphic.GetSizePixel());
        m_pVirDev->DrawBitmapEx(Point(0, 0), aGraphic.GetBitmapEx());
        m_pImage->set_image(m_pVirDev.get());
        m_pVirDev.disposeAndClear();
    }
}

IMPL_LINK_NOARG(ToolbarmodeDialog, SelectToolbarmode, weld::ToggleButton&, void)
{
    if (m_pStandard->get_active())
        UpdateImage("default.png");
    else if (m_pSingle->get_active())
        UpdateImage("single.png");
    else if (m_pSidebar->get_active())
        UpdateImage("sidebar.png");
    else if (m_pNotebookBarTabbed->get_active())
        UpdateImage("notebookbar.png");
    else if (m_pNotebookBarTabbedCompact->get_active())
        UpdateImage("notebookbar_compact.png");
    else if (m_pNotebookBarGroupedBarCompact->get_active())
        UpdateImage("notebookbar_groupedbar_compact.png");
    else if (m_pNotebookBarGroupedBarFull->get_active())
        UpdateImage("notebookbar_groupedbar_full.png");
    else if (m_pNotebookBarSingle->get_active())
        UpdateImage("notebookbar_single.png");
    else if (m_pNotebookBarGroups->get_active())
        UpdateImage("notebookbar_groups.png");
}

IMPL_LINK_NOARG(ToolbarmodeDialog, OnApplyClick, weld::Button&, void)
{

    OUString aTargetURL;

    if (m_pStandard->get_active())
        aTargetURL="Default";
    else if (m_pSingle->get_active())
        aTargetURL="Single";
    else if (m_pSidebar->get_active())
        aTargetURL="Sidebar";
    else if (m_pNotebookBarTabbed->get_active())
        aTargetURL="notebookbar.ui";
    else if (m_pNotebookBarTabbedCompact->get_active())
        aTargetURL="notebookbar_compact.ui";
    else if (m_pNotebookBarGroupedBarCompact->get_active())
        aTargetURL="notebookbar_groupedbar_compact.ui";
    else if (m_pNotebookBarGroupedBarFull->get_active())
        aTargetURL="notebookbar_groupedbar_full.ui";
    else if (m_pNotebookBarSingle->get_active())
        aTargetURL="notebookbar_single.ui";
    else if (m_pNotebookBarGroups->get_active())
        aTargetURL="notebookbar_groups.ui";

    SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
    if (!pCurrentShell)
        return;
    SfxDispatcher* pDispatcher = pCurrentShell->GetDispatcher();
    if (!pDispatcher)
        return;
   const SfxStringItem aMediaURLItem( SID_TOOLBAR_MODE, aTargetURL );
   pDispatcher->ExecuteList(SID_TOOLBAR_MODE, SfxCallMode::ASYNCHRON,
                { &aMediaURLItem });
}
