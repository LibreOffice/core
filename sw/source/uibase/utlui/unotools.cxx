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
#include <osl/diagnose.h>
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

const sal_Char cFrameControl[] = "com.sun.star.frame.FrameControl";
const sal_Char cFactory[] = "private:factory/swriter";

bool SwOneExampleFrame::bShowServiceNotAvailableMessage = true;

SwOneExampleFrame::SwOneExampleFrame( vcl::Window& rWin,
                                        sal_uInt32 nFlags,
                                        const Link<SwOneExampleFrame&,void>* pInitializedLink,
                                        const OUString* pURL ) :
    aTopWindow(VclPtr<SwFrameCtrlWindow>::Create(&rWin, this)),
    aLoadedIdle("sw uibase SwOneExampleFrame Loaded"),
    aMenuRes(SW_RES(RES_FRMEX_MENU)),
    pModuleView(SW_MOD()->GetView()),
    nStyleFlags(nFlags),
    bIsInitialized(false),
    bServiceAvailable(false)
{
    if (pURL && !pURL->isEmpty())
        sArgumentURL = *pURL;

    aTopWindow->SetPosSizePixel(Point(0, 0), rWin.GetSizePixel());

    if( pInitializedLink )
        aInitializedLink = *pInitializedLink;

    // the controller is asynchronously set
    aLoadedIdle.SetIdleHdl(LINK(this, SwOneExampleFrame, TimeoutHdl));
    aLoadedIdle.SetPriority(SchedulerPriority::HIGH);

    CreateControl();

    aTopWindow->Show();
}

