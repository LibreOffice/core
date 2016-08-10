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

#include <swtypes.hxx>
#include <globals.hrc>
#include <misc.hrc>

#include <utlui.hrc>
#include <unotools.hrc>
#include <unotools.hxx>
#include <unoprnms.hxx>
#include <i18nutil/unicode.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XScreenCursor.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <shellio.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <TextCursorHelper.hxx>
#include <unocrsr.hxx>
#include <doc.hxx>

#include <unomid.h>

using namespace ::com::sun::star;

const sal_Char cFactory[] = "private:factory/swriter";

bool SwOneExampleFrame::bShowServiceNotAvailableMessage = true;

SwOneExampleFrame::SwOneExampleFrame( vcl::Window& rWin,
                                        sal_uInt32 nFlags,
                                        const Link<SwOneExampleFrame&,void>* pInitializedLink,
                                        const OUString* pURL ) :
    m_aTopWindow(VclPtr<SwFrameCtrlWindow>::Create(&rWin, this)),
    m_aLoadedIdle("sw uibase SwOneExampleFrame Loaded"),
    m_aMenuRes(ResId(RES_FRMEX_MENU, *pSwResMgr)),
    m_pModuleView(SW_MOD()->GetView()),
    m_nStyleFlags(nFlags),
    m_bIsInitialized(false),
    m_bServiceAvailable(false)
{
    if (pURL && !pURL->isEmpty())
        m_sArgumentURL = *pURL;

    m_aTopWindow->SetPosSizePixel(Point(0, 0), rWin.GetSizePixel());

    if( pInitializedLink )
        m_aInitializedLink = *pInitializedLink;

    // the controller is asynchronously set
    m_aLoadedIdle.SetInvokeHandler(LINK(this, SwOneExampleFrame, TimeoutHdl));
    m_aLoadedIdle.SetPriority(TaskPriority::HIGH_IDLE);

    CreateControl();

    m_aTopWindow->Show();
}

void SwOneExampleFrame::CreateErrorMessage()
{
    if(SwOneExampleFrame::bShowServiceNotAvailableMessage)
    {
        OUString sInfo(SwResId(STR_SERVICE_UNAVAILABLE));
        sInfo += "com.sun.star.frame.FrameControl";
        ScopedVclPtrInstance<InfoBox>(nullptr, sInfo)->Execute();
        SwOneExampleFrame::bShowServiceNotAvailableMessage = false;
    }
}

SwOneExampleFrame::~SwOneExampleFrame()
{
    DisposeControl();
}

void SwOneExampleFrame::CreateControl()
{
    if(m_xControl.is())
        return ;
    uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = comphelper::getProcessServiceFactory();
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    uno::Reference< uno::XInterface >  xInst = xMgr->createInstance( "com.sun.star.frame.FrameControl" );
    m_xControl.set(xInst, uno::UNO_QUERY);
    if(m_xControl.is())
    {
        uno::Reference< awt::XWindowPeer >  xParent( m_aTopWindow->GetComponentInterface() );

        uno::Reference< awt::XToolkit >  xToolkit( awt::Toolkit::create(xContext), uno::UNO_QUERY_THROW );

        m_xControl->createPeer( xToolkit, xParent );

        uno::Reference< awt::XWindow >  xWin( m_xControl, uno::UNO_QUERY );
        xWin->setVisible(false);
        Size aWinSize(m_aTopWindow->GetOutputSizePixel());
        xWin->setPosSize( 0, 0, aWinSize.Width(), aWinSize.Height(), awt::PosSize::SIZE );

        uno::Reference< beans::XPropertySet >  xPrSet(xInst, uno::UNO_QUERY);
        uno::Any aURL;
        // create new doc
        OUString sTempURL(cFactory);
        if(!m_sArgumentURL.isEmpty())
            sTempURL = m_sArgumentURL;
        aURL <<= sTempURL;

        uno::Sequence<beans::PropertyValue> aSeq( comphelper::InitPropertySequence({
                { "OpenFlags", uno::Any(OUString("-RB")) },
                { "Referer", uno::Any(OUString("private:user")) },
                { "ReadOnly", uno::Any(sTempURL != cFactory) }
            }));
        uno::Any aArgs(aSeq);

        xPrSet->setPropertyValue( "LoaderArguments", aArgs );
        //save and set readonly???

        xPrSet->setPropertyValue("ComponentURL", aURL);

        m_aLoadedIdle.Start();
        m_bServiceAvailable = true;
    }
}

void    SwOneExampleFrame::DisposeControl()
{
    m_aLoadedIdle.Stop();
    m_aTopWindow.clear();
    m_xCursor = nullptr;
    if(m_xControl.is())
        m_xControl->dispose();
    m_xControl = nullptr;
    m_xModel = nullptr;
    m_xController = nullptr;
}

