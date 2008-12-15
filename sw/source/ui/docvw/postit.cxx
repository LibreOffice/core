/************************************************************************* *
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postit.cxx,v $
 * $Revision: 1.10 $
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

#include <hintids.hxx>
#include "viewopt.hxx"
#include "cmdid.h"

#include <vcl/help.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
#include <vcl/gradient.hxx>
#include <tools/poly.hxx>   // Polygon
#include <vcl/salbtype.hxx> // FRound

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
#include <svx/unolingu.hxx>

#include <svtools/languageoptions.hxx>
#include <svtools/langtab.hxx>
#include <svtools/slstitm.hxx>
#include <svtools/securityoptions.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/svmedit.hxx>

#include <linguistic/lngprops.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/mnumgr.hxx>

#include <vcl/vclenum.hxx>
#include <vcl/edit.hxx>

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
#include <doc.hxx>
#include <txtfld.hxx>
#include <redline.hxx>
#include <uitool.hxx>
#include <SwUndoField.hxx>
#include <editsh.hxx>

using namespace ::com::sun::star;

#define ANKORLINE_WIDTH         1
#define METABUTTON_WIDTH        16
#define METABUTTON_HEIGHT       18
#define METABUTTON_AREA_WIDTH   30
#define POSTIT_META_HEIGHT  (sal_Int32)     30
#define POSTIT_MINIMUMSIZE_WITHOUT_META     50

#define POSTIT_SHADOW_BRIGHT    Color(180,180,180)
#define POSTIT_SHADOW_DARK      Color(83,83,83)

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
    mpPostIt = static_cast<SwPostIt*>(GetParent());
}

PostItTxt::~PostItTxt()
{
    RemoveEventListener( LINK( this, PostItTxt, WindowEventListener ) );
}

void PostItTxt::GetFocus()
{
    BOOL bLockView = mpPostIt->DocView()->GetWrtShell().IsViewLocked();
    mpPostIt->DocView()->GetWrtShell().LockView( TRUE );

    if(mpPostIt && !mpPostIt->IsPreview())
         mpPostIt->Mgr()->SetActivePostIt(mpPostIt);
    Window::GetFocus();
    if (!mMouseOver)
        Invalidate();

    mpPostIt->DocView()->GetWrtShell().LockView( bLockView );
    mpPostIt->Mgr()->MakeVisible(mpPostIt);
}

void PostItTxt::LoseFocus()
{
    // write the visible text back into the SwField
    if ( mpPostIt )
        mpPostIt->UpdateData();

    Window::LoseFocus();
    if (!mMouseOver)
        Invalidate();
}

void PostItTxt::RequestHelp(const HelpEvent &rEvt)
{
    USHORT nResId = 0;
    switch( mpPostIt->GetStatus() )
    {
        case SwPostItHelper::INSERTED:  nResId = STR_REDLINE_INSERT; break;
        case SwPostItHelper::DELETED:   nResId = STR_REDLINE_DELETE; break;
        default: nResId = 0;
    }

    SwContentAtPos aCntntAtPos( SwContentAtPos::SW_REDLINE );
    if ( nResId && mpPostIt->DocView()->GetWrtShell().GetContentAtPos( mpPostIt->GetAnkorRect().Pos(), aCntntAtPos ) )
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
                Gradient(GRADIENT_LINEAR,mpPostIt->ColorDark(),mpPostIt->ColorDark()));
        else
            DrawGradient(Rectangle(Point(0,0),PixelToLogic(GetSizePixel())),
                Gradient(GRADIENT_LINEAR,mpPostIt->ColorLight(),mpPostIt->ColorDark()));
     }

    mpOutlinerView->Paint( rRect );

    if (mpPostIt->GetStatus()==SwPostItHelper::DELETED)
    {
        SetLineColor(mpPostIt->GetChangeColor());
        DrawLine(PixelToLogic(GetPosPixel()),PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width(),GetSizePixel().Height())));
        DrawLine(PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width(),0)),PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height())));
    }
}

void PostItTxt::KeyInput( const KeyEvent& rKeyEvt )
{
    const KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    USHORT nKey = rKeyCode.GetCode();
    SwView* pView = mpPostIt->DocView();

    if ( (nKey== KEY_N) && rKeyCode.IsMod1() && rKeyCode.IsMod2())
    {
        if ( mpPostIt->Mgr()->GetActivePostIt() == mpPostIt )
            mpPostIt->Mgr()->SetActivePostIt(0);
        mpPostIt->SwitchToFieldPos();
    }
    else
    if ((rKeyCode.IsMod1() && rKeyCode.IsMod2()) && ((nKey == KEY_PAGEUP) || (nKey == KEY_PAGEDOWN)))
    {
        mpPostIt->SwitchToPostIt(nKey);
    }
    else
    if ((nKey == KEY_ESCAPE) || (rKeyCode.IsMod1() && ((nKey == KEY_PAGEUP) || (nKey == KEY_PAGEDOWN))))
    {
        if ( mpPostIt->Mgr()->GetActivePostIt() == mpPostIt )
            mpPostIt->Mgr()->SetActivePostIt(0);
        if (!mpPostIt->IsReadOnly() && (mpPostIt->GetStatus()!=SwPostItHelper::DELETED) &&
                mpPostIt->Engine()->GetEditEngine().GetText()==String(::rtl::OUString::createFromAscii("")))
            mpPostIt->Delete();
        else
            mpPostIt->SwitchToFieldPos();
    }
    else
    if (nKey == KEY_INSERT)
    {
        if (!rKeyCode.IsMod1() && !rKeyCode.IsMod2())
            mpPostIt->ToggleInsMode();
    }
    else
    {
        //let's make sure we see our note
        mpPostIt->Mgr()->MakeVisible(mpPostIt);

        long aOldHeight = mpPostIt->GetPostItTextHeight();
        bool bDone = false;

        /// HACK: need to switch off processing of Undo/Redo in Outliner
        if ( !( (nKey == KEY_Z || nKey == KEY_Y) && rKeyCode.IsMod1()) )
        {
            SwPostItHelper::SwLayoutStatus aStatus = mpPostIt->GetStatus();
            if ( (aStatus!=SwPostItHelper::DELETED) ||
                    ( (aStatus==SwPostItHelper::DELETED) && (!mpPostIt->Engine()->GetEditEngine().DoesKeyChangeText(rKeyEvt))) )
                bDone = mpOutlinerView->PostKeyEvent( rKeyEvt );
        }
        if (bDone)
            mpPostIt->ResizeIfNeccessary(aOldHeight,mpPostIt->GetPostItTextHeight());
        else
        {
            // write back data first when saving or showing navigator
            //otherwise new content could be lost
            if ( (rKeyCode.IsMod1() && nKey== KEY_S) || (nKey==KEY_F5) )
                mpPostIt->UpdateData();
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
                    SwWrtShell &rSh = mpPostIt->DocView()->GetWrtShell();
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
    mpPostIt->DocView()->GetViewFrame()->GetBindings().InvalidateAll(FALSE);
}

void PostItTxt::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mpOutlinerView )
        mpOutlinerView->MouseButtonUp( rMEvt );
}

IMPL_LINK(PostItTxt, OnlineSpellCallback, SpellCallbackInfo*, pInfo)
{
    if (mpPostIt && pInfo->nCommand == SPELLCMD_STARTSPELLDLG)
        mpPostIt->DocView()->GetViewFrame()->GetDispatcher()->Execute( FN_SPELL_GRAMMAR_DIALOG, SFX_CALLMODE_ASYNCHRON);
    return 0;
}

IMPL_LINK( PostItTxt, Select, Menu*, pSelMenu )
{
    mpPostIt->ExecuteCommand( pSelMenu->GetCurItemId() );
    return 0;
}

void PostItTxt::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        if (!mpPostIt->IsReadOnly() &&  (mpPostIt->GetStatus()!=SwPostItHelper::DELETED) &&
            mpOutlinerView->IsWrongSpelledWordAtPos( rCEvt.GetMousePosPixel(),TRUE ))
        {
            Link aLink = LINK(this, PostItTxt, OnlineSpellCallback);
            mpOutlinerView->ExecuteSpellPopup(rCEvt.GetMousePosPixel(),&aLink);
        }
        else
        {
            SfxPopupMenuManager* aMgr = mpPostIt->DocView()->GetViewFrame()->GetDispatcher()->Popup(0, this,&rCEvt.GetMousePosPixel());
            XubString aText = ((PopupMenu*)aMgr->GetSVMenu())->GetItemText( FN_DELETE_NOTE_AUTHOR );
            SwRewriter aRewriter;
            aRewriter.AddRule(UNDO_ARG1, mpPostIt->GetAuthor());
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
        if (mpPostIt->Scrollbar()->IsVisible())
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if (pData->IsShift() || pData->IsMod1() || pData->IsMod2())
            {
                mpPostIt->DocView()->HandleWheelCommands(rCEvt);
            }
            else
            {
                HandleScrollCommand( rCEvt, 0 , mpPostIt->Scrollbar());

                /*
                long nLines = pData->GetNotchDelta() * (long)pData->GetScrollLines();
                if ( ((mpPostIt->Scrollbar()->GetRange().Min() == mpPostIt->Scrollbar()->GetThumbPos()) && (nLines > 0)) ||
                    ( (mpPostIt->Scrollbar()->GetRange().Max() == mpPostIt->Scrollbar()->GetThumbPos()+mpPostIt->Scrollbar()->GetVisibleSize()) &&  (nLines < 0)) )
                {
                    mpPostIt->DocView()->HandleWheelCommands(rCEvt);
                }
                else
                {
                    HandleScrollCommand( rCEvt, 0 , mpPostIt->Scrollbar());
                }
                */
            }
        }
        else
        {
            mpPostIt->DocView()->HandleWheelCommands(rCEvt);
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
                    mpPostIt->SetShadowState(SS_VIEW);
                    Invalidate();
                }
            }
            else if ( pMouseEvt->IsLeaveWindow())
            {
                if (mpPostIt->IsPreview())
                {
                    //mpPostIt->doLazyDelete();
                }
                else
                {
                    mMouseOver = false;
                    if (!mbShowPopup && !HasFocus())
                    {
                        mpPostIt->SetShadowState(SS_NORMAL);
                        Invalidate();
                    }
                }
            }
        }
    }
    return sal_True;
}


