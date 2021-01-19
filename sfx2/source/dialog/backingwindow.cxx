/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "backingwindow.hxx"
#include <vcl/accel.hxx>
#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <vcl/menu.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/virdev.hxx>

#include <unotools/historyoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/openfiledroptargetlistener.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/langhelp.hxx>
#include <templateviewitem.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/app.hxx>
#include <officecfg/Office/Common.hxx>

#include <tools/diagnose_ex.h>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::document;

constexpr OUStringLiteral SERVICENAME_CFGREADACCESS = u"com.sun.star.configuration.ConfigurationAccess";

class BrandImage : public weld::CustomWidgetController
{
private:
    BitmapEx maBrandImage;

public:
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override
    {
        weld::CustomWidgetController::SetDrawingArea(pDrawingArea);
        SetPointer(PointerStyle::RefHand);
    }

    virtual void StyleUpdated() override
    {
        if (!maBrandImage)
        {
            const bool bIsDark = Application::GetSettings().GetStyleSettings().GetDialogColor().IsDark();
            SfxApplication::loadBrandSvg(bIsDark ? "shell/logo-sc_inverted" : "shell/logo-sc",
                                        maBrandImage, GetOutputSizePixel().Width());

            const Size aBmpSize(maBrandImage.GetSizePixel());
            set_size_request(aBmpSize.Width(), aBmpSize.Height());

            weld::CustomWidgetController::StyleUpdated();
        }
    }

    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override
    {
        if (rMEvt.IsLeft())
        {
            OUString sURL = officecfg::Office::Common::Menus::VolunteerURL::get();
            localizeWebserviceURI(sURL);

            Reference<css::system::XSystemShellExecute> const xSystemShellExecute(
                css::system::SystemShellExecute::create(
                    ::comphelper::getProcessComponentContext()));
            xSystemShellExecute->execute(sURL, OUString(),
                                         css::system::SystemShellExecuteFlags::URIS_ONLY);
        }
        return true;
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override
    {
        rRenderContext.DrawBitmapEx(Point(0, 0), maBrandImage);
    }
};

// increase size of the text in the buttons on the left fMultiplier-times
float const g_fMultiplier = 1.4f;

BackingWindow::BackingWindow(vcl::Window* i_pParent)
    : InterimItemWindow(i_pParent, "sfx/ui/startcenter.ui", "StartCenter", false)
    , mxOpenButton(m_xBuilder->weld_button("open_all"))
    , mxRecentButton(m_xBuilder->weld_menu_toggle_button("open_recent"))
    , mxRemoteButton(m_xBuilder->weld_button("open_remote"))
    , mxTemplateButton(m_xBuilder->weld_menu_toggle_button("templates_all"))
    , mxCreateLabel(m_xBuilder->weld_label("create_label"))
    , mxAltHelpLabel(m_xBuilder->weld_label("althelplabel"))
    , mxWriterAllButton(m_xBuilder->weld_button("writer_all"))
    , mxCalcAllButton(m_xBuilder->weld_button("calc_all"))
    , mxImpressAllButton(m_xBuilder->weld_button("impress_all"))
    , mxDrawAllButton(m_xBuilder->weld_button("draw_all"))
    , mxDBAllButton(m_xBuilder->weld_button("database_all"))
    , mxMathAllButton(m_xBuilder->weld_button("math_all"))
    , mxBrandImage(new BrandImage)
    , mxBrandImageWeld(new weld::CustomWeld(*m_xBuilder, "daBrand", *mxBrandImage))
    , mxHelpButton(m_xBuilder->weld_button("help"))
    , mxExtensionsButton(m_xBuilder->weld_button("extensions"))
    , mxAllButtonsBox(m_xBuilder->weld_container("all_buttons_box"))
    , mxButtonsBox(m_xBuilder->weld_container("buttons_box"))
    , mxSmallButtonsBox(m_xBuilder->weld_container("small_buttons_box"))
    , mxAllRecentThumbnails(new sfx2::RecentDocsView(m_xBuilder->weld_scrolled_window("scrollrecent", true),
                                                     m_xBuilder->weld_menu("recentmenu")))
    , mxAllRecentThumbnailsWin(new weld::CustomWeld(*m_xBuilder, "all_recent", *mxAllRecentThumbnails))
    , mxLocalView(new TemplateDefaultView(m_xBuilder->weld_scrolled_window("scrolllocal", true),
                                          m_xBuilder->weld_menu("localmenu")))
    , mxLocalViewWin(new weld::CustomWeld(*m_xBuilder, "local_view", *mxLocalView))
    , mbLocalViewInitialized(false)
    , mbInitControls(false)
{
    // init background
    SetPaintTransparent(false);
    SetBackground(svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor);

    //set an alternative help label that doesn't hotkey the H of the Help menu
    mxHelpButton->set_label(mxAltHelpLabel->get_label());
    mxHelpButton->connect_clicked(LINK(this, BackingWindow, ClickHelpHdl));

    mxDropTarget = mxAllRecentThumbnails->GetDropTarget();

    try
    {
        mxContext.set( ::comphelper::getProcessComponentContext(), uno::UNO_SET_THROW );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "fwk", "BackingWindow" );
    }

    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    // get dispatch provider
    Reference<XDesktop2> xDesktop = Desktop::create( comphelper::getProcessComponentContext() );
    mxDesktopDispatchProvider = xDesktop;

