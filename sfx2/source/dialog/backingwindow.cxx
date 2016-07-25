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
#include <sfx2/inputdlg.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include "backingwindowsearchview.hxx"
#include "templatesearchviewitem.hxx"

#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/openfiledroptargetlistener.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/langhelp.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templatedlg.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <toolkit/awt/vclxmenu.hxx>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <officecfg/Office/Common.hxx>

#define MNI_WRITER           1
#define MNI_CALC             2
#define MNI_IMPRESS          3
#define MNI_DRAW             4

class SearchView_Keyword;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::document;

const char SERVICENAME_CFGREADACCESS[] = "com.sun.star.configuration.ConfigurationAccess";

// increase size of the text in the buttons on the left fMultiplier-times
float fMultiplier = 1.4f;

BackingWindow::BackingWindow( vcl::Window* i_pParent ) :
    Window( i_pParent ),
    mbLocalViewInitialized(false),
    maButtonsTextColor(officecfg::Office::Common::Help::StartCenter::StartCenterTextColor::get()),
    mbInitControls( false ),
    mnHideExternalLinks( 0 )
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), "sfx/ui/startcenter.ui", "StartCenter" );

    get(mpOpenButton, "open_all");
    get(mpRemoteButton, "open_remote");
    get(mpRecentButton, "open_recent");
    get(mpTemplateButton, "templates_all");

    get(mpSearchFilter, "search_filter");
    get(mpCBApp, "filter_application");
    get(mpSearchView, "search_view");

    get(mpCreateLabel, "create_label");

    get(mpWriterAllButton, "writer_all");
    get(mpCalcAllButton, "calc_all");
    get(mpImpressAllButton, "impress_all");
    get(mpDrawAllButton, "draw_all");
    get(mpDBAllButton, "database_all");
    get(mpMathAllButton, "math_all");

    get(mpHelpButton, "help");
    //set an alternative help label that doesn't hotkey the H of the Help menu
    mpHelpButton->SetText(get<Window>("althelplabel")->GetText());
    get(mpExtensionsButton, "extensions");

    //Containers are invisible to cursor traversal
    //So on pressing "right" when in Help the
    //extension button is considered as a candidate

    //But the containers are not invisible to the PushButton ctor which checks
    //if the preceding window of its parent is a button and if it then
    //defaults to grouping with it and if it is not a button defaults to
    //setting itself as the start of a new group.

    //So here take the second button and set it as explicitly not the start
    //of a group, i.e. allow it to be grouped with the preceding
    //PushButton so when seen as a candidate by cursor travelling
    //it will be accepted as a continuation of the group.
    WinBits nBits = mpExtensionsButton->GetStyle();
    nBits &= ~WB_GROUP;
    nBits |= WB_NOGROUP;
    mpExtensionsButton->SetStyle(nBits);
    assert(mpHelpButton->GetStyle() & WB_GROUP);
    assert(!(mpExtensionsButton->GetStyle() & WB_GROUP));

    get(mpAllButtonsBox, "all_buttons_box");
    get(mpButtonsBox, "buttons_box");
    get(mpSmallButtonsBox, "small_buttons_box");

    get(mpAllRecentThumbnails, "all_recent");
    get(mpLocalView, "local_view");

    maDndWindows.push_back(mpAllRecentThumbnails);

    try
    {
        mxContext.set( ::comphelper::getProcessComponentContext(), uno::UNO_SET_THROW );
        Reference<lang::XMultiServiceFactory> xConfig = configuration::theDefaultProvider::get( mxContext );
        Sequence<Any> args(1);
        PropertyValue val(
            "nodepath",
            0,
            Any(OUString("/org.openoffice.Office.Common/Help/StartCenter")),
            PropertyState_DIRECT_VALUE);
        args.getArray()[0] <<= val;
        Reference<container::XNameAccess> xNameAccess(xConfig->createInstanceWithArguments(SERVICENAME_CFGREADACCESS,args), UNO_QUERY);
        if( xNameAccess.is() )
        {
            //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
            Any value( xNameAccess->getByName("StartCenterHideExternalLinks") );
            mnHideExternalLinks = value.get<sal_Int32>();
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN( "fwk", "BackingWindow - caught an exception! " << e.Message );
    }

    // fdo#34392: we do the layout dynamically, the layout depends on the font,
    // so we should handle data changed events (font changing) of the last child
    // control, at this point all the controls have updated settings (i.e. font).

    EnableChildTransparentMode();

    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    // get dispatch provider
    Reference<XDesktop2> xDesktop = Desktop::create( comphelper::getProcessComponentContext() );
    mxDesktopDispatchProvider = xDesktop;

    // init background
    SetBackground();
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
        for (auto aI = maDndWindows.begin(), aEnd = maDndWindows.end(); aI != aEnd; ++aI)
        {
            vcl::Window *pDndWin = *aI;
            css::uno::Reference< css::datatransfer::dnd::XDropTarget > xDropTarget =
                    pDndWin->GetDropTarget();
            if (xDropTarget.is())
            {
                xDropTarget->removeDropTargetListener(mxDropTargetListener);
                xDropTarget->setActive(false);
            }
        }
        mxDropTargetListener.clear();
    }
    disposeBuilder();
    maDndWindows.clear();
    mpOpenButton.clear();
    mpRemoteButton.clear();
    mpRecentButton.clear();
    mpTemplateButton.clear();
    mpCreateLabel.clear();
    mpWriterAllButton.clear();
    mpCalcAllButton.clear();
    mpImpressAllButton.clear();
    mpDrawAllButton.clear();
    mpDBAllButton.clear();
    mpMathAllButton.clear();
    mpHelpButton.clear();
    mpExtensionsButton.clear();
    mpAllButtonsBox.clear();
    mpButtonsBox.clear();
    mpSearchFilter.clear();
    mpSearchView.clear();
    mpCBApp.clear();
    mpSmallButtonsBox.clear();
    mpAllRecentThumbnails.clear();
    mpLocalView.clear();
    vcl::Window::dispose();
}