void SwOneExampleFrame::CreateErrorMessage()
{
    if(SwOneExampleFrame::bShowServiceNotAvailableMessage)
    {
        OUString sInfo(SW_RES(STR_SERVICE_UNAVAILABLE));
        sInfo += cFrameControl;
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
    if(_xControl.is())
        return ;
    uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = comphelper::getProcessServiceFactory();
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    uno::Reference< uno::XInterface >  xInst = xMgr->createInstance( "com.sun.star.frame.FrameControl" );
    _xControl.set(xInst, uno::UNO_QUERY);
    if(_xControl.is())
    {
        uno::Reference< awt::XWindowPeer >  xParent( aTopWindow->GetComponentInterface() );

        uno::Reference< awt::XToolkit >  xToolkit( awt::Toolkit::create(xContext), uno::UNO_QUERY_THROW );

        _xControl->createPeer( xToolkit, xParent );

        uno::Reference< awt::XWindow >  xWin( _xControl, uno::UNO_QUERY );
        xWin->setVisible(false);
        Size aWinSize(aTopWindow->GetOutputSizePixel());
        xWin->setPosSize( 0, 0, aWinSize.Width(), aWinSize.Height(), awt::PosSize::SIZE );

        uno::Reference< beans::XPropertySet >  xPrSet(xInst, uno::UNO_QUERY);
        uno::Any aURL;
        // create new doc
        OUString sTempURL(cFactory);
        if(!sArgumentURL.isEmpty())
            sTempURL = sArgumentURL;
        aURL <<= sTempURL;

        uno::Sequence<beans::PropertyValue> aSeq(2);
        beans::PropertyValue* pValues = aSeq.getArray();

        pValues[0].Name = "OpenFlags";
        pValues[0].Value <<= OUString("-RB");
        pValues[1].Name = "Referer";
        pValues[1].Value <<= OUString("private:user");
        uno::Any aArgs;
        aArgs.setValue(&aSeq, cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get());

        xPrSet->setPropertyValue( "LoaderArguments", aArgs );
        //save and set readonly???

        xPrSet->setPropertyValue("ComponentURL", aURL);

        aLoadedIdle.Start();
        bServiceAvailable = true;
    }
}

void    SwOneExampleFrame::DisposeControl()
{
    aLoadedIdle.Stop();
    aTopWindow.clear();
    _xCursor = nullptr;
    if(_xControl.is())
        _xControl->dispose();
    _xControl = nullptr;
    _xModel = nullptr;
    _xController = nullptr;
}

static void disableScrollBars(uno::Reference< beans::XPropertySet > xViewProps,
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

IMPL_LINK_TYPED( SwOneExampleFrame, TimeoutHdl, Idle*, pTimer, void )
{
    if(!_xControl.is())
        return;

    // now get the model
    uno::Reference< beans::XPropertySet >  xPrSet(_xControl, uno::UNO_QUERY);
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

    _xController = xFrame->getController();
    if(_xController.is())
    {
        _xModel = _xController->getModel();
        //now the ViewOptions should be set properly
        uno::Reference< view::XViewSettingsSupplier >  xSettings(_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        const uno::Any aTrueSet( true );
        const uno::Any aFalseSet( false );

        if( !bIsInitialized )
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

            if(0 ==(nStyleFlags&EX_SHOW_ONLINE_LAYOUT))
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
                if(EX_SHOW_BUSINESS_CARDS == nStyleFlags)
                {
                    nZoomValue = 80;
                }
                aZoom <<= nZoomValue;
                xViewProps->setPropertyValue(UNO_NAME_ZOOM_VALUE, aZoom);
            }

            // set onlinelayout property after setting the zoom
            disableScrollBars(xViewProps, (nStyleFlags&EX_SHOW_ONLINE_LAYOUT) != 0);
            bIsInitialized = true;
        }

        uno::Reference< text::XTextDocument >  xDoc(_xModel, uno::UNO_QUERY);
        uno::Reference< text::XText >  xText = xDoc->getText();
        _xCursor = xText->createTextCursor();

        //From here, a cursor is defined, which goes through the template,
        //and overwrites the template words where it is necessary.

        uno::Reference< lang::XUnoTunnel> xTunnel( _xCursor, uno::UNO_QUERY);
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
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_HEADING1));
                  }
                  else if (pSh->GetCurWord() == "ENTRY1")
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_ENTRY1));
                  }
                  else if (pSh->GetCurWord() == "HEADING11")
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_HEADING11));
                  }
                  else if (pSh->GetCurWord() == "ENTRY11")
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_ENTRY11));
                  }
                  else if (pSh->GetCurWord() == "HEADING12")
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_HEADING12));
                  }
                  else if (pSh->GetCurWord() == "ENTRY12")
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_ENTRY12));
                  }
                  else if (pSh->GetCurWord() == "TABLE1")
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_TABLE1));
                  }
                  else if (pSh->GetCurWord() == "IMAGE1")
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_IMAGE1));
                  }
                  else
                  {;}
                }
                while(pSh->Right(sal_uInt16(1), sal_uInt16(1), true));
            }
        }

        uno::Reference< beans::XPropertySet >  xCursorProp(_xCursor, uno::UNO_QUERY);
        uno::Any aPageStyle = xCursorProp->getPropertyValue(UNO_NAME_PAGE_STYLE_NAME);
        OUString sPageStyle;
        aPageStyle >>= sPageStyle;

        uno::Reference< style::XStyleFamiliesSupplier >  xSSupp( xDoc, uno::UNO_QUERY);
        uno::Reference< container::XNameAccess >  xStyles = xSSupp->getStyleFamilies();
        uno::Any aPFamily = xStyles->getByName( "PageStyles" );
        uno::Reference< container::XNameContainer >  xPFamily;

        if( EX_SHOW_DEFAULT_PAGE != nStyleFlags
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
            aSize.setValue(&aPSize, ::cppu::UnoType<awt::Size>::get());
            xPProp->setPropertyValue(UNO_NAME_SIZE, aSize);

            uno::Any aZero; aZero <<= (sal_Int32)0;
            xPProp->setPropertyValue(UNO_NAME_LEFT_MARGIN, aZero);
            xPProp->setPropertyValue(UNO_NAME_RIGHT_MARGIN, aZero);
        }

        uno::Reference< awt::XWindow >  xWin( _xControl, uno::UNO_QUERY );
        Size aWinSize(aTopWindow->GetOutputSizePixel());
        xWin->setPosSize( 0, 0, aWinSize.Width(), aWinSize.Height(), awt::PosSize::SIZE );

        // can only be done here - the SFX changes the ScrollBar values
        disableScrollBars(xViewProps, (nStyleFlags&EX_SHOW_ONLINE_LAYOUT) != 0);

        aInitializedLink.Call(*this);

        uno::Reference< text::XTextViewCursorSupplier >  xCursorSupp(_xController, uno::UNO_QUERY);
        uno::Reference< view::XScreenCursor >  xScrCursor(xCursorSupp->getViewCursor(), uno::UNO_QUERY);
        if(xScrCursor.is())
            xScrCursor->screenUp();

        xWin->setVisible( true );
        aTopWindow->Show();

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

        SW_MOD()->SetView(pModuleView);
    }
    else
        pTimer->Start();
}