    mxOpenButton->grab_focus();
}

IMPL_LINK(BackingWindow, ClickHelpHdl, weld::Button&, rButton, void)
{
    if (Help* pHelp = Application::GetHelp())
        pHelp->Start(OUString::fromUtf8(m_xContainer->get_help_id()), &rButton);
}

BackingWindow::~BackingWindow()
{
    disposeOnce();
}

void BackingWindow::dispose()
{
    // deregister drag&drop helper
    if (mxDropTargetListener.is())
    {
        if (mxDropTarget.is())
        {
            mxDropTarget->removeDropTargetListener(mxDropTargetListener);
            mxDropTarget->setActive(false);
        }
        mxDropTargetListener.clear();
    }
    mxDropTarget.clear();
    mxOpenButton.reset();
    mxRemoteButton.reset();
    mxRecentButton.reset();
    mxTemplateButton.reset();
    mxCreateLabel.reset();
    mxAltHelpLabel.reset();
    mxWriterAllButton.reset();
    mxCalcAllButton.reset();
    mxImpressAllButton.reset();
    mxDrawAllButton.reset();
    mxDBAllButton.reset();
    mxMathAllButton.reset();
    mxBrandImage.reset();
    mxHelpButton.reset();
    mxExtensionsButton.reset();
    mxAllButtonsBox.reset();
    mxButtonsBox.reset();
    mxSmallButtonsBox.reset();
    mxAllRecentThumbnailsWin.reset();
    mxAllRecentThumbnails.reset();
    mxLocalViewWin.reset();
    mxLocalView.reset();
    InterimItemWindow::dispose();
}

void BackingWindow::initControls()
{
    if( mbInitControls )
        return;

    mbInitControls = true;

    // collect the URLs of the entries in the File/New menu
    SvtModuleOptions    aModuleOptions;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::WRITER))
        mxAllRecentThumbnails->mnFileTypes |= sfx2::ApplicationType::TYPE_WRITER;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::CALC))
        mxAllRecentThumbnails->mnFileTypes |= sfx2::ApplicationType::TYPE_CALC;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::IMPRESS))
        mxAllRecentThumbnails->mnFileTypes |= sfx2::ApplicationType::TYPE_IMPRESS;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::DRAW))
        mxAllRecentThumbnails->mnFileTypes |= sfx2::ApplicationType::TYPE_DRAW;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::DATABASE))
        mxAllRecentThumbnails->mnFileTypes |= sfx2::ApplicationType::TYPE_DATABASE;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::MATH))
        mxAllRecentThumbnails->mnFileTypes |= sfx2::ApplicationType::TYPE_MATH;

    mxAllRecentThumbnails->mnFileTypes |= sfx2::ApplicationType::TYPE_OTHER;
    mxAllRecentThumbnails->Reload();
    mxAllRecentThumbnails->ShowTooltips( true );
    mxRecentButton->set_active(true);

    //initialize Template view
    mxLocalView->Hide();

    //set handlers
    mxLocalView->setCreateContextMenuHdl(LINK(this, BackingWindow, CreateContextMenuHdl));
    mxLocalView->setOpenTemplateHdl(LINK(this, BackingWindow, OpenTemplateHdl));
    mxLocalView->setEditTemplateHdl(LINK(this, BackingWindow, EditTemplateHdl));
    mxLocalView->ShowTooltips( true );

    checkInstalledModules();

    mxExtensionsButton->connect_clicked(LINK(this, BackingWindow, ExtLinkClickHdl));

    mxOpenButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxRemoteButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxRecentButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxTemplateButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxWriterAllButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxDrawAllButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxCalcAllButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxDBAllButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxImpressAllButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));
    mxMathAllButton->connect_clicked(LINK(this, BackingWindow, ClickHdl));

    mxRecentButton->connect_selected(LINK(this, BackingWindow, MenuSelectHdl));
    mxTemplateButton->connect_selected(LINK(this, BackingWindow, MenuSelectHdl));

    ApplyStyleSettings();
}

