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
#include <vcl/notebookbar/NotebookBarAddonsMerger.hxx>
#include <vector>
#include <unordered_map>
#include <vcl/WeldedTabbedNotebookbar.hxx>

using namespace sfx2;
using namespace css::uno;
using namespace css::ui;
using namespace css;

constexpr OUString MENUBAR_STR = u"private:resource/menubar/menubar"_ustr;

const char MERGE_NOTEBOOKBAR_URL[] = "URL";

bool SfxNotebookBar::m_bLock = false;
bool SfxNotebookBar::m_bHide = false;

namespace
{

/** View specific notebook bar data */
struct NotebookBarViewData
{
    std::unique_ptr<WeldedTabbedNotebookbar> m_pWeldedWrapper;
    VclPtr<NotebookBar> m_pNotebookBar;
    std::unique_ptr<ToolbarUnoDispatcher> m_pToolbarUnoDispatcher;

    ~NotebookBarViewData()
    {
        if (m_pNotebookBar)
            m_pNotebookBar.disposeAndClear();
    }
};

/** Notebookbar instance manager is a singleton that is used for track the
 *  per-view instances of view specific data contained in NotebookBarViewData
 *  class.
 **/
class NotebookBarViewManager final
{
private:
    // map contains a view data instance for a view (SfxViewShell pointer)
    std::unordered_map<const SfxViewShell*, std::unique_ptr<NotebookBarViewData>> m_pViewDataList;

    // private constructor to prevent any other instantiation outside of get() method
    NotebookBarViewManager() = default;

    // prevent class copying
    NotebookBarViewManager(const NotebookBarViewManager&) = delete;
    NotebookBarViewManager& operator=(const NotebookBarViewManager&) = delete;

public:
    // Singleton get method - creates an instance on first get() call
    static NotebookBarViewManager& get()
    {
        static NotebookBarViewManager gNotebookBarManager;
        return gNotebookBarManager;
    }

    NotebookBarViewData& getViewData(const SfxViewShell* pViewShell)
    {
        auto aFound = m_pViewDataList.find(pViewShell);
        if (aFound != m_pViewDataList.end()) // found
            return *aFound->second;

        // Create new view data instance
        NotebookBarViewData* pViewData = new NotebookBarViewData;
        m_pViewDataList.emplace(pViewShell, std::unique_ptr<NotebookBarViewData>(pViewData));
        return *pViewData;
    }

    void removeViewData(const SfxViewShell* pViewShell)
    {
        m_pViewDataList.erase(pViewShell);
    }
};

} // end anonymous namespace

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
                    aImage = Image(aAddonsItems.GetImageFromURL(sImage, isBigImage));
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

void SfxNotebookBar::RemoveCurrentLOKWrapper()
{
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    auto& rViewData = NotebookBarViewManager::get().getViewData(pViewShell);

    if (rViewData.m_pNotebookBar)
    {
        // Calls STATIC_LINK SfxNotebookBar -> VclDisposeHdl
        // which clears the whole InstanceManager
        rViewData.m_pNotebookBar.disposeAndClear();
    }
}

void SfxNotebookBar::CloseMethod(SfxBindings& rBindings)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    CloseMethod(rFrame.GetSystemWindow());
}

void SfxNotebookBar::CloseMethod(SystemWindow* pSysWindow)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        RemoveCurrentLOKWrapper();
        return;
    }

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

    if (bConsiderSingleToolbar && aActive == "Single")
        return true;

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