/******* SwPostIt **************************************/
SwPostIt::SwPostIt( Window* pParent, WinBits nBits, SwFmtFld* aField,SwPostItMgr* aMgr,SwPostItBits aBits) : Window(pParent, nBits),
    mpOutlinerView(0),
    mpOutliner(0),
    mpPostItTxt(0),
    mpMeta(0),
    mpVScrollbar(0),
    mpFmtFld(aField),
    mpFld( static_cast<SwPostItField*>(aField->GetFld())),
    mpAnkor(0),
    mpShadow(0),
    mpMgr(aMgr),
    mbMeta(true),
    mpButtonPopup(new PopupMenu(SW_RES(MN_ANNOTATION_BUTTON))),
    nFlags(aBits)
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

    InitControls();
    SetPostItText();
}

void SwPostIt::SetPostItText()
{
    // get text from SwPostItField and insert into our textview
    mpOutliner->SetModifyHdl( Link() );
    mpOutliner->EnableUndo( FALSE );
    mpFld = static_cast<SwPostItField*>(mpFmtFld->GetFld());
    if( mpFld->GetTextObject() )
        mpOutliner->SetText( *mpFld->GetTextObject() );
    else
    {
        mpOutliner->Clear();
        SfxItemSet item( mpView->GetDocShell()->GetPool() );
        item.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
        item.Put(SvxFontItem(FAMILY_SWISS,GetSettings().GetStyleSettings().GetFieldFont().GetName(),
                            rtl::OUString::createFromAscii(""),PITCH_DONTKNOW,RTL_TEXTENCODING_DONTKNOW,EE_CHAR_FONTINFO));
        mpOutlinerView->SetAttribs(item);
        mpOutlinerView->InsertText(mpFld->GetPar2(),false);
    }

    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();
    mpOutliner->EnableUndo( TRUE );
    mpOutliner->SetModifyHdl( LINK( this, SwPostIt, ModifyHdl ) );
    Invalidate();
}