void BackingWindow::DataChanged(const DataChangedEvent& rDCEvt)
{
    if ((rDCEvt.GetType() != DataChangedEventType::SETTINGS)
        || !(rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        InterimItemWindow::DataChanged(rDCEvt);
        return;
    }

    ApplyStyleSettings();
    Invalidate();
}

template <typename WidgetClass>
void BackingWindow::setLargerFont(WidgetClass& pWidget, const vcl::Font& rFont)
{
    vcl::Font aFont(rFont);
    aFont.SetFontSize(Size(0, aFont.GetFontSize().Height() * g_fMultiplier));
    pWidget->set_font(aFont);
}

void BackingWindow::ApplyStyleSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color aButtonsBackground(rStyleSettings.GetWindowColor());
    const vcl::Font& aButtonFont(rStyleSettings.GetPushButtonFont());
    const vcl::Font& aLabelFont(rStyleSettings.GetLabelFont());

    // setup larger fonts
    setLargerFont(mxOpenButton, aButtonFont);
    setLargerFont(mxOpenButton, aButtonFont);
    setLargerFont(mxRemoteButton, aButtonFont);
    setLargerFont(mxRecentButton, aButtonFont);
    setLargerFont(mxTemplateButton, aButtonFont);
    setLargerFont(mxWriterAllButton, aButtonFont);
    setLargerFont(mxDrawAllButton, aButtonFont);
    setLargerFont(mxCalcAllButton, aButtonFont);
    setLargerFont(mxDBAllButton, aButtonFont);
    setLargerFont(mxImpressAllButton, aButtonFont);
    setLargerFont(mxMathAllButton, aButtonFont);
    setLargerFont(mxCreateLabel, aLabelFont);

    mxAllButtonsBox->set_background(aButtonsBackground);
    mxSmallButtonsBox->set_background(aButtonsBackground);

    // compute the menubar height
    sal_Int32 nMenuHeight = 0;
    if (SystemWindow* pSystemWindow = GetSystemWindow())
    {
        MenuBar* pMenuBar = pSystemWindow->GetMenuBar();
        if (pMenuBar)
            nMenuHeight = pMenuBar->ImplGetWindow()->GetOutputSizePixel().Height();
    }

    // fdo#34392: we do the layout dynamically, the layout depends on the font,
    // so we should handle data changed events (font changing) of the last child
    // control, at this point all the controls have updated settings (i.e. font).
    Size aPrefSize(mxAllButtonsBox->get_preferred_size());
    set_width_request(aPrefSize.Width());
    set_height_request(nMenuHeight + aPrefSize.Height());
}

void BackingWindow::initializeLocalView()
{
    if (!mbLocalViewInitialized)
    {
        mbLocalViewInitialized = true;
        mxLocalView->Populate();
        mxLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));
        mxLocalView->showAllTemplates();
    }
}

void BackingWindow::checkInstalledModules()
{
    SvtModuleOptions aModuleOpt;

    mxWriterAllButton->set_sensitive( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ));
    mxCalcAllButton->set_sensitive( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) );
    mxImpressAllButton->set_sensitive( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) );
    mxDrawAllButton->set_sensitive( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) );
    mxMathAllButton->set_sensitive(aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ));
    mxDBAllButton->set_sensitive(aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ));
}