static void disableScrollBars(uno::Reference< beans::XPropertySet > const & xViewProps,
    bool bEnableOnlineMode)
{
    //the scrollbar logic is kind of busted looking in writer, when the hori scrollbar
    //property is changed then the hori scrollbar is enabled if the property is
    //true or browse (online) mode is enabled. So...
    //disable online mode
    //turn off scrollbars
    //turn back on online mode if that's what we want
    //which subverts the (dodgy/buggy) scrollbar setting

    //To reproduce this problem, in edit->autotext and click through
    //the examples and see if the preview gets a horizontal scrollbar
    uno::Any aFalseSet(uno::makeAny(false));
    xViewProps->setPropertyValue(UNO_NAME_SHOW_ONLINE_LAYOUT, aFalseSet);

    xViewProps->setPropertyValue(UNO_NAME_SHOW_HORI_SCROLL_BAR, aFalseSet);
    xViewProps->setPropertyValue(UNO_NAME_SHOW_VERT_SCROLL_BAR, aFalseSet);

    if (bEnableOnlineMode)
    {
        xViewProps->setPropertyValue(UNO_NAME_SHOW_ONLINE_LAYOUT, uno::makeAny(true));
    }
}

IMPL_LINK( SwOneExampleFrame, TimeoutHdl, Timer*, pTimer, void )
{
    if(!m_xControl.is())
        return;

    // now get the model
    uno::Reference< beans::XPropertySet >  xPrSet(m_xControl, uno::UNO_QUERY);
    uno::Any aFrame = xPrSet->getPropertyValue("Frame");
    uno::Reference< frame::XFrame >  xFrame;
    aFrame >>= xFrame;

    uno::Reference< beans::XPropertySet > xPropSet( xFrame, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            uno::Reference< frame::XLayoutManager > xLayoutManager;
            uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= xLayoutManager;
            if ( xLayoutManager.is() )
                xLayoutManager->setVisible( false );
        }
        catch (const uno::Exception&)
        {
        }
    }

    m_xController = xFrame->getController();
    if(m_xController.is())
    {
        m_xModel = m_xController->getModel();
        //now the ViewOptions should be set properly
        uno::Reference< view::XViewSettingsSupplier >  xSettings(m_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        const uno::Any aTrueSet( true );
        const uno::Any aFalseSet( false );

        if( !m_bIsInitialized )
        {
            xViewProps->setPropertyValue(UNO_NAME_SHOW_BREAKS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_DRAWINGS, aTrueSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_FIELD_COMMANDS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_GRAPHICS, aTrueSet);
            xViewProps->setPropertyValue(UNO_NAME_HIDE_WHITESPACE, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_HIDDEN_PARAGRAPHS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_HIDDEN_TEXT, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_HORI_RULER, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_PARA_BREAKS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_PROTECTED_SPACES, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_SOFT_HYPHENS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_SPACES, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_TABLES, aTrueSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_TABSTOPS, aFalseSet);
            xViewProps->setPropertyValue(UNO_NAME_SHOW_VERT_RULER, aFalseSet);

            if(0 ==(m_nStyleFlags&EX_SHOW_ONLINE_LAYOUT))
            {
                uno::Any aZoom;
                aZoom <<= (sal_Int16)view::DocumentZoomType::PAGE_WIDTH_EXACT;
                xViewProps->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);
            }
            else
            {
                uno::Any aZoom;
                aZoom <<= (sal_Int16)view::DocumentZoomType::BY_VALUE;
                xViewProps->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);

                sal_Int16 nZoomValue = 50;
                if(EX_SHOW_BUSINESS_CARDS == m_nStyleFlags)
                {
                    nZoomValue = 80;
                }
                aZoom <<= nZoomValue;
                xViewProps->setPropertyValue(UNO_NAME_ZOOM_VALUE, aZoom);
            }

            // set onlinelayout property after setting the zoom
            disableScrollBars(xViewProps, (m_nStyleFlags&EX_SHOW_ONLINE_LAYOUT) != 0);
            m_bIsInitialized = true;
        }

        uno::Reference< text::XTextDocument >  xDoc(m_xModel, uno::UNO_QUERY);
        uno::Reference< text::XText >  xText = xDoc->getText();
        m_xCursor = xText->createTextCursor();

        //From here, a cursor is defined, which goes through the template,
        //and overwrites the template words where it is necessary.

        uno::Reference< lang::XUnoTunnel> xTunnel( m_xCursor, uno::UNO_QUERY);
        if( xTunnel.is() )
        {
            OTextCursorHelper* pCursor = reinterpret_cast<OTextCursorHelper*>( xTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() ));
            if( pCursor )
            {
                SwEditShell* pSh = pCursor->GetDoc()->GetEditShell();

                do
                {
                  if (pSh->GetCurWord() == "HEADING1")
                  {
                    pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_HEADING1));
                  }
                  else if (pSh->GetCurWord() == "ENTRY1")
                  {
                    pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_ENTRY1));
                  }
                  else if (pSh->GetCurWord() == "HEADING11")
                  {
                    pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_HEADING11));
                  }
                  else if (pSh->GetCurWord() == "ENTRY11")
                  {
                    pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_ENTRY11));
                  }
                  else if (pSh->GetCurWord() == "HEADING12")
                  {
                    pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_HEADING12));
                  }
                  else if (pSh->GetCurWord() == "ENTRY12")
                  {
                    pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_ENTRY12));
                  }
                  else if (pSh->GetCurWord() == "TABLE1")
                  {
                    pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_TABLE1));
                  }
                  else if (pSh->GetCurWord() == "IMAGE1")
                  {
                    pSh->Overwrite(SwResId(STR_IDXEXAMPLE_IDXTXT_IMAGE1));
                  }
                  else
                  {}
                }
                while(pSh->Right(sal_uInt16(1), sal_uInt16(1), true));
            }
        }

        uno::Reference< beans::XPropertySet >  xCursorProp(m_xCursor, uno::UNO_QUERY);
        uno::Any aPageStyle = xCursorProp->getPropertyValue(UNO_NAME_PAGE_STYLE_NAME);
        OUString sPageStyle;
        aPageStyle >>= sPageStyle;

        uno::Reference< style::XStyleFamiliesSupplier >  xSSupp( xDoc, uno::UNO_QUERY);
        uno::Reference< container::XNameAccess >  xStyles = xSSupp->getStyleFamilies();
        uno::Any aPFamily = xStyles->getByName( "PageStyles" );
        uno::Reference< container::XNameContainer >  xPFamily;

        if( EX_SHOW_DEFAULT_PAGE != m_nStyleFlags
                && (aPFamily >>= xPFamily) && !sPageStyle.isEmpty() )
        {
            uno::Any aPStyle = xPFamily->getByName( sPageStyle );
            uno::Reference< style::XStyle >  xPStyle;
            aPStyle >>= xPStyle;
            uno::Reference< beans::XPropertySet >  xPProp(xPStyle, uno::UNO_QUERY);
            uno::Any aSize = xPProp->getPropertyValue(UNO_NAME_SIZE);
            awt::Size aPSize;
            aSize >>= aPSize;
            //TODO: set page width to card width
            aPSize.Width = 10000;
            aSize <<= aPSize;
            xPProp->setPropertyValue(UNO_NAME_SIZE, aSize);

            uno::Any aZero; aZero <<= (sal_Int32)0;
            xPProp->setPropertyValue(UNO_NAME_LEFT_MARGIN, aZero);
            xPProp->setPropertyValue(UNO_NAME_RIGHT_MARGIN, aZero);
        }

        uno::Reference< awt::XWindow >  xWin( m_xControl, uno::UNO_QUERY );
        Size aWinSize(m_aTopWindow->GetOutputSizePixel());
        xWin->setPosSize( 0, 0, aWinSize.Width(), aWinSize.Height(), awt::PosSize::SIZE );

        // can only be done here - the SFX changes the ScrollBar values
        disableScrollBars(xViewProps, (m_nStyleFlags&EX_SHOW_ONLINE_LAYOUT) != 0);

        m_aInitializedLink.Call(*this);

        uno::Reference< text::XTextViewCursorSupplier >  xCursorSupp(m_xController, uno::UNO_QUERY);
        uno::Reference< view::XScreenCursor >  xScrCursor(xCursorSupp->getViewCursor(), uno::UNO_QUERY);
        if(xScrCursor.is())
            xScrCursor->screenUp();

        xWin->setVisible( true );
        m_aTopWindow->Show();

        if( xTunnel.is() )
        {
            OTextCursorHelper* pCursor = reinterpret_cast<OTextCursorHelper*>( xTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() ));
            if( pCursor )
            {
                SwEditShell* pSh = pCursor->GetDoc()->GetEditShell();
                if( pSh->ActionCount() )
                {
                    pSh->EndAllAction();
                    pSh->UnlockPaint();
                }
            }
        }

        SW_MOD()->SetView(m_pModuleView);
    }
    else
        pTimer->Start();
}