SwPostIt::~SwPostIt()
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
}

void SwPostIt::Paint( const Rectangle& rRect)
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
            Gradient aGradient(GRADIENT_LINEAR,ColorFromAlphaColor(15,mColorAnkor,mColorDark),ColorFromAlphaColor(80,mColorAnkor,mColorDark));
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

void SwPostIt::SetPosSizePixelRect(long nX, long nY,long nWidth, long nHeight,const SwRect &aRect, const long aPageBorder)
{
    mbMeta = true;
    mPosSize = Rectangle(Point(nX,nY),Size(nWidth,nHeight));
    mAnkorRect = aRect;
    mPageBorder = aPageBorder;
}

void SwPostIt::SetSize( const Size& rNewSize )
{
    mPosSize.SetSize(rNewSize);
}

void SwPostIt::SetVirtualPosSize( const Point& aPoint, const Size& aSize)
{
    mPosSize = Rectangle(aPoint,aSize);
}

void SwPostIt::TranslateTopPosition(const long aAmount)
{
    mPosSize.Move(0,aAmount);
}

void SwPostIt::ShowAnkorOnly(const Point &aPoint)
{
    HideNote();
    SetPosAndSize();
    if (mpAnkor)
    {
        mpAnkor->SetSixthPosition(basegfx::B2DPoint(aPoint.X(),aPoint.Y()));
        mpAnkor->SetSeventhPosition(basegfx::B2DPoint(aPoint.X(),aPoint.Y()));
        mpAnkor->setVisible(true);
    }
}

void SwPostIt::InitControls()
{
    // actual window which holds the user text
    //mpPostItTxt = new PostItTxt(this, 0x00000100);
    mpPostItTxt = new PostItTxt(this, 0);
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

    CheckMetaText();

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
                            rtl::OUString::createFromAscii(""),PITCH_DONTKNOW,RTL_TEXTENCODING_DONTKNOW,EE_CHAR_FONTINFO));
    mpOutlinerView->SetAttribs(item);

    // TODO: ??
    EEHorizontalTextDirection aDefHoriTextDir = Application::GetSettings().GetLayoutRTL() ? EE_HTEXTDIR_R2L : EE_HTEXTDIR_L2R;
    mpOutliner->SetDefaultHorizontalTextDirection( aDefHoriTextDir );

    //create Scrollbars
    mpVScrollbar = new ScrollBar(this, WB_3DLOOK |WB_VSCROLL|WB_DRAG);
    mpVScrollbar->EnableNativeWidget(false);
    mpVScrollbar->EnableRTL( false );
    mpVScrollbar->SetScrollHdl(LINK(this, SwPostIt, ScrollHdl));
    mpVScrollbar->EnableDrag();
    mpVScrollbar->AddEventListener( LINK( mpPostItTxt, PostItTxt, WindowEventListener ) );

    mpButtonPopup->SetMenuFlags(MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES);

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

    XubString aText = mpButtonPopup->GetItemText( FN_DELETE_NOTE_AUTHOR );
    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1,GetAuthor());
    aText = aRewriter.Apply(aText);
    mpButtonPopup->SetItemText(FN_DELETE_NOTE_AUTHOR,aText);

    // TODO: why does this not work?
    //mpOutliner->SetDefaultLanguage(mpFld->GetLanguage());

    // set initial language for outliner
    USHORT nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( mpFld->GetLanguage() );
    USHORT nLangWhichId = 0;
    switch (nScriptType)
    {
        case SCRIPTTYPE_LATIN :    nLangWhichId = EE_CHAR_LANGUAGE ; break;
        case SCRIPTTYPE_ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
        case SCRIPTTYPE_COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
    }
    SetLanguage(SvxLanguageItem(mpFld->GetLanguage(),nLangWhichId));
    mpOutlinerView->StartSpeller();

    mpMeta->Show();
    mpVScrollbar->Show();
    mpPostItTxt->Show();
}

