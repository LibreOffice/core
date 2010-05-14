/************************************************************************* *
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postit.cxx,v $
 * $Revision: 1.8.42.11 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "precompiled_sw.hxx"
#include <postit.hxx>
#include <PostItMgr.hxx>

#include <popup.hrc>
#include <docvw.hrc>
#include <app.hrc>

#include <hintids.hxx>
#include "viewopt.hxx"
#include "cmdid.h"

#include <tools/poly.hxx>   // Polygon

#include <svx/postitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/eeitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/bulitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/colritem.hxx>
#include <svx/flditem.hxx>
#include <svx/frmdir.hxx>
#include <svx/frmdiritem.hxx>
#include <svx/langitem.hxx>
#include <svx/adjitem.hxx>
#include <svx/editview.hxx>
#include <svx/svdview.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/editstat.hxx> //EditEngine flags
#include <svx/outliner.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/unolingu.hxx>

#include <svtools/langtab.hxx>
#include <svtools/slstitm.hxx>
#include <svtools/securityoptions.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/languageoptions.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/svmedit.hxx>

#include <linguistic/lngprops.hxx>

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/mnumgr.hxx>

#include <vcl/vclenum.hxx>
#include <vcl/edit.hxx>
#include <vcl/help.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
#include <vcl/gradient.hxx>
#include <vcl/salbtype.hxx> // FRound

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <swrect.hxx>
#include <docufld.hxx> // SwPostItField
#include <edtwin.hxx>
#include <view.hxx>
#include <viewsh.hxx>
#include <docsh.hxx>
#include <shellres.hxx>
#include <fmtfld.hxx>
#include <wrtsh.hxx>
#include <textsh.hxx>
#include <doc.hxx>
#include <txtfld.hxx>
#include <redline.hxx>
#include <uitool.hxx>
#include <SwUndoField.hxx>
#include <editsh.hxx>
#include <swmodule.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <langhelper.hxx>

#include <sw_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>

using namespace ::com::sun::star;

#define METABUTTON_WIDTH        16
#define METABUTTON_HEIGHT       18
#define METABUTTON_AREA_WIDTH   30
#define POSTIT_META_HEIGHT  (sal_Int32)     30
#define POSTIT_MINIMUMSIZE_WITHOUT_META     50

#define POSTIT_SHADOW_BRIGHT    Color(180,180,180)
#define POSTIT_SHADOW_DARK      Color(83,83,83)

#define LINEBREAK               rtl::OUString::createFromAscii("\n")
#define EMPTYSTRING             rtl::OUString::createFromAscii("")

TYPEINIT0(SwMarginWin);
TYPEINIT1(SwPostIt,SwMarginWin);
//TYPEINIT1(SwRedComment,SwMarginWin);

Color ColorFromAlphaColor(UINT8 aTransparency, Color &aFront, Color &aBack )
{
    return Color((UINT8)(aFront.GetRed()    * aTransparency/(double)255 + aBack.GetRed()    * (1-aTransparency/(double)255)),
                 (UINT8)(aFront.GetGreen()  * aTransparency/(double)255 + aBack.GetGreen()  * (1-aTransparency/(double)255)),
                 (UINT8)(aFront.GetBlue()   * aTransparency/(double)255 + aBack.GetBlue()   * (1-aTransparency/(double)255)));
}

/************ PostItTxt **************************************/
PostItTxt::PostItTxt(Window* pParent, WinBits nBits) : Window(pParent, nBits), mpOutlinerView(0),mMouseOver(false),mbShowPopup(FALSE)
{
    SetHelpId(26276);
    AddEventListener( LINK( this, PostItTxt, WindowEventListener ) );
    mpMarginWin = static_cast<SwMarginWin*>(GetParent());
}

PostItTxt::~PostItTxt()
{
    RemoveEventListener( LINK( this, PostItTxt, WindowEventListener ) );
}

void PostItTxt::GetFocus()
{
    BOOL bLockView = mpMarginWin->DocView()->GetWrtShell().IsViewLocked();
    mpMarginWin->DocView()->GetWrtShell().LockView( TRUE );

    if(mpMarginWin && !mpMarginWin->IsPreview())
         mpMarginWin->Mgr()->SetActivePostIt(mpMarginWin);
    Window::GetFocus();
    if (!mMouseOver)
        Invalidate();

    mpMarginWin->DocView()->GetWrtShell().LockView( bLockView );
    mpMarginWin->Mgr()->MakeVisible(mpMarginWin);
}

void PostItTxt::LoseFocus()
{
    // write the visible text back into the SwField
    if ( mpMarginWin )
        mpMarginWin->UpdateData();

    Window::LoseFocus();
    if (!mMouseOver)
        Invalidate();
}

void PostItTxt::RequestHelp(const HelpEvent &rEvt)
{
    USHORT nResId = 0;
    switch( mpMarginWin->GetStatus() )
    {
        case SwPostItHelper::INSERTED:  nResId = STR_REDLINE_INSERT; break;
        case SwPostItHelper::DELETED:   nResId = STR_REDLINE_DELETE; break;
        default: nResId = 0;
    }

    SwContentAtPos aCntntAtPos( SwContentAtPos::SW_REDLINE );
    if ( nResId && mpMarginWin->DocView()->GetWrtShell().GetContentAtPos( mpMarginWin->GetAnkorRect().Pos(), aCntntAtPos ) )
    {
        String sTxt;
        sTxt = SW_RESSTR( nResId );
        sTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM(": " ));
        sTxt += aCntntAtPos.aFnd.pRedl->GetAuthorString();
        sTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ));
        sTxt += GetAppLangDateTimeString( aCntntAtPos.aFnd.pRedl->GetTimeStamp() );
        Help::ShowQuickHelp( this,PixelToLogic(Rectangle(rEvt.GetMousePosPixel(),Size(50,10))),sTxt);
    }
}

void PostItTxt::Paint( const Rectangle& rRect)
{
    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        if (mMouseOver || HasFocus())
            DrawGradient(Rectangle(Point(0,0),PixelToLogic(GetSizePixel())),
                Gradient(GRADIENT_LINEAR,mpMarginWin->ColorDark(),mpMarginWin->ColorDark()));
        else
            DrawGradient(Rectangle(Point(0,0),PixelToLogic(GetSizePixel())),
                Gradient(GRADIENT_LINEAR,mpMarginWin->ColorLight(),mpMarginWin->ColorDark()));
     }

    mpOutlinerView->Paint( rRect );

    if (mpMarginWin->GetStatus()==SwPostItHelper::DELETED)
    {
        SetLineColor(static_cast<SwPostIt*>(mpMarginWin)->GetChangeColor());
        DrawLine(PixelToLogic(GetPosPixel()),PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width(),GetSizePixel().Height())));
        DrawLine(PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width(),0)),PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height())));
    }
}

void PostItTxt::KeyInput( const KeyEvent& rKeyEvt )
{
    const KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    USHORT nKey = rKeyCode.GetCode();
    SwView* pView = mpMarginWin->DocView();
    if ((rKeyCode.IsMod1() && rKeyCode.IsMod2()) && ((nKey == KEY_PAGEUP) || (nKey == KEY_PAGEDOWN)))
        mpMarginWin->SwitchToPostIt(nKey);
    else
    if ((nKey == KEY_ESCAPE) || (rKeyCode.IsMod1() && ((nKey == KEY_PAGEUP) || (nKey == KEY_PAGEDOWN))))
            mpMarginWin->SwitchToFieldPos();
    else
    if (nKey == KEY_INSERT)
    {
        if (!rKeyCode.IsMod1() && !rKeyCode.IsMod2())
            mpMarginWin->ToggleInsMode();
    }
    else
    {
        //let's make sure we see our note
        mpMarginWin->Mgr()->MakeVisible(mpMarginWin);

        long aOldHeight = mpMarginWin->GetPostItTextHeight();
        bool bDone = false;

        /// HACK: need to switch off processing of Undo/Redo in Outliner
        if ( !( (nKey == KEY_Z || nKey == KEY_Y) && rKeyCode.IsMod1()) )
        {
            /*
            SwPostItHelper::SwLayoutStatus aStatus = mpMarginWin->GetStatus();
            if ( (aStatus!=SwPostItHelper::DELETED) ||
                    ( (aStatus==SwPostItHelper::DELETED) && (!mpMarginWin->Engine()->GetEditEngine().DoesKeyChangeText(rKeyEvt))) )
            */
            bool bIsProtected = mpMarginWin->IsProtected();
            if (!bIsProtected || (bIsProtected && !mpMarginWin->Engine()->GetEditEngine().DoesKeyChangeText(rKeyEvt)) )
                bDone = mpOutlinerView->PostKeyEvent( rKeyEvt );
        }
        if (bDone)
            mpMarginWin->ResizeIfNeccessary(aOldHeight,mpMarginWin->GetPostItTextHeight());
        else
        {
            // write back data first when showing navigator
            if ( nKey==KEY_F5 )
                mpMarginWin->UpdateData();
            if (!pView->KeyInput(rKeyEvt))
                Window::KeyInput(rKeyEvt);
        }
    }

    pView->GetViewFrame()->GetBindings().InvalidateAll(FALSE);
}

void PostItTxt::MouseMove( const MouseEvent& rMEvt )
{
    if ( mpOutlinerView )
    {
        mpOutlinerView->MouseMove( rMEvt );
        // mba: why does OutlinerView not handle the modifier setting?!
        // this forces the postit to handle *all* pointer types
        SetPointer( mpOutlinerView->GetPointer( rMEvt.GetPosPixel() ) );

        const EditView& aEV = mpOutlinerView->GetEditView();
        const SvxFieldItem* pItem = aEV.GetFieldUnderMousePointer();
        if ( pItem )
        {
            const SvxFieldData* pFld = pItem->GetField();
            const SvxURLField* pURL = PTR_CAST( SvxURLField, pFld );
            if ( pURL )
            {
                String sURL( pURL->GetURL() );
                SvtSecurityOptions aSecOpts;
                if ( aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK) )
                {
                    sURL.InsertAscii( ": ", 0 );
                    sURL.Insert( ViewShell::GetShellRes()->aHyperlinkClick, 0 );
                }
                Help::ShowQuickHelp( this,PixelToLogic(Rectangle(GetPosPixel(),Size(50,10))),sURL);
            }
        }
    }
}

void PostItTxt::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (mpOutlinerView )
    {
        SvtSecurityOptions aSecOpts;
        bool bExecuteMod = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK);

        if ( !bExecuteMod || (bExecuteMod && rMEvt.GetModifier() == KEY_MOD1))
        {
            const EditView& aEV = mpOutlinerView->GetEditView();
            const SvxFieldItem* pItem = aEV.GetFieldUnderMousePointer();
            if ( pItem )
            {
                const SvxFieldData* pFld = pItem->GetField();
                const SvxURLField* pURL = PTR_CAST( SvxURLField, pFld );
                if ( pURL )
                {
                    mpOutlinerView->MouseButtonDown( rMEvt );
                    SwWrtShell &rSh = mpMarginWin->DocView()->GetWrtShell();
                    String sURL( pURL->GetURL() );
                    String sTarget( pURL->GetTargetFrame() );
                    ::LoadURL( sURL, &rSh, URLLOAD_NOFILTER, &sTarget);
                    return;
                }
            }
        }
    }

    GrabFocus();
    if ( mpOutlinerView )
        mpOutlinerView->MouseButtonDown( rMEvt );
    mpMarginWin->DocView()->GetViewFrame()->GetBindings().InvalidateAll(FALSE);
}

void PostItTxt::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mpOutlinerView )
        mpOutlinerView->MouseButtonUp( rMEvt );
}