void SfxNotebookBar::ResetActiveToolbarModeToDefault(vcl::EnumContext::Application eApp)
{
    const OUString appName( lcl_getAppName( eApp ) );

    if ( appName.isEmpty() )
        return;

    const OUString aPath = "org.openoffice.Office.UI.ToolbarMode/Applications/" + appName;

    utl::OConfigurationTreeRoot aAppNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath,
                                        true);
    if ( !aAppNode.isValid() )
        return;

    aAppNode.setNodeValue( "Active", Any( OUString( "Default" ) ) );
    aAppNode.commit();
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

    const SfxViewShell* pViewShell = SfxViewShell::Current();
    auto& rViewData = NotebookBarViewManager::get().getViewData(pViewShell);
    bool hasWeldedWrapper = bool(rViewData.m_pWeldedWrapper);

    if (IsActive())
    {
        css::uno::Reference<css::uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
        OUString aModuleName = xModuleManager->identify( xFrame );
        vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( aModuleName );
        bool bIsLOK = comphelper::LibreOfficeKit::isActive();

        OUString sFile;
        if (bIsLOK)
            sFile = "notebookbar_online.ui";
        else
            sFile = lcl_getNotebookbarFileName( eApp );

        OUString sNewFile = rUIFile + sFile;
        OUString sCurrentFile;
        VclPtr<NotebookBar> pNotebookBar = pSysWindow->GetNotebookBar();
        if ( pNotebookBar )
            sCurrentFile = pNotebookBar->GetUIFilePath();

        bool bChangedFile = sNewFile != sCurrentFile;

        if ((!bIsLOK && (
                (!sFile.isEmpty() && bChangedFile) ||
                (!pNotebookBar || !pNotebookBar->IsVisible()) ||
                bReloadNotebookbar)
            ) || (bIsLOK && !hasWeldedWrapper))
        {
            OUString aBuf = rUIFile + sFile;

            //Addons For Notebookbar
            std::vector<Image> aImageValues;
            std::vector<css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > > aExtensionValues;
            NotebookBarAddonsItem aNotebookBarAddonsItem;
            NotebookbarAddonValues(aImageValues , aExtensionValues);
            aNotebookBarAddonsItem.aAddonValues = aExtensionValues;
            aNotebookBarAddonsItem.aImageValues = aImageValues;

            if (bIsLOK)
            {
                if (!pViewShell)
                    return false;

                // Notebookbar was loaded too early what caused:
                //   * in LOK: Paste Special feature was incorrectly initialized
                // Skip first request so Notebookbar will be initialized after document was loaded
                static std::map<const void*, bool> bSkippedFirstInit;
                if (eApp == vcl::EnumContext::Application::Writer
                    && bSkippedFirstInit.find(pViewShell) == bSkippedFirstInit.end())
                {
                    bSkippedFirstInit[pViewShell] = true;
                    ResetActiveToolbarModeToDefault(eApp);
                    return false;
                }

                // update the current LOK language and locale for the dialog tunneling
                comphelper::LibreOfficeKit::setLanguageTag(pViewShell->GetLOKLanguageTag());
                comphelper::LibreOfficeKit::setLocale(pViewShell->GetLOKLocale());

                pNotebookBar = VclPtr<NotebookBar>::Create(pSysWindow, "NotebookBar", aBuf, xFrame, aNotebookBarAddonsItem);
                rViewData.m_pNotebookBar = pNotebookBar;
                assert(pNotebookBar->IsWelded());

                sal_uInt64 nWindowId = reinterpret_cast<sal_uInt64>(pViewShell);
                rViewData.m_pWeldedWrapper.reset(
                        new WeldedTabbedNotebookbar(pNotebookBar->GetMainContainer(),
                                                    pNotebookBar->GetUIFilePath(),
                                                    xFrame, nWindowId));
                pNotebookBar->SetDisposeCallback(LINK(nullptr, SfxNotebookBar, VclDisposeHdl), pViewShell);

                rViewData.m_pToolbarUnoDispatcher.reset(
                    new ToolbarUnoDispatcher(rViewData.m_pWeldedWrapper->getWeldedToolbar(),
                                             rViewData.m_pWeldedWrapper->getBuilder(), xFrame));

                return true;
            }

            RemoveListeners(pSysWindow);

            pSysWindow->SetNotebookBar(aBuf, xFrame, aNotebookBarAddonsItem , bReloadNotebookbar);
            pNotebookBar = pSysWindow->GetNotebookBar();
            pNotebookBar->Show();

            pNotebookBar->GetParent()->Resize();

            utl::OConfigurationTreeRoot aRoot(lcl_getCurrentImplConfigRoot());
            const utl::OConfigurationNode aModeNode(lcl_getCurrentImplConfigNode(xFrame, aRoot));
            SfxNotebookBar::ShowMenubar( comphelper::getBOOL( aModeNode.getNodeValue( "HasMenubar" ) ) );

            SfxViewFrame* pView = SfxViewFrame::Current();

            if(pView)
            {
                pNotebookBar->SetupListener(true);
            }
        }

        return true;
    }
    else if (comphelper::LibreOfficeKit::isActive())
    {
        // don't do anything to not close notebookbar of other session
        return hasWeldedWrapper;
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
    if (auto pNotebookBar = pSysWindow->GetNotebookBar())
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
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
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
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    const Reference<frame::XFrame>& xFrame = pViewFrm->GetFrame().GetFrameInterface();
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
    if (!SfxNotebookBar::IsActive())
        return;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!pViewShell)
        return;
    sfx2::SfxNotebookBar::StateMethod(pViewShell->GetViewFrame().GetBindings(), sUIPath, true);
}

IMPL_STATIC_LINK(SfxNotebookBar, VclDisposeHdl, const SfxViewShell*, pViewShell, void)
{
    NotebookBarViewManager::get().removeViewData(pViewShell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