void SwPostIt::CheckMetaText()
{
    /*
    String sDateTime;
    SvNumberFormatter* pNumFormatter = mpView->GetDocShell()->GetDoc()->GetNumberFormatter();
    const ULONG nFormatDate = pNumFormatter->GetStandardFormat( NUMBERFORMAT_DATETIME , Application::GetSettings().GetLanguage());
    const DateTime aDateTime( mpFld->GetDate(), mpFld->GetTime());
    pNumFormatter->GetOutputString( aDateTime - DateTime( *pNumFormatter->GetNullDate()), nFormatDate, sDateTime, &pColor );
    */
    const LocaleDataWrapper& rLocalData = SvtSysLocale().GetLocaleData();
    String sMeta = mpFld->GetPar1();
    if (sMeta.Len() > 22)
    {
        sMeta.Erase(20);
        sMeta = sMeta + rtl::OUString::createFromAscii("...");
    }
    bool bValidTimeStamp = true;
    if (mpFld->GetDate()==Date())
        sMeta = sMeta + rtl::OUString::createFromAscii("\n") + String(SW_RES(STR_POSTIT_TODAY));
    else if (mpFld->GetDate()==Date(Date()-1))
        sMeta = sMeta + rtl::OUString::createFromAscii("\n") + String(SW_RES(STR_POSTIT_YESTERDAY));
    else if (mpFld->GetDate().IsValid())
        sMeta = sMeta + rtl::OUString::createFromAscii("\n") + rLocalData.getDate(mpFld->GetDate());
    else
        bValidTimeStamp = false;
    if (bValidTimeStamp)
        sMeta = sMeta + rtl::OUString::createFromAscii(" ")  + rLocalData.getTime(mpFld->GetTime(), false);
    if (mpMeta->GetText() != sMeta)
        mpMeta->SetText(sMeta);
}

void SwPostIt::Rescale()
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

void SwPostIt::SetPosAndSize()
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
                                            basegfx::B2DPoint( aLineEnd.X(),aLineEnd.Y()) , mColorAnkor,LineInfo(LINE_DASH,ANKORLINE_WIDTH*15), false);
                mpAnkor->SetHeight(mAnkorRect.Height());
                mpAnkor->setVisible(false);
                if (HasChildPathFocus())
                    mpAnkor->SetLineInfo(LineInfo(LINE_SOLID,ANKORLINE_WIDTH*15));
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
}

void SwPostIt::DoResize()
{
    long aTextHeight        =   LogicToPixel( mpOutliner->CalcTextSize()).Height();
    unsigned long aWidth    =   GetSizePixel().Width();
    long aHeight            =   GetSizePixel().Height();

    if (mbMeta)
    {
        aHeight -= GetMetaHeight();
        mpMeta->Show();
        mpPostItTxt->SetQuickHelpText(rtl::OUString::createFromAscii(""));
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

void SwPostIt::SetSizePixel( const Size& rNewSize )
{
    Window::SetSizePixel(rNewSize);

    if (mpShadow)
    {
        Point aStart = EditWin()->PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height()));
        Point aEnd = EditWin()->PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width()-1,GetSizePixel().Height()));
        mpShadow->SetPosition(basegfx::B2DPoint(aStart.X(),aStart.Y()), basegfx::B2DPoint(aEnd.X(),aEnd.Y()));
    }
}

void SwPostIt::SetScrollbar()
{
    mpVScrollbar->SetThumbPos( mpOutlinerView->GetVisArea().Top()+ mpOutlinerView->GetEditView().GetCursor()->GetOffsetY());
}

void SwPostIt::ResizeIfNeccessary(long aOldHeight, long aNewHeight)
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
                {
                    SetSizePixel(Size(GetSizePixel().Width(),GetMinimumSizeWithoutMeta() + GetMetaHeight()));
                    DoResize();
                    Invalidate();
                }
            }
        }
        else
        {
            SetScrollbar();
        }
    }
    else
    {
        SetScrollbar();
    }
}

void SwPostIt::SetColor(Color aColorDark,Color aColorLight, Color aColorAnkor)
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

void SwPostIt::SetChangeTracking(SwPostItHelper::SwLayoutStatus& aStatus,Color aColor)
{
    if ( (mStatus != aStatus) || (mChangeColor != aColor) )
    {
        mStatus = aStatus;
        mChangeColor = aColor;
        Invalidate();
    }
}

void SwPostIt::SetMarginSide(bool aMarginSide)
{
    mbMarginSide = aMarginSide;
}

void SwPostIt::SetReadonly(BOOL bSet)
{
    mbReadonly = bSet;
    View()->SetReadOnly(bSet);
}

void SwPostIt::SetLanguage(const SvxLanguageItem aNewItem)
{
    mpOutliner->SetModifyHdl( Link() );
    ESelection aOld = mpOutlinerView->GetSelection();

    ESelection aNewSelection( 0, 0, (USHORT)mpOutliner->GetParagraphCount()-1, USHRT_MAX );
    mpOutlinerView->SetSelection( aNewSelection );
    SfxItemSet aEditAttr(mpOutlinerView->GetAttribs());
    aEditAttr.Put(aNewItem);
    mpOutlinerView->SetAttribs( aEditAttr );

    mpOutlinerView->SetSelection(aOld);
    mpOutliner->SetModifyHdl( LINK( this, SwPostIt, ModifyHdl ) );

    const SwViewOption* pVOpt = mpView->GetWrtShellPtr()->GetViewOptions();
    ULONG nCntrl = mpOutliner->GetControlWord();
    // turn off
    if (!pVOpt->IsOnlineSpell())
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    //turn back on
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    mpOutliner->CompleteOnlineSpelling();
    Invalidate();
}