IMPL_LINK(PostItTxt, OnlineSpellCallback, SpellCallbackInfo*, pInfo)
{
    if ( mpMarginWin && (pInfo->nCommand == SPELLCMD_STARTSPELLDLG) )
        mpMarginWin->DocView()->GetViewFrame()->GetDispatcher()->Execute( FN_SPELL_GRAMMAR_DIALOG, SFX_CALLMODE_ASYNCHRON);
    return 0;
}

IMPL_LINK( PostItTxt, Select, Menu*, pSelMenu )
{
    mpMarginWin->ExecuteCommand( pSelMenu->GetCurItemId() );
    return 0;
}

void PostItTxt::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        if (!mpMarginWin->IsProtected() &&
        //if (!mpMarginWin->IsReadOnly() &&  (mpMarginWin->GetStatus()!=SwPostItHelper::DELETED) &&
            mpOutlinerView->IsWrongSpelledWordAtPos( rCEvt.GetMousePosPixel(),TRUE ))
        {
            Link aLink = LINK(this, PostItTxt, OnlineSpellCallback);
            mpOutlinerView->ExecuteSpellPopup(rCEvt.GetMousePosPixel(),&aLink);
        }
        else
        {
            SfxPopupMenuManager* aMgr = mpMarginWin->DocView()->GetViewFrame()->GetDispatcher()->Popup(0, this,&rCEvt.GetMousePosPixel());
            XubString aText = ((PopupMenu*)aMgr->GetSVMenu())->GetItemText( FN_DELETE_NOTE_AUTHOR );
            SwRewriter aRewriter;
            aRewriter.AddRule(UNDO_ARG1, mpMarginWin->GetAuthor());
            aText = aRewriter.Apply(aText);
            ((PopupMenu*)aMgr->GetSVMenu())->SetItemText(FN_DELETE_NOTE_AUTHOR,aText);
            // SwPostItLinkForwarder_Impl aFwd( ((PopupMenu*)aMgr->GetSVMenu())->pSvMenu->GetSelectHdl(), mpPostIt );
            // ((PopupMenu*)aMgr->GetSVMenu())->pSvMenu->SetSelectHdl( LINK(&aFwd, SwPostItLinkForwarder_Impl, Select) );

            ((PopupMenu*)aMgr->GetSVMenu())->SetSelectHdl( LINK(this, PostItTxt, Select) );

            if (rCEvt.IsMouseEvent())
                ((PopupMenu*)aMgr->GetSVMenu())->Execute(this,rCEvt.GetMousePosPixel());
            else
            {
                const Size aSize = GetSizePixel();
                const Point aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );
                ((PopupMenu*)aMgr->GetSVMenu())->Execute(this,aPos);
            }
            delete aMgr;
        }
    }
    else
    if (rCEvt.GetCommand() == COMMAND_WHEEL)
    {
        if (mpMarginWin->Scrollbar()->IsVisible())
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if (pData->IsShift() || pData->IsMod1() || pData->IsMod2())
            {
                mpMarginWin->DocView()->HandleWheelCommands(rCEvt);
            }
            else
            {
                HandleScrollCommand( rCEvt, 0 , mpMarginWin->Scrollbar());

                /*
                long nLines = pData->GetNotchDelta() * (long)pData->GetScrollLines();
                if ( ((mpMarginWin->Scrollbar()->GetRange().Min() == mpMarginWin->Scrollbar()->GetThumbPos()) && (nLines > 0)) ||
                    ( (mpMarginWin->Scrollbar()->GetRange().Max() == mpMarginWin->Scrollbar()->GetThumbPos()+mpMarginWin->Scrollbar()->GetVisibleSize()) &&  (nLines < 0)) )
                {
                    mpMarginWin->DocView()->HandleWheelCommands(rCEvt);
                }
                else
                {
                    HandleScrollCommand( rCEvt, 0 , mpMarginWin->Scrollbar());
                }
                */
            }
        }
        else
        {
            mpMarginWin->DocView()->HandleWheelCommands(rCEvt);
        }
    }
    else if (rCEvt.GetCommand() == COMMAND_SELECTIONCHANGE)
    {
        if ( mpOutlinerView )
        {
            const CommandSelectionChangeData *pData = rCEvt.GetSelectionChangeData();
            ESelection aSelection = mpOutlinerView->GetEditView().GetSelection();
            aSelection.nStartPos = sal::static_int_cast<sal_uInt16, ULONG>(pData->GetStart());
            aSelection.nEndPos = sal::static_int_cast<sal_uInt16, ULONG>(pData->GetEnd());
            mpOutlinerView->GetEditView().SetSelection(aSelection);
        }
    }
    else if (rCEvt.GetCommand() == COMMAND_PREPARERECONVERSION)
    {
        if ( mpOutlinerView && mpOutlinerView->HasSelection() )
        {
            EditEngine *aEditEngine = mpOutlinerView->GetEditView().GetEditEngine();
            ESelection aSelection = mpOutlinerView->GetEditView().GetSelection();
            aSelection.Adjust();
            if( aSelection.nStartPara != aSelection.nEndPara )
            {
                xub_StrLen aParaLen = aEditEngine->GetTextLen( aSelection.nStartPara );
                aSelection.nEndPara = aSelection.nStartPara;
                aSelection.nEndPos = aParaLen;
                mpOutlinerView->GetEditView().SetSelection( aSelection );
            }
        }
    }
    else
    {
        if ( mpOutlinerView )
            mpOutlinerView->Command( rCEvt );
        else
            Window::Command(rCEvt);
    }
}

void PostItTxt::DataChanged( const DataChangedEvent& aData)
{
    Window::DataChanged( aData );
}

XubString PostItTxt::GetSurroundingText() const
{
    if( mpOutlinerView )
    {
        EditEngine *aEditEngine = mpOutlinerView->GetEditView().GetEditEngine();
        if( mpOutlinerView->HasSelection() )
            return mpOutlinerView->GetSelected();
        else
        {
            ESelection aSelection = mpOutlinerView->GetEditView().GetSelection();
            XubString aStr = aEditEngine->GetText(aSelection.nStartPara);
            return aStr;
        }
    }
    else
        return XubString::EmptyString();
}

Selection PostItTxt::GetSurroundingTextSelection() const
{
    if( mpOutlinerView )
    {
        if( mpOutlinerView->HasSelection() )
            return Selection( 0, mpOutlinerView->GetSelected().Len() );
        else
        {
            ESelection aSelection = mpOutlinerView->GetEditView().GetSelection();
            return Selection( aSelection.nStartPos, aSelection.nEndPos );
        }
    }
    else
        return Selection( 0, 0 );
}

IMPL_LINK( PostItTxt, WindowEventListener, VclSimpleEvent*, pWinEvent )
{
    if ( pWinEvent && pWinEvent->ISA( VclWindowEvent ) )
    {
        VclWindowEvent *pEvent = (VclWindowEvent*)pWinEvent;
        if (pEvent->GetId() == VCLEVENT_WINDOW_MOUSEMOVE)
        {
            MouseEvent* pMouseEvt = (MouseEvent*)pEvent->GetData();
            if ( pMouseEvt->IsEnterWindow() )
            {
                mMouseOver = true;
                if (!mbShowPopup && !HasFocus())
                {
                    mpMarginWin->SetViewState(SS_VIEW);
                    Invalidate();
                }
            }
            else if ( pMouseEvt->IsLeaveWindow())
            {
                if (mpMarginWin->IsPreview())
                {
                    //mpMarginWin->doLazyDelete();
                }
                else
                {
                    mMouseOver = false;
                    if (!mbShowPopup && !HasFocus())
                    {
                        mpMarginWin->SetViewState(SS_NORMAL);
                        Invalidate();
                    }
                }
            }
        }
    }
    return sal_True;
}

/************** SwMarginWin***********************************++*/
SwMarginWin::SwMarginWin(Window* pParent, WinBits nBits,SwPostItMgr* aMgr,SwPostItBits aBits)
: Window(pParent, nBits),
    mnEventId(0),
    mpOutlinerView(0),
    mpOutliner(0),
    mpPostItTxt(0),
    mpMeta(0),
    mpVScrollbar(0),
    mpAnkor(0),
    mpShadow(0),
    mpMgr(aMgr),
    mbMeta(true),
    nFlags(aBits),
    mbMouseOverButton(false),
    mpButtonPopup(0),
    mbIsFollow(false)
{
    SwEditWin* aWin = static_cast<SwEditWin*>(GetParent());
    mpView = &aWin->GetView();

    SdrPaintWindow* pPaintWindow = mpView->GetDrawView()->GetPaintWindow(0);
    if(pPaintWindow)
    {
        pOverlayManager = pPaintWindow->GetOverlayManager();

        mpShadow = new SwPostItShadow(basegfx::B2DPoint(0,0),basegfx::B2DPoint(0,0),Color(0,0,0),SS_NORMAL);
        mpShadow->setVisible(false);
        pOverlayManager->add(*mpShadow);
    }
}

SwMarginWin::~SwMarginWin()
{
    if (mpOutlinerView)
    {
        delete mpOutlinerView;
    }

    if (mpOutliner)
    {
        delete mpOutliner;
    }

    if (mpMeta)
    {
        mpMeta->RemoveEventListener( LINK( mpPostItTxt, PostItTxt, WindowEventListener ) );
        delete mpMeta;
    }

    if (mpPostItTxt)
    {
        delete mpPostItTxt;
    }

    if (mpVScrollbar)
    {
        delete mpVScrollbar;
    }

    if (mpAnkor)
    {
        if (mpAnkor->getOverlayManager())
        {
            // remove this object from the chain
            mpAnkor->getOverlayManager()->remove(*mpAnkor);
        }
        delete mpAnkor;
    }

    if (mpShadow)
    {
        if (mpShadow->getOverlayManager())
        {
            mpShadow->getOverlayManager()->remove(*mpShadow);
        }
        delete mpShadow;
    }

    if (mpButtonPopup)
    {
        delete mpButtonPopup;
    }

    if (mnEventId)
        Application::RemoveUserEvent( mnEventId );
}

void SwMarginWin::Paint( const Rectangle& rRect)
{
    Window::Paint(rRect);

    if (mpMeta->IsVisible() )
    {
        //draw left over space
        if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
            SetFillColor(COL_BLACK);
        else
            SetFillColor(mColorDark);
        SetLineColor();
        DrawRect(PixelToLogic(Rectangle(Point(mpMeta->GetPosPixel().X()+mpMeta->GetSizePixel().Width(),mpMeta->GetPosPixel().Y()),Size(GetMetaButtonAreaWidth(),mpMeta->GetSizePixel().Height()))));

        if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        {
            //draw rect around button
            SetFillColor(COL_BLACK);
            SetLineColor(COL_WHITE);
        }
        else
        {
            //draw button
            Gradient aGradient;
            if (mbMouseOverButton)
                aGradient = Gradient(GRADIENT_LINEAR,ColorFromAlphaColor(80,mColorAnkor,mColorDark),ColorFromAlphaColor(15,mColorAnkor,mColorDark));
            else
                aGradient = Gradient(GRADIENT_LINEAR,ColorFromAlphaColor(15,mColorAnkor,mColorDark),ColorFromAlphaColor(80,mColorAnkor,mColorDark));
            DrawGradient(mRectMetaButton,aGradient);
            //draw rect around button
            SetFillColor();
            SetLineColor(ColorFromAlphaColor(90,mColorAnkor,mColorDark));
        }
        DrawRect(mRectMetaButton);

        if (IsPreview())
        {
            Font aOldFont( GetFont());
            Font aFont(aOldFont);
            Color aCol( COL_BLACK);
            aFont.SetColor( aCol );
            aFont.SetHeight(200);
            aFont.SetWeight(WEIGHT_MEDIUM);
            SetFont( aFont );
            DrawText(mRectMetaButton,rtl::OUString::createFromAscii("Edit Note"),TEXT_DRAW_CENTER);
            SetFont( aOldFont );
        }
        else
        {
            //draw arrow
            if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                SetFillColor(COL_WHITE);
            else
                SetFillColor(COL_BLACK);
            SetLineColor();
            DrawPolygon(Polygon(aPopupTriangle));
        }
    }
}