void BackingWindow::initControls()
{
    if( mbInitControls )
        return;

    mbInitControls = true;

    // collect the URLs of the entries in the File/New menu
    SvtModuleOptions    aModuleOptions;
    std::set< OUString > aFileNewAppsAvailable;
    SvtDynamicMenuOptions aOpt;
    Sequence < Sequence < PropertyValue > > aNewMenu = aOpt.GetMenu( E_NEWMENU );
    const OUString sURLKey( "URL"  );

    const Sequence< PropertyValue >* pNewMenu = aNewMenu.getConstArray();
    const Sequence< PropertyValue >* pNewMenuEnd = aNewMenu.getConstArray() + aNewMenu.getLength();
    for ( ; pNewMenu != pNewMenuEnd; ++pNewMenu )
    {
        comphelper::SequenceAsHashMap aEntryItems( *pNewMenu );
        OUString sURL( aEntryItems.getUnpackedValueOrDefault( sURLKey, OUString() ) );
        if ( !sURL.isEmpty() )
            aFileNewAppsAvailable.insert( sURL );
    }

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::WRITER))
        mpAllRecentThumbnails->mnFileTypes |= TYPE_WRITER;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::CALC))
        mpAllRecentThumbnails->mnFileTypes |= TYPE_CALC;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::IMPRESS))
        mpAllRecentThumbnails->mnFileTypes |= TYPE_IMPRESS;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::DRAW))
        mpAllRecentThumbnails->mnFileTypes |= TYPE_DRAW;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::DATABASE))
        mpAllRecentThumbnails->mnFileTypes |= TYPE_DATABASE;

    if (aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::MATH))
        mpAllRecentThumbnails->mnFileTypes |= TYPE_MATH;

    mpAllRecentThumbnails->mnFileTypes |= TYPE_OTHER;
    mpAllRecentThumbnails->Reload();
    mpAllRecentThumbnails->ShowTooltips( true );
    mpRecentButton->SetActive(true);

    mpCBApp->SelectEntryPos(0);
    mpCBApp->SetSelectHdl(LINK(this, BackingWindow, SelectApplicationHdl));

    //initialize Template view
    mpLocalView->SetStyle( mpLocalView->GetStyle() | WB_VSCROLL);
    mpLocalView->Hide();

    mpTemplateButton->SetMenuMode( MENUBUTTON_MENUMODE_TIMED );
    mpRecentButton->SetMenuMode( MENUBUTTON_MENUMODE_TIMED );

    //set handlers
    mpLocalView->setCreateContextMenuHdl(LINK(this, BackingWindow, CreateContextMenuHdl));
    mpLocalView->setOpenTemplateHdl(LINK(this, BackingWindow, OpenTemplateHdl));
    mpLocalView->setEditTemplateHdl(LINK(this, BackingWindow, EditTemplateHdl));
    mpLocalView->ShowTooltips( true );

    //initialize Template view
    mpSearchView->SetStyle( mpLocalView->GetStyle() | WB_VSCROLL);
    mpSearchView->Hide();

    mpSearchView->setCreateContextMenuHdl(LINK(this,BackingWindow, CreateContextMenuHdl));
    mpSearchView->setOpenTemplateHdl(LINK(this,BackingWindow,OpenTemplateHdl));
    mpSearchView->setEditTemplateHdl(LINK(this,BackingWindow, EditTemplateHdl));

    mpSearchView->ShowTooltips(true);

    mpSearchFilter->SetUpdateDataHdl(LINK(this, BackingWindow, SearchUpdateHdl));
    mpSearchFilter->EnableUpdateData();
    mpSearchFilter->SetGetFocusHdl(LINK( this, BackingWindow, GetFocusHdl ));

    setupButton( mpOpenButton );
    setupButton( mpRemoteButton );
    setupButton( mpRecentButton );
    setupButton( mpTemplateButton );
    setupButton( mpWriterAllButton );
    setupButton( mpDrawAllButton );
    setupButton( mpCalcAllButton );
    setupButton( mpDBAllButton );
    setupButton( mpImpressAllButton );
    setupButton( mpMathAllButton );

    checkInstalledModules();

    mpExtensionsButton->SetClickHdl(LINK(this, BackingWindow, ExtLinkClickHdl));

    // setup nice colors
    mpCreateLabel->SetControlForeground(maButtonsTextColor);
    vcl::Font aFont(mpCreateLabel->GetSettings().GetStyleSettings().GetLabelFont());
    aFont.SetFontSize(Size(0, aFont.GetFontSize().Height() * fMultiplier));
    mpCreateLabel->SetControlFont(aFont);

    mpHelpButton->SetControlForeground(maButtonsTextColor);
    mpExtensionsButton->SetControlForeground(maButtonsTextColor);

    const Color aButtonsBackground(officecfg::Office::Common::Help::StartCenter::StartCenterBackgroundColor::get());

    mpAllButtonsBox->SetBackground(aButtonsBackground);
    mpSmallButtonsBox->SetBackground(aButtonsBackground);

    // motif image under the buttons
    Wallpaper aWallpaper(get<FixedImage>("motif")->GetImage().GetBitmapEx());
    aWallpaper.SetStyle(WallpaperStyle::BottomRight);
    aWallpaper.SetColor(aButtonsBackground);

    mpButtonsBox->SetBackground(aWallpaper);

    Resize();

    // compute the menubar height
    sal_Int32 nMenuHeight = 0;
    SystemWindow* pSystemWindow = GetSystemWindow();
    if (pSystemWindow)
    {
        MenuBar* pMenuBar = pSystemWindow->GetMenuBar();
        if (pMenuBar)
            nMenuHeight = pMenuBar->ImplGetWindow()->GetOutputSizePixel().Height();
    }

    set_width_request(mpAllRecentThumbnails->get_width_request() + mpAllButtonsBox->GetOptimalSize().Width());
    set_height_request(nMenuHeight + mpAllButtonsBox->GetOptimalSize().Height());
}