void SwPostIt::DataChanged( const DataChangedEvent& aEvent)
{
    Window::DataChanged( aEvent );
}

void SwPostIt::GetFocus()
{
    if (mpPostItTxt)
        mpPostItTxt->GrabFocus();
}

void SwPostIt::LoseFocus()
{
}

void SwPostIt::ShowNote()
{
    SetPosAndSize();
    if (!IsVisible())
        Window::Show();
    if (mpAnkor)
        mpAnkor->setVisible(true);
    if (mpShadow)
        mpShadow->setVisible(true);
}

void SwPostIt::HideNote()
{
    if (IsVisible())
        Window::Hide();
    if (mpAnkor)
        mpAnkor->setVisible(false);
    if (mpShadow)
        mpShadow->setVisible(false);
}

void SwPostIt::ActivatePostIt()
{
    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();

    CheckMetaText();
    SetShadowState(SS_EDIT);
    View()->ShowCursor();

    mpOutlinerView->GetEditView().SetInsertMode(mpView->GetWrtShellPtr()->IsInsMode());

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        View()->SetBackgroundColor(mColorDark);

    //  mpPostItTxt->SetBackground(Wallpaper(mColorDark));
}

void SwPostIt::DeactivatePostIt()
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

    SetShadowState(SS_NORMAL);
    // write the visible text back into the SwField
    UpdateData();

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        View()->SetBackgroundColor(COL_TRANSPARENT);

    //mpPostItTxt->SetBackground(Gradient(GRADIENT_LINEAR,mColorLight,mColorDark));
}

void SwPostIt::UpdateData()
{
    if ( mpOutliner->IsModified() )
    {
        SwPosition * pPos = mpFmtFld->GetTxtFld()->GetPosition();
        if ( pPos )
        {
            SwField* pOldField = mpFld->Copy();
              mpFld->SetPar2(mpOutliner->GetEditEngine().GetText());
            mpFld->SetTextObject(mpOutliner->CreateParaObject());
            mpView->GetDocShell()->GetDoc()->AppendUndo(new SwUndoFieldFromDoc(*pPos, *pOldField, *mpFld, 0, true));
            delete pOldField;
            delete pPos;
            // so we get a new layout of notes (ankor position is still the same and we would otherwise not get one)
            mpMgr->SetLayout();
            mpView->GetDocShell()->SetModified();
        }
    }
    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();
}

void SwPostIt::ToggleInsMode()
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

void SwPostIt::Delete()
{
    if ( Mgr()->GetActivePostIt() == this)
        Mgr()->SetActivePostIt(0);
    // we delete the field directly, the Mgr cleans up the PostIt by listening
    mpView->GetWrtShellPtr()->GotoField(*mpFmtFld);
    mpView->GetWrtShellPtr()->DelRight();
}

SwEditWin*  SwPostIt::EditWin()
{
    return &mpView->GetEditWin();
}

String SwPostIt::GetAuthor() const
{
    return mpFld->GetPar1();
}

long SwPostIt::GetPostItTextHeight()
{
    return mpOutliner ? LogicToPixel(mpOutliner->CalcTextSize()).Height() : 0;
}

/*
void SwPostIt::SwitchToPostIt(bool aDirection)
{
    if (aDirection)
        SwitchToPostIt(KEY_PAGEDOWN);
    else
        SwitchToPostIt(KEY_PAGEUP);
}
*/
void SwPostIt::SwitchToPostIt(USHORT aDirection)
{
    if (mpMgr)
    {
        SwPostIt* pPostIt = mpMgr->GetNextPostIt(aDirection, this);
        if (pPostIt)
            pPostIt->GrabFocus();
    }
}

void SwPostIt::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (mRectMetaButton.IsInside(PixelToLogic(rMEvt.GetPosPixel())) && rMEvt.IsLeft())
    {
        if (IsPreview())
        {
            doLazyDelete();
            if (mpMgr)
            {
                SwPostIt* pPostIt = mpMgr->GetPostIt(mpFmtFld);
                if (pPostIt)
                {
                    pPostIt->GrabFocus();
                    mpMgr->MakeVisible(pPostIt);
                }
            }

        }
        else
        {
            if ( mbReadonly )
            {
                mpButtonPopup->EnableItem(FN_DELETE_NOTE,false);
                mpButtonPopup->EnableItem(FN_DELETE_NOTE_AUTHOR,false);
                mpButtonPopup->EnableItem(FN_DELETE_ALL_NOTES,false);
            }
            else
            {
                if (mStatus==SwPostItHelper::DELETED)
                    mpButtonPopup->EnableItem(FN_DELETE_NOTE,false);
                else
                    mpButtonPopup->EnableItem(FN_DELETE_NOTE,true);
                mpButtonPopup->EnableItem(FN_DELETE_NOTE_AUTHOR,true);
                mpButtonPopup->EnableItem(FN_DELETE_ALL_NOTES,true);
            }

            ExecuteCommand( mpButtonPopup->Execute( this,Rectangle(LogicToPixel(mRectMetaButton.BottomLeft()),LogicToPixel(mRectMetaButton.BottomLeft())),POPUPMENU_EXECUTE_DOWN | POPUPMENU_NOMOUSEUPCLOSE) );
        }
    }
}