void SwMarginWin::SetPosSizePixelRect(long nX, long nY,long nWidth, long nHeight,const SwRect &aRect, const long aPageBorder)
{
    mbMeta = true;
    mPosSize = Rectangle(Point(nX,nY),Size(nWidth,nHeight));
    mAnkorRect = aRect;
    mPageBorder = aPageBorder;
}

void SwMarginWin::SetSize( const Size& rNewSize )
{
    mPosSize.SetSize(rNewSize);
}

void SwMarginWin::SetVirtualPosSize( const Point& aPoint, const Size& aSize)
{
    mPosSize = Rectangle(aPoint,aSize);
}

void SwMarginWin::TranslateTopPosition(const long aAmount)
{
    mPosSize.Move(0,aAmount);
}

void SwMarginWin::ShowAnkorOnly(const Point &aPoint)
{
    HideNote();
    SetPosAndSize();
    if (mpAnkor)
    {
        mpAnkor->SetSixthPosition(basegfx::B2DPoint(aPoint.X(),aPoint.Y()));
        mpAnkor->SetSeventhPosition(basegfx::B2DPoint(aPoint.X(),aPoint.Y()));
        mpAnkor->SetAnkorState(AS_ALL);
        mpAnkor->setVisible(true);
    }
    if (mpShadow)
        mpShadow->setVisible(false);
}

void SwMarginWin::InitControls()
{
    // actual window which holds the user text
    mpPostItTxt = new PostItTxt(this, WB_NODIALOGCONTROL);
    mpPostItTxt->SetPointer(Pointer(POINTER_TEXT));

    // window control for author and date
    mpMeta = new MultiLineEdit(this,0);
    mpMeta->SetReadOnly();
    mpMeta->SetRightToLeft(Application::GetSettings().GetLayoutRTL());
    mpMeta->AlwaysDisableInput(true);
    mpMeta->SetCallHandlersOnInputDisabled(true);
    mpMeta->AddEventListener( LINK( mpPostItTxt, PostItTxt, WindowEventListener ) );
    AddEventListener( LINK( mpPostItTxt, PostItTxt, WindowEventListener ) );

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    AllSettings aSettings = mpMeta->GetSettings();
    StyleSettings aStyleSettings = aSettings.GetStyleSettings();
    Font aFont = aStyleSettings.GetFieldFont();
    aFont.SetHeight(8);
    aStyleSettings.SetFieldFont(aFont);
    aSettings.SetStyleSettings(aStyleSettings);
    mpMeta->SetSettings(aSettings);

    SwDocShell* aShell = mpView->GetDocShell();
    mpOutliner = new Outliner(&aShell->GetPool(),OUTLINERMODE_TEXTOBJECT);
    aShell->GetDoc()->SetCalcFieldValueHdl( mpOutliner );
    // mpOutliner->EnableUndo( FALSE );
    mpOutliner->SetUpdateMode( TRUE );
    Rescale();

    OutputDevice* pDev = aShell->GetDoc()->getReferenceDevice(TRUE);
    if ( pDev )
    {
        mpOutliner->SetRefDevice( pDev );
    }

    mpOutlinerView = new OutlinerView ( mpOutliner, mpPostItTxt );
    mpOutlinerView->SetBackgroundColor(COL_TRANSPARENT);
    mpOutliner->InsertView(mpOutlinerView );
    mpPostItTxt->SetTextView(mpOutlinerView);
    mpOutlinerView->SetOutputArea( PixelToLogic( Rectangle(0,0,1,1) ) );

    SfxItemSet item(aShell->GetPool());
    item.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
    item.Put(SvxFontItem(FAMILY_SWISS,GetSettings().GetStyleSettings().GetFieldFont().GetName(),
                            EMPTYSTRING,PITCH_DONTKNOW,RTL_TEXTENCODING_DONTKNOW,EE_CHAR_FONTINFO));
    mpOutlinerView->SetAttribs(item);

    // TODO: ??
    EEHorizontalTextDirection aDefHoriTextDir = Application::GetSettings().GetLayoutRTL() ? EE_HTEXTDIR_R2L : EE_HTEXTDIR_L2R;
    mpOutliner->SetDefaultHorizontalTextDirection( aDefHoriTextDir );

    //create Scrollbars
    mpVScrollbar = new ScrollBar(this, WB_3DLOOK |WB_VSCROLL|WB_DRAG);
    mpVScrollbar->EnableNativeWidget(false);
    mpVScrollbar->EnableRTL( false );
    mpVScrollbar->SetScrollHdl(LINK(this, SwMarginWin, ScrollHdl));
    mpVScrollbar->EnableDrag();
    mpVScrollbar->AddEventListener( LINK( mpPostItTxt, PostItTxt, WindowEventListener ) );

    const SwViewOption* pVOpt = mpView->GetWrtShellPtr()->GetViewOptions();
    ULONG nCntrl = mpOutliner->GetControlWord();
    // TODO: crash when AUTOCOMPLETE enabled
    nCntrl |= EE_CNTRL_MARKFIELDS | EE_CNTRL_PASTESPECIAL | EE_CNTRL_AUTOCORRECT  | EV_CNTRL_AUTOSCROLL | EE_CNTRL_URLSFXEXECUTE; // | EE_CNTRL_AUTOCOMPLETE;
    if (pVOpt->IsFieldShadings())
        nCntrl |= EE_CNTRL_MARKFIELDS;
    else
        nCntrl &= ~EE_CNTRL_MARKFIELDS;
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    sal_uInt16 aIndex = SW_MOD()->InsertRedlineAuthor(GetAuthor());
    SetColor(mpMgr->GetColorDark(aIndex),mpMgr->GetColorLight(aIndex),mpMgr->GetColorAnkor(aIndex));

    CheckMetaText();
    SetPopup();
    SetLanguage(GetLanguage());
    View()->StartSpeller();
    SetPostItText();
    Engine()->CompleteOnlineSpelling();

    mpMeta->Show();
    mpVScrollbar->Show();
    mpPostItTxt->Show();
}

void SwMarginWin::CheckMetaText()
{
    const LocaleDataWrapper& rLocalData = SvtSysLocale().GetLocaleData();
    String sMeta = GetAuthor();
    if (sMeta==String(EMPTYSTRING))
        sMeta = String(SW_RES(STR_NOAUTHOR));
    if (sMeta.Len() > 22)
    {
        sMeta.Erase(20);
        sMeta = sMeta + rtl::OUString::createFromAscii("...");
    }
    Date aDate = GetDate();
    if (aDate==Date())
        sMeta = sMeta + LINEBREAK + String(SW_RES(STR_POSTIT_TODAY));
    else
    if (aDate == Date(Date()-1))
        sMeta = sMeta + LINEBREAK + String(SW_RES(STR_POSTIT_YESTERDAY));
    else
    if (aDate.IsValid() )
        sMeta = sMeta + LINEBREAK + rLocalData.getDate(aDate);
    else
        sMeta = sMeta + LINEBREAK + String(SW_RES(STR_NODATE));
    if (GetTime()!=0)
        sMeta = sMeta + rtl::OUString::createFromAscii(" ")  + rLocalData.getTime( GetTime(),false );
    if (mpMeta->GetText() != sMeta)
        mpMeta->SetText(sMeta);
}

void SwMarginWin::Rescale()
{
    MapMode aMode = GetParent()->GetMapMode();
    aMode.SetOrigin( Point() );
    //aMode.SetScaleX( aMode.GetScaleX() * Fraction( 8, 10 ) );
    //aMode.SetScaleY( aMode.GetScaleY() * Fraction( 8, 10 ) );
    mpOutliner->SetRefMapMode( aMode );
    SetMapMode( aMode );
    mpPostItTxt->SetMapMode( aMode );
    if ( mpMeta )
    {
        Font aFont( mpMeta->GetSettings().GetStyleSettings().GetFieldFont() );
        sal_Int32 nHeight = aFont.GetHeight();
        nHeight = nHeight * aMode.GetScaleY().GetNumerator() / aMode.GetScaleY().GetDenominator();
        aFont.SetHeight( nHeight );
        mpMeta->SetControlFont( aFont );
    }
}

void SwMarginWin::SetPosAndSize()
{
    bool bChange = false;

    if (GetSizePixel() != mPosSize.GetSize())
    {
        bChange = true;
        SetSizePixel(mPosSize.GetSize());
        DoResize();
    }

    if (GetPosPixel().X() != mPosSize.TopLeft().X() || (abs(GetPosPixel().Y() - mPosSize.TopLeft().Y()) > 5) )
    {
        bChange = true;
        SetPosPixel(mPosSize.TopLeft());

        Point aLineStart;
        Point aLineEnd ;
        if (mbMarginSide)
        {
            aLineStart = EditWin()->PixelToLogic( Point(GetPosPixel().X()+GetSizePixel().Width(),GetPosPixel().Y()-1) );
            aLineEnd = EditWin()->PixelToLogic( Point(GetPosPixel().X(),GetPosPixel().Y()-1) );
        }
        else
        {
            aLineStart = EditWin()->PixelToLogic( Point(GetPosPixel().X(),GetPosPixel().Y()-1) );
            aLineEnd = EditWin()->PixelToLogic( Point(GetPosPixel().X()+GetSizePixel().Width(),GetPosPixel().Y()-1) );
        }

        if (!IsPreview())
        {
            if (mpAnkor)
            {
                mpAnkor->SetAllPosition(basegfx::B2DPoint( mAnkorRect.Left() , mAnkorRect.Bottom() - 5* 15),
                                        basegfx::B2DPoint( mAnkorRect.Left()-5*15 , mAnkorRect.Bottom()+5*15),
                                        basegfx::B2DPoint( mAnkorRect.Left()+5*15 , mAnkorRect.Bottom()+5*15),
                                        basegfx::B2DPoint( mAnkorRect.Left(), mAnkorRect.Bottom()+2*15),
                                        basegfx::B2DPoint( mPageBorder ,mAnkorRect.Bottom()+2*15),
                                        basegfx::B2DPoint( aLineStart.X(),aLineStart.Y()),
                                        basegfx::B2DPoint( aLineEnd.X(),aLineEnd.Y()));
                mpAnkor->SetHeight(mAnkorRect.Height());
            }
            else
            {
                mpAnkor = new SwPostItAnkor(basegfx::B2DPoint( mAnkorRect.Left() , mAnkorRect.Bottom()-5*15),
                                            basegfx::B2DPoint( mAnkorRect.Left()-5*15 , mAnkorRect.Bottom()+5*15),
                                            basegfx::B2DPoint( mAnkorRect.Left()+5*15 , mAnkorRect.Bottom()+5*15),
                                            basegfx::B2DPoint( mAnkorRect.Left(), mAnkorRect.Bottom()+2*15),
                                            basegfx::B2DPoint( mPageBorder ,mAnkorRect.Bottom()+2*15),
                                            basegfx::B2DPoint( aLineStart.X(),aLineStart.Y()),
                                            basegfx::B2DPoint( aLineEnd.X(),aLineEnd.Y()) ,
                                            mColorAnkor,
                                            false,
                                            false);
                mpAnkor->SetHeight(mAnkorRect.Height());
                mpAnkor->setVisible(true);
                mpAnkor->SetAnkorState(AS_TRI);
                if (HasChildPathFocus())
                {
                    mpAnkor->setLineSolid(true);
                }
                pOverlayManager->add(*mpAnkor);
            }
        }
    }
    else
    {
        if ( mpAnkor && (mpAnkor->getBasePosition() != basegfx::B2DPoint( mAnkorRect.Left() , mAnkorRect.Bottom()-5*15)) )
            mpAnkor->SetTriPosition(basegfx::B2DPoint( mAnkorRect.Left() , mAnkorRect.Bottom() - 5* 15),
                                    basegfx::B2DPoint( mAnkorRect.Left()-5*15 , mAnkorRect.Bottom()+5*15),
                                    basegfx::B2DPoint( mAnkorRect.Left()+5*15 , mAnkorRect.Bottom()+5*15),
                                    basegfx::B2DPoint( mAnkorRect.Left(), mAnkorRect.Bottom()+2*15),
                                    basegfx::B2DPoint( mPageBorder ,mAnkorRect.Bottom()+2*15));
    }

    if (bChange)
    {
        Point aStart = EditWin()->PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height()));
        Point aEnd = EditWin()->PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width()-1,GetSizePixel().Height()));
        mpShadow->SetPosition(basegfx::B2DPoint(aStart.X(),aStart.Y()), basegfx::B2DPoint(aEnd.X(),aEnd.Y()));
    }

    if (mpMgr->ShowNotes())
    {
        if (IsFollow() && !HasChildPathFocus())
        {
            mpAnkor->SetAnkorState(AS_END);
        }
        else
        {
            mpAnkor->SetAnkorState(AS_ALL);
            SwMarginWin* pWin = GetTopReplyNote();
            if (IsFollow() && pWin )
                pWin->Ankor()->SetAnkorState(AS_END);
        }
    }
}