void SwOneExampleFrame::ClearDocument()
{
    uno::Reference< lang::XUnoTunnel> xTunnel( _xCursor, uno::UNO_QUERY);
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

            if( aLoadedIdle.IsActive())
            {
                pSh->EndAllAction();
                pSh->UnlockPaint();
            }
            aLoadedIdle.Start();
        }
        else
        {
            _xCursor->gotoStart(false);
            _xCursor->gotoEnd(true);
            _xCursor->setString(OUString());
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
    PopupMenu aPop;
    PopupMenu aSubPop1;
    ResStringArray& rArr = aMenuRes.GetMenuArray();

    aPop.InsertItem(ITEM_UP,   rArr.GetString(rArr.FindIndex(ST_MENU_UP )));
    aPop.InsertItem(ITEM_DOWN, rArr.GetString(rArr.FindIndex(ST_MENU_DOWN )));

    Link<Menu*,bool> aSelLk = LINK(this, SwOneExampleFrame, PopupHdl );
    aPop.SetSelectHdl(aSelLk);
    if(EX_SHOW_ONLINE_LAYOUT == nStyleFlags)
    {
        aPop.InsertItem(ITEM_ZOOM, rArr.GetString(rArr.FindIndex(ST_MENU_ZOOM   )));

        uno::Reference< view::XViewSettingsSupplier >  xSettings(_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        uno::Any aZoom = xViewProps->getPropertyValue(UNO_NAME_ZOOM_VALUE);
        sal_Int16 nZoom = 0;
        aZoom >>= nZoom;

        for (sal_uInt16 i = 0; i < SAL_N_ELEMENTS(nZoomValues); ++i)
        {
            OUString sTemp = unicode::formatPercent(nZoomValues[i],
                Application::GetSettings().GetUILanguageTag());
            aSubPop1.InsertItem( ITEM_ZOOM + i + 1, sTemp);
            if(nZoom == nZoomValues[i])
                aSubPop1.CheckItem(ITEM_ZOOM + i + 1);
        }
        aPop.SetPopupMenu( ITEM_ZOOM, &aSubPop1 );
        aSubPop1.SetSelectHdl(aSelLk);
    }
    aPop.Execute( aTopWindow.get(), rPt );

}

IMPL_LINK_TYPED(SwOneExampleFrame, PopupHdl, Menu*, pMenu, bool )
{
    sal_uInt16 nId = pMenu->GetCurItemId();
    if ((nId > ITEM_ZOOM) &&
        (nId <= (ITEM_ZOOM + SAL_N_ELEMENTS(nZoomValues))))
    {
        sal_Int16 nZoom = nZoomValues[nId - ITEM_ZOOM - 1];
        uno::Reference< view::XViewSettingsSupplier >  xSettings(_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        uno::Any aZoom;
        aZoom <<= nZoom;
        xViewProps->setPropertyValue(UNO_NAME_ZOOM_VALUE, aZoom);
        aZoom <<= (sal_Int16)view::DocumentZoomType::BY_VALUE;
        xViewProps->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);
    }
    else if(ITEM_UP == nId || ITEM_DOWN == nId)
    {
        uno::Reference< text::XTextViewCursorSupplier >  xCursorSupp(_xController, uno::UNO_QUERY);
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
    return LogicToPixel(Size(82, 124), MapMode(MAP_APPFONT));
}

void SwFrameCtrlWindow::Resize()
{
    VclEventBox::Resize();
    pExampleFrame->ClearDocument();
}

MenuResource::MenuResource(const ResId& rResId) :
    Resource(rResId),
    aMenuArray(ResId(1,*rResId.GetResMgr()))
{
    FreeResource();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
