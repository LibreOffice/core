/*************************************************************************
 *
 *  $RCSfile: unotools.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 15:44:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <swtypes.hxx>
#include <globals.hrc>
#include <misc.hrc>

#include <utlui.hrc>
#include <unotools.hrc>
#ifndef _UNOTOOLS_HXX
#include <unotools.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTVIEWCURSORSUPPLIER_HPP_
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSCREENCURSOR_HPP_
#include <com/sun/star/view/XScreenCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_DOCUMENTZOOMTYPE_HPP_
#include <com/sun/star/view/DocumentZoomType.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XVIEWSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen wg. SfxDispatcher
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen wg. SfxStringItem
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen wg. SFX_APP
#include <sfx2/app.hxx>
#endif
#include <shellio.hxx>
#ifndef _SWDOCSH_HXX //autogen wg. SwDocShell
#include <docsh.hxx>
#endif
#ifndef _SWVIEW_HXX //autogen wg. SwView
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX //autogen wg. SwWrtShell
#include <wrtsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif


#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

using namespace ::com::sun::star;
using namespace ::rtl;

const sal_Char cFrameControl[] = "com.sun.star.frame.FrameControl";
const sal_Char cFactory[] = "private:factory/swriter";
/* -----------------09.06.99 14:39-------------------
 *
 * --------------------------------------------------*/
SwRenameXNamedDlg::SwRenameXNamedDlg( Window* pWin,
            uno::Reference< container::XNamed > & xN,
            uno::Reference< container::XNameAccess > & xNA ) :
    ModalDialog(pWin, SW_RES(DLG_RENAME_XNAMED)),
   xNamed(xN),
   xNameAccess(xNA),
   aNewNameFT(this, ResId(FT_NEW_NAME)),
   aNewNameED(this, ResId(ED_NEW_NAME)),
   aNameFL(this, ResId(FL_NAME)),
   aOk(this, ResId(PB_OK)),
   aCancel(this, ResId(PB_CANCEL)),
   aHelp(this, ResId(PB_HELP))
{
    FreeResource();
    sRemoveWarning = String(SW_RES(STR_REMOVE_WARNING));

    String sTmp(GetText());
    aNewNameED.SetText(xNamed->getName());
    sTmp += String(xNamed->getName());
    SetText(sTmp);

    aOk.SetClickHdl(LINK(this, SwRenameXNamedDlg, OkHdl));
    aNewNameED.SetModifyHdl(LINK(this, SwRenameXNamedDlg, ModifyHdl));
    aOk.Enable(sal_False);
}
/* -----------------09.06.99 15:34-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(SwRenameXNamedDlg, OkHdl, OKButton*, pOk)
{
    try
    {
        xNamed->setName(aNewNameED.GetText());
    }
    catch(uno::RuntimeException&)
    {
        DBG_ERROR("Name wurde nicht geaendert")
    }
    EndDialog(RET_OK);
    return 0;
}
/* -----------------09.06.99 15:48-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(SwRenameXNamedDlg, ModifyHdl, NoSpaceEdit*, pEdit)
{
    String sTmp(pEdit->GetText());

    // prevent from pasting illegal characters
    sal_uInt16 nLen = sTmp.Len();
    String sMsg;
    for(sal_uInt16 i = 0; i < pEdit->GetForbiddenChars().Len(); i++)
    {
        sal_uInt16 nTmpLen = sTmp.Len();
        sTmp.EraseAllChars(pEdit->GetForbiddenChars().GetChar(i));
        if(sTmp.Len() != nTmpLen)
            sMsg += pEdit->GetForbiddenChars().GetChar(i);
    }
    if(sTmp.Len() != nLen)
    {
        pEdit->SetText(sTmp);
        String sWarning(sRemoveWarning);
        sWarning += sMsg;
        InfoBox(this, sWarning).Execute();
    }

    aOk.Enable(sTmp.Len() && !xNameAccess->hasByName(sTmp)
    && (!xSecondAccess.is() || !xSecondAccess->hasByName(sTmp))
    && (!xThirdAccess.is() || !xThirdAccess->hasByName(sTmp))
    );
    return 0;
}

/************************************************************************

************************************************************************/

sal_Bool SwOneExampleFrame::bShowServiceNotAvailableMessage = sal_True;

/* -----------------27.07.99 15:26-------------------

 --------------------------------------------------*/