void SwMarginWin::DoResize()
{
    long aTextHeight        =   LogicToPixel( mpOutliner->CalcTextSize()).Height();
    unsigned long aWidth    =   GetSizePixel().Width();
    long aHeight            =   GetSizePixel().Height();

    if (mbMeta)
    {
        aHeight -= GetMetaHeight();
        mpMeta->Show();
        mpPostItTxt->SetQuickHelpText(EMPTYSTRING);
    }
    else
    {
        mpMeta->Hide();
        mpPostItTxt->SetQuickHelpText(mpMeta->GetText());
    }

    if ((aTextHeight > aHeight) && !IsPreview())
    {   // we need vertical scrollbars and have to reduce the width
        aWidth -= GetScrollbarWidth();
        mpVScrollbar->Show();
    }
    else
    {
        mpVScrollbar->Hide();
    }

    mpPostItTxt->SetPosSizePixel(0, 0, aWidth, aHeight);
    mpMeta->SetPosSizePixel(0,aHeight,GetSizePixel().Width()-GetMetaButtonAreaWidth(),GetMetaHeight());
    mpOutliner->SetPaperSize( PixelToLogic( Size(aWidth,aHeight) ) ) ;
    mpOutlinerView->SetOutputArea( PixelToLogic( Rectangle(0,0,aWidth,aHeight) ) );
    if (!mpVScrollbar->IsVisible())
    {   // if we do not have a scrollbar anymore, we want to see the complete text
        mpOutlinerView->SetVisArea( PixelToLogic( Rectangle(0,0,aWidth,aHeight) ) );
    }
    mpVScrollbar->SetPosSizePixel( aWidth, 0, GetScrollbarWidth(), aHeight      );
    mpVScrollbar->SetVisibleSize( PixelToLogic(Size(0,aHeight)).Height() );
    mpVScrollbar->SetPageSize( PixelToLogic(Size(0,aHeight)).Height() * 8 / 10 );
    mpVScrollbar->SetLineSize( mpOutliner->GetTextHeight() / 10 );
    //mpVScrollbar->SetThumbPos( mpOutlinerView->GetVisArea().Top()+ mpOutlinerView->GetEditView().GetCursor()->GetOffsetY());
    SetScrollbar();
    mpVScrollbar->SetRange( Range(0, mpOutliner->GetTextHeight()));

    //calculate rects for meta- button
    const Fraction& fx( GetMapMode().GetScaleX() );
    const Fraction& fy( GetMapMode().GetScaleY() );

    Point aPos( mpMeta->GetPosPixel());
    Point aBase( aPos.X() + aPos.X() + GetSizePixel().Width(), aPos.Y() );
    Point aLeft = PixelToLogic( Point( aBase.X() - (METABUTTON_WIDTH+5)*fx.GetNumerator()/fx.GetDenominator(), aBase.Y()+17*fy.GetNumerator()/fx.GetDenominator() ) );
    Point aRight = PixelToLogic( Point( aBase.X() - (METABUTTON_WIDTH-1)*fx.GetNumerator()/fx.GetDenominator(), aBase.Y()+17*fy.GetNumerator()/fy.GetDenominator() ) );
    Point aBottom = PixelToLogic( Point( aBase.X() - (METABUTTON_WIDTH+2)*fx.GetNumerator()/fx.GetDenominator(), aBase.Y()+20*fy.GetNumerator()/fy.GetDenominator() ) );

    //Point aLeft       = PixelToLogic(Point(mpMeta->GetPosPixel().X()+mpMeta->GetPosPixel().X()+GetSizePixel().Width()-(GetMetaButtonWidth()+10)+5,mpMeta->GetPosPixel().Y()+17));
    //Point aRight  = PixelToLogic(Point(mpMeta->GetPosPixel().X()+mpMeta->GetPosPixel().X()+GetSizePixel().Width()-(GetMetaButtonWidth()+10)+11,mpMeta->GetPosPixel().Y()+17));
    //Point aBottom = PixelToLogic(Point(mpMeta->GetPosPixel().X()+mpMeta->GetPosPixel().X()+GetSizePixel().Width()-(GetMetaButtonWidth()+10)+8,mpMeta->GetPosPixel().Y()+20));

    aPopupTriangle.clear();
    aPopupTriangle.append(basegfx::B2DPoint(aLeft.X(),aLeft.Y()));
    aPopupTriangle.append(basegfx::B2DPoint(aRight.X(),aRight.Y()));
    aPopupTriangle.append(basegfx::B2DPoint(aBottom.X(),aBottom.Y()));
    aPopupTriangle.setClosed(true);
    if (IsPreview())
        mRectMetaButton = PixelToLogic( Rectangle( Point(
                aPos.X()+GetSizePixel().Width()-(METABUTTON_WIDTH*4+10)*fx.GetNumerator()/fx.GetDenominator(),
                aPos.Y()+5*fy.GetNumerator()/fy.GetDenominator() ),
                Size( METABUTTON_WIDTH*4*fx.GetNumerator()/fx.GetDenominator(), METABUTTON_HEIGHT*fy.GetNumerator()/fy.GetDenominator() ) ) );
    else
        mRectMetaButton = PixelToLogic( Rectangle( Point(
                aPos.X()+GetSizePixel().Width()-(METABUTTON_WIDTH+10)*fx.GetNumerator()/fx.GetDenominator(),
                aPos.Y()+5*fy.GetNumerator()/fy.GetDenominator() ),
                Size( METABUTTON_WIDTH*fx.GetNumerator()/fx.GetDenominator(), METABUTTON_HEIGHT*fy.GetNumerator()/fy.GetDenominator() ) ) );
}

void SwMarginWin::SetSizePixel( const Size& rNewSize )
{
    Window::SetSizePixel(rNewSize);

    if (mpShadow)
    {
        Point aStart = EditWin()->PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height()));
        Point aEnd = EditWin()->PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width()-1,GetSizePixel().Height()));
        mpShadow->SetPosition(basegfx::B2DPoint(aStart.X(),aStart.Y()), basegfx::B2DPoint(aEnd.X(),aEnd.Y()));
    }
}

void SwMarginWin::SetScrollbar()
{
    mpVScrollbar->SetThumbPos( mpOutlinerView->GetVisArea().Top()+ mpOutlinerView->GetEditView().GetCursor()->GetOffsetY());
}

void SwMarginWin::ResizeIfNeccessary(long aOldHeight, long aNewHeight)
{
    if (aOldHeight != aNewHeight)
    {
        //check for lower border or next note
        long aBorder = mpMgr->GetNextBorder();
        if (aBorder != -1)
        {
            if (aNewHeight > GetMinimumSizeWithoutMeta())
            {
                long aNewLowerValue = GetPosPixel().Y() + aNewHeight + GetMetaHeight();
                if (aNewLowerValue < aBorder)
                    SetSizePixel(Size(GetSizePixel().Width(),aNewHeight+GetMetaHeight()));
                else
                    SetSizePixel(Size(GetSizePixel().Width(),aBorder - GetPosPixel().Y()));
                DoResize();
                Invalidate();
            }
            else
            {
                if (GetSizePixel().Height() != GetMinimumSizeWithoutMeta() + GetMetaHeight())
                    SetSizePixel(Size(GetSizePixel().Width(),GetMinimumSizeWithoutMeta() + GetMetaHeight()));
                DoResize();
                Invalidate();
            }
        }
        else
        {
            DoResize();
            Invalidate();
        }
    }
    else
    {
        SetScrollbar();
    }
}

void SwMarginWin::SetColor(Color aColorDark,Color aColorLight, Color aColorAnkor)
{
    mColorDark =  aColorDark;
    mColorLight = aColorLight;
    mColorAnkor = aColorAnkor;

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        //Wallpaper aWall(Gradient(GRADIENT_LINEAR,mColorLight,mColorDark));
        //mpPostItTxt->SetBackground(aWall);

        mpMeta->SetControlBackground(mColorDark);
        AllSettings aSettings = mpMeta->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        aStyleSettings.SetFieldTextColor(aColorAnkor);
        aSettings.SetStyleSettings(aStyleSettings);
        mpMeta->SetSettings(aSettings);

        AllSettings aSettings2 = mpVScrollbar->GetSettings();
        StyleSettings aStyleSettings2 = aSettings2.GetStyleSettings();
        aStyleSettings2.SetButtonTextColor(Color(0,0,0));
        //aStyleSettings2.SetLightColor(mColorAnkor);
        aStyleSettings2.SetCheckedColor(mColorLight); //hintergund
        //aStyleSettings2.SetLightBorderColor(mColorAnkor);
        aStyleSettings2.SetShadowColor(mColorAnkor);
        aStyleSettings2.SetFaceColor(mColorDark);
        aSettings2.SetStyleSettings(aStyleSettings2);
        mpVScrollbar->SetSettings(aSettings2);
    }
}

void SwMarginWin::SetMarginSide(bool aMarginSide)
{
    mbMarginSide = aMarginSide;
}

void SwMarginWin::SetReadonly(BOOL bSet)
{
    mbReadonly = bSet;
    View()->SetReadOnly(bSet);
}

void SwMarginWin::SetLanguage(const SvxLanguageItem aNewItem)
{
    Engine()->SetModifyHdl( Link() );
    ESelection aOld = View()->GetSelection();

    ESelection aNewSelection( 0, 0, (USHORT)Engine()->GetParagraphCount()-1, USHRT_MAX );
    View()->SetSelection( aNewSelection );
    SfxItemSet aEditAttr(View()->GetAttribs());
    aEditAttr.Put(aNewItem);
    View()->SetAttribs( aEditAttr );

    View()->SetSelection(aOld);
    Engine()->SetModifyHdl( LINK( this, SwPostIt, ModifyHdl ) );

    const SwViewOption* pVOpt = DocView()->GetWrtShellPtr()->GetViewOptions();
    ULONG nCntrl = Engine()->GetControlWord();
    // turn off
    if (!pVOpt->IsOnlineSpell())
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    Engine()->SetControlWord(nCntrl);

    //turn back on
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    Engine()->SetControlWord(nCntrl);

    Engine()->CompleteOnlineSpelling();
    Invalidate();
}