void SwOneExampleFrame::ClearDocument()
{
    uno::Reference< lang::XUnoTunnel> xTunnel( m_xCursor, uno::UNO_QUERY);
    if( xTunnel.is() )
    {
        OTextCursorHelper* pCursor = reinterpret_cast<OTextCursorHelper*>(xTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId()) );
        if( pCursor )
        {
            SwDoc* pDoc = pCursor->GetDoc();
            SwEditShell* pSh = pDoc->GetEditShell();
            pSh->LockPaint();
            pSh->StartAllAction();
            pSh->KillPams();
            pSh->ClearMark();
            pDoc->ClearDoc();
            pSh->ClearUpCursors();

            if( m_aLoadedIdle.IsActive())
            {
                pSh->EndAllAction();
                pSh->UnlockPaint();
            }
            m_aLoadedIdle.Start();
        }
        else
        {
            m_xCursor->gotoStart(false);
            m_xCursor->gotoEnd(true);
            m_xCursor->setString(OUString());
        }
    }
}

static const sal_Int16 nZoomValues[] =
{
    20,
    40,
    50,
    75,
    100
};

#define ITEM_UP     100
#define ITEM_DOWN   200
#define ITEM_ZOOM   300

void SwOneExampleFrame::CreatePopup(const Point& rPt)
{
    ScopedVclPtrInstance<PopupMenu> aPop;

    aPop->InsertItem(ITEM_UP,   m_aMenuRes.GetString(m_aMenuRes.FindIndex(ST_MENU_UP)));
    aPop->InsertItem(ITEM_DOWN, m_aMenuRes.GetString(m_aMenuRes.FindIndex(ST_MENU_DOWN)));

    Link<Menu*,bool> aSelLk = LINK(this, SwOneExampleFrame, PopupHdl );
    aPop->SetSelectHdl(aSelLk);
    if(EX_SHOW_ONLINE_LAYOUT == m_nStyleFlags)
    {
        aPop->InsertItem(ITEM_ZOOM, m_aMenuRes.GetString(m_aMenuRes.FindIndex(ST_MENU_ZOOM)));

        uno::Reference< view::XViewSettingsSupplier >  xSettings(m_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        uno::Any aZoom = xViewProps->getPropertyValue(UNO_NAME_ZOOM_VALUE);
        sal_Int16 nZoom = 0;
        aZoom >>= nZoom;

        VclPtrInstance<PopupMenu> aSubPop1;
        for (sal_uInt16 i = 0; i < SAL_N_ELEMENTS(nZoomValues); ++i)
        {
            OUString sTemp = unicode::formatPercent(nZoomValues[i],
                Application::GetSettings().GetUILanguageTag());
            aSubPop1->InsertItem( ITEM_ZOOM + i + 1, sTemp);
            if(nZoom == nZoomValues[i])
                aSubPop1->CheckItem(ITEM_ZOOM + i + 1);
        }
        aPop->SetPopupMenu( ITEM_ZOOM, aSubPop1.get() );
        aSubPop1->SetSelectHdl(aSelLk);
    }
    aPop->Execute( m_aTopWindow.get(), rPt );
}

IMPL_LINK(SwOneExampleFrame, PopupHdl, Menu*, pMenu, bool )
{
    sal_uInt16 nId = pMenu->GetCurItemId();
    if ((nId > ITEM_ZOOM) &&
        (nId <= (ITEM_ZOOM + SAL_N_ELEMENTS(nZoomValues))))
    {
        sal_Int16 nZoom = nZoomValues[nId - ITEM_ZOOM - 1];
        uno::Reference< view::XViewSettingsSupplier >  xSettings(m_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        uno::Any aZoom;
        aZoom <<= nZoom;
        xViewProps->setPropertyValue(UNO_NAME_ZOOM_VALUE, aZoom);
        aZoom <<= (sal_Int16)view::DocumentZoomType::BY_VALUE;
        xViewProps->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);
    }
    else if(ITEM_UP == nId || ITEM_DOWN == nId)
    {
        uno::Reference< text::XTextViewCursorSupplier >  xCursorSupp(m_xController, uno::UNO_QUERY);
        uno::Reference< view::XScreenCursor >  xScrCursor(xCursorSupp->getViewCursor(), uno::UNO_QUERY);
        if(ITEM_UP == nId)
            xScrCursor->screenUp();
        else
            xScrCursor->screenDown();
    }
    return false;
};

SwFrameCtrlWindow::SwFrameCtrlWindow(vcl::Window* pParent, SwOneExampleFrame* pFrame)
    : VclEventBox(pParent)
    , pExampleFrame(pFrame)
{
    set_expand(true);
    set_fill(true);
}

void SwFrameCtrlWindow::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {
        case CommandEventId::ContextMenu:
        {
            //#125881# quickly clicking crashes because the control is not fully initialized
            if(pExampleFrame->GetController().is())
                pExampleFrame->CreatePopup(rCEvt.GetMousePosPixel());
        }
        break;
        case CommandEventId::Wheel:
        case CommandEventId::StartAutoScroll:
        case CommandEventId::AutoScroll:
        break;
        default:;
    }
}

Size SwFrameCtrlWindow::GetOptimalSize() const
{
    return LogicToPixel(Size(82, 124), MapMode(MapUnit::MapAppFont));
}

void SwFrameCtrlWindow::Resize()
{
    VclEventBox::Resize();
    pExampleFrame->ClearDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