bool BackingWindow::PreNotify(NotifyEvent& rNEvt)
{
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKeyCode(pEvt->GetKeyCode());

        bool bThumbnailHasFocus = mxAllRecentThumbnails->HasFocus() || mxLocalView->HasFocus();

        // Subwindows of BackingWindow: Sidebar and Thumbnail view
        if( rKeyCode.GetCode() == KEY_F6 )
        {
            if( rKeyCode.IsShift() ) // Shift + F6
            {
                if (bThumbnailHasFocus)
                {
                    mxOpenButton->grab_focus();
                    return true;
                }
            }
            else if ( rKeyCode.IsMod1() ) // Ctrl + F6
            {
                if(mxAllRecentThumbnails->IsVisible())
                {
                    mxAllRecentThumbnails->GrabFocus();
                    return true;
                }
                else if(mxLocalView->IsVisible())
                {
                    mxLocalView->GrabFocus();
                    return true;
                }
            }
            else // F6
            {
                if (!bThumbnailHasFocus)
                {
                    if(mxAllRecentThumbnails->IsVisible())
                    {
                        mxAllRecentThumbnails->GrabFocus();
                        return true;
                    }
                    else if(mxLocalView->IsVisible())
                    {
                        mxLocalView->GrabFocus();
                        return true;
                    }
                }
            }
        }

        // try the 'normal' accelerators (so that eg. Ctrl+Q works)
        if (!mpAccExec)
        {
            mpAccExec = svt::AcceleratorExecute::createAcceleratorHelper();
            mpAccExec->init( comphelper::getProcessComponentContext(), mxFrame);
        }

        const OUString aCommand = mpAccExec->findCommand(svt::AcceleratorExecute::st_VCLKey2AWTKey(rKeyCode));
        if ((aCommand != "vnd.sun.star.findbar:FocusToFindbar") && pEvt && mpAccExec->execute(rKeyCode))
            return true;
    }
    else if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        Accelerator::ToggleMnemonicsOnHierarchy(*rNEvt.GetCommandEvent(), this);
    }
    return InterimItemWindow::PreNotify( rNEvt );
}

void BackingWindow::GetFocus()
{
    GetFocusFlags nFlags = GetParent()->GetGetFocusFlags();
    if( nFlags & GetFocusFlags::F6 )
    {
        if( nFlags & GetFocusFlags::Forward ) // F6
        {
            mxOpenButton->grab_focus();
            return;
        }
        else // Shift + F6 or Ctrl + F6
        {
            if(mxAllRecentThumbnails->IsVisible())
                mxAllRecentThumbnails->GrabFocus();
            else if(mxLocalView->IsVisible())
                mxLocalView->GrabFocus();
            return;
        }
    }
    InterimItemWindow::GetFocus();
}

void BackingWindow::setOwningFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    mxFrame = xFrame;
    if( ! mbInitControls )
        initControls();

    // establish drag&drop mode
    mxDropTargetListener.set(new OpenFileDropTargetListener(mxContext, mxFrame));

    if (mxDropTarget.is())
    {
        mxDropTarget->addDropTargetListener(mxDropTargetListener);
        mxDropTarget->setActive(true);
    }

    css::uno::Reference<XFramesSupplier> xFramesSupplier(mxDesktopDispatchProvider, UNO_QUERY);
    if (xFramesSupplier)
        xFramesSupplier->setActiveFrame(mxFrame);
}

IMPL_LINK(BackingWindow, ExtLinkClickHdl, weld::Button&, rButton, void)
{
    OUString aNode;

    if (&rButton == mxExtensionsButton.get())
        aNode = "AddFeatureURL";

    if (aNode.isEmpty())
        return;

    try
    {
        uno::Sequence<uno::Any> args(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(OUString("/org.openoffice.Office.Common/Help/StartCenter"))}
        }));

        Reference<lang::XMultiServiceFactory> xConfig = configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() );
        Reference<container::XNameAccess> xNameAccess(xConfig->createInstanceWithArguments(SERVICENAME_CFGREADACCESS, args), UNO_QUERY);
        if (xNameAccess.is())
        {
            OUString sURL;
            Any value(xNameAccess->getByName(aNode));

            sURL = value.get<OUString>();
            localizeWebserviceURI(sURL);

            Reference<css::system::XSystemShellExecute> const
                xSystemShellExecute(
                    css::system::SystemShellExecute::create(
                        ::comphelper::getProcessComponentContext()));
            xSystemShellExecute->execute(sURL, OUString(),
                css::system::SystemShellExecuteFlags::URIS_ONLY);
        }
    }
    catch (const Exception&)
    {
    }
}