void SwMarginWin::DataChanged( const DataChangedEvent& aEvent)
{
    Window::DataChanged( aEvent );
}

void SwMarginWin::GetFocus()
{
    if (mpPostItTxt)
        mpPostItTxt->GrabFocus();
}

void SwMarginWin::LoseFocus()
{
}

void SwMarginWin::ShowNote()
{
    SetPosAndSize();
    if (!IsVisible())
        Window::Show();
    if (mpAnkor && !mpShadow->isVisible())
        mpShadow->setVisible(true);
    if (mpAnkor && !mpAnkor->isVisible())
        mpAnkor->setVisible(true);
}

void SwMarginWin::HideNote()
{
    if (IsVisible())
        Window::Hide();
    if (mpAnkor)
    {
        if (mpMgr->IsShowAnkor())
            mpAnkor->SetAnkorState(AS_TRI);
        else
            mpAnkor->setVisible(false);
    }
    if (mpShadow && mpShadow->isVisible())
        mpShadow->setVisible(false);
}

void SwMarginWin::ActivatePostIt()
{
    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();

    CheckMetaText();
    SetViewState(SS_EDIT);
    View()->ShowCursor();

    mpOutlinerView->GetEditView().SetInsertMode(mpView->GetWrtShellPtr()->IsInsMode());

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        View()->SetBackgroundColor(mColorDark);

    //  mpPostItTxt->SetBackground(Wallpaper(mColorDark));
}

void SwMarginWin::DeactivatePostIt()
{
    // remove selection, #i87073#
    if (View()->GetEditView().HasSelection())
    {
        ESelection aSelection = View()->GetEditView().GetSelection();
        aSelection.nEndPara = aSelection.nStartPara;
        aSelection.nEndPos = aSelection.nStartPos;
        View()->GetEditView().SetSelection(aSelection);
    }

    mpOutliner->CompleteOnlineSpelling();

    SetViewState(SS_NORMAL);
    // write the visible text back into the SwField
    UpdateData();

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        View()->SetBackgroundColor(COL_TRANSPARENT);

    //mpPostItTxt->SetBackground(Gradient(GRADIENT_LINEAR,mColorLight,mColorDark));

    if ( !IsProtected() &&
    //if (!IsReadOnly() && (GetStatus()!=SwPostItHelper::DELETED) &&
                Engine()->GetEditEngine().GetText()==String(EMPTYSTRING) )
        mnEventId = Application::PostUserEvent( LINK( this, SwMarginWin, DeleteHdl), 0 );
}

void SwMarginWin::ToggleInsMode()
{
    if (!mpView->GetWrtShell().IsRedlineOn())
    {
        //change outliner
        mpOutlinerView->GetEditView().SetInsertMode(!mpOutlinerView->GetEditView().IsInsertMode());
        //change documnet
        mpView->GetWrtShell().ToggleInsMode();
        //update statusbar
        SfxBindings &rBnd = mpView->GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_ATTR_INSERT);
        rBnd.Update(SID_ATTR_INSERT);
    }
}

void SwMarginWin::ExecuteCommand(USHORT nSlot)
{
    switch (nSlot)
    {
        case FN_POSTIT:
        case FN_REPLY:
        {
            // if this note is empty, it will be deleted once losing the focus, so no reply, but only a new note
            // will be created
            SwView* pView = DocView();
            if (Engine()->GetEditEngine().GetText() != String(EMPTYSTRING))
            {
                OutlinerParaObject* pPara = new OutlinerParaObject(*View()->GetEditView().CreateTextObject());
                Mgr()->RegisterAnswer(pPara);
            }
            if (Mgr()->GetActivePostIt())
                Mgr()->SetActivePostIt(0);
            SwitchToFieldPos();
            pView->GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT);
            break;
        }
        case FN_DELETE_COMMENT:
        case FN_DELETE_NOTE:

                //Delete(); // do not kill the parent of our open popup menu
                mnEventId = Application::PostUserEvent( LINK( this, SwMarginWin, DeleteHdl), 0 );
            break;
        /*
        case FN_HIDE_NOTE:
            if ( Mgr()->GetActivePostIt() == this )
            {
                Mgr()->SetActivePostIt(0);
                // put the cursor back into the document
                SwitchToFieldPos();
            }
            Mgr()->Hide(mpFld);
            break;
        */
        case FN_DELETE_ALL_NOTES:
        case FN_HIDE_ALL_NOTES:
            // not possible as slot as this would require that "this" is the active postit
            DocView()->GetViewFrame()->GetBindings().Execute( nSlot, 0, 0, SFX_CALLMODE_ASYNCHRON );
            break;
        case FN_DELETE_NOTE_AUTHOR:
        case FN_HIDE_NOTE_AUTHOR:
        {
            // not possible as slot as this would require that "this" is the active postit
            SfxStringItem aItem( nSlot, GetAuthor() );
            const SfxPoolItem* aItems[2];
            aItems[0] = &aItem;
            aItems[1] = 0;
            DocView()->GetViewFrame()->GetBindings().Execute( nSlot, aItems, 0, SFX_CALLMODE_ASYNCHRON );
        }
        default:
            mpView->GetViewFrame()->GetBindings().Execute( nSlot );
            break;
    }
}

bool SwMarginWin::CalcFollow()
{
    return false;
}

SwEditWin*  SwMarginWin::EditWin()
{
    return &mpView->GetEditWin();
}

String SwPostIt::GetAuthor()
{
    return mpFld->GetPar1();
}

Date SwPostIt::GetDate()
{
    return mpFld->GetDate();
}

Time SwPostIt::GetTime()
{
    return mpFld->GetTime();
}

long SwMarginWin::GetPostItTextHeight()
{
    return mpOutliner ? LogicToPixel(mpOutliner->CalcTextSize()).Height() : 0;
}

void SwMarginWin::SwitchToPostIt(USHORT aDirection)
{
    if (mpMgr)
    {
        SwMarginWin* pPostIt = mpMgr->GetNextPostIt(aDirection, this);
        if (pPostIt)
            pPostIt->GrabFocus();
    }
}

void SwMarginWin::MouseButtonDown( const MouseEvent& /*rMEvt */)
{
}

void SwMarginWin::MouseMove( const MouseEvent& rMEvt )
{
    if (mRectMetaButton.IsInside(PixelToLogic(rMEvt.GetPosPixel())))
    {
        if (!mbMouseOverButton)
        {
            Invalidate(mRectMetaButton);
            mbMouseOverButton = true;
        }
    }
    else
    {
        if (mbMouseOverButton)
        {
            Invalidate(mRectMetaButton);
            mbMouseOverButton = false;
        }
    }
}

void SwMarginWin::Delete()
{
    if ( Mgr()->GetActivePostIt() == this)
    {
        Mgr()->SetActivePostIt(0);
        // if the note is empty, the previous line will send a delete event, but we are already there
        if (mnEventId)
        {
            Application::RemoveUserEvent( mnEventId );
            mnEventId = 0;
        }
    }
}

IMPL_LINK(SwMarginWin, ScrollHdl, ScrollBar*, pScroll)
{
    long nDiff = View()->GetEditView().GetVisArea().Top() - pScroll->GetThumbPos();
    View()->Scroll( 0, nDiff );
    return 0;
}

IMPL_LINK(SwMarginWin, ModifyHdl, void*, pVoid)
{
    // no warnings, please
    pVoid=0;
    DocView()->GetDocShell()->SetModified(sal_True);
    return 0;
}

IMPL_LINK(SwMarginWin, DeleteHdl, void*, pVoid)
{
    // no warnings, please
    pVoid=0;
    mnEventId = 0;
    Delete();
    return 0;
}


void SwMarginWin::ResetAttributes()
{
    mpOutlinerView->RemoveAttribsKeepLanguages(TRUE);
    mpOutliner->RemoveFields(TRUE);
    SfxItemSet aSet( mpView->GetDocShell()->GetPool() );
    aSet.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
    mpOutlinerView->SetAttribs(aSet);
}

sal_Int32 SwMarginWin::GetScrollbarWidth()
{
    return mpView->GetWrtShell().GetViewOptions()->GetZoom() / 10;
}

