/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <vcl/notebookbar/notebookbar.hxx>
#include <vcl/syswin.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxsids.hrc>
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
#include <vcl/notebookbar/NotebookBarAddonsMerger.hxx>
#include <vector>
#include <map>
#include <vcl/WeldedTabbedNotebookbar.hxx>

using namespace sfx2;
using namespace css::uno;
using namespace css::ui;
using namespace css;

constexpr OUStringLiteral MENUBAR_STR = u"private:resource/menubar/menubar";

const char MERGE_NOTEBOOKBAR_URL[] = "URL";

bool SfxNotebookBar::m_bLock = false;
bool SfxNotebookBar::m_bHide = false;
std::map<const SfxViewShell*, std::shared_ptr<WeldedTabbedNotebookbar>> SfxNotebookBar::m_pNotebookBarWeldedWrapper;

static void NotebookbarAddonValues(
    std::vector<Image>& aImageValues,
    std::vector<css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>>>&
        aExtensionValues)
{
    framework::AddonsOptions aAddonsItems;

    for (int nIdx = 0; nIdx < aAddonsItems.GetAddonsNotebookBarCount(); nIdx++)
    {
        const css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>> aExtension
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
                    aImage = Image(framework::AddonsOptions().GetImageFromURL(sImage, isBigImage));
                }
            }
            aImageValues.push_back(aImage);
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
            Any aValue = xPropSet->getPropertyValue("LayoutManager");
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
            return "Writer";
        case vcl::EnumContext::Application::Calc:
            return "Calc";
        case vcl::EnumContext::Application::Impress:
            return "Impress";
        case vcl::EnumContext::Application::Draw:
            return "Draw";
        case vcl::EnumContext::Application::Formula:
            return "Formula";
        default:
            return OUString();
    }
}

static void lcl_setNotebookbarFileName( vcl::EnumContext::Application eApp, const OUString& sFileName )
{
    std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
                comphelper::ConfigurationChanges::create( ::comphelper::getProcessComponentContext() ) );
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
                                       "org.openoffice.Office.UI.ToolbarMode/",
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
        const utl::OConfigurationNode aImplNode( aImplsNode.openNode( rModeNodeName ) );
        if ( !aImplNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aImplNode.getNodeValue( "CommandArg" ) );

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
        RemoveListeners(pSysWindow);
        if(pSysWindow->GetNotebookBar())
            pSysWindow->CloseNotebookBar();
        if (SfxViewFrame::Current())
            SfxNotebookBar::ShowMenubar(SfxViewFrame::Current(), true);
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