void BackingWindow::initializeLocalView()
{
    if (!mbLocalViewInitialized)
    {
        mbLocalViewInitialized = true;
        mpLocalView->Populate();
        mpLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));
        mpLocalView->showAllTemplates();
    }
}

void BackingWindow::setupButton( PushButton* pButton )
{
    // the buttons should have a bit bigger font
    vcl::Font aFont(pButton->GetSettings().GetStyleSettings().GetPushButtonFont());
    aFont.SetFontSize(Size(0, aFont.GetFontSize().Height() * fMultiplier));
    pButton->SetControlFont(aFont);

    // color that fits the theme
    pButton->SetControlForeground(maButtonsTextColor);
    pButton->SetClickHdl( LINK( this, BackingWindow, ClickHdl ) );
}

void BackingWindow::setupButton( MenuToggleButton* pButton )
{
    vcl::Font aFont(pButton->GetSettings().GetStyleSettings().GetPushButtonFont());
    aFont.SetFontSize(Size(0, aFont.GetFontSize().Height() * fMultiplier));
    pButton->SetControlFont(aFont);

    // color that fits the theme
    pButton->SetControlForeground(maButtonsTextColor);

    PopupMenu* pMenu = pButton->GetPopupMenu();
    pMenu->SetMenuFlags(pMenu->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries);

    pButton->SetClickHdl(LINK(this, BackingWindow, ClickHdl));
    pButton->SetSelectHdl(LINK(this, BackingWindow, MenuSelectHdl));
}