sal_Int32 SwMarginWin::GetMetaButtonAreaWidth()
{
    const Fraction& f( GetMapMode().GetScaleX() );
    if (IsPreview())
        return 3 * METABUTTON_AREA_WIDTH * f.GetNumerator() / f.GetDenominator();
    else
        return METABUTTON_AREA_WIDTH * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwMarginWin::GetMetaHeight()
{
    const Fraction& f( GetMapMode().GetScaleY() );
    return POSTIT_META_HEIGHT * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwMarginWin::GetMinimumSizeWithMeta()
{
    return mpMgr->GetMinimumSizeWithMeta();
}

sal_Int32 SwMarginWin::GetMinimumSizeWithoutMeta()
{
    const Fraction& f( GetMapMode().GetScaleY() );
    return POSTIT_MINIMUMSIZE_WITHOUT_META * f.GetNumerator() / f.GetDenominator();
}

void SwMarginWin::SetSpellChecking()
{
    const SwViewOption* pVOpt = mpView->GetWrtShellPtr()->GetViewOptions();
    ULONG nCntrl = mpOutliner->GetControlWord();
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    mpOutliner->CompleteOnlineSpelling();
    Invalidate();
}

void SwMarginWin::SetViewState(ShadowState bState)
{
    switch (bState)
    {
        case SS_EDIT:
        {
            if (mpAnkor)
            {
                mpAnkor->SetAnkorState(AS_ALL);
                SwMarginWin* pWin = GetTopReplyNote();
                if (IsFollow() && pWin)
                    pWin->Ankor()->SetAnkorState(AS_END);
                mpAnkor->setLineSolid(true);
            }
            if (mpShadow)
                mpShadow->SetShadowState(bState);
            break;
        }
        case SS_VIEW:
        {
            if (mpAnkor)
                mpAnkor->setLineSolid(true);
            if (mpShadow)
                mpShadow->SetShadowState(bState);
            break;
        }
        case SS_NORMAL:
        {
            if (mpAnkor)
            {
                if (IsFollow())
                {
                    // if there is no visible parent note, we want to see the complete anchor ??
                    //if (IsAnyStackParentVisible())
                    mpAnkor->SetAnkorState(AS_END);
                    SwMarginWin* pTopWinSelf = GetTopReplyNote();
                    SwMarginWin* pTopWinActive = mpMgr->GetActivePostIt() ? mpMgr->GetActivePostIt()->GetTopReplyNote() : 0;
                    if (pTopWinSelf && (pTopWinSelf!=pTopWinActive))
                    {
                        if (pTopWinSelf!=mpMgr->GetActivePostIt())
                            pTopWinSelf->Ankor()->setLineSolid(false);
                        pTopWinSelf->Ankor()->SetAnkorState(AS_ALL);
                    }
                }
                mpAnkor->setLineSolid(false);
            }
            if (mpShadow)
                mpShadow->SetShadowState(bState);
            break;
        }
    }
}

bool SwMarginWin::IsAnyStackParentVisible()
{
    SwMarginWin* pMarginWin = mpMgr->GetNextPostIt(KEY_PAGEUP, this);
    while (pMarginWin)
    {
        if (pMarginWin->IsFollow())
        {
            pMarginWin = mpMgr->GetNextPostIt(KEY_PAGEUP, pMarginWin);
            if (pMarginWin && pMarginWin->IsVisible())
                return true;
        }
        else
            return pMarginWin && pMarginWin->IsVisible() ? true : false;
    }
    return false;
}

SwMarginWin* SwMarginWin::GetTopReplyNote()
{
    SwMarginWin* pTopNote = 0;
    SwMarginWin* pMarginWin = mpMgr->GetNextPostIt(KEY_PAGEUP, this);
    while (pMarginWin)
    {
        pTopNote = pMarginWin;
        pMarginWin = pMarginWin->IsFollow() ? mpMgr->GetNextPostIt(KEY_PAGEUP, pMarginWin) : 0;
    }
    return pTopNote;
}

void SwMarginWin::SwitchToFieldPos()
{
    if ( Mgr()->GetActivePostIt() == this )
            Mgr()->SetActivePostIt(0);
    GotoPos();
    sal_uInt32 aCount = MoveCaret();
    if (aCount)
        DocView()->GetDocShell()->GetWrtShell()->SwCrsrShell::Right(aCount, 0, FALSE);
    GrabFocusToDocument();
}

String SwMarginWin::GetAuthor()
{
    return String(EMPTYSTRING);
}

Date SwMarginWin::GetDate()
{
    return Date(0);
}

Time SwMarginWin::GetTime()
{
    return Time(0);
}

SvxLanguageItem SwMarginWin::GetLanguage(void)
{
    return SvxLanguageItem(SwLangHelper::GetLanguage(DocView()->GetWrtShell(),RES_CHRATR_LANGUAGE),RES_CHRATR_LANGUAGE);
}

/***** SwPostIt *********************************************/
SwPostIt::SwPostIt( Window* pParent, WinBits nBits, SwFmtFld* aField,SwPostItMgr* aMgr,SwPostItBits aBits)
    : SwMarginWin(pParent,nBits,aMgr,aBits),
    mpFmtFld(aField),
    mpFld( static_cast<SwPostItField*>(aField->GetFld())),
    mStatus(SwPostItHelper::INVISIBLE)
{
}

void SwPostIt::SetPostItText()
{
    // get text from SwPostItField and insert into our textview
    Engine()->SetModifyHdl( Link() );
    Engine()->EnableUndo( FALSE );
    mpFld = static_cast<SwPostItField*>(mpFmtFld->GetFld());
    if( mpFld->GetTextObject() )
        Engine()->SetText( *mpFld->GetTextObject() );
    else
    {
        Engine()->Clear();
        SfxItemSet item( DocView()->GetDocShell()->GetPool() );
        item.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
        item.Put(SvxFontItem(FAMILY_SWISS,GetSettings().GetStyleSettings().GetFieldFont().GetName(),
                            EMPTYSTRING,PITCH_DONTKNOW,RTL_TEXTENCODING_DONTKNOW,EE_CHAR_FONTINFO));
        View()->SetAttribs(item);
        View()->InsertText(mpFld->GetPar2(),false);
    }

    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
    Engine()->EnableUndo( TRUE );
    Engine()->SetModifyHdl( LINK( this, SwPostIt, ModifyHdl ) );
    Invalidate();
}

void SwPostIt::UpdateData()
{
    if ( Engine()->IsModified() )
    {
        SwPosition * pPos = mpFmtFld->GetTxtFld()->GetPosition();
        if ( pPos )
        {
            SwField* pOldField = mpFld->Copy();
              mpFld->SetPar2(Engine()->GetEditEngine().GetText());
            mpFld->SetTextObject(Engine()->CreateParaObject());
            DocView()->GetDocShell()->GetDoc()->AppendUndo(new SwUndoFieldFromDoc(*pPos, *pOldField, *mpFld, 0, true));
            delete pOldField;
            delete pPos;
            // so we get a new layout of notes (ankor position is still the same and we would otherwise not get one)
            Mgr()->SetLayout();
            DocView()->GetDocShell()->SetModified();
        }
    }
    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
}

void SwPostIt::SetChangeTracking(SwPostItHelper::SwLayoutStatus& aStatus,Color aColor)
{
    if ( (mStatus != aStatus) || (mChangeColor != aColor) )
    {
        mStatus = aStatus;
        mChangeColor = aColor;
        Invalidate();
    }
}

void SwPostIt::Delete()
{
    SwMarginWin::Delete();
    // we delete the field directly, the Mgr cleans up the PostIt by listening
    DocView()->GetWrtShellPtr()->GotoField(*mpFmtFld);
    GrabFocusToDocument();
    DocView()->GetWrtShellPtr()->DelRight();
}

void SwPostIt::GotoPos()
{
    DocView()->GetDocShell()->GetWrtShell()->GotoField(*mpFmtFld);
}

sal_uInt32 SwPostIt::MoveCaret()
{
    // if this is an answer, do not skip over all following ones, but insert directly behind the current one
    // but when just leaving a note, skip all following ones as well to continue typing
    return Mgr()->IsAnswer() ? 1 : 1 + CountFollowing();
}

//returns true, if there is another note right before this note
bool SwPostIt::CalcFollow()
{
    SwPosition * pPos = mpFmtFld->GetTxtFld()->GetPosition();
    const SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
    SwTxtAttr* pTxtAttr = pTxtNd ? pTxtNd->GetTxtAttr( pPos->nContent.GetIndex()-1,RES_TXTATR_FIELD ) : 0;
    const SwField* pFld = pTxtAttr ? pTxtAttr->GetFld().GetFld() : 0;
    delete pPos;
    return pFld && (pFld->Which()== RES_POSTITFLD);
}

// counts how many SwPostItField we have right after the current one
sal_uInt32 SwPostIt::CountFollowing()
{
    sal_uInt32 aCount = 1;  // we start with 1, so we have to subtract one at the end again
    SwPosition * pPos = mpFmtFld->GetTxtFld()->GetPosition();
    const SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();

    SwTxtAttr* pTxtAttr = pTxtNd ? pTxtNd->GetTxtAttr( pPos->nContent.GetIndex()+1,RES_TXTATR_FIELD ) : 0;
    SwField* pFld = pTxtAttr ? const_cast<SwField*>(pTxtAttr->GetFld().GetFld()) : 0;
    while (pFld && (pFld->Which()== RES_POSTITFLD))
    {
        aCount++;
        pTxtAttr = pTxtNd ? pTxtNd->GetTxtAttr( pPos->nContent.GetIndex() + aCount,RES_TXTATR_FIELD ) : 0;
        pFld = pTxtAttr ? const_cast<SwField*>(pTxtAttr->GetFld().GetFld()) : 0;
    }
    delete pPos;
    return aCount - 1;
}

void SwPostIt::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (mRectMetaButton.IsInside(PixelToLogic(rMEvt.GetPosPixel())) && rMEvt.IsLeft())
    {
        if (IsPreview())
        {
            doLazyDelete();
            if (Mgr())
            {
                SwPostIt* pPostIt = dynamic_cast<SwPostIt*>(Mgr()->GetPostIt(mpFmtFld));
                if (pPostIt)
                {
                    pPostIt->GrabFocus();
                    Mgr()->MakeVisible(pPostIt);
                }
            }

        }
        else
        {
            if ( mbReadonly )
            {
                mpButtonPopup->EnableItem(FN_REPLY,false);
                mpButtonPopup->EnableItem(FN_DELETE_NOTE,false);
                mpButtonPopup->EnableItem(FN_DELETE_NOTE_AUTHOR,false);
                mpButtonPopup->EnableItem(FN_DELETE_ALL_NOTES,false);
            }
            else
            {
                if (IsProtected())
                    mpButtonPopup->EnableItem(FN_DELETE_NOTE,false);
                else
                    mpButtonPopup->EnableItem(FN_DELETE_NOTE,true);
                mpButtonPopup->EnableItem(FN_DELETE_NOTE_AUTHOR,true);
                mpButtonPopup->EnableItem(FN_DELETE_ALL_NOTES,true);
            }
            SvtUserOptions aUserOpt;
            String sAuthor;
            if( !(sAuthor = aUserOpt.GetFullName()).Len())
                if( !(sAuthor = aUserOpt.GetID()).Len() )
                    sAuthor = String( SW_RES( STR_REDLINE_UNKNOWN_AUTHOR ));
            // do not allow to reply to ourself and no answer possible if this note is in a protected section
            if ((sAuthor == GetAuthor()) || (IsProtected()))
                mpButtonPopup->EnableItem(FN_REPLY,false);
            else
                mpButtonPopup->EnableItem(FN_REPLY,true);

            // show the popup menu and execute the selected command
            ExecuteCommand( mpButtonPopup->Execute( this,Rectangle(LogicToPixel(mRectMetaButton.BottomLeft()),LogicToPixel(mRectMetaButton.BottomLeft())),POPUPMENU_EXECUTE_DOWN | POPUPMENU_NOMOUSEUPCLOSE) );
        }
    }
}

void SwPostIt::SetPopup()
{
    mpButtonPopup = new PopupMenu(SW_RES(MN_ANNOTATION_BUTTON));
    //mpButtonPopup->SetMenuFlags(MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES);
    XubString aText = mpButtonPopup->GetItemText( FN_DELETE_NOTE_AUTHOR );
    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1,GetAuthor());
    aText = aRewriter.Apply(aText);
    mpButtonPopup->SetItemText(FN_DELETE_NOTE_AUTHOR,aText);
}

void SwPostIt::InitAnswer(OutlinerParaObject* pText)
{
    //collect our old meta data
    SwMarginWin* pWin = Mgr()->GetNextPostIt(KEY_PAGEUP, this);
    const LocaleDataWrapper& rLocalData = SvtSysLocale().GetLocaleData();
    String aText = String(SW_RES(STR_REPLY));
           SwRewriter aRewriter;
        aRewriter.AddRule(UNDO_ARG1, pWin->GetAuthor());
           aText = aRewriter.Apply(aText);
           aText.Append(String(rtl::OUString::createFromAscii(" (") +
        String(rLocalData.getDate( pWin->GetDate())) + rtl::OUString::createFromAscii(", ") +
        String(rLocalData.getTime( pWin->GetTime(),false)) + rtl::OUString::createFromAscii("): \"")));
    View()->InsertText(aText,false);

    // insert old, selected text or "..."
    // TOOD: iterate over all paragraphs, not only first one to find out if it is empty
    if (pText->GetTextObject().GetText(0) != String(EMPTYSTRING))
        View()->GetEditView().InsertText(pText->GetTextObject());
    else
        View()->InsertText(rtl::OUString::createFromAscii("..."),false);
    View()->InsertText(rtl::OUString::createFromAscii("\"\n"),false);

    View()->SetSelection(ESelection(0x0,0x0,0xFFFF,0xFFFF));
    SfxItemSet aAnswerSet( DocView()->GetDocShell()->GetPool() );
    aAnswerSet.Put(SvxFontHeightItem(200,80,EE_CHAR_FONTHEIGHT));
    aAnswerSet.Put(SvxPostureItem(ITALIC_NORMAL,EE_CHAR_ITALIC));
    View()->SetAttribs(aAnswerSet);
    View()->SetSelection(ESelection(0xFFFF,0xFFFF,0xFFFF,0xFFFF));

    //remove all attributes and reset our standard ones
    View()->GetEditView().RemoveAttribsKeepLanguages(true);
    SfxItemSet aNormalSet( DocView()->GetDocShell()->GetPool() );
    aNormalSet.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
    aNormalSet.Put(SvxFontItem(FAMILY_SWISS,GetSettings().GetStyleSettings().GetFieldFont().GetName(),
                            EMPTYSTRING,PITCH_DONTKNOW,RTL_TEXTENCODING_DONTKNOW,EE_CHAR_FONTINFO));
    View()->SetAttribs(aNormalSet);
    // lets insert an undo step so the initial text can be easily deleted
    // but do not use UpdateData() directly, would set modified state again and reentrance into Mgr
    Engine()->SetModifyHdl( Link() );
    SwPosition * pPos = mpFmtFld->GetTxtFld()->GetPosition();
    if ( pPos )
    {
        SwField* pOldField = mpFld->Copy();
        mpFld->SetPar2(Engine()->GetEditEngine().GetText());
        mpFld->SetTextObject(Engine()->CreateParaObject());
        DocView()->GetDocShell()->GetDoc()->AppendUndo(new SwUndoFieldFromDoc(*pPos, *pOldField, *mpFld, 0, true));
        delete pOldField;
        delete pPos;
    }
    Engine()->SetModifyHdl( LINK( this, SwPostIt, ModifyHdl ) );
    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
}