bool SfxNotebookBar::IsActive()
{
    if (m_bHide)
        return false;

    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;

    if (SfxViewFrame::Current())
    {
        const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
        if (!xFrame.is())
            return false;

        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
        try
        {
            eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
        }
        catch (css::frame::UnknownModuleException& e)
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

    OUString aActive = comphelper::getString( aAppNode.getNodeValue( "Active" ) );

    if (comphelper::LibreOfficeKit::isActive() && aActive == "notebookbar_online.ui")
        return true;

    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const Sequence<OUString> aModeNodeNames( aModesNode.getNodeNames() );

    for ( const auto& rModeNodeName : aModeNodeNames )
    {
        const utl::OConfigurationNode aModeNode( aModesNode.openNode( rModeNodeName ) );
        if ( !aModeNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( "CommandArg" ) );

        if ( aCommandArg == aActive )
        {
            return comphelper::getBOOL( aModeNode.getNodeValue( "HasNotebookbar" ) );
        }
    }
    return false;
}

void SfxNotebookBar::ExecMethod(SfxBindings& rBindings, const OUString& rUIName)
{
    // Save active UI file name
    if ( !rUIName.isEmpty() && SfxViewFrame::Current() )
    {
        const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
        if (xFrame.is())
        {
            const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
            vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
            lcl_setNotebookbarFileName( eApp, rUIName );
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
        if (SfxViewFrame::Current() && SfxViewFrame::Current()->GetWindow().GetSystemWindow())
            pSysWindow = SfxViewFrame::Current()->GetWindow().GetSystemWindow();
        else
            return false;
    }

    if (IsActive())
    {
        css::uno::Reference<css::uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
        OUString aModuleName = xModuleManager->identify( xFrame );
        vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( aModuleName );
        OUString sFile = lcl_getNotebookbarFileName( eApp );
        if (comphelper::LibreOfficeKit::isActive())
            sFile = "notebookbar_online.ui";
        OUString sNewFile = rUIFile + sFile;
        OUString sCurrentFile;
        VclPtr<NotebookBar> pNotebookBar = pSysWindow->GetNotebookBar();
        if ( pNotebookBar )
            sCurrentFile = pNotebookBar->GetUIFilePath();

        bool bChangedFile = sNewFile != sCurrentFile;

        if ((!sFile.isEmpty() && bChangedFile) || !pNotebookBar || !pNotebookBar->IsVisible()
            || bReloadNotebookbar || comphelper::LibreOfficeKit::isActive())
        {
            const SfxViewShell* pViewShell = SfxViewShell::Current();

            // Notebookbar was loaded too early what caused:
            //   * in LOK: Paste Special feature was incorrectly initialized
            // Skip first request so Notebookbar will be initialized after document was loaded
            static std::map<const void*, bool> bSkippedFirstInit;
            if (comphelper::LibreOfficeKit::isActive() && eApp == vcl::EnumContext::Application::Writer
                && bSkippedFirstInit.find(pViewShell) == bSkippedFirstInit.end())
            {
                bSkippedFirstInit[pViewShell] = true;
                return false;
            }

            RemoveListeners(pSysWindow);

            OUString aBuf = rUIFile + sFile;

            //Addons For Notebookbar
            std::vector<Image> aImageValues;
            std::vector<css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > > aExtensionValues;
            NotebookBarAddonsItem aNotebookBarAddonsItem;
            NotebookbarAddonValues(aImageValues , aExtensionValues);
            aNotebookBarAddonsItem.aAddonValues = aExtensionValues;
            aNotebookBarAddonsItem.aImageValues = aImageValues;

            // setup if necessary
            if (comphelper::LibreOfficeKit::isActive())
            {
                // update the current LOK language and locale for the dialog tunneling
                comphelper::LibreOfficeKit::setLanguageTag(pViewShell->GetLOKLanguageTag());
                comphelper::LibreOfficeKit::setLocale(pViewShell->GetLOKLocale());
            }

            pSysWindow->SetNotebookBar(aBuf, xFrame, aNotebookBarAddonsItem , bReloadNotebookbar);
            pNotebookBar = pSysWindow->GetNotebookBar();
            pNotebookBar->Show();


            bool hasWeldedWrapper = m_pNotebookBarWeldedWrapper.find(pViewShell) != m_pNotebookBarWeldedWrapper.end();
            if ((!hasWeldedWrapper || bReloadNotebookbar) && pNotebookBar->IsWelded())
            {
                sal_uInt64 nWindowId = reinterpret_cast<sal_uInt64>(pViewShell);
                m_pNotebookBarWeldedWrapper.emplace(std::make_pair(pViewShell,
                        new WeldedTabbedNotebookbar(pNotebookBar->GetMainContainer(),
                                                    pNotebookBar->GetUIFilePath(),
                                                    xFrame,
                                                    nWindowId)));
                pNotebookBar->SetDisposeCallback(LINK(nullptr, SfxNotebookBar, VclDisposeHdl), pViewShell);
            }

            pNotebookBar->GetParent()->Resize();

            utl::OConfigurationTreeRoot aRoot(lcl_getCurrentImplConfigRoot());
            const utl::OConfigurationNode aModeNode(lcl_getCurrentImplConfigNode(xFrame, aRoot));
            SfxNotebookBar::ShowMenubar( comphelper::getBOOL( aModeNode.getNodeValue( "HasMenubar" ) ) );

            SfxViewFrame* pView = SfxViewFrame::Current();

            if(pView)
            {
                pNotebookBar->ControlListenerForCurrentController(true);
            }
        }

        return true;
    }
    else if (auto pNotebookBar = pSysWindow->GetNotebookBar())
    {
        vcl::Window* pParent = pNotebookBar->GetParent();
        RemoveListeners(pSysWindow);
        pSysWindow->CloseNotebookBar();
        pParent->Resize();
        SfxNotebookBar::ShowMenubar(true);
    }

    return false;
}

void SfxNotebookBar::RemoveListeners(SystemWindow const * pSysWindow)
{
    if (auto pNotebookBar = pSysWindow->GetNotebookBar())
    {
        pNotebookBar->StopListeningAllControllers();
    }
}

void SfxNotebookBar::ShowMenubar(bool bShow)
{
    if (m_bLock)
        return;

    m_bLock = true;

    Reference<frame::XFrame> xFrame;
    vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    const Reference<frame::XModuleManager> xModuleManager = frame::ModuleManager::create( xContext );

    if ( SfxViewFrame::Current() )
    {
        xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
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
                const Reference<frame::XLayoutManager>& xLayoutManager =
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
        const Reference<frame::XLayoutManager>& xLayoutManager = lcl_getLayoutManager(xFrame);
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
    if (!SfxViewFrame::Current())
        return;

    const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
    if (!xFrame.is())
        return;

    const Reference<frame::XLayoutManager>& xLayoutManager =
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
        aModeNode.setNodeValue( "HasMenubar", toAny<bool>( bShow ) );
        aRoot.commit();
    }
}

void SfxNotebookBar::ReloadNotebookBar(std::u16string_view sUIPath)
{
    if (SfxNotebookBar::IsActive())
    {
        SfxViewShell* pViewShell = SfxViewShell::Current();
        sfx2::SfxNotebookBar::StateMethod(pViewShell->GetViewFrame()->GetBindings(), sUIPath, true);
    }
}

IMPL_STATIC_LINK(SfxNotebookBar, VclDisposeHdl, const SfxViewShell*, pViewShell, void)
{
    m_pNotebookBarWeldedWrapper.erase(pViewShell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
