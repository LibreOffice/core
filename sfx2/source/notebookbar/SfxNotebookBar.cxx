/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/builder.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <vcl/notebookbar/notebookbar.hxx>
#include <vcl/syswin.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/weldutils.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/lok.hxx>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <unotools/confignode.hxx>
#include <comphelper/types.hxx>
#include <framework/addonsoptions.hxx>
#include <vcl/notebookbar/NotebookBarAddonsItem.hxx>
#include <vector>
#include <unordered_map>

using namespace sfx2;
using namespace css::uno;
using namespace css::ui;
using namespace css;

constexpr OUString MENUBAR_STR = u"private:resource/menubar/menubar"_ustr;

const char MERGE_NOTEBOOKBAR_URL[] = "URL";

bool SfxNotebookBar::m_bLock = false;
bool SfxNotebookBar::m_bHide = false;

static void NotebookbarAddonValues(
    std::vector<Image>& aImageValues,
    std::vector<css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>>>&
        aExtensionValues)
{
    if (comphelper::LibreOfficeKit::isActive())
        return;

    framework::AddonsOptions aAddonsItems;

    for (int nIdx = 0; nIdx < aAddonsItems.GetAddonsNotebookBarCount(); nIdx++)
    {
        const css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>>& aExtension
            = aAddonsItems.GetAddonsNotebookBarPart(nIdx);
        for (const css::uno::Sequence<css::beans::PropertyValue>& rExtensionVal : aExtension)
        {
            Image aImage;
            bool isBigImage = true;
            for (const auto& rProp : rExtensionVal)
            {
                if (rProp.Name == MERGE_NOTEBOOKBAR_URL)
                {
                    OUString sImage;
                    rProp.Value >>= sImage;
                    aImage = Image(aAddonsItems.GetImageFromURL(sImage, isBigImage));
                }
            }
            aImageValues.push_back(std::move(aImage));
        }
        aExtensionValues.push_back(aExtension);
    }
}

static Reference<frame::XLayoutManager> lcl_getLayoutManager( const Reference<frame::XFrame>& xFrame )
{
    css::uno::Reference<css::frame::XLayoutManager> xLayoutManager;

    if (xFrame.is())
    {
        Reference<css::beans::XPropertySet> xPropSet(xFrame, UNO_QUERY);

        if (xPropSet.is())
        {
            Any aValue = xPropSet->getPropertyValue(u"LayoutManager"_ustr);
            aValue >>= xLayoutManager;
        }
    }

    return xLayoutManager;
}

static OUString lcl_getAppName( vcl::EnumContext::Application eApp )
{
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            return u"Writer"_ustr;
        case vcl::EnumContext::Application::Calc:
            return u"Calc"_ustr;
        case vcl::EnumContext::Application::Impress:
            return u"Impress"_ustr;
        case vcl::EnumContext::Application::Draw:
            return u"Draw"_ustr;
        case vcl::EnumContext::Application::Formula:
            return u"Formula"_ustr;
        default:
            return OUString();
    }
}

static void lcl_setNotebookbarFileName( vcl::EnumContext::Application eApp, const OUString& sFileName )
{
    std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
                comphelper::ConfigurationChanges::create() );
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            officecfg::Office::UI::ToolbarMode::ActiveWriter::set( sFileName, aBatch );
            break;
        case vcl::EnumContext::Application::Calc:
            officecfg::Office::UI::ToolbarMode::ActiveCalc::set( sFileName, aBatch );
            break;
        case vcl::EnumContext::Application::Impress:
            officecfg::Office::UI::ToolbarMode::ActiveImpress::set( sFileName, aBatch );
            break;
        case vcl::EnumContext::Application::Draw:
            officecfg::Office::UI::ToolbarMode::ActiveDraw::set( sFileName, aBatch );
            break;
        default:
            break;
    }
    aBatch->commit();
}