IMPL_LINK( BackingWindow, ClickHdl, weld::Button&, rButton, void )
{
    // dispatch the appropriate URL and end the dialog
    if( &rButton == mxWriterAllButton.get() )
        dispatchURL( "private:factory/swriter" );
    else if( &rButton == mxCalcAllButton.get() )
        dispatchURL( "private:factory/scalc" );
    else if( &rButton == mxImpressAllButton.get() )
        dispatchURL( "private:factory/simpress?slot=6686" );
    else if( &rButton == mxDrawAllButton.get() )
        dispatchURL( "private:factory/sdraw" );
    else if( &rButton == mxDBAllButton.get() )
        dispatchURL( "private:factory/sdatabase?Interactive" );
    else if( &rButton == mxMathAllButton.get() )
        dispatchURL( "private:factory/smath" );
    else if( &rButton == mxOpenButton.get() )
    {
        Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

        Sequence< css::beans::PropertyValue > aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = "Referer";
        pArg[0].Value <<= OUString("private:user");

        dispatchURL( ".uno:Open", OUString(), xFrame, aArgs );
    }
    else if( &rButton == mxRemoteButton.get() )
    {
        Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

        Sequence< css::beans::PropertyValue > aArgs(0);

        dispatchURL( ".uno:OpenRemote", OUString(), xFrame, aArgs );
    }
    else if( &rButton == mxRecentButton.get() )
    {
        mxLocalView->Hide();
        mxAllRecentThumbnails->Show();
        mxAllRecentThumbnails->GrabFocus();
        mxRecentButton->set_active(true);
        mxTemplateButton->set_active(false);
    }
    else if( &rButton == mxTemplateButton.get() )
    {
        mxAllRecentThumbnails->Hide();
        initializeLocalView();
        mxLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));
        mxLocalView->Show();
        mxLocalView->reload();
        mxLocalView->GrabFocus();
        mxRecentButton->set_active(false);
        mxTemplateButton->set_active(true);
    }
}

IMPL_LINK (BackingWindow, MenuSelectHdl, const OString&, rId, void)
{
    if (rId == "clear_all")
    {
        SvtHistoryOptions().Clear(EHistoryType::PickList);
        mxAllRecentThumbnails->Reload();
        return;
    }
    else if (!rId.isEmpty())
    {
        initializeLocalView();

        if( rId == "filter_writer" )
        {
            mxLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::WRITER));
        }
        else if( rId == "filter_calc" )
        {
            mxLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::CALC));
        }
        else if( rId == "filter_impress" )
        {
            mxLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::IMPRESS));
        }
        else if( rId == "filter_draw" )
        {
            mxLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::DRAW));
        }
        else if( rId == "manage" )
        {
            Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

            Sequence< css::beans::PropertyValue > aArgs(1);
            PropertyValue* pArg = aArgs.getArray();
            pArg[0].Name = "Referer";
            pArg[0].Value <<= OUString("private:user");

            dispatchURL( ".uno:NewDoc", OUString(), xFrame, aArgs );
            return;
        }

        mxAllRecentThumbnails->Hide();
        mxLocalView->Show();
        mxLocalView->reload();
        mxLocalView->GrabFocus();
        mxRecentButton->set_active(false);
        mxTemplateButton->set_active(true);
    }
}

IMPL_LINK(BackingWindow, CreateContextMenuHdl, ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<TemplateViewItem*>(pItem);

    if (pViewItem)
        mxLocalView->createContextMenu();
}