void BackingWindow::checkInstalledModules()
{
    SvtModuleOptions aModuleOpt;

    mpWriterAllButton->Enable( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ));

    mpCalcAllButton->Enable( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) );

    mpImpressAllButton->Enable( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) );

    mpDrawAllButton->Enable( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) );

    mpMathAllButton->Enable(aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ));

    mpDBAllButton->Enable(aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ));
}

void BackingWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    Resize();

    Wallpaper aBack(svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor);
    vcl::Region aClip(Rectangle(Point(0, 0), GetOutputSizePixel()));

    aClip.Exclude(maStartCentButtons);

    rRenderContext.Push(PushFlags::CLIPREGION);
    rRenderContext.IntersectClipRegion(aClip);
    rRenderContext.DrawWallpaper(Rectangle(Point(0, 0), GetOutputSizePixel()), aBack);
    rRenderContext.Pop();

    ScopedVclPtrInstance<VirtualDevice> pVDev(rRenderContext);
    pVDev->EnableRTL(rRenderContext.IsRTLEnabled());
    pVDev->SetOutputSizePixel(maStartCentButtons.GetSize());
    Point aOffset(Point(0, 0) - maStartCentButtons.TopLeft());
    pVDev->DrawWallpaper(Rectangle(aOffset, GetOutputSizePixel()), aBack);

    rRenderContext.DrawOutDev(maStartCentButtons.TopLeft(), maStartCentButtons.GetSize(),
                              Point(0, 0), maStartCentButtons.GetSize(),
                              *pVDev.get());
}