SwOneExampleFrame::SwOneExampleFrame( Window& rWin,
                                        sal_uInt32 nFlags,
                                        const Link* pInitializedLink,
                                        String* pURL ) :
    aTopWindow( rWin.GetParent(), 0, this ),
    pModuleView(SW_MOD()->GetView()),
    rWindow(rWin),
    aMenuRes(SW_RES(RES_FRMEX_MENU)),
    nStyleFlags(nFlags),
    bIsInitialized(sal_False),
    bServiceAvailable(sal_False)
{
    if(pURL && pURL->Len())
        sArgumentURL = *pURL;

    aTopWindow.SetPaintTransparent(sal_True);
    aTopWindow.SetPosSizePixel(rWin.GetPosPixel(), rWin.GetSizePixel());
    aTopWindow.SetZOrder( &rWin, WINDOW_ZORDER_FIRST );

    if( pInitializedLink )
        aInitializedLink = *pInitializedLink;

    // the controller is asynchronously set
    aLoadedTimer.SetTimeoutHdl(LINK(this, SwOneExampleFrame, TimeoutHdl));
//      aLoadedTimer.SetTimeout(500);
    aLoadedTimer.SetTimeout(200);

    rWin.Enable(sal_False);
    CreateControl();

    aTopWindow.Show();
}

/* -----------------------------08.12.99 13:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwOneExampleFrame::CreateErrorMessage(Window* pParent)
{
    if(SwOneExampleFrame::bShowServiceNotAvailableMessage)
    {
        String sInfo(SW_RES(STR_SERVICE_UNAVAILABLE));
        sInfo += C2S(cFrameControl);
        InfoBox(pParent, sInfo).Execute();
        SwOneExampleFrame::bShowServiceNotAvailableMessage = sal_False;
    }
}
/* -----------------27.07.99 15:26-------------------

 --------------------------------------------------*/
SwOneExampleFrame::~SwOneExampleFrame()
{
    DisposeControl();
}
/* -----------------------------21.12.00 10:16--------------------------------

 ---------------------------------------------------------------------------*/
void    SwOneExampleFrame::CreateControl()
{
    if(_xControl.is())
        return ;
    uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = comphelper::getProcessServiceFactory();
    uno::Reference< uno::XInterface >  xInst = xMgr->createInstance( C2U("com.sun.star.frame.FrameControl") );
    _xControl = uno::Reference< awt::XControl >(xInst, uno::UNO_QUERY);
    if(_xControl.is())
    {
        uno::Reference< awt::XWindowPeer >  xParent( rWindow.GetComponentInterface() );

        uno::Reference< awt::XToolkit >  xToolkit( xMgr->createInstance( C2U("com.sun.star.awt.Toolkit") ), uno::UNO_QUERY );
        if(xToolkit.is())
        {
            _xControl->createPeer( xToolkit, xParent );
            uno::Reference< beans::XPropertySet >  xPrSet(xInst, uno::UNO_QUERY);
            uno::Any aURL;
            //
            // create new doc
            String sTempURL = C2S(cFactory);
            if(sArgumentURL.Len())
                sTempURL = sArgumentURL;
            aURL <<= OUString(sTempURL);

            uno::Sequence<beans::PropertyValue> aSeq(3);
            beans::PropertyValue* pValues = aSeq.getArray();
            pValues[0].Name = C2U("ReadOnly");
            BOOL bTrue = sal_True;
            pValues[0].Value.setValue(&bTrue, ::getBooleanCppuType());
            pValues[1].Name = C2U("OpenFlags");
            pValues[1].Value <<= C2U("-RB");
            pValues[2].Name = C2U("Referer");
            pValues[2].Value <<= C2U("private:user");
            uno::Any aArgs;
            aArgs.setValue(&aSeq, ::getCppuType((uno::Sequence<beans::PropertyValue>*)0));

            xPrSet->setPropertyValue( C2U("LoaderArguments"), aArgs );
            //save and set readonly???

            xPrSet->setPropertyValue(C2U("ComponentURL"), aURL);

            uno::Reference< awt::XWindow >  xWin( _xControl, uno::UNO_QUERY );
            xWin->setVisible( sal_False );
            Size aWinSize(rWindow.GetOutputSizePixel());
            xWin->setPosSize( 0, 0, aWinSize.Width(), aWinSize.Height(), awt::PosSize::SIZE );

            aLoadedTimer.Start();
            bServiceAvailable = sal_True;
        }
    }
}


/* -----------------------------21.12.00 10:16--------------------------------

 ---------------------------------------------------------------------------*/