void SwPostIt::ExecuteCommand(USHORT nSlot)
{
    switch (nSlot)
    {
        case FN_DELETE_NOTE:
            Delete();
            break;
        case FN_HIDE_NOTE:
            if ( mpMgr->GetActivePostIt() == this )
            {
                mpMgr->SetActivePostIt(0);
                // put the cursor back into the document
                SwitchToFieldPos();
            }
            mpMgr->Hide(mpFld);
            break;
        case FN_DELETE_ALL_NOTES:
        case FN_HIDE_ALL_NOTES:
            // not possible as slot as this would require that "this" is the active postit
            mpView->GetViewFrame()->GetBindings().Execute( nSlot, 0, 0, SFX_CALLMODE_ASYNCHRON );
            break;
        case FN_DELETE_NOTE_AUTHOR:
        case FN_HIDE_NOTE_AUTHOR:
        {
            // not possible as slot as this would require that "this" is the active postit
            SfxStringItem aItem( nSlot, mpFld->GetPar1() );
            const SfxPoolItem* aItems[2];
            aItems[0] = &aItem;
            aItems[1] = 0;
            mpView->GetViewFrame()->GetBindings().Execute( nSlot, aItems, 0, SFX_CALLMODE_ASYNCHRON );
        }
        default:
            mpView->GetViewFrame()->GetBindings().Execute( nSlot );
            break;
    }
}

void SwPostIt::SwitchToFieldPos(bool bAfter)
{
    mpView->GetDocShell()->GetWrtShell()->GotoField(*mpFmtFld);
    if (bAfter)
        mpView->GetDocShell()->GetWrtShell()->SwCrsrShell::Right(1, 0, FALSE);
    GrabFocusToDocument();
}

IMPL_LINK(SwPostIt, ScrollHdl, ScrollBar*, pScroll)
{
    long nDiff = View()->GetEditView().GetVisArea().Top() - pScroll->GetThumbPos();
    View()->Scroll( 0, nDiff );
    return 0;
}

IMPL_LINK(SwPostIt, ModifyHdl, void*, pVoid)
{
    // no warnings, please
    pVoid=0;
    mpView->GetDocShell()->SetModified(sal_True);
    return 0;
}

void SwPostIt::ResetAttributes()
{
    mpOutlinerView->RemoveAttribsKeepLanguages(TRUE);
    mpOutliner->RemoveFields(TRUE);
    SfxItemSet aSet( mpView->GetDocShell()->GetPool() );
    aSet.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
    mpOutlinerView->SetAttribs(aSet);
}

sal_Int32 SwPostIt::GetScrollbarWidth()
{
    return mpView->GetWrtShell().GetViewOptions()->GetZoom() / 10;
}

