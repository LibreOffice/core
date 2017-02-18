/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/notebookbar.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include "NotebookBarPopupMenu.hxx"
#include <officecfg/Office/UI/Notebookbar.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <unotools/confignode.hxx>
#include <comphelper/types.hxx>

using namespace sfx2;
using namespace css::uno;
using namespace css::ui;
using namespace css;

#define MENUBAR_STR "private:resource/menubar/menubar"

bool SfxNotebookBar::m_bLock = false;
bool SfxNotebookBar::m_bHide = false;

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
        case vcl::EnumContext::Application::Application_Writer:
            return OUString( "Writer" );
            break;
        case vcl::EnumContext::Application::Application_Calc:
            return OUString( "Calc" );
            break;
        case vcl::EnumContext::Application::Application_Impress:
            return OUString( "Impress" );
            break;
        default:
            return OUString( "" );
            break;
    }
}

static void lcl_setNotebookbarFileName( vcl::EnumContext::Application eApp, const OUString& sFileName )
{
    std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
                comphelper::ConfigurationChanges::create( ::comphelper::getProcessComponentContext() ) );
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Application_Writer:
            officecfg::Office::UI::Notebookbar::ActiveWriter::set( sFileName, aBatch );
            break;
        case vcl::EnumContext::Application::Application_Calc:
            officecfg::Office::UI::Notebookbar::ActiveCalc::set( sFileName, aBatch );
            break;
        case vcl::EnumContext::Application::Application_Impress:
            officecfg::Office::UI::Notebookbar::ActiveImpress::set( sFileName, aBatch );
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
        case vcl::EnumContext::Application::Application_Writer:
            return officecfg::Office::UI::Notebookbar::ActiveWriter::get();
            break;
        case vcl::EnumContext::Application::Application_Calc:
            return officecfg::Office::UI::Notebookbar::ActiveCalc::get();
            break;
        case vcl::EnumContext::Application::Application_Impress:
            return officecfg::Office::UI::Notebookbar::ActiveImpress::get();
            break;
        default:
            break;
    }
    return OUString();
}

static const utl::OConfigurationNode lcl_getCurrentImplConfigNode( const Reference<css::frame::XFrame>& xFrame,
                                                                   utl::OConfigurationTreeRoot& aNotebookbarNode )
{
    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );

    OUStringBuffer aPath("org.openoffice.Office.UI.Notebookbar/");

    aNotebookbarNode = utl::OConfigurationTreeRoot(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath.makeStringAndClear(),
                                        true);
    if ( !aNotebookbarNode.isValid() )
        return utl::OConfigurationNode();

    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xFrame ) );
    OUString aActive = lcl_getNotebookbarFileName( eApp );

    const utl::OConfigurationNode aImplsNode = aNotebookbarNode.openNode("Applications/" + lcl_getAppName( eApp) + "/Implementations");
    const Sequence<OUString> aModeNodeNames( aImplsNode.getNodeNames() );
    const sal_Int32 nCount( aModeNodeNames.getLength() );

    for ( sal_Int32 nReadIndex = 0; nReadIndex < nCount; ++nReadIndex )
    {
        const utl::OConfigurationNode aImplNode( aImplsNode.openNode( aModeNodeNames[nReadIndex] ) );
        if ( !aImplNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aImplNode.getNodeValue( "File" ) );

        if ( aCommandArg.compareTo( aActive ) == 0 )
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
        SfxNotebookBar::ShowMenubar(true);
    }
}

void SfxNotebookBar::LockNotebookBar()
{
    m_bHide = true;
}

void SfxNotebookBar::UnlockNotebookBar()
{
    m_bHide = false;
}

bool SfxNotebookBar::IsActive()
{
    if (m_bHide)
        return false;

    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Application_Any;

    if (SfxViewFrame::Current())
    {
        const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
        if (!xFrame.is())
            return false;

        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
        eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
    }

    OUStringBuffer aPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
    aPath.append( lcl_getAppName( eApp ) );

    const utl::OConfigurationTreeRoot aAppNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath.makeStringAndClear(),
                                        false);
    if ( !aAppNode.isValid() )
        return false;

    OUString aActive = comphelper::getString( aAppNode.getNodeValue( "Active" ) );

    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const Sequence<OUString> aModeNodeNames( aModesNode.getNodeNames() );
    const sal_Int32 nCount( aModeNodeNames.getLength() );

    for ( sal_Int32 nReadIndex = 0; nReadIndex < nCount; ++nReadIndex )
    {
        const utl::OConfigurationNode aModeNode( aModesNode.openNode( aModeNodeNames[nReadIndex] ) );
        if ( !aModeNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( "CommandArg" ) );

        if ( aCommandArg.compareTo( aActive ) == 0 )
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

bool SfxNotebookBar::StateMethod(SfxBindings& rBindings, const OUString& rUIFile)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    return StateMethod(rFrame.GetSystemWindow(), rFrame.GetFrameInterface(), rUIFile);
}