void    SwOneExampleFrame::DisposeControl()
{
    _xCursor = 0;
    if(_xControl.is())
        _xControl->dispose();
    _xControl = 0;
    _xModel = 0;
    _xController = 0;
}
/* -----------------27.07.99 15:26-------------------

 --------------------------------------------------*/
IMPL_LINK( SwOneExampleFrame, TimeoutHdl, Timer*, pTimer )
{
    if(!_xControl.is())
        return 0;

    // now get the model
    uno::Reference< beans::XPropertySet >  xPrSet(_xControl, uno::UNO_QUERY);
    uno::Any aFrame = xPrSet->getPropertyValue(C2U("Frame"));
    uno::Reference< frame::XFrame >  xFrm;
    aFrame >>= xFrm;
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
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_BREAKS              )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_DRAWINGS             )), aTrueSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_FIELD_COMMANDS       )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_FOOTNOTE_BACKGROUND  )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_GRAPHICS             )), aTrueSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_HIDDEN_PARAGRAPHS    )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_HIDDEN_TEXT          )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_HORI_RULER          )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_INDEX_MARK_BACKGROUND)), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_PARA_BREAKS          )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_PROTECTED_SPACES     )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_SOFT_HYPHENS         )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_SPACES               )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_TABLE_BOUNDARIES     )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_TABLES               )), aTrueSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_TABSTOPS             )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_TEXT_BOUNDARIES      )), aFalseSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_TEXT_FIELD_BACKGROUND)), aTrueSet);
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_VERT_RULER          )), aFalseSet);

            if(0 ==(nStyleFlags&EX_SHOW_ONLINE_LAYOUT))
            {
                uno::Any aZoom;
                aZoom <<= (sal_Int16)view::DocumentZoomType::PAGE_WIDTH_EXACT;
                xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ZOOM_TYPE)), aZoom);
            }
            else
            {
                uno::Any aZoom;
                aZoom <<= (sal_Int16)view::DocumentZoomType::BY_VALUE;
                xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ZOOM_TYPE)), aZoom);

                sal_Int16 nZoomValue = 50;
                if(EX_SHOW_BUSINESS_CARDS == nStyleFlags)
                {
                    nZoomValue = 80;
                }
                aZoom <<= nZoomValue;
                xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ZOOM_VALUE)), aZoom);
            }
            // set onlinelayout property behind setting the zoom
            xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_ONLINE_LAYOUT)),
                    (nStyleFlags&EX_SHOW_ONLINE_LAYOUT) ? aTrueSet : aFalseSet );
            bIsInitialized = sal_True;
        }

        uno::Reference< text::XTextDocument >  xDoc(_xModel, uno::UNO_QUERY);
        uno::Reference< text::XText >  xText = xDoc->getText();
        _xCursor = xText->createTextCursor();
        uno::Reference< beans::XPropertySet >  xCrsrProp(_xCursor, uno::UNO_QUERY);
        uno::Any aPageStyle = xCrsrProp->getPropertyValue(
                                            C2U(SW_PROP_NAME_STR(UNO_NAME_PAGE_STYLE_NAME)));
        OUString sPageStyle;
        aPageStyle >>= sPageStyle;

        uno::Reference< style::XStyleFamiliesSupplier >  xSSupp( xDoc, uno::UNO_QUERY);
        uno::Reference< container::XNameAccess >  xStyles = xSSupp->getStyleFamilies();
        uno::Any aPFamily = xStyles->getByName( C2U("PageStyles" ) );
        uno::Reference< container::XNameContainer >  xPFamily;
        if( (aPFamily >>= xPFamily) && sPageStyle.getLength() )
        {
            uno::Any aPStyle = xPFamily->getByName( sPageStyle );
            uno::Reference< style::XStyle >  xPStyle;
            aPStyle >>= xPStyle;
            uno::Reference< beans::XPropertySet >  xPProp(xPStyle, uno::UNO_QUERY);
            uno::Any aSize = xPProp->getPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SIZE)));
            awt::Size aPSize;
            aSize >>= aPSize;
            //TODO: set page width to card width
            aPSize.Width = 10000;
            aSize.setValue(&aPSize, ::getCppuType((awt::Size*)0));
            xPProp->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SIZE)), aSize);

            uno::Any aZero; aZero <<= (sal_Int32)0;
            xPProp->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_LEFT_MARGIN)), aZero);
            xPProp->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_RIGHT_MARGIN)), aZero);
        }

        // can only be done here - the SFX changes the ScrollBar values
        xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_HORI_SCROLL_BAR )), aFalseSet);
        xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SHOW_VERT_SCROLL_BAR )), aFalseSet);

        if( aInitializedLink.IsSet() )
        {
            rWindow.Enable(sal_False, sal_True);
            //rWindow.Enable(sal_True, sal_False);
               aInitializedLink.Call(this);
        }

        uno::Reference< text::XTextViewCursorSupplier >  xCrsrSupp(_xController, uno::UNO_QUERY);
        uno::Reference< view::XScreenCursor >  xScrCrsr(xCrsrSupp->getViewCursor(), uno::UNO_QUERY);
        if(xScrCrsr.is())
            xScrCrsr->screenUp();

        uno::Reference< awt::XWindow >  xWin( _xControl, uno::UNO_QUERY );
        xWin->setVisible( sal_True );
        rWindow.Show();

        uno::Reference< lang::XUnoTunnel> xTunnel( _xCursor, uno::UNO_QUERY);
        if( xTunnel.is() )
        {
            OTextCursorHelper* pCrsr = (OTextCursorHelper*)xTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() );
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
/* -----------------------------27.12.99 09:59--------------------------------

 ---------------------------------------------------------------------------*/