sal_Int32 SwPostIt::GetMetaButtonAreaWidth()
{
    const Fraction& f( GetMapMode().GetScaleX() );
    if (IsPreview())
        return 3 * METABUTTON_AREA_WIDTH * f.GetNumerator() / f.GetDenominator();
    else
        return METABUTTON_AREA_WIDTH * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwPostIt::GetMetaHeight()
{
    const Fraction& f( GetMapMode().GetScaleY() );
    return POSTIT_META_HEIGHT * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwPostIt::GetMinimumSizeWithMeta()
{
    return mpMgr->GetMinimumSizeWithMeta();
}

sal_Int32 SwPostIt::GetMinimumSizeWithoutMeta()
{
    const Fraction& f( GetMapMode().GetScaleY() );
    return POSTIT_MINIMUMSIZE_WITHOUT_META * f.GetNumerator() / f.GetDenominator();
}

void SwPostIt::SetSpellChecking()
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

void SwPostIt::SetShadowState(ShadowState bState)
{
    switch (bState)
    {
        case SS_EDIT:
        {
            if ( Ankor() )
                Ankor()->SetLineInfo(LineInfo(LINE_SOLID,ANKORLINE_WIDTH*15));
            if ( Shadow() )
                Shadow()->SetShadowState(SS_EDIT);
            break;
        }
        case SS_VIEW:
        {
            if (mpAnkor)
                mpAnkor->SetLineInfo(LineInfo(LINE_SOLID,ANKORLINE_WIDTH*15));
            if (mpShadow)
                mpShadow->SetShadowState(SS_VIEW);
            break;
        }
        case SS_NORMAL:
        {
            if (mpAnkor)
                mpAnkor->SetLineInfo(LineInfo(LINE_DASH,ANKORLINE_WIDTH*15));
            if (mpShadow)
                mpShadow->SetShadowState(SS_NORMAL);
            break;
        }
    }
}

/****** SwPostItShadow  ***********************************************************/

SwPostItShadow::SwPostItShadow(const basegfx::B2DPoint& rBasePos,const basegfx::B2DPoint& rSecondPosition,
                               Color aBaseColor,ShadowState aState)
        :   OverlayObjectWithBasePosition(rBasePos, aBaseColor),
            maSecondPosition(rSecondPosition),
            mShadowState(aState)
{
//  mbAllowsAnimation = sal_True;
}

SwPostItShadow::~SwPostItShadow()
{
}

void SwPostItShadow::Trigger(sal_uInt32 /*nTime*/)
{
}

void SwPostItShadow::drawGeometry(OutputDevice& rOutputDevice)
{
    rOutputDevice.SetLineColor();
    rOutputDevice.SetFillColor();

    const Fraction& f( rOutputDevice.GetMapMode().GetScaleY() );
    sal_Int32 aBigHeight( 4 * f.GetNumerator() / f.GetDenominator());
    sal_Int32 aSmallHeight( 2 * f.GetNumerator() / f.GetDenominator());

    const Point aStart(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
    const Point aEndSmall(FRound(GetSecondPosition().getX()), FRound(GetSecondPosition().getY() + rOutputDevice.PixelToLogic(Size(0,aSmallHeight)).Height()));
    const Point aEndBig(FRound(GetSecondPosition().getX()), FRound(GetSecondPosition().getY() + rOutputDevice.PixelToLogic(Size(0,aBigHeight)).Height()));
    Rectangle aSmallRectangle(aStart, aEndSmall);
    Rectangle aBigRectangle(aStart, aEndBig);

    switch(mShadowState)
    {
        case SS_NORMAL:
        {
            Gradient aGradient(GRADIENT_LINEAR,Color(230,230,230),POSTIT_SHADOW_BRIGHT);
            aGradient.SetAngle(1800);
            rOutputDevice.DrawGradient(aSmallRectangle, aGradient);
            break;
        }
        case SS_VIEW:
        {
            Gradient aGradient(GRADIENT_LINEAR,Color(230,230,230),POSTIT_SHADOW_BRIGHT);
            aGradient.SetAngle(1800);
            rOutputDevice.DrawGradient(aBigRectangle, aGradient);
            break;
        }
        case SS_EDIT:
        {
            Gradient aGradient(GRADIENT_LINEAR,Color(230,230,230),POSTIT_SHADOW_DARK);
            aGradient.SetAngle(1800);
            rOutputDevice.DrawGradient(aBigRectangle, aGradient);
            break;
        }
        default:
        {
            break;
        }
    }
}

void SwPostItShadow::createBaseRange(OutputDevice& rOutputDevice)
{
    maBaseRange.reset();

    const Fraction& f( rOutputDevice.GetMapMode().GetScaleY() );
    sal_Int32 aBigHeight( 4 * f.GetNumerator() / f.GetDenominator());

    Rectangle aRect(Point(FRound(getBasePosition().getX()),FRound(getBasePosition().getY())),
                    Point(FRound(GetSecondPosition().getX()),FRound(GetSecondPosition().getY()+rOutputDevice.PixelToLogic(Size(0,aBigHeight)).Height())));
    maBaseRange.expand(basegfx::B2DPoint(aRect.Left(), aRect.Top()));
    maBaseRange.expand(basegfx::B2DPoint(aRect.Right(), aRect.Bottom()));
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
    maBasePosition = rPoint1;
    maSecondPosition = rPoint2;

    objectChange();
}
void SwPostItShadow::transform(const basegfx::B2DHomMatrix& rMatrix)
{
    if(!rMatrix.isIdentity())
    {
        // transform base position
        OverlayObjectWithBasePosition::transform(rMatrix);
        maSecondPosition = rMatrix * GetSecondPosition();
        objectChange();
    }
}

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

void SwPostItAnkor::implDrawGeometry(OutputDevice& rOutputDevice, Color aColor, double fOffX, double fOffY)
{
    basegfx::B2DPolygon aTri(maTriangle);
    basegfx::B2DPolygon aLin(maLine);
    const Polygon aLinTop(maLineTop);

    if(0.0 != fOffX || 0.0 != fOffY)
    {
        // transform polygons
        basegfx::B2DHomMatrix aTranslate;
        aTranslate.set(0, 2, fOffX);
        aTranslate.set(1, 2, fOffY);

        aTri.transform(aTranslate);
        aLin.transform(aTranslate);
    }

    // draw triangle
    rOutputDevice.SetLineColor();
    rOutputDevice.SetFillColor(aColor);
    rOutputDevice.DrawPolygon(Polygon(aTri));

    /*
    basegfx::B2DHomMatrix aMatrix;
    aMatrix.translate(-aTri.getB2DPoint(0).getX(),-aTri.getB2DPoint(0).getY());
    aMatrix.scale(1.0, -1.0);
    aMatrix.translate(aTri.getB2DPoint(0).getX(),aTri.getB2DPoint(0).getY());
    aMatrix.translate(0,(mHeight*-1) + 13 * 15 );
    aTri.transform(aMatrix);
    rOutputDevice.DrawPolygon(Polygon(aTri));
    */

    // draw line
    rOutputDevice.SetLineColor(aColor);
    rOutputDevice.SetFillColor();
    rOutputDevice.DrawPolyLine(Polygon(aLin), mLineInfo);
    rOutputDevice.DrawPolyLine(aLinTop,LineInfo(LINE_SOLID,ANKORLINE_WIDTH*15));
}

Color SwPostItAnkor::implBlendColor(const Color aOriginal, sal_Int16 nChange)
{
    if(0 != nChange)
    {
        sal_Int16 nR(aOriginal.GetRed() + nChange);
        sal_Int16 nG(aOriginal.GetGreen() + nChange);
        sal_Int16 nB(aOriginal.GetBlue() + nChange);

        // truncate R, G and B
        if(nR > 0xff)
            nR = 0xff;
        else if(nR < 0)
            nR = 0;

        if(nG > 0xff)
            nG = 0xff;
        else if(nG < 0)
            nG = 0;

        if(nB > 0xff)
            nB = 0xff;
        else if(nB < 0)
            nB = 0;

        return Color((sal_uInt8)nR, (sal_uInt8)nG, (sal_uInt8)nB);
    }
    else
    {
        return aOriginal;
    }
}

SwPostItAnkor::SwPostItAnkor(const basegfx::B2DPoint& rBasePos,
                        const basegfx::B2DPoint& rSecondPos,
                        const basegfx::B2DPoint& rThirdPos,
                        const basegfx::B2DPoint& rFourthPos,
                        const basegfx::B2DPoint& rFifthPos,
                        const basegfx::B2DPoint& rSixthPos,
                        const basegfx::B2DPoint& rSeventhPos,
                        Color aBaseColor,
                        const LineInfo &aLineInfo,
                        bool bShadowedEffect)
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
            mLineInfo(aLineInfo),
            mHeight(0),
            mbShadowedEffect(bShadowedEffect)
{
    if (mLineInfo.GetStyle()==LINE_DASH)
    {
        mLineInfo.SetDistance( 3 * 15);
        mLineInfo.SetDashLen(  5 * 15);
        mLineInfo.SetDashCount(100);
    }
    //mbAllowsAnimation = sal_True;
}

SwPostItAnkor::~SwPostItAnkor()
{
}

void SwPostItAnkor::Trigger(sal_uInt32 /*nTime*/)
{
}

void SwPostItAnkor::drawGeometry(OutputDevice& rOutputDevice)
{
    implEnsureGeometry();

    if(getShadowedEffect())
    {
        // calculate one pixel offset
        const basegfx::B2DVector aOnePixelOffset(rOutputDevice.GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));
        const Color aLighterColor(implBlendColor(getBaseColor(), 20));
        const Color aDarkerColor(implBlendColor(getBaseColor(), -20));

        // draw top-left
        implDrawGeometry(rOutputDevice, aLighterColor, -aOnePixelOffset.getX(), -aOnePixelOffset.getY());

        // draw bottom-right
        implDrawGeometry(rOutputDevice, aDarkerColor, aOnePixelOffset.getX(), aOnePixelOffset.getY());
    }

    // draw original
    implDrawGeometry(rOutputDevice, getBaseColor(), 0.0, 0.0);
}

void SwPostItAnkor::createBaseRange(OutputDevice& rOutputDevice)
{
   // get range from geometry
    implEnsureGeometry();
    maBaseRange = basegfx::tools::getRange(maTriangle);
    maBaseRange.expand(basegfx::tools::getRange(maLine));
    maBaseRange.expand(basegfx::tools::getRange(maLineTop));

    /*
    basegfx::B2DHomMatrix aMatrix;
    aMatrix.translate(-maTriangle.getB2DPoint(0).getX(),-maTriangle.getB2DPoint(0).getY());
    aMatrix.scale(1.0,-1.0);
    aMatrix.translate(maTriangle.getB2DPoint(0).getX(),maTriangle.getB2DPoint(0).getY());
    aMatrix.translate(0,(mHeight*-1) + 13 * 15 );
    basegfx::B2DRange MyRange(basegfx::tools::getRange(maTriangle));
    MyRange.transform(aMatrix);
    maBaseRange.expand(MyRange);
    */

    // expand range for thick lines and shadowed geometry
    double fExpand(0.0);

    // take fat line into account
    if(0 != mLineInfo.GetWidth())
    {
        // expand range for logic half line width
        fExpand += static_cast< double >(mLineInfo.GetWidth()) / 2.0;
    }

    // take shadowed into account
    if(getShadowedEffect())
    {
        const basegfx::B2DVector aOnePixelOffset(rOutputDevice.GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));
        fExpand += ::std::max(aOnePixelOffset.getX(), aOnePixelOffset.getY());
    }

    if(0.0 != fExpand)
    {
        maBaseRange.grow(fExpand);
    }
}


void SwPostItAnkor::SetAllPosition(const basegfx::B2DPoint& rPoint1,
            const basegfx::B2DPoint& rPoint2,
            const basegfx::B2DPoint& rPoint3,
            const basegfx::B2DPoint& rPoint4,
            const basegfx::B2DPoint& rPoint5,
            const basegfx::B2DPoint& rPoint6,
            const basegfx::B2DPoint& rPoint7)
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
    maBasePosition = rPoint1;
    maSecondPosition = rPoint2;
    maThirdPosition = rPoint3;
    maFourthPosition = rPoint4;
    maFifthPosition = rPoint5;

    implResetGeometry();
    objectChange();
}

