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
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
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
#include <com/sun/star/frame/FrameControl.hpp>
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

SwOneExampleFrame::SwOneExampleFrame( Window& rWin,
                                        sal_uInt32 nFlags,
                                        const Link* pInitializedLink,
                                        String* pURL ) :
    aTopWindow(&rWin, this),
    aMenuRes(SW_RES(RES_FRMEX_MENU)),
    pModuleView(SW_MOD()->GetView()),
    nStyleFlags(nFlags),
    bIsInitialized(sal_False),
    bServiceAvailable(sal_False)
{
    if (pURL && pURL->Len())
        sArgumentURL = *pURL;

    aTopWindow.SetPosSizePixel(Point(0, 0), rWin.GetSizePixel());

    if( pInitializedLink )
        aInitializedLink = *pInitializedLink;

    // the controller is asynchronously set
    aLoadedTimer.SetTimeoutHdl(LINK(this, SwOneExampleFrame, TimeoutHdl));
    aLoadedTimer.SetTimeout(200);

    CreateControl();

    aTopWindow.Show();
}

void SwOneExampleFrame::CreateErrorMessage(Window* pParent)
{
    if(SwOneExampleFrame::bShowServiceNotAvailableMessage)
    {
        String sInfo(SW_RES(STR_SERVICE_UNAVAILABLE));
        sInfo += OUString(cFrameControl);
        InfoBox(pParent, sInfo).Execute();
        SwOneExampleFrame::bShowServiceNotAvailableMessage = false;
    }
}

SwOneExampleFrame::~SwOneExampleFrame()
{
    DisposeControl();
}

void SwOneExampleFrame::CreateControl()
{
    if(m_xFrameControl.is())
        return ;
    uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = comphelper::getProcessServiceFactory();
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    try
    {
        m_xFrameControl = frame::FrameControl::create(xContext);
    }
    catch ( css::uno::DeploymentException& )
    {
        return;
    }

    uno::Reference< awt::XWindowPeer >  xParent( aTopWindow.GetComponentInterface() );

    uno::Reference< awt::XToolkit >  xToolkit( awt::Toolkit::create(xContext), uno::UNO_QUERY_THROW );

    m_xFrameControl->createPeer( xToolkit, xParent );

    m_xFrameControl->setVisible(sal_False);
    Size aWinSize(aTopWindow.GetOutputSizePixel());
    m_xFrameControl->setPosSize( 0, 0, aWinSize.Width(), aWinSize.Height(), awt::PosSize::SIZE );

    // create new doc
    OUString sTempURL(cFactory);
    if(sArgumentURL.Len())
        sTempURL = sArgumentURL;

    uno::Sequence<beans::PropertyValue> aSeq(3);
    beans::PropertyValue* pValues = aSeq.getArray();
    pValues[0].Name = "ReadOnly";
    sal_Bool bTrue = sal_True;
    pValues[0].Value.setValue(&bTrue, ::getBooleanCppuType());
    pValues[1].Name = "OpenFlags";
    pValues[1].Value <<= OUString("-RB");
    pValues[2].Name = "Referer";
    pValues[2].Value <<= OUString("private:user");

    m_xFrameControl->setLoaderArguments( aSeq );
    //save and set readonly???

    m_xFrameControl->setComponentURL( sTempURL );

    aLoadedTimer.Start();
    bServiceAvailable = sal_True;
}

void    SwOneExampleFrame::DisposeControl()
{
    _xCursor = 0;
    m_xFrameControl.clear();
    _xModel = 0;
    _xController = 0;
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
    //
    //To reproduce this problem, in edit->autotext and click through
    //the examples and see if the preview gets a horizontal scrollbar
    uno::Any aFalseSet(uno::makeAny(sal_False));
    xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_ONLINE_LAYOUT)), aFalseSet);

    xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_HORI_SCROLL_BAR )), aFalseSet);
    xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_VERT_SCROLL_BAR )), aFalseSet);

    if (bEnableOnlineMode)
    {
        xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_ONLINE_LAYOUT)), uno::makeAny(sal_True));
    }
}