SvxLanguageItem SwPostIt::GetLanguage(void)
{
    // set initial language for outliner
    USHORT nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( mpFld->GetLanguage() );
    USHORT nLangWhichId = 0;
    switch (nScriptType)
    {
        case SCRIPTTYPE_LATIN :    nLangWhichId = EE_CHAR_LANGUAGE ; break;
        case SCRIPTTYPE_ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
        case SCRIPTTYPE_COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
        default: DBG_ERROR("GetLanguage: wrong script tye");
    }
    return SvxLanguageItem(mpFld->GetLanguage(),nLangWhichId);
}

bool SwPostIt::IsProtected()
{
    bool aReturn;
    aReturn = mpFmtFld ? (SwMarginWin::IsProtected() || (mStatus==SwPostItHelper::DELETED) ||
        mpFmtFld->IsProtect()) : (SwMarginWin::IsProtected() || (mStatus==SwPostItHelper::DELETED));
    return aReturn;
}

/********** SwRedComment**************/
/*
SwRedComment::SwRedComment( Window* pParent, WinBits nBits,SwPostItMgr* aMgr,SwPostItBits aBits,SwRedline* pRed)
    : SwMarginWin(pParent,nBits,aMgr,aBits),
    pRedline(pRed)
{
}

void SwRedComment::SetPopup()
{
    mpButtonPopup = new PopupMenu(SW_RES(MN_REDCOMMENT_BUTTON));
    //mpButtonPopup->SetMenuFlags(MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES);
}

void SwRedComment::UpdateData()
{
    if ( Engine()->IsModified() )
    {
        // so we get a new layout of notes (ankor position is still the same and we would otherwise not get one)
        Mgr()->SetLayout();
        // SetRedline is calling SetModified already
        DocView()->GetWrtShell().SetRedlineComment(Engine()->GetEditEngine().GetText());
    }
    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
}

void SwRedComment::SetPostItText()
{
    Engine()->SetModifyHdl( Link() );
    Engine()->EnableUndo( FALSE );

    Engine()->Clear();
    SfxItemSet item( DocView()->GetDocShell()->GetPool() );
    item.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
    item.Put(SvxFontItem(FAMILY_SWISS,GetSettings().GetStyleSettings().GetFieldFont().GetName(),
                        EMPTYSTRING,PITCH_DONTKNOW,RTL_TEXTENCODING_DONTKNOW,EE_CHAR_FONTINFO));
    View()->SetAttribs(item);
    View()->InsertText(pRedline->GetComment(),false);

    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
    Engine()->EnableUndo( TRUE );
    Engine()->SetModifyHdl( LINK( this, SwMarginWin, ModifyHdl ) );
    Invalidate();
}

void SwRedComment::DeactivatePostIt()
{
    SwMarginWin::DeactivatePostIt();
    // current Redline is still selected
    DocView()->GetWrtShellPtr()->ClearMark();
}

void SwRedComment::ActivatePostIt()
{
    SwMarginWin::ActivatePostIt();

    // do we want the redline selected?
    // otherwise, SwRedComment::ActivatePostIt() as well as SwRedComment::DeactivatePostIt()
    // can be thrown out completly
    DocView()->GetDocShell()->GetWrtShell()->GotoRedline(
        DocView()->GetDocShell()->GetWrtShell()->FindRedlineOfData(pRedline->GetRedlineData()),true);
}

void SwRedComment::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (mRectMetaButton.IsInside(PixelToLogic(rMEvt.GetPosPixel())) && rMEvt.IsLeft())
    {
        ExecuteCommand( mpButtonPopup->Execute( this,Rectangle(LogicToPixel(mRectMetaButton.BottomLeft()),LogicToPixel(mRectMetaButton.BottomLeft())),POPUPMENU_EXECUTE_DOWN | POPUPMENU_NOMOUSEUPCLOSE) );
    }
}

void SwRedComment::Delete()
{
    SwMarginWin::Delete();
    // we are not neccessarily on our redline, so let's move there
    GotoPos();
    DocView()->GetWrtShell().SetRedlineComment(EMPTYSTRING);
    DocView()->GetWrtShell().ClearMark();
    // so we get a new layout of notes (ankor position is still the same and we would otherwise not get one)
    Mgr()->SetLayout();
    Mgr()->RemoveItem(pRedline);
}

void SwRedComment::GotoPos()
{
    DocView()->GetDocShell()->GetWrtShell()->GotoRedline(
        DocView()->GetDocShell()->GetWrtShell()->FindRedlineOfData(pRedline->GetRedlineData()));
}

String SwRedComment::GetAuthor()
{
    return pRedline->GetAuthorString();
}

Date SwRedComment::GetDate()
{
    return pRedline->GetTimeStamp().GetDate();
}

Time SwRedComment::GetTime()
{
    return pRedline->GetTimeStamp().GetTime();
}

bool SwRedComment::IsProtected()
{
    return SwMarginWin::IsProtected() || pRedline->Start()->nNode.GetNode().GetTxtNode()->IsInProtectSect();
}
*/

//////////////////////////////////////////////////////////////////////////////
// helper SwPostItShadowPrimitive
//
// Used to allow view-dependent primitive definition. For that purpose, the
// initially created primitive (this one) always has to be view-independent,
// but the decomposition is made view-dependent. Very simple primitive which
// just remembers the discrete data and applies it at decomposition time.

class SwPostItShadowPrimitive : public drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D
{
private:
    basegfx::B2DPoint           maBasePosition;
    basegfx::B2DPoint           maSecondPosition;
    ShadowState                 maShadowState;

protected:
    virtual drawinglayer::primitive2d::Primitive2DSequence createLocalDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    SwPostItShadowPrimitive(
        const basegfx::B2DPoint& rBasePosition,
        const basegfx::B2DPoint& rSecondPosition,
        ShadowState aShadowState)
    :   drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D(),
        maBasePosition(rBasePosition),
        maSecondPosition(rSecondPosition),
        maShadowState(aShadowState)
    {}

    // data access
    const basegfx::B2DPoint& getBasePosition() const { return maBasePosition; }
    const basegfx::B2DPoint& getSecondPosition() const { return maSecondPosition; }
    ShadowState getShadowState() const { return maShadowState; }

    virtual bool operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const;

    DeclPrimitrive2DIDBlock()
};

drawinglayer::primitive2d::Primitive2DSequence SwPostItShadowPrimitive::createLocalDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // get logic sizes in object coordinate system
    drawinglayer::primitive2d::Primitive2DSequence xRetval;
    basegfx::B2DRange aRange(getBasePosition());

    switch(maShadowState)
    {
        case SS_NORMAL:
        {
            aRange.expand(basegfx::B2DTuple(getSecondPosition().getX(), getSecondPosition().getY() + (2.0 * getDiscreteUnit())));
            const drawinglayer::attribute::FillGradientAttribute aFillGradientAttribute(
                drawinglayer::attribute::GRADIENTSTYLE_LINEAR,
                0.0,
                0.5,
                0.5,
                1800.0 * F_PI1800,
                basegfx::BColor(230.0/255.0,230.0/255.0,230.0/255.0),
                basegfx::BColor(180.0/255.0,180.0/255.0,180.0/255.0),
                2);

            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::FillGradientPrimitive2D(
                    aRange,
                    aFillGradientAttribute));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            break;
        }
        case SS_VIEW:
        {
            aRange.expand(basegfx::B2DTuple(getSecondPosition().getX(), getSecondPosition().getY() + (4.0 * getDiscreteUnit())));
            const drawinglayer::attribute::FillGradientAttribute aFillGradientAttribute(
                drawinglayer::attribute::GRADIENTSTYLE_LINEAR,
                0.0,
                0.5,
                0.5,
                1800.0 * F_PI1800,
                basegfx::BColor(230.0/255.0,230.0/255.0,230.0/255.0),
                basegfx::BColor(180.0/255.0,180.0/255.0,180.0/255.0),
                4);

            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::FillGradientPrimitive2D(
                    aRange,
                    aFillGradientAttribute));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            break;
        }
        case SS_EDIT:
        {
            aRange.expand(basegfx::B2DTuple(getSecondPosition().getX(), getSecondPosition().getY() + (4.0 * getDiscreteUnit())));
            const drawinglayer::attribute::FillGradientAttribute aFillGradientAttribute(
                drawinglayer::attribute::GRADIENTSTYLE_LINEAR,
                0.0,
                0.5,
                0.5,
                1800.0 * F_PI1800,
                basegfx::BColor(230.0/255.0,230.0/255.0,230.0/255.0),
                basegfx::BColor(83.0/255.0,83.0/255.0,83.0/255.0),
                4);

            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::FillGradientPrimitive2D(
                    aRange,
                    aFillGradientAttribute));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            break;
        }
        default:
        {
            break;
        }
    }

    return xRetval;
}