static OUString lcl_getNotebookbarFileName( vcl::EnumContext::Application eApp )
{
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            return officecfg::Office::UI::ToolbarMode::ActiveWriter::get();
        case vcl::EnumContext::Application::Calc:
            return officecfg::Office::UI::ToolbarMode::ActiveCalc::get();
        case vcl::EnumContext::Application::Impress:
            return officecfg::Office::UI::ToolbarMode::ActiveImpress::get();
        case vcl::EnumContext::Application::Draw:
            return officecfg::Office::UI::ToolbarMode::ActiveDraw::get();

        default:
            break;
    }
    return OUString();
}

static utl::OConfigurationTreeRoot lcl_getCurrentImplConfigRoot()
{
    return utl::OConfigurationTreeRoot(::comphelper::getProcessComponentContext(),
                                       u"org.openoffice.Office.UI.ToolbarMode/"_ustr,
                                       true);
}

static utl::OConfigurationNode lcl_getCurrentImplConfigNode(const Reference<css::frame::XFrame>& xFrame,
                                                                  utl::OConfigurationTreeRoot const & rNotebookbarNode )
{
    if (!rNotebookbarNode.isValid())
        return utl::OConfigurationNode();

    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );

    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xFrame ) );
    OUString aActive = lcl_getNotebookbarFileName( eApp );

    const utl::OConfigurationNode aImplsNode = rNotebookbarNode.openNode("Applications/" + lcl_getAppName( eApp) + "/Modes");
    const Sequence<OUString> aModeNodeNames( aImplsNode.getNodeNames() );

    for ( const auto& rModeNodeName : aModeNodeNames )
    {
        utl::OConfigurationNode aImplNode( aImplsNode.openNode( rModeNodeName ) );
        if ( !aImplNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aImplNode.getNodeValue( u"CommandArg"_ustr ) );

        if ( aCommandArg == aActive )
        {
            return aImplNode;
        }
    }

    return utl::OConfigurationNode();
}

void SfxNotebookBar::CloseMethod(SfxBindings& rBindings)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    CloseMethod(rFrame.GetSystemWindow());
}

void SfxNotebookBar::CloseMethod(SystemWindow* pSysWindow)
{
    if (pSysWindow)
    {
        if(pSysWindow->GetNotebookBar())
            pSysWindow->CloseNotebookBar();
        if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
            SfxNotebookBar::ShowMenubar(pViewFrm, true);
    }
}

void SfxNotebookBar::LockNotebookBar()
{
    m_bHide = true;
    m_bLock = true;
}

void SfxNotebookBar::UnlockNotebookBar()
{
    m_bHide = false;
    m_bLock = false;
}

bool SfxNotebookBar::IsActive(bool bConsiderSingleToolbar)
{
    if (m_bHide)
        return false;

    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;

    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        const Reference<frame::XFrame>& xFrame = pViewFrm->GetFrame().GetFrameInterface();
        if (!xFrame.is())
            return false;

        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
        try
        {
            eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
        }
        catch (css::uno::Exception& e)
        {
            SAL_WARN("sfx.appl", "SfxNotebookBar::IsActive(): " + e.Message);
            return false;
        }
    }
    else
        return false;

    OUString appName(lcl_getAppName( eApp ));

    if (appName.isEmpty())
        return false;


    OUString aPath = "org.openoffice.Office.UI.ToolbarMode/Applications/" + appName;

    const utl::OConfigurationTreeRoot aAppNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath,
                                        false);
    if ( !aAppNode.isValid() )
        return false;

    OUString aActive = comphelper::getString( aAppNode.getNodeValue( u"Active"_ustr ) );

    if (bConsiderSingleToolbar && aActive == "Single")
        return true;

    const utl::OConfigurationNode aModesNode = aAppNode.openNode(u"Modes"_ustr);
    const Sequence<OUString> aModeNodeNames( aModesNode.getNodeNames() );

    for ( const auto& rModeNodeName : aModeNodeNames )
    {
        const utl::OConfigurationNode aModeNode( aModesNode.openNode( rModeNodeName ) );
        if ( !aModeNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( u"CommandArg"_ustr ) );

        if ( aCommandArg == aActive )
        {
            return comphelper::getBOOL( aModeNode.getNodeValue( u"HasNotebookbar"_ustr ) );
        }
    }
    return false;
}

