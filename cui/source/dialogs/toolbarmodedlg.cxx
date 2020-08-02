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
#include <com/sun/star/frame/XDispatchProvider.hpp>

#include <sfx2/viewsh.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/htmlmode.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/objsh.hxx>
#include <svl/poolitem.hxx>
#include <svl/intitem.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <unodispatch.hxx>

#include <sal/log.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

ToolbarmodeDialog::ToolbarmodeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/toolbarmodedialog.ui", "ToolbarmodeDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pApply(m_xBuilder->weld_button("btnApply"))
    , m_pStandard(m_xBuilder->weld_radio_button("rbButton1"))
    , m_pSingle(m_xBuilder->weld_radio_button("rbButton2"))
    , m_pSidebar(m_xBuilder->weld_radio_button("rbButton3"))
    , m_pTabbed(m_xBuilder->weld_radio_button("rbButton4"))
    , m_pTabbedCompact(m_xBuilder->weld_radio_button("rbButton5"))
{
    // todo: use the actual toolbarmodes
    Link<weld::ToggleButton&,void> aLink = LINK(this, ToolbarmodeDialog, SelectToolbarmode);
    m_pStandard->connect_toggled(aLink);
    m_pSingle->connect_toggled(aLink);
    m_pSidebar->connect_toggled(aLink);
    m_pTabbed->connect_toggled(aLink);
    m_pTabbedCompact->connect_toggled(aLink);

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
        UpdateImage("standard.png");
    else if (m_pSingle->get_active())
        UpdateImage("single.png");
    else if (m_pSidebar->get_active())
        UpdateImage("sidebar.png");
    else if (m_pTabbed->get_active())
        UpdateImage("tabbed.png");
    else if (m_pTabbedCompact->get_active())
        UpdateImage("tabbedcompact.png");
}

IMPL_LINK_NOARG(ToolbarmodeDialog, OnApplyClick, weld::Button&, void)
{
    SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
    if (!pCurrentShell)
        return;
    SfxDispatcher* pDispatcher = pCurrentShell->GetDispatcher();
    if (!pDispatcher)
        return;

//   pDispatcher->Execute( SID_TOOLBAR_MODE , SfxCallMode::ASYNCHRON );

   URL aTargetURL;
   aTargetURL.Complete = ".uno:Notebookbar?File:string=Default";
   Sequence<PropertyValue> aArgs;
   SfXDispatch::dispatch(aTargetURL,aArgs);

//SfxGetpApp()->GetDispatcher_Impl()->Execute(SID_TOOLBAR_MODE);

/*
    if (m_pStandard->get_active())
        pDispatcher->Execute( SID_TOOLBAR_MODE , SfxCallMode::ASYNCHRON );
    else if (m_pSingle->get_active())
        pDispatcher->Execute( SID_TOOLBAR_MODE , SfxCallMode::ASYNCHRON );
    else if (m_pSidebar->get_active())
        pDispatcher->Execute( SID_TOOLBAR_MODE , SfxCallMode::ASYNCHRON );
    else if (m_pTabbed->get_active())
        pDispatcher->Execute( SID_TOOLBAR_MODE , SfxCallMode::ASYNCHRON );
    else if (m_pTabbedCompact->get_active())
        pDispatcher->Execute( SID_TOOLBAR_MODE , SfxCallMode::ASYNCHRON );
*/

/*        URL aTargetURL;
        Sequence<PropertyValue> aArgs;

        aTargetURL.Complete = ".uno:Notebookbar?File:string=" + aCmd;
        Reference< XURLTransformer >        xURLTransformer;
        Reference< XFrame >                 xFrame;

        xURLTransformer->parseStrict( aTargetURL );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( xDispatchProvider.is() )
        {
            Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch(
                                                aTargetURL, OUString(), 0 );

            ExecuteInfo* pExecuteInfo = new ExecuteInfo;
            pExecuteInfo->xDispatch     = xDispatch;
            pExecuteInfo->aTargetURL    = aTargetURL;
            pExecuteInfo->aArgs         = aArgs;
            Application::PostUserEvent( LINK(nullptr,ToolbarModeMenuController, ExecuteHdl_Impl), pExecuteInfo );
        }
*/
}