void SwOneExampleFrame::ClearDocument( BOOL bStartUpdateTimer )
{
    uno::Reference< lang::XUnoTunnel> xTunnel( _xCursor, uno::UNO_QUERY);
    if( xTunnel.is() )
    {
        OTextCursorHelper* pCrsr = (OTextCursorHelper*)xTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() );
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
            _xCursor->gotoStart(FALSE);
            _xCursor->gotoEnd(TRUE);
            _xCursor->setString(OUString());
        }
    }
}
/* -----------------------------15.12.99 11:09--------------------------------

 ---------------------------------------------------------------------------*/
static const sal_Int16 nZoomValues[] =
{
    20,
    40,
    50,
    75,
    100
};
//---------------------------------------------------------------------------
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

        uno::Any aZoom = xViewProps->getPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ZOOM_VALUE)));
        sal_Int16 nZoom;
        aZoom >>= nZoom;

        for(sal_uInt16 i = 0; i < 5; i++ )
        {
            String sTemp;
            sTemp = String::CreateFromInt32(nZoomValues[i]);
            sTemp += String::CreateFromAscii(" %");
            aSubPop1.InsertItem( ITEM_ZOOM + i + 1, sTemp);
            if(nZoom == nZoomValues[i])
                aSubPop1.CheckItem(ITEM_ZOOM + i + 1);
        }
        aPop.SetPopupMenu( ITEM_ZOOM, &aSubPop1 );
        aSubPop1.SetSelectHdl(aSelLk);
    }
    aPop.Execute( &aTopWindow, rPt );

}
/* -----------------------------15.12.99 11:09--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SwOneExampleFrame, PopupHdl, Menu*, pMenu )
{
    sal_uInt16 nId = pMenu->GetCurItemId();
    if( nId > ITEM_ZOOM && nId < ITEM_ZOOM + 100 )
    {
        sal_Int16 nZoom = nZoomValues[nId - ITEM_ZOOM - 1];
        uno::Reference< view::XViewSettingsSupplier >  xSettings(_xController, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet >  xViewProps = xSettings->getViewSettings();

        uno::Any aZoom;
        aZoom <<= nZoom;
        xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ZOOM_VALUE)), aZoom);
        aZoom <<= (sal_Int16)view::DocumentZoomType::BY_VALUE;
        xViewProps->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ZOOM_TYPE)), aZoom);
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
/* -----------------------------15.12.99 10:37--------------------------------

 ---------------------------------------------------------------------------*/
SwFrmCtrlWindow::SwFrmCtrlWindow(Window* pParent, WinBits nBits,
                                SwOneExampleFrame*  pFrame) :
    Window(pParent, nBits),
    pExampleFrame(pFrame)
{
}
/* -----------------------------15.12.99 09:57--------------------------------

 ---------------------------------------------------------------------------*/
void SwFrmCtrlWindow::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
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
/* -----------------------------15.12.99 12:57--------------------------------

 ---------------------------------------------------------------------------*/
MenuResource::MenuResource(const ResId& rResId) :
    Resource(rResId),
    aMenuArray(ResId(1))
{
    FreeResource();
}