IMPL_LINK( SwOneExampleFrame, TimeoutHdl, Timer*, pTimer )
{
    if(!m_xFrameControl.is())
        return 0;

    // now get the model
    uno::Reference< frame::XFrame >  xFrm = m_xFrameControl->getFrame();

    uno::Reference< beans::XPropertySet > xPropSet( xFrm, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            uno::Reference< frame::XLayoutManager > xLayoutManager;
            uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= xLayoutManager;
            if ( xLayoutManager.is() )
                xLayoutManager->setVisible( sal_False );
        }
        catch (const uno::Exception&)
        {
        }
    }

    _xController = xFrm->getController();
    if(_xController.is())
    {
        _xModel = _xController->getModel();
        //now the ViewOptions should be set properly
        uno::Reference< view::XViewSettingsSupplier >  xSettings(_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        sal_Bool bTrue = sal_True;
        sal_Bool bFalse = sal_False;
        uno::Any aTrueSet( &bTrue, ::getBooleanCppuType() );
        uno::Any aFalseSet( &bFalse, ::getBooleanCppuType() );

        if( !bIsInitialized )
        {
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_BREAKS)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_DRAWINGS)), aTrueSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_FIELD_COMMANDS)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_GRAPHICS)), aTrueSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_HIDDEN_PARAGRAPHS)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_HIDDEN_TEXT)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_HORI_RULER)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_PARA_BREAKS)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_PROTECTED_SPACES)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_SOFT_HYPHENS)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_SPACES)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_TABLES)), aTrueSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_TABSTOPS)), aFalseSet);
            xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SHOW_VERT_RULER)), aFalseSet);

            if(0 ==(nStyleFlags&EX_SHOW_ONLINE_LAYOUT))
            {
                uno::Any aZoom;
                aZoom <<= (sal_Int16)view::DocumentZoomType::PAGE_WIDTH_EXACT;
                xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_ZOOM_TYPE)), aZoom);
            }
            else
            {
                uno::Any aZoom;
                aZoom <<= (sal_Int16)view::DocumentZoomType::BY_VALUE;
                xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_ZOOM_TYPE)), aZoom);

                sal_Int16 nZoomValue = 50;
                if(EX_SHOW_BUSINESS_CARDS == nStyleFlags)
                {
                    nZoomValue = 80;
                }
                aZoom <<= nZoomValue;
                xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_ZOOM_VALUE)), aZoom);
            }

            // set onlinelayout property after setting the zoom
            disableScrollBars(xViewProps, nStyleFlags&EX_SHOW_ONLINE_LAYOUT);
            bIsInitialized = sal_True;
        }

        uno::Reference< text::XTextDocument >  xDoc(_xModel, uno::UNO_QUERY);
        uno::Reference< text::XText >  xText = xDoc->getText();
        _xCursor = xText->createTextCursor();

        //From here, a cursor is defined, which goes through the template,
        //and overwrites the template words where it is necessary.

        uno::Reference< lang::XUnoTunnel> xTunnel( _xCursor, uno::UNO_QUERY);
        if( xTunnel.is() )
        {
            OTextCursorHelper* pCrsr = reinterpret_cast<OTextCursorHelper*>( xTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() ));
            if( pCrsr )
            {
                SwEditShell* pSh = pCrsr->GetDoc()->GetEditShell();

                do
                {
                  if (pSh->GetCurWord() == String("HEADING1"))
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_HEADING1));
                  }
                  else if (pSh->GetCurWord() == String("ENTRY1"))
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_ENTRY1));
                  }
                  else if (pSh->GetCurWord() == String("HEADING11"))
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_HEADING11));
                  }
                  else if (pSh->GetCurWord() == String("ENTRY11"))
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_ENTRY11));
                  }
                  else if (pSh->GetCurWord() == String("HEADING12"))
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_HEADING12));
                  }
                  else if (pSh->GetCurWord() == String("ENTRY12"))
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_ENTRY12));
                  }
                  else if (pSh->GetCurWord() == String("TABLE1"))
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_TABLE1));
                  }
                  else if (pSh->GetCurWord() == String("IMAGE1"))
                  {
                    pSh->Overwrite(SW_RESSTR(STR_IDXEXAMPLE_IDXTXT_IMAGE1));
                  }
                  else
                  {;}
                }
                while(pSh->Right(sal_uInt16(1), sal_uInt16(1), sal_True) == sal_True);
            }
        }

        uno::Reference< beans::XPropertySet >  xCrsrProp(_xCursor, uno::UNO_QUERY);
        uno::Any aPageStyle = xCrsrProp->getPropertyValue(
                                            OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_PAGE_STYLE_NAME)));
        OUString sPageStyle;
        aPageStyle >>= sPageStyle;

        uno::Reference< style::XStyleFamiliesSupplier >  xSSupp( xDoc, uno::UNO_QUERY);
        uno::Reference< container::XNameAccess >  xStyles = xSSupp->getStyleFamilies();
        uno::Any aPFamily = xStyles->getByName( "PageStyles" );
        uno::Reference< container::XNameContainer >  xPFamily;

        if( 0 == (EX_SHOW_DEFAULT_PAGE == nStyleFlags)
                && (aPFamily >>= xPFamily) && !sPageStyle.isEmpty() )
        {
            uno::Any aPStyle = xPFamily->getByName( sPageStyle );
            uno::Reference< style::XStyle >  xPStyle;
            aPStyle >>= xPStyle;
            uno::Reference< beans::XPropertySet >  xPProp(xPStyle, uno::UNO_QUERY);
            uno::Any aSize = xPProp->getPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SIZE)));
            awt::Size aPSize;
            aSize >>= aPSize;
            //TODO: set page width to card width
            aPSize.Width = 10000;
            aSize.setValue(&aPSize, ::getCppuType((awt::Size*)0));
            xPProp->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SIZE)), aSize);

            uno::Any aZero; aZero <<= (sal_Int32)0;
            xPProp->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LEFT_MARGIN)), aZero);
            xPProp->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_RIGHT_MARGIN)), aZero);
        }

        Size aWinSize(aTopWindow.GetOutputSizePixel());
        m_xFrameControl->setPosSize( 0, 0, aWinSize.Width(), aWinSize.Height(), awt::PosSize::SIZE );

        // can only be done here - the SFX changes the ScrollBar values
        disableScrollBars(xViewProps, nStyleFlags&EX_SHOW_ONLINE_LAYOUT);

        if (aInitializedLink.IsSet())
            aInitializedLink.Call(this);

        uno::Reference< text::XTextViewCursorSupplier >  xCrsrSupp(_xController, uno::UNO_QUERY);
        uno::Reference< view::XScreenCursor >  xScrCrsr(xCrsrSupp->getViewCursor(), uno::UNO_QUERY);
        if(xScrCrsr.is())
            xScrCrsr->screenUp();

        m_xFrameControl->setVisible( sal_True );
        aTopWindow.Show();

        if( xTunnel.is() )
        {
            OTextCursorHelper* pCrsr = reinterpret_cast<OTextCursorHelper*>( xTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() ));
            if( pCrsr )
            {
                SwEditShell* pSh = pCrsr->GetDoc()->GetEditShell();
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
    return 0;
}