void SfxNotebookBar::ExecMethod(SfxBindings& rBindings, const OUString& rUIName)
{
    // Save active UI file name
    if (!rUIName.isEmpty())
    {
        if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        {
            const Reference<frame::XFrame>& xFrame = pViewFrm->GetFrame().GetFrameInterface();
            if (xFrame.is())
            {
                const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
                vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
                lcl_setNotebookbarFileName( eApp, rUIName );
            }
        }
    }

    // trigger the StateMethod
    rBindings.Invalidate(SID_NOTEBOOKBAR);
    rBindings.Update();
}

bool SfxNotebookBar::StateMethod(SfxBindings& rBindings, std::u16string_view rUIFile,
                                 bool bReloadNotebookbar)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    return StateMethod(rFrame.GetSystemWindow(), rFrame.GetFrameInterface(), rUIFile,
                       bReloadNotebookbar);
}

bool SfxNotebookBar::StateMethod(SystemWindow* pSysWindow,
                                 const Reference<css::frame::XFrame>& xFrame,
                                 std::u16string_view rUIFile, bool bReloadNotebookbar)
{
    if (!pSysWindow)
    {
        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        if (pViewFrm && pViewFrm->GetWindow().GetSystemWindow())
            pSysWindow = pViewFrm->GetWindow().GetSystemWindow();
        else
            return false;
    }

    if (IsActive())
    {
        const css::uno::Reference<css::uno::XComponentContext>& xContext = comphelper::getProcessComponentContext();
        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
        OUString aModuleName = xModuleManager->identify( xFrame );
        vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( aModuleName );
        bool bIsLOK = comphelper::LibreOfficeKit::isActive();

        OUString sFile = lcl_getNotebookbarFileName( eApp );

        OUString sNewFile = rUIFile + sFile;
        OUString sCurrentFile;
        VclPtr<NotebookBar> pNotebookBar = pSysWindow->GetNotebookBar();
        if ( pNotebookBar )
            sCurrentFile = pNotebookBar->GetUIFilePath();

        bool bChangedFile = sNewFile != sCurrentFile;

        if (!bIsLOK && (
                (!sFile.isEmpty() && bChangedFile) ||
                (!pNotebookBar || !pNotebookBar->IsVisible()) ||
                bReloadNotebookbar))
        {
            OUString aBuf = rUIFile + sFile;

            //Addons For Notebookbar
            std::vector<Image> aImageValues;
            std::vector<css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > > aExtensionValues;
            std::unique_ptr<NotebookBarAddonsItem> pNotebookBarAddonsItem;
            if (!bIsLOK)
            {
                pNotebookBarAddonsItem = std::make_unique<NotebookBarAddonsItem>();
                NotebookbarAddonValues(aImageValues , aExtensionValues);
                pNotebookBarAddonsItem->aAddonValues = std::move(aExtensionValues);
                pNotebookBarAddonsItem->aImageValues = std::move(aImageValues);
            }

            // tdf#164899 don't call SystemWindow::SetNotebookBar recursively
            // if NoteBookBar is in process of getting set
            if (pSysWindow->isSettingUpNoteBookBar())
                return false;

            RemoveListeners(pSysWindow);

            pSysWindow->SetNotebookBar(aBuf, xFrame, std::move(pNotebookBarAddonsItem), bReloadNotebookbar);
            pNotebookBar = pSysWindow->GetNotebookBar();
            pNotebookBar->Show();

            pNotebookBar->GetParent()->Resize();

            utl::OConfigurationTreeRoot aRoot(lcl_getCurrentImplConfigRoot());
            const utl::OConfigurationNode aModeNode(lcl_getCurrentImplConfigNode(xFrame, aRoot));
            SfxNotebookBar::ShowMenubar( comphelper::getBOOL( aModeNode.getNodeValue( u"HasMenubar"_ustr ) ) );

            SfxViewFrame* pView = SfxViewFrame::Current();

            if(pView)
            {
                pNotebookBar->SetupListener(true);
            }
        }

        return true;
    }
    else if (auto pNotebookBar = pSysWindow->GetNotebookBar())
    {
        vcl::Window* pParent = pNotebookBar->GetParent();
        pSysWindow->CloseNotebookBar();
        pParent->Resize();
        SfxNotebookBar::ShowMenubar(true);
    }

    return false;
}

