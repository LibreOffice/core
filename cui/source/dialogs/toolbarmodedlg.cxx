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
#include <strings.hrc>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graphicfilter.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <unotools/confignode.hxx>
#include <unotools/moduleoptions.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/EnumContext.hxx>

#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

OUString GetCurrentMode()
{
    Reference<XFrame> xCurrentFrame;
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;

    OUString aCurrentMode;
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();

    if (pViewFrame)
    {
        xCurrentFrame = pViewFrame->GetFrame().GetFrameInterface();

        const Reference<frame::XModuleManager> xModuleManager
            = frame::ModuleManager::create(xContext);
        eCurrentApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xCurrentFrame));

        OUStringBuffer aPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
        if (eCurrentApp == vcl::EnumContext::Application::Writer)
            aPath.append("Writer");
        else if (eCurrentApp == vcl::EnumContext::Application::Calc)
            aPath.append("Calc");
        else if (eCurrentApp == vcl::EnumContext::Application::Impress)
            aPath.append("Impress");
        else if (eCurrentApp == vcl::EnumContext::Application::Draw)
            aPath.append("Draw");
        else if (eCurrentApp == vcl::EnumContext::Application::Formula)
            aPath.append("Formula");
        else if (eCurrentApp == vcl::EnumContext::Application::Base)
            aPath.append("Base");

        const utl::OConfigurationTreeRoot aAppNode(xContext, aPath.makeStringAndClear(), true);

        if (aAppNode.isValid())
            aCurrentMode = comphelper::getString(aAppNode.getNodeValue("Active"));
    }
    return aCurrentMode;
}

ToolbarmodeDialog::ToolbarmodeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/toolbarmodedialog.ui", "ToolbarmodeDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pApply(m_xBuilder->weld_button("btnApply"))
    , m_pDefault(m_xBuilder->weld_radio_button("rbButton1"))
    , m_pSingle(m_xBuilder->weld_radio_button("rbButton2"))
    , m_pSidebar(m_xBuilder->weld_radio_button("rbButton3"))
    , m_pNotebookBarTabbed(m_xBuilder->weld_radio_button("rbButton4"))
    , m_pNotebookBarTabbedCompact(m_xBuilder->weld_radio_button("rbButton5"))
    , m_pNotebookBarGroupedBarCompact(m_xBuilder->weld_radio_button("rbButton6"))
    , m_pNotebookBarGroupedBarFull(m_xBuilder->weld_radio_button("rbButton7"))
    , m_pNotebookBarSingle(m_xBuilder->weld_radio_button("rbButton8"))
    , m_pNotebookBarGroups(m_xBuilder->weld_radio_button("rbButton9"))
    , m_pInfoLabel(m_xBuilder->weld_label("lbInfo"))
{
    Link<weld::ToggleButton&, void> aLink = LINK(this, ToolbarmodeDialog, SelectToolbarmode);
    m_pDefault->connect_toggled(aLink);
    m_pSingle->connect_toggled(aLink);
    m_pSidebar->connect_toggled(aLink);
    m_pNotebookBarTabbed->connect_toggled(aLink);
    m_pNotebookBarTabbedCompact->connect_toggled(aLink);
    m_pNotebookBarGroupedBarCompact->connect_toggled(aLink);
    m_pNotebookBarGroupedBarFull->connect_toggled(aLink);
    m_pNotebookBarSingle->connect_toggled(aLink);
    m_pNotebookBarGroups->connect_toggled(aLink);

    //select active item
    OUString aCurrentMode = GetCurrentMode();
    if (aCurrentMode == "Default")
    {
        m_pDefault->set_active(true);
        UpdateImage("default.png");
    }
    else if (aCurrentMode == "Single")
        m_pSingle->set_active(true);
    else if (aCurrentMode == "Sidebar")
        m_pSidebar->set_active(true);
    else if (aCurrentMode == "notebookbar.ui")
        m_pNotebookBarTabbed->set_active(true);
    else if (aCurrentMode == "notebookbar_compact.ui")
        m_pNotebookBarTabbedCompact->set_active(true);
    else if (aCurrentMode == "notebookbar_groupedbar_compact.ui")
        m_pNotebookBarGroupedBarCompact->set_active(true);
    else if (aCurrentMode == "notebookbar_groupedbar_full.ui")
        m_pNotebookBarGroupedBarFull->set_active(true);
    else if (aCurrentMode == "notebookbar_single.ui")
        m_pNotebookBarSingle->set_active(true);
    else if (aCurrentMode == "notebookbar_groups.ui")
        m_pNotebookBarGroups->set_active(true);

    m_pApply->connect_clicked(LINK(this, ToolbarmodeDialog, OnApplyClick));
}