void SwOneExampleFrame::ClearDocument( sal_Bool bStartUpdateTimer )
{
    uno::Reference< lang::XUnoTunnel> xTunnel( _xCursor, uno::UNO_QUERY);
    if( xTunnel.is() )
    {
        OTextCursorHelper* pCrsr = reinterpret_cast<OTextCursorHelper*>(xTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId()) );
        if( pCrsr )
        {
            SwDoc* pDoc = pCrsr->GetDoc();
            SwEditShell* pSh = pDoc->GetEditShell();
            pSh->LockPaint();
            pSh->StartAllAction();
            pDoc->ClearDoc();

            if( aLoadedTimer.IsActive() || !bStartUpdateTimer )
            {
                pSh->EndAllAction();
                pSh->UnlockPaint();
            }
            if( bStartUpdateTimer )
                aLoadedTimer.Start();
        }
        else
        {
            _xCursor->gotoStart(sal_False);
            _xCursor->gotoEnd(sal_True);
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

    Link aSelLk = LINK(this, SwOneExampleFrame, PopupHdl );
    aPop.SetSelectHdl(aSelLk);
    if(EX_SHOW_ONLINE_LAYOUT == nStyleFlags)
    {
        aPop.InsertItem(ITEM_ZOOM, rArr.GetString(rArr.FindIndex(ST_MENU_ZOOM   )));

        uno::Reference< view::XViewSettingsSupplier >  xSettings(_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        uno::Any aZoom = xViewProps->getPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_ZOOM_VALUE)));
        sal_Int16 nZoom = 0;
        aZoom >>= nZoom;

        for (sal_uInt16 i = 0;
                i < (sizeof(nZoomValues)/sizeof(nZoomValues[0])); ++i)
        {
            String sTemp;
            sTemp = OUString::number(nZoomValues[i]);
            sTemp += OUString(" %");
            aSubPop1.InsertItem( ITEM_ZOOM + i + 1, sTemp);
            if(nZoom == nZoomValues[i])
                aSubPop1.CheckItem(ITEM_ZOOM + i + 1);
        }
        aPop.SetPopupMenu( ITEM_ZOOM, &aSubPop1 );
        aSubPop1.SetSelectHdl(aSelLk);
    }
    aPop.Execute( &aTopWindow, rPt );

}