void SfxNotebookBar::RemoveListeners(SystemWindow const * pSysWindow)
{
    if (const auto& pNotebookBar = pSysWindow->GetNotebookBar())
    {
        pNotebookBar->SetupListener(false);
    }
}

void SfxNotebookBar::ShowMenubar(bool bShow)
{
    if (m_bLock)
        return;

    m_bLock = true;

    Reference<frame::XFrame> xFrame;
    vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;
    const uno::Reference< uno::XComponentContext >& xContext = comphelper::getProcessComponentContext();
    const Reference<frame::XModuleManager> xModuleManager = frame::ModuleManager::create( xContext );

    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        xFrame = pViewFrm->GetFrame().GetFrameInterface();
        eCurrentApp = vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xFrame ) );
    }

    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    while( pViewFrame )
    {
        xFrame = pViewFrame->GetFrame().GetFrameInterface();
        if ( xFrame.is() )
        {
            vcl::EnumContext::Application eApp =
                    vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xFrame ) );

            if ( eApp == eCurrentApp )
            {
                const Reference<frame::XLayoutManager> xLayoutManager =
                                                        lcl_getLayoutManager( xFrame );

                if (xLayoutManager.is())
                {
                    if (xLayoutManager->getElement(MENUBAR_STR).is())
                    {
                        if (xLayoutManager->isElementVisible(MENUBAR_STR) && !bShow)
                            xLayoutManager->hideElement(MENUBAR_STR);
                        else if(!xLayoutManager->isElementVisible(MENUBAR_STR) && bShow)
                            xLayoutManager->showElement(MENUBAR_STR);
                    }
                }
            }
        }

        pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
    }
    m_bLock = false;
}

void SfxNotebookBar::ShowMenubar(SfxViewFrame const * pViewFrame, bool bShow)
{
    if (m_bLock)
        return;

    m_bLock = true;

    Reference<frame::XFrame> xFrame = pViewFrame->GetFrame().GetFrameInterface();
    if (xFrame.is())
    {
        const Reference<frame::XLayoutManager> xLayoutManager = lcl_getLayoutManager(xFrame);
        if (xLayoutManager.is())
        {
            if (xLayoutManager->getElement(MENUBAR_STR).is())
            {
                if (xLayoutManager->isElementVisible(MENUBAR_STR) && !bShow)
                    xLayoutManager->hideElement(MENUBAR_STR);
                else if (!xLayoutManager->isElementVisible(MENUBAR_STR) && bShow)
                    xLayoutManager->showElement(MENUBAR_STR);
            }
        }
    }
    m_bLock = false;
}

void SfxNotebookBar::ToggleMenubar()
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    const Reference<frame::XFrame>& xFrame = pViewFrm->GetFrame().GetFrameInterface();
    if (!xFrame.is())
        return;

    const Reference<frame::XLayoutManager> xLayoutManager =
                                            lcl_getLayoutManager(xFrame);

    bool bShow = true;
    if (xLayoutManager.is() && xLayoutManager->getElement(MENUBAR_STR).is())
    {
        if (xLayoutManager->isElementVisible(MENUBAR_STR))
        {
            SfxNotebookBar::ShowMenubar(false);
            bShow = false;
        }
        else
            SfxNotebookBar::ShowMenubar(true);
    }

    // Save menubar settings
    if (IsActive())
    {
        utl::OConfigurationTreeRoot aRoot(lcl_getCurrentImplConfigRoot());
        utl::OConfigurationNode aModeNode(lcl_getCurrentImplConfigNode(xFrame, aRoot));
        aModeNode.setNodeValue( u"HasMenubar"_ustr, toAny<bool>( bShow ) );
        aRoot.commit();
    }
}

void SfxNotebookBar::ReloadNotebookBar(std::u16string_view sUIPath)
{
    if (!SfxNotebookBar::IsActive())
        return;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!pViewShell)
        return;
    sfx2::SfxNotebookBar::StateMethod(pViewShell->GetViewFrame().GetBindings(), sUIPath, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