bool BackingWindow::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKeyCode(pEvt->GetKeyCode());

        // Subwindows of BackingWindow: Sidebar and Thumbnail view
        if( rKeyCode.GetCode() == KEY_F6 )
        {
            if( rKeyCode.IsShift() ) // Shift + F6
            {
                if( mpAllRecentThumbnails->HasFocus() || mpLocalView->HasFocus())
                {
                    mpOpenButton->GrabFocus();
                    return true;
                }
            }
            else if ( rKeyCode.IsMod1() ) // Ctrl + F6
            {
                if(mpAllRecentThumbnails->IsVisible())
                {
                    mpAllRecentThumbnails->GrabFocus();
                    return true;
                }
                else if(mpLocalView->IsVisible())
                {
                    mpLocalView->GrabFocus();
                    return true;
                }
            }
            else // F6
            {
                if(mpAllRecentThumbnails->IsVisible())
                {
                    mpAllRecentThumbnails->GrabFocus();
                    return true;
                }
                else if(mpLocalView->IsVisible())
                {
                    mpLocalView->GrabFocus();
                    return true;
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

    return Window::PreNotify( rNEvt );
}

FILTER_APPLICATION BackingWindow::getCurrentApplicationFilter()
{
    const sal_Int16 nCurAppId = mpCBApp->GetSelectEntryPos();

    if (nCurAppId == MNI_WRITER)
        return FILTER_APPLICATION::WRITER;
    else if (nCurAppId == MNI_IMPRESS)
        return FILTER_APPLICATION::IMPRESS;
    else if (nCurAppId == MNI_CALC)
        return FILTER_APPLICATION::CALC;
    else if (nCurAppId == MNI_DRAW)
        return FILTER_APPLICATION::DRAW;

    return FILTER_APPLICATION::NONE;
}

IMPL_LINK_NOARG_TYPED(BackingWindow, SelectApplicationHdl, ListBox&, void)
{
    if(mpLocalView->IsVisible())
    {
        mpLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mpLocalView->showAllTemplates();
    }

    if(mpSearchView->IsVisible())
        SearchUpdateHdl(*mpSearchFilter);
}

IMPL_LINK_NOARG_TYPED(BackingWindow, GetFocusHdl, Control&, void)
{
    mpLocalView->deselectItems();
    mpSearchView->deselectItems();
}

IMPL_LINK_NOARG_TYPED(BackingWindow, SearchUpdateHdl, Edit&, void)
{
    OUString aKeyword = mpSearchFilter->GetText();

    if (!aKeyword.isEmpty())
    {
        mpSearchView->Clear();

        // if the search view is hidden, hide the folder view and display search one
        if (!mpSearchView->IsVisible())
        {
            mpLocalView->deselectItems();
            mpSearchView->Show();
            mpLocalView->Hide();
        }

        std::vector<TemplateItemProperties> aItems =
                mpLocalView->getFilteredItems(SearchView_Keyword(aKeyword, getCurrentApplicationFilter()));

        for (TemplateItemProperties& rItem : aItems)
        {
            OUString aFolderName;

            aFolderName = mpLocalView->getRegionName(rItem.nRegionId);

            mpSearchView->AppendItem(rItem.nId,mpLocalView->getRegionId(rItem.nRegionId),
                                     rItem.nDocId,
                                     rItem.aName,
                                     aFolderName,
                                     rItem.aPath,
                                     rItem.aThumbnail);
        }

        mpSearchView->Invalidate();
    }
    else
    {
        mpSearchView->deselectItems();
        mpSearchView->Hide();
        mpLocalView->Show();
        mpLocalView->filterItems(ViewFilter_Application(getCurrentApplicationFilter()));
        mpLocalView->reload();
    }
}


void BackingWindow::GetFocus()
{
    GetFocusFlags nFlags = GetParent()->GetGetFocusFlags();
    if( nFlags & GetFocusFlags::F6 )
    {
        if( nFlags & GetFocusFlags::Forward ) // F6
        {
            mpOpenButton->GrabFocus();
            return;
        }
        else // Shift + F6 or Ctrl + F6
        {
            mpAllRecentThumbnails->GrabFocus();
            return;
        }
    }
    Window::GetFocus();
}

void BackingWindow::setOwningFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    mxFrame = xFrame;
    if( ! mbInitControls )
        initControls();

    // establish drag&drop mode
    mxDropTargetListener.set(new OpenFileDropTargetListener(mxContext, mxFrame));

    for (auto aI = maDndWindows.begin(), aEnd = maDndWindows.end(); aI != aEnd; ++aI)
    {
        vcl::Window *pDndWin = *aI;
        css::uno::Reference< css::datatransfer::dnd::XDropTarget > xDropTarget =
            pDndWin->GetDropTarget();
        if (xDropTarget.is())
        {
            xDropTarget->addDropTargetListener(mxDropTargetListener);
            xDropTarget->setActive(true);
        }
    }
}

void BackingWindow::Resize()
{
    maStartCentButtons = Rectangle( Point(0, 0), GetOutputSizePixel() );

    if (isLayoutEnabled(this))
        VclContainer::setLayoutAllocation(*GetWindow(GetWindowType::FirstChild),
            maStartCentButtons.TopLeft(), maStartCentButtons.GetSize());

    if (!IsInPaint())
        Invalidate();
}

IMPL_LINK_TYPED(BackingWindow, ExtLinkClickHdl, Button*, pButton, void)
{
    OUString aNode;

    if (pButton == mpExtensionsButton)
        aNode = "AddFeatureURL";

    if (!aNode.isEmpty())
    {
        try
        {
            Sequence<Any> args(1);
            PropertyValue val("nodepath", 0, Any(OUString("/org.openoffice.Office.Common/Help/StartCenter")), PropertyState_DIRECT_VALUE);
            args.getArray()[0] <<= val;

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
}

IMPL_LINK_TYPED( BackingWindow, ClickHdl, Button*, pButton, void )
{
    // dispatch the appropriate URL and end the dialog
    if( pButton == mpWriterAllButton )
        dispatchURL( "private:factory/swriter" );
    else if( pButton == mpCalcAllButton )
        dispatchURL( "private:factory/scalc" );
    else if( pButton == mpImpressAllButton )
        dispatchURL( "private:factory/simpress?slot=6686" );
    else if( pButton == mpDrawAllButton )
        dispatchURL( "private:factory/sdraw" );
    else if( pButton == mpDBAllButton )
        dispatchURL( "private:factory/sdatabase?Interactive" );
    else if( pButton == mpMathAllButton )
        dispatchURL( "private:factory/smath" );
    else if( pButton == mpOpenButton )
    {
        Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

        Sequence< css::beans::PropertyValue > aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = "Referer";
        pArg[0].Value <<= OUString("private:user");

        dispatchURL( ".uno:Open", OUString(), xFrame, aArgs );
    }
    else if( pButton == mpRemoteButton )
    {
        Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

        Sequence< css::beans::PropertyValue > aArgs(0);

        dispatchURL( ".uno:OpenRemote", OUString(), xFrame, aArgs );
    }
    else if( pButton == mpRecentButton )
    {
        mpLocalView->Hide();
        mpAllRecentThumbnails->Show();
        mpAllRecentThumbnails->GrabFocus();
        mpRecentButton->SetActive(true);
        mpTemplateButton->SetActive(false);
        mpTemplateButton->Invalidate();
    }
    else if( pButton == mpTemplateButton )
    {
        mpAllRecentThumbnails->Hide();
        initializeLocalView();
        mpLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::NONE));
        mpLocalView->Show();
        mpLocalView->reload();
        mpLocalView->GrabFocus();
        mpRecentButton->SetActive(false);
        mpRecentButton->Invalidate();
        mpTemplateButton->SetActive(true);
    }
}

IMPL_LINK_TYPED( BackingWindow, MenuSelectHdl, MenuButton*, pButton, void )
{
    if(pButton == mpRecentButton)
    {
        SvtHistoryOptions().Clear(ePICKLIST);
        mpAllRecentThumbnails->Reload();
        return;
    }
    else if(pButton == mpTemplateButton)
    {
        initializeLocalView();

        OString sId = pButton->GetCurItemIdent();

        if( sId == "filter_writer" )
        {
            mpLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::WRITER));
        }
        else if( sId == "filter_calc" )
        {
            mpLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::CALC));
        }
        else if( sId == "filter_impress" )
        {
            mpLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::IMPRESS));
        }
        else if( sId == "filter_draw" )
        {
            mpLocalView->filterItems(ViewFilter_Application(FILTER_APPLICATION::DRAW));
        }
        else if( sId == "manage" )
        {
            Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

            Sequence< css::beans::PropertyValue > aArgs(1);
            PropertyValue* pArg = aArgs.getArray();
            pArg[0].Name = "Referer";
            pArg[0].Value <<= OUString("private:user");

            dispatchURL( ".uno:NewDoc", OUString(), xFrame, aArgs );

        }

        mpAllRecentThumbnails->Hide();
        mpLocalView->Show();
        mpLocalView->reload();
        mpLocalView->GrabFocus();
        mpRecentButton->SetActive(false);
        mpTemplateButton->SetActive(true);
        mpRecentButton->Invalidate();
    }
}