IMPL_LINK(SwOneExampleFrame, PopupHdl, Menu*, pMenu )
{
    sal_uInt16 nId = pMenu->GetCurItemId();
    if ((nId > ITEM_ZOOM) &&
        (nId <= (ITEM_ZOOM + (sizeof(nZoomValues)/sizeof(nZoomValues[0])))))
    {
        sal_Int16 nZoom = nZoomValues[nId - ITEM_ZOOM - 1];
        uno::Reference< view::XViewSettingsSupplier >  xSettings(_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        uno::Any aZoom;
        aZoom <<= nZoom;
        xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_ZOOM_VALUE)), aZoom);
        aZoom <<= (sal_Int16)view::DocumentZoomType::BY_VALUE;
        xViewProps->setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_ZOOM_TYPE)), aZoom);
    }
    else if(ITEM_UP == nId || ITEM_DOWN == nId)
    {
        uno::Reference< text::XTextViewCursorSupplier >  xCrsrSupp(_xController, uno::UNO_QUERY);
        uno::Reference< view::XScreenCursor >  xScrCrsr(xCrsrSupp->getViewCursor(), uno::UNO_QUERY);
        if(ITEM_UP == nId)
            xScrCrsr->screenUp();
        else
            xScrCrsr->screenDown();
    }
    return 0;
};

SwFrmCtrlWindow::SwFrmCtrlWindow(Window* pParent, SwOneExampleFrame* pFrame)
    : VclEventBox(pParent)
    , pExampleFrame(pFrame)
{
    set_expand(true);
    set_fill(true);
}

void SwFrmCtrlWindow::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            //#125881# quickly clicking crashes because the control is not fully initialized
            if(pExampleFrame->GetController().is())
                pExampleFrame->CreatePopup(rCEvt.GetMousePosPixel());
        }
        break;
        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        break;
        default:;
    }
}

Size SwFrmCtrlWindow::GetOptimalSize() const
{
    return LogicToPixel(Size(82, 124), MapMode(MAP_APPFONT));
}

void SwFrmCtrlWindow::Resize()
{
    VclEventBox::Resize();
    pExampleFrame->ClearDocument(true);
}

MenuResource::MenuResource(const ResId& rResId) :
    Resource(rResId),
    aMenuArray(ResId(1,*rResId.GetResMgr()))
{
    FreeResource();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