bool SwPostItShadowPrimitive::operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const
{
    if(drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
    {
        const SwPostItShadowPrimitive& rCompare = static_cast< const SwPostItShadowPrimitive& >(rPrimitive);

        return (getBasePosition() == rCompare.getBasePosition()
            && getSecondPosition() == rCompare.getSecondPosition()
            && getShadowState() == rCompare.getShadowState());
    }

    return false;
}

ImplPrimitrive2DIDBlock(SwPostItShadowPrimitive, PRIMITIVE2D_ID_SWPOSTITSHADOWPRIMITIVE)

/****** SwPostItShadow  ***********************************************************/
SwPostItShadow::SwPostItShadow(const basegfx::B2DPoint& rBasePos,const basegfx::B2DPoint& rSecondPosition,
                                   Color aBaseColor,ShadowState aState)
        :   OverlayObjectWithBasePosition(rBasePos, aBaseColor),
            maSecondPosition(rSecondPosition),
            mShadowState(aState)
{
//  mbAllowsAnimation = false;
}

SwPostItShadow::~SwPostItShadow()
{
}

drawinglayer::primitive2d::Primitive2DSequence SwPostItShadow::createOverlayObjectPrimitive2DSequence()
{
    const drawinglayer::primitive2d::Primitive2DReference aReference(
        new SwPostItShadowPrimitive(
            getBasePosition(),
            GetSecondPosition(),
            GetShadowState()));
    return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
}

void SwPostItShadow::SetShadowState(ShadowState aState)
{
    if (mShadowState != aState)
    {
        mShadowState = aState;

        objectChange();
    }
}

void SwPostItShadow::SetPosition(const basegfx::B2DPoint& rPoint1,
                                const basegfx::B2DPoint& rPoint2)
{
    if(!rPoint1.equal(getBasePosition()) || !rPoint2.equal(GetSecondPosition()))
    {
        maBasePosition = rPoint1;
        maSecondPosition = rPoint2;

        objectChange();
    }
}

//////////////////////////////////////////////////////////////////////////////
// helper class: Primitive for discrete visualisation

class SwPostItAnkorPrimitive : public drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D
{
private:
    basegfx::B2DPolygon             maTriangle;
    basegfx::B2DPolygon             maLine;
    basegfx::B2DPolygon             maLineTop;
    AnkorState                      maAnkorState;
    basegfx::BColor                 maColor;

    // discrete line width
    double                          mfLogicLineWidth;

    // bitfield
    bool                            mbShadow : 1;
    bool                            mbLineSolid : 1;

protected:
    virtual drawinglayer::primitive2d::Primitive2DSequence createLocalDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    SwPostItAnkorPrimitive(
        const basegfx::B2DPolygon& rTriangle,
        const basegfx::B2DPolygon& rLine,
        const basegfx::B2DPolygon& rLineTop,
        AnkorState aAnkorState,
        const basegfx::BColor& rColor,
        double fLogicLineWidth,
        bool bShadow,
        bool bLineSolid)
    :   drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D(),
        maTriangle(rTriangle),
        maLine(rLine),
        maLineTop(rLineTop),
        maAnkorState(aAnkorState),
        maColor(rColor),
        mfLogicLineWidth(fLogicLineWidth),
        mbShadow(bShadow),
        mbLineSolid(bLineSolid)
    {}

    // data access
    const basegfx::B2DPolygon& getTriangle() const { return maTriangle; }
    const basegfx::B2DPolygon& getLine() const { return maLine; }
    const basegfx::B2DPolygon& getLineTop() const { return maLineTop; }
    AnkorState getAnkorState() const { return maAnkorState; }
    const basegfx::BColor& getColor() const { return maColor; }
    double getLogicLineWidth() const { return mfLogicLineWidth; }
    bool getShadow() const { return mbShadow; }
    bool getLineSolid() const { return mbLineSolid; }

    virtual bool operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const;

    DeclPrimitrive2DIDBlock()
};

drawinglayer::primitive2d::Primitive2DSequence SwPostItAnkorPrimitive::createLocalDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;

    if(AS_TRI == getAnkorState() || AS_ALL == getAnkorState() || AS_START == getAnkorState())
    {
        // create triangle
        const drawinglayer::primitive2d::Primitive2DReference aTriangle(
            new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                basegfx::B2DPolyPolygon(getTriangle()),
                getColor()));

        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aTriangle);
    }

    if(AS_ALL == getAnkorState() || AS_START == getAnkorState())
    {
        // create line start
        const drawinglayer::attribute::LineAttribute aLineAttribute(
            getColor(),
            getLogicLineWidth() / (basegfx::fTools::equalZero(getDiscreteUnit()) ? 1.0 : getDiscreteUnit()));

        if(getLineSolid())
        {
            const drawinglayer::primitive2d::Primitive2DReference aSolidLine(
                new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                    getLine(),
                    aLineAttribute));

            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aSolidLine);
        }
        else
        {
            ::std::vector< double > aDotDashArray;
            const double fDistance(3.0 * 15.0);
            const double fDashLen(5.0 * 15.0);

            aDotDashArray.push_back(fDashLen);
            aDotDashArray.push_back(fDistance);

            const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(
                aDotDashArray,
                fDistance + fDashLen);

            const drawinglayer::primitive2d::Primitive2DReference aStrokedLine(
                new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                    getLine(),
                    aLineAttribute,
                    aStrokeAttribute));

            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aStrokedLine);
        }
    }

    if(aRetval.hasElements() && getShadow())
    {
        // shadow is only for triangle and line start, and in upper left
        // and lower right direction, in different colors
        const double fColorChange(20.0 / 255.0);
        const basegfx::B3DTuple aColorChange(fColorChange, fColorChange, fColorChange);
        basegfx::BColor aLighterColor(getColor() + aColorChange);
        basegfx::BColor aDarkerColor(getColor() - aColorChange);

        aLighterColor.clamp();
        aDarkerColor.clamp();

        // create shadow sequence
        drawinglayer::primitive2d::Primitive2DSequence aShadows(2);
        basegfx::B2DHomMatrix aTransform;

        aTransform.set(0, 2, -getDiscreteUnit());
        aTransform.set(1, 2, -getDiscreteUnit());

        aShadows[0] = drawinglayer::primitive2d::Primitive2DReference(
            new drawinglayer::primitive2d::ShadowPrimitive2D(
                aTransform,
                aLighterColor,
                aRetval));

        aTransform.set(0, 2, getDiscreteUnit());
        aTransform.set(1, 2, getDiscreteUnit());

        aShadows[1] = drawinglayer::primitive2d::Primitive2DReference(
            new drawinglayer::primitive2d::ShadowPrimitive2D(
                aTransform,
                aDarkerColor,
                aRetval));

        // add shadow before geometry to make it be proccessed first
        const drawinglayer::primitive2d::Primitive2DSequence aTemporary(aRetval);

        aRetval = aShadows;
        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, aTemporary);
    }

    if(AS_ALL == getAnkorState() || AS_END == getAnkorState())
    {
        // LineTop has to be created, too, but uses no shadow, so add after
        // the other parts are created
        const drawinglayer::attribute::LineAttribute aLineAttribute(
            getColor(),
            getLogicLineWidth() / (basegfx::fTools::equalZero(getDiscreteUnit()) ? 1.0 : getDiscreteUnit()));

        const drawinglayer::primitive2d::Primitive2DReference aLineTop(
            new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                getLineTop(),
                aLineAttribute));

        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aLineTop);
    }

    return aRetval;
}

bool SwPostItAnkorPrimitive::operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const
{
    if(drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
    {
        const SwPostItAnkorPrimitive& rCompare = static_cast< const SwPostItAnkorPrimitive& >(rPrimitive);

        return (getTriangle() == rCompare.getTriangle()
            && getLine() == rCompare.getLine()
            && getLineTop() == rCompare.getLineTop()
            && getAnkorState() == rCompare.getAnkorState()
            && getColor() == rCompare.getColor()
            && getLogicLineWidth() == rCompare.getLogicLineWidth()
            && getShadow() == rCompare.getShadow()
            && getLineSolid() == rCompare.getLineSolid());
    }

    return false;
}

ImplPrimitrive2DIDBlock(SwPostItAnkorPrimitive, PRIMITIVE2D_ID_SWPOSTITANKORPRIMITIVE)

/****** SwPostItAnkor   ***********************************************************/

void SwPostItAnkor::implEnsureGeometry()
{
    if(!maTriangle.count())
    {
        maTriangle.append(getBasePosition());
        maTriangle.append(GetSecondPosition());
        maTriangle.append(GetThirdPosition());
        maTriangle.setClosed(true);
    }

    if(!maLine.count())
    {
        maLine.append(GetFourthPosition());
        maLine.append(GetFifthPosition());
        maLine.append(GetSixthPosition());
    }

    if(!maLineTop.count())
    {
        maLineTop.append(GetSixthPosition());
        maLineTop.append(GetSeventhPosition());
    }
}

void SwPostItAnkor::implResetGeometry()
{
    maTriangle.clear();
    maLine.clear();
    maLineTop.clear();
}

SwPostItAnkor::SwPostItAnkor(const basegfx::B2DPoint& rBasePos,
                        const basegfx::B2DPoint& rSecondPos,
                        const basegfx::B2DPoint& rThirdPos,
                        const basegfx::B2DPoint& rFourthPos,
                        const basegfx::B2DPoint& rFifthPos,
                        const basegfx::B2DPoint& rSixthPos,
                        const basegfx::B2DPoint& rSeventhPos,
                        Color aBaseColor,
                        bool bShadowedEffect,
                        bool bLineSolid)
        :   OverlayObjectWithBasePosition(rBasePos, aBaseColor),
            maSecondPosition(rSecondPos),
            maThirdPosition(rThirdPos),
            maFourthPosition(rFourthPos),
            maFifthPosition(rFifthPos),
            maSixthPosition(rSixthPos),
            maSeventhPosition(rSeventhPos),
            maTriangle(),
            maLine(),
            maLineTop(),
            mHeight(0),
            mAnkorState(AS_ALL),
            mbShadowedEffect(bShadowedEffect),
            mbLineSolid(bLineSolid)
{
    //mbAllowsAnimation = true;
}

SwPostItAnkor::~SwPostItAnkor()
{
}

drawinglayer::primitive2d::Primitive2DSequence SwPostItAnkor::createOverlayObjectPrimitive2DSequence()
{
    implEnsureGeometry();

    const drawinglayer::primitive2d::Primitive2DReference aReference(
        new SwPostItAnkorPrimitive(
            maTriangle,
            maLine,
            maLineTop,
            GetAnkorState(),
            getBaseColor().getBColor(),
            ANKORLINE_WIDTH * 15.0,
            getShadowedEffect(),
            getLineSolid()));

    return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
}

void SwPostItAnkor::SetAllPosition(const basegfx::B2DPoint& rPoint1,
            const basegfx::B2DPoint& rPoint2,
            const basegfx::B2DPoint& rPoint3,
            const basegfx::B2DPoint& rPoint4,
            const basegfx::B2DPoint& rPoint5,
            const basegfx::B2DPoint& rPoint6,
            const basegfx::B2DPoint& rPoint7)
{
    if(rPoint1 != getBasePosition()
        || rPoint2 != GetSecondPosition()
        || rPoint3 != GetThirdPosition()
        || rPoint4 != GetFourthPosition()
        || rPoint5 != GetFifthPosition()
        || rPoint6 != GetSixthPosition()
        || rPoint7 != GetSeventhPosition())
    {
        maBasePosition = rPoint1;
        maSecondPosition = rPoint2;
        maThirdPosition = rPoint3;
        maFourthPosition = rPoint4;
        maFifthPosition = rPoint5;
        maSixthPosition = rPoint6;
        maSeventhPosition = rPoint7;

        implResetGeometry();
        objectChange();
    }
}

void SwPostItAnkor::SetSixthPosition(const basegfx::B2DPoint& rNew)
{
    if(rNew != maSixthPosition)
    {
        maSixthPosition = rNew;
        implResetGeometry();
        objectChange();
    }
}

void SwPostItAnkor::SetSeventhPosition(const basegfx::B2DPoint& rNew)
{
    if(rNew != maSeventhPosition)
    {
        maSeventhPosition = rNew;
        implResetGeometry();
        objectChange();
    }
}

void SwPostItAnkor::SetTriPosition(const basegfx::B2DPoint& rPoint1,const basegfx::B2DPoint& rPoint2,const basegfx::B2DPoint& rPoint3,
                                    const basegfx::B2DPoint& rPoint4,const basegfx::B2DPoint& rPoint5)
{
    if(rPoint1 != getBasePosition()
        || rPoint2 != GetSecondPosition()
        || rPoint3 != GetThirdPosition()
        || rPoint4 != GetFourthPosition()
        || rPoint5 != GetFifthPosition())
    {
        maBasePosition = rPoint1;
        maSecondPosition = rPoint2;
        maThirdPosition = rPoint3;
        maFourthPosition = rPoint4;
        maFifthPosition = rPoint5;

        implResetGeometry();
        objectChange();
    }
}

void SwPostItAnkor::setLineSolid(bool bNew)
{
    if(bNew != getLineSolid())
    {
        mbLineSolid = bNew;
        objectChange();
    }
}

void SwPostItAnkor::SetAnkorState(AnkorState aState)
{
    if (mAnkorState != aState)
    {
        mAnkorState = aState;
        objectChange();
    }
}