IMPL_LINK_TYPED(BackingWindow, CreateContextMenuHdl, ThumbnailViewItem*, pItem, void)
{
    const TemplateViewItem *pViewItem = dynamic_cast<TemplateViewItem*>(pItem);

    if (pViewItem)
        mpLocalView->createContextMenu();
}

IMPL_LINK_TYPED(BackingWindow, OpenTemplateHdl, ThumbnailViewItem*, pItem, void)
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

IMPL_LINK_TYPED(BackingWindow, EditTemplateHdl, ThumbnailViewItem*, pItem, void)
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
    ~ImplDelayedDispatch() {}
};

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

    // get an URL transformer to clean up the URL
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
            ImplDelayedDispatch* pDisp = new ImplDelayedDispatch( xDispatch, aDispatchURL, i_rArgs );
            if( Application::PostUserEvent( Link<void*,void>( nullptr, implDispatchDelayed ), pDisp ) == nullptr )
                delete pDisp; // event could not be posted for unknown reason, at least don't leak
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

Size BackingWindow::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));

    return Window::GetOptimalSize();
}

void BackingWindow::clearRecentFileList()
{
    mpAllRecentThumbnails->Clear();
    set_width_request(mpAllRecentThumbnails->get_width_request() + mpAllButtonsBox->GetOptimalSize().Width());
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab:*/