ToolbarmodeDialog::~ToolbarmodeDialog() {}

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
        m_pVirDev->DrawBitmapEx(Point(0,0), aGraphic.GetBitmapEx());
        m_pImage->set_image(m_pVirDev.get());
        m_pVirDev.disposeAndClear();
    }
}

IMPL_LINK_NOARG(ToolbarmodeDialog, SelectToolbarmode, weld::ToggleButton&, void)
{

    if (m_pDefault->get_active())
    {
        UpdateImage("default.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_DEFAULT));
    }
    else if (m_pSingle->get_active())
    {
        UpdateImage("single.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_SINGLE));
    }
    else if (m_pSidebar->get_active())
    {
        UpdateImage("sidebar.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_SIDEBAR));
    }
    else if (m_pNotebookBarTabbed->get_active())
    {
        UpdateImage("notebookbar.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_NBTabbed));
    }
    else if (m_pNotebookBarTabbedCompact->get_active())
    {
        UpdateImage("notebookbar_compact.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_NBTabbedCompact));
    }
    else if (m_pNotebookBarGroupedBarCompact->get_active())
    {
        UpdateImage("notebookbar_groupedbar_compact.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_NBGroupedCompact));
    }
    else if (m_pNotebookBarGroupedBarFull->get_active())
    {
        UpdateImage("notebookbar_groupedbar_full.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_NBGroupedFull));
    }
    else if (m_pNotebookBarSingle->get_active())
    {
        UpdateImage("notebookbar_single.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_NBSingle));
    }
    else if (m_pNotebookBarGroups->get_active())
    {
        UpdateImage("notebookbar_groups.png");
        m_pInfoLabel->set_label(CuiResId(RID_SVXSTR_TOOLBARMODE_NBGroups));
    }
}

IMPL_LINK_NOARG(ToolbarmodeDialog, OnApplyClick, weld::Button&, void)
{
    OUString aTargetURL;

    if (m_pDefault->get_active())
        aTargetURL = "Default";
    else if (m_pSingle->get_active())
        aTargetURL = "Single";
    else if (m_pSidebar->get_active())
        aTargetURL = "Sidebar";
    else if (m_pNotebookBarTabbed->get_active())
        aTargetURL = "notebookbar.ui";
    else if (m_pNotebookBarTabbedCompact->get_active())
        aTargetURL = "notebookbar_compact.ui";
    else if (m_pNotebookBarGroupedBarCompact->get_active())
        aTargetURL = "notebookbar_groupedbar_compact.ui";
    else if (m_pNotebookBarGroupedBarFull->get_active())
        aTargetURL = "notebookbar_groupedbar_full.ui";
    else if (m_pNotebookBarSingle->get_active())
        aTargetURL = "notebookbar_single.ui";
    else if (m_pNotebookBarGroups->get_active())
        aTargetURL = "notebookbar_groups.ui";

    SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
    if (!pCurrentShell)
        return;
    SfxDispatcher* pDispatcher = pCurrentShell->GetDispatcher();
    if (!pDispatcher)
        return;
    const SfxStringItem aURLItem(SID_TOOLBAR_MODE, aTargetURL);
    pDispatcher->ExecuteList(SID_TOOLBAR_MODE, SfxCallMode::ASYNCHRON, { &aURLItem });
}