bool SfxNotebookBar::StateMethod(SystemWindow* pSysWindow,
                                 const Reference<css::frame::XFrame> & xFrame,
                                 const OUString& rUIFile)
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
        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
        vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
        OUString sFile = lcl_getNotebookbarFileName( eApp );
        OUString sNewFile = rUIFile + sFile;
        OUString sCurrentFile;
        VclPtr<NotebookBar> pNotebookBar = pSysWindow->GetNotebookBar();
        if ( pNotebookBar )
            sCurrentFile = OStringToOUString( pNotebookBar->getUIFile(), RTL_TEXTENCODING_ASCII_US );

        bool bChangedFile = true;
        if ( sCurrentFile.getLength() && sNewFile.getLength() )
        {
            // delete "/"
            sCurrentFile = sCurrentFile.copy( 0, sCurrentFile.getLength() - 1 );
            // delete ".ui"
            sNewFile = sNewFile.copy( 0, sNewFile.getLength() - 3 );

            bChangedFile = ( sNewFile.compareTo( sCurrentFile ) != 0 );
        }

        if ( ( !sFile.isEmpty() && bChangedFile ) || !pNotebookBar || !pNotebookBar->IsVisible() )
        {
            RemoveListeners(pSysWindow);

            OUStringBuffer aBuf(rUIFile);
            aBuf.append( sFile );

            // setup if necessary
            pSysWindow->SetNotebookBar(aBuf.makeStringAndClear(), xFrame);
            pNotebookBar = pSysWindow->GetNotebookBar();
            pNotebookBar->Show();
            pNotebookBar->GetParent()->Resize();
            pNotebookBar->SetIconClickHdl( LINK( nullptr, SfxNotebookBar, OpenNotebookbarPopupMenu ) );

            utl::OConfigurationTreeRoot aRoot;
            const utl::OConfigurationNode aModeNode( lcl_getCurrentImplConfigNode( xFrame, aRoot ) );
            SfxNotebookBar::ShowMenubar( comphelper::getBOOL( aModeNode.getNodeValue( "HasMenubar" ) ) );

            SfxViewFrame* pView = SfxViewFrame::Current();

            if(pView)
            {
                Reference<XContextChangeEventMultiplexer> xMultiplexer
                            = ContextChangeEventMultiplexer::get(
                                    ::comphelper::getProcessComponentContext());

                if(xFrame.is() && xMultiplexer.is())
                {
                    xMultiplexer->addContextChangeEventListener(
                                        pNotebookBar->getContextChangeEventListener(),
                                        xFrame->getController());
                }
            }
        }

        return true;
    }
    else if (auto pNotebookBar = pSysWindow->GetNotebookBar())
    {
        pNotebookBar->Hide();
        pNotebookBar->GetParent()->Resize();
        SfxNotebookBar::ShowMenubar(true);
    }

    return false;
}

void SfxNotebookBar::RemoveListeners(SystemWindow* pSysWindow)
{
    Reference<XContextChangeEventMultiplexer> xMultiplexer
                        = ContextChangeEventMultiplexer::get(
                                ::comphelper::getProcessComponentContext());

    if (pSysWindow->GetNotebookBar() && xMultiplexer.is())
    {
        xMultiplexer->removeAllContextChangeEventListeners(
                           pSysWindow->GetNotebookBar()->getContextChangeEventListener());
    }
}

IMPL_STATIC_LINK(SfxNotebookBar, OpenNotebookbarPopupMenu, NotebookBar*, pNotebookbar, void)
{
    if (pNotebookbar)
    {
        ScopedVclPtrInstance<NotebookBarPopupMenu> pMenu;
        if (SfxViewFrame::Current())
        {
            const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
            if (xFrame.is())
                pMenu->Execute(pNotebookbar, xFrame);
        }
    }
}

void SfxNotebookBar::ShowMenubar(bool bShow)
{
    if (!m_bLock)
    {
        m_bLock = true;

        Reference<frame::XFrame> xFrame;
        vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::Application_None;
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
                        xLayoutManager->lock();

                        if (xLayoutManager->getElement(MENUBAR_STR).is())
                        {
                            if (xLayoutManager->isElementVisible(MENUBAR_STR) && !bShow)
                                xLayoutManager->hideElement(MENUBAR_STR);
                            else if(!xLayoutManager->isElementVisible(MENUBAR_STR) && bShow)
                                xLayoutManager->showElement(MENUBAR_STR);
                        }

                        xLayoutManager->unlock();
                    }
                }
            }

            pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
        }
        m_bLock = false;
    }
}

void SfxNotebookBar::ToggleMenubar()
{
    if (SfxViewFrame::Current())
    {
        const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
        if (xFrame.is())
        {
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
                utl::OConfigurationTreeRoot aRoot;
                utl::OConfigurationNode aModeNode( lcl_getCurrentImplConfigNode( xFrame, aRoot ) );
                aModeNode.setNodeValue( "HasMenubar", toAny<bool>( bShow ) );
                aRoot.commit();
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