IMPL_LINK(BackingWindow, OpenTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    uno::Sequence< PropertyValue > aArgs(4);
    aArgs[0].Name = "AsTemplate";
    aArgs[0].Value <<= true;
    aArgs[1].Name = "MacroExecutionMode";
    aArgs[1].Value <<= MacroExecMode::USE_CONFIG;
    aArgs[2].Name = "UpdateDocMode";
    aArgs[2].Value <<= UpdateDocMode::ACCORDING_TO_CONFIG;
    aArgs[3].Name = "InteractionHandler";
    aArgs[3].Value <<= task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), nullptr );

    TemplateViewItem *pTemplateItem = static_cast<TemplateViewItem*>(pItem);

    Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

    try
    {
        dispatchURL( pTemplateItem->getPath(), "_default", xFrame, aArgs );
    }
    catch( const uno::Exception& )
    {
    }
}

IMPL_LINK(BackingWindow, EditTemplateHdl, ThumbnailViewItem*, pItem, void)
{
    uno::Sequence< PropertyValue > aArgs(3);
    aArgs[0].Name = "AsTemplate";
    aArgs[0].Value <<= false;
    aArgs[1].Name = "MacroExecutionMode";
    aArgs[1].Value <<= MacroExecMode::USE_CONFIG;
    aArgs[2].Name = "UpdateDocMode";
    aArgs[2].Value <<= UpdateDocMode::ACCORDING_TO_CONFIG;

    TemplateViewItem *pViewItem = static_cast<TemplateViewItem*>(pItem);

    Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

    try
    {
        dispatchURL( pViewItem->getPath(), "_default", xFrame, aArgs );
    }
    catch( const uno::Exception& )
    {
    }
}

namespace {

struct ImplDelayedDispatch
{
    Reference< XDispatch >      xDispatch;
    css::util::URL   aDispatchURL;
    Sequence< PropertyValue >   aArgs;

    ImplDelayedDispatch( const Reference< XDispatch >& i_xDispatch,
                         const css::util::URL& i_rURL,
                         const Sequence< PropertyValue >& i_rArgs )
    : xDispatch( i_xDispatch ),
      aDispatchURL( i_rURL ),
      aArgs( i_rArgs )
    {
    }
};

}

static void implDispatchDelayed( void*, void* pArg )
{
    struct ImplDelayedDispatch* pDispatch = static_cast<ImplDelayedDispatch*>(pArg);
    try
    {
        pDispatch->xDispatch->dispatch( pDispatch->aDispatchURL, pDispatch->aArgs );
    }
    catch (const Exception&)
    {
    }

    // clean up
    delete pDispatch;
}

void BackingWindow::dispatchURL( const OUString& i_rURL,
                                 const OUString& rTarget,
                                 const Reference< XDispatchProvider >& i_xProv,
                                 const Sequence< PropertyValue >& i_rArgs )
{
    // if no special dispatch provider is given, get the desktop
    Reference< XDispatchProvider > xProvider( i_xProv.is() ? i_xProv : mxDesktopDispatchProvider );

    // check for dispatch provider
    if( !xProvider.is())
        return;

    // get a URL transformer to clean up the URL
    css::util::URL aDispatchURL;
    aDispatchURL.Complete = i_rURL;

    Reference < css::util::XURLTransformer > xURLTransformer(
        css::util::URLTransformer::create( comphelper::getProcessComponentContext() ) );
    try
    {
        // clean up the URL
        xURLTransformer->parseStrict( aDispatchURL );
        // get a Dispatch for the URL and target
        Reference< XDispatch > xDispatch(
            xProvider->queryDispatch( aDispatchURL, rTarget, 0 )
            );
        // dispatch the URL
        if ( xDispatch.is() )
        {
            std::unique_ptr<ImplDelayedDispatch> pDisp(new ImplDelayedDispatch( xDispatch, aDispatchURL, i_rArgs ));
            if( Application::PostUserEvent( Link<void*,void>( nullptr, implDispatchDelayed ), pDisp.get() ) )
                pDisp.release();
        }
    }
    catch (const css::uno::RuntimeException&)
    {
        throw;
    }
    catch (const css::uno::Exception&)
    {
    }
}

void BackingWindow::clearRecentFileList()
{
    mxAllRecentThumbnails->Clear();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab:*/