void SwPostItAnkor::transform(const basegfx::B2DHomMatrix& rMatrix)
{
    if(!rMatrix.isIdentity())
    {
        // transform base position
        OverlayObjectWithBasePosition::transform(rMatrix);

        maSecondPosition = rMatrix * GetSecondPosition();
        maThirdPosition = rMatrix * GetThirdPosition();
        maFourthPosition = rMatrix * GetFourthPosition();
        maFifthPosition = rMatrix * GetFifthPosition();
        maSixthPosition = rMatrix * GetSixthPosition();
        maSeventhPosition = rMatrix * GetSeventhPosition();

        implResetGeometry();
        objectChange();
    }
}

void SwPostItAnkor::SetLineInfo(const LineInfo &aLineInfo)
{
    if (aLineInfo != mLineInfo)
    {
        mLineInfo = aLineInfo;
        if (mLineInfo.GetStyle()==LINE_DASH)
        {
            mLineInfo.SetDistance( 3 * 15);
            mLineInfo.SetDashLen(  5 * 15);
            mLineInfo.SetDashCount(100);
        }
        //remove and add overlayobject, so it is the last one inside the manager to draw
        //therefore this line is on top
        sdr::overlay::OverlayManager* pMgr = getOverlayManager();
        if (pMgr)
        {
            pMgr->remove(*this);
            pMgr->add(*this);
        }
        objectChange();
    }
}
