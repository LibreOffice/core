/*************************************************************************
 *
 *  $RCSfile: EditWindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:07:21 $
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

#include "EditWindow.hxx"

#include "sdmod.hxx"

#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif
#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _SDRESID_HXX
#include "sdresid.hxx"
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <svtools/linguprops.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif

#define SCROLL_LINE         24

using namespace com::sun::star::accessibility;
using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace sd { namespace notes {

EditWindow::EditWindow (Window* pParentWindow, SfxItemPool* pItemPool)
    : Window (pParentWindow, WinBits()),
      DropTargetHelper(this),
      mpEditView(NULL),
      mpHorizontalScrollBar(NULL),
      mpVerticalScrollBar(NULL),
      mpScrollBox(NULL),
      mpEditEngine(NULL)
{
    SetMapMode(MAP_PIXEL);

    // compare DataChanged
    SetBackground (GetSettings().GetStyleSettings().GetWindowColor());

    maModifyTimer.SetTimeout(2000);
    maModifyTimer.Start();

    maCursorMoveTimer.SetTimeout(500);

    CreateEditView();

    SvxFontHeightItem aItem (GetFont().GetSize().Height(), 100,
        EE_CHAR_FONTHEIGHT);
    pItemPool->SetPoolDefaultItem (aItem);
    aItem.SetWhich(EE_CHAR_FONTHEIGHT_CJK);
    pItemPool->SetPoolDefaultItem (aItem);
    aItem.SetWhich(EE_CHAR_FONTHEIGHT_CTL);
    pItemPool->SetPoolDefaultItem (aItem);

    InsertText (UniString::CreateFromAscii("EditWindow created and ready.\n"));
}


EditWindow::~EditWindow (void)
{
    maCursorMoveTimer.Stop();
    maModifyTimer.Stop();

    if (mpEditView != NULL)
    {
        EditEngine *pEditEngine = mpEditView->GetEditEngine();
        if (pEditEngine)
        {
            pEditEngine->SetStatusEventHdl(Link());
            pEditEngine->RemoveView (mpEditView);
        }
    }
    delete mpEditView;
    delete mpHorizontalScrollBar;
    delete mpVerticalScrollBar;
    delete mpScrollBox;

}

////////////////////////////////////////


void SmGetLeftSelectionPart(const ESelection aSel,
                            USHORT &nPara, USHORT &nPos)
    // returns paragraph number and position of the selections left part
{
    // compare start and end of selection and use the one that comes first
    if (    aSel.nStartPara <  aSel.nEndPara
        ||  aSel.nStartPara == aSel.nEndPara  &&  aSel.nStartPos < aSel.nEndPos)
    {   nPara = aSel.nStartPara;
        nPos  = aSel.nStartPos;
    }
    else
    {   nPara = aSel.nEndPara;
        nPos  = aSel.nEndPos;
    }
}




EditEngine * EditWindow::GetEditEngine (void)
{
    if (mpEditEngine == NULL)
        mpEditEngine = CreateEditEngine ();
    return mpEditEngine;
}




EditEngine* EditWindow::CreateEditEngine (void)
{
    EditEngine* pEditEngine = mpEditEngine;
    if (pEditEngine == NULL)
    {
        mpEditEngineItemPool = EditEngine::CreatePool();

        //
        // set fonts to be used
        //
        SvtLinguOptions aOpt;
        SvtLinguConfig().GetOptions( aOpt );
        //
        struct FontDta {
            INT16       nFallbackLang;
            INT16       nLang;
            USHORT      nFontType;
            USHORT      nFontInfoId;
            } aTable[3] =
        {
            // info to get western font to be used
            {   LANGUAGE_ENGLISH_US,    LANGUAGE_NONE,
                DEFAULTFONT_SERIF,      EE_CHAR_FONTINFO },
            // info to get CJK font to be used
            {   LANGUAGE_JAPANESE,      LANGUAGE_NONE,
                DEFAULTFONT_CJK_TEXT,   EE_CHAR_FONTINFO_CJK },
            // info to get CTL font to be used
            {   LANGUAGE_ARABIC,        LANGUAGE_NONE,
                DEFAULTFONT_CTL_TEXT,   EE_CHAR_FONTINFO_CTL }
        };
        aTable[0].nLang = aOpt.nDefaultLanguage;
        aTable[1].nLang = aOpt.nDefaultLanguage_CJK;
        aTable[2].nLang = aOpt.nDefaultLanguage_CTL;
        //
        for (int i = 0;  i < 3;  ++i)
        {
            const FontDta &rFntDta = aTable[i];
            LanguageType nLang = (LANGUAGE_NONE == rFntDta.nLang) ?
                rFntDta.nFallbackLang : rFntDta.nLang;
            Font aFont = Application::GetDefaultDevice()->GetDefaultFont(
                rFntDta.nFontType, nLang, DEFAULTFONT_FLAGS_ONLYONE);
            mpEditEngineItemPool->SetPoolDefaultItem(
                SvxFontItem(
                    aFont.GetFamily(),
                    aFont.GetName(),
                    aFont.GetStyleName(),
                    aFont.GetPitch(),
                    aFont.GetCharSet(),
                    rFntDta.nFontInfoId));
        }

        // set font heights
        SvxFontHeightItem aFontHeigt(
            Application::GetDefaultDevice()->LogicToPixel(
                Size (0, 10), MapMode (MAP_POINT)).Height(), 100,
            EE_CHAR_FONTHEIGHT );
        mpEditEngineItemPool->SetPoolDefaultItem( aFontHeigt);
        aFontHeigt.SetWhich (EE_CHAR_FONTHEIGHT_CJK);
        mpEditEngineItemPool->SetPoolDefaultItem( aFontHeigt);
        aFontHeigt.SetWhich (EE_CHAR_FONTHEIGHT_CTL);
        mpEditEngineItemPool->SetPoolDefaultItem( aFontHeigt);

        pEditEngine = new EditEngine (mpEditEngineItemPool);

        pEditEngine->EnableUndo (TRUE);
        pEditEngine->SetDefTab (USHORT(
            Application::GetDefaultDevice()->GetTextWidth(
                UniString::CreateFromAscii("XXXX"))));

        pEditEngine->SetControlWord(
                (pEditEngine->GetControlWord()
                    | EE_CNTRL_AUTOINDENTING) &
                (~EE_CNTRL_UNDOATTRIBS) &
                (~EE_CNTRL_PASTESPECIAL));

        pEditEngine->SetWordDelimiters (
            UniString::CreateFromAscii(" .=+-*/(){}[];\""));
        pEditEngine->SetRefMapMode (MAP_PIXEL);
        pEditEngine->SetPaperSize (Size(800, 0));
        pEditEngine->EraseVirtualDevice();
        pEditEngine->ClearModifyFlag();
    }

    return pEditEngine;
}




void EditWindow::DataChanged (const DataChangedEvent&)
{
    const StyleSettings aSettings (GetSettings().GetStyleSettings());

    SetBackground( aSettings.GetWindowColor() );

    // edit fields in other Applications use this font instead of
    // the application font thus we use this one too
    SetPointFont( aSettings.GetFieldFont() );
    EditEngine* pEditEngine = GetEditEngine();

    if (pEditEngine!=NULL && mpEditEngineItemPool!=NULL)
    {
        //!
        //! see also SmDocShell::GetEditEngine() !
        //!

        //      pEditEngine->SetDefTab( USHORT( GetTextWidth( C2S("XXXX") ) ) );

        USHORT aFntInfoId[3] = {
                EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK, EE_CHAR_FONTINFO_CTL };
        for (int i = 0;  i < 3;  ++i)
        {
            const SfxPoolItem *pItem;
            if ((pItem = mpEditEngineItemPool->GetPoolDefaultItem(  aFntInfoId[i] )))
            {
                const SvxFontItem *pFntItem = ((const SvxFontItem *) pItem);
                const Font &rFnt = aSettings.GetFieldFont();
                SvxFontItem aFntItem( rFnt.GetFamily(), rFnt.GetName(),
                        rFnt.GetStyleName(), rFnt.GetPitch(),
                        pFntItem->GetCharSet(),
                        aFntInfoId[i] );
                mpEditEngineItemPool->SetPoolDefaultItem( aFntItem );
            }
        }

        SvxFontHeightItem aItem( GetFont().GetSize().Height(), 100,
                                 EE_CHAR_FONTHEIGHT );
        mpEditEngineItemPool->SetPoolDefaultItem( aItem );
        aItem.SetWhich( EE_CHAR_FONTHEIGHT_CJK );
        mpEditEngineItemPool->SetPoolDefaultItem( aItem );
        aItem.SetWhich( EE_CHAR_FONTHEIGHT_CTL );
        mpEditEngineItemPool->SetPoolDefaultItem( aItem );

        // forces new settings to be used
        // unfortunately this resets the whole edit engine
        // thus we need to save at least the text
        String aTxt( pEditEngine->GetText( LINEEND_LF ) );
        pEditEngine->Clear();   //#77957 incorrect font size
        pEditEngine->SetText( aTxt );
    }

    String aText (mpEditEngine->GetText (LINEEND_LF));
    mpEditEngine->Clear();
    mpEditEngine->SetText (aText);

    AdjustScrollBars();
    Resize();
}




void EditWindow::Resize (void)
{
    if (!mpEditView)
        CreateEditView();

    if (mpEditView != NULL)
    {
        mpEditView->SetOutputArea(AdjustScrollBars());
        mpEditView->ShowCursor();

        DBG_ASSERT( mpEditView->GetEditEngine(), "EditEngine missing" );
        const long nMaxVisAreaStart = mpEditView->GetEditEngine()->GetTextHeight() -
                                      mpEditView->GetOutputArea().GetHeight();
        if (mpEditView->GetVisArea().Top() > nMaxVisAreaStart)
        {
            Rectangle aVisArea(mpEditView->GetVisArea() );
            aVisArea.Top() = (nMaxVisAreaStart > 0 ) ? nMaxVisAreaStart : 0;
            aVisArea.SetSize(mpEditView->GetOutputArea().GetSize());
            mpEditView->SetVisArea(aVisArea);
            mpEditView->ShowCursor();
        }
        InitScrollBars();
    }
    Invalidate();
}




void EditWindow::MouseButtonUp(const MouseEvent &rEvt)
{
    if (mpEditView != NULL)
        mpEditView->MouseButtonUp(rEvt);
    else
        Window::MouseButtonUp (rEvt);

    // ggf FormulaCursor neu positionieren
    //  CursorMoveTimerHdl(&aCursorMoveTimer);
}




void EditWindow::MouseButtonDown(const MouseEvent &rEvt)
{
    if (mpEditView != NULL)
        mpEditView->MouseButtonDown(rEvt);
    else
        Window::MouseButtonDown (rEvt);

    GrabFocus();
}




void EditWindow::Command(const CommandEvent& rCEvt)
{
    /*  if (rCEvt.GetCommand() == COMMAND_CONTEXTMENU)
    {
        GetParent()->ToTop();

        Point aPoint = rCEvt.GetMousePosPixel();
        PopupMenu* pPopupMenu = new PopupMenu(SmResId(RID_COMMANDMENU));

        // added for replaceability of context menus #96085, #93782
        Menu* pMenu = NULL;
        ::com::sun::star::ui::ContextMenuExecuteEvent aEvent;
        aEvent.SourceWindow = VCLUnoHelper::GetInterface( this );
        aEvent.ExecutePosition.X = aPoint.X();
        aEvent.ExecutePosition.Y = aPoint.Y();
        if ( GetView()->TryContextMenuInterception( *pPopupMenu, pMenu, aEvent ) )
        {
            if ( pMenu )
            {
                delete pPopupMenu;
                pPopupMenu = (PopupMenu*) pMenu;
            }
        }

        pPopupMenu->SetSelectHdl(LINK(this, EditWindow, MenuSelectHdl));

        pPopupMenu->Execute( this, aPoint );
        delete pPopupMenu;
    }
    else*/ if (mpEditView)
        mpEditView->Command( rCEvt );
    else
        Window::Command (rCEvt);

}
IMPL_LINK_INLINE_START( EditWindow, MenuSelectHdl, Menu *, pMenu )
{
    /*    SmViewShell *pViewSh = rCmdBox.GetView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERTCOMMAND, SFX_CALLMODE_STANDARD,
                new SfxInt16Item(SID_INSERTCOMMAND, pMenu->GetCurItemId()), 0L);
*/
    return 0;
}
IMPL_LINK_INLINE_END( EditWindow, MenuSelectHdl, Menu *, pMenu )

void EditWindow::KeyInput(const KeyEvent& rKEvt)
{
    /*  if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        BOOL bCallBase = TRUE;
        SfxViewShell* pViewShell = SfxViewShell::Current();
        if ( pViewShell && pViewShell->ISA(SmViewShell) )
        {
            SmDocShell* pDocSh = (SmDocShell*) pViewShell->GetViewFrame()->GetObjectShell();
            if (pDocSh)
            {
    // fuert zum (sofortigen) Zerstoeren von this!
                pDocSh->DoInPlaceActivate( FALSE );
                bCallBase = FALSE;
            }
        }
        if ( bCallBase )
            Window::KeyInput( rKEvt );
    }
    else
    {
        // Timer neu starten, um den Handler (auch bei längeren Eingaben)
        // möglichst nur einmal am Ende aufzurufen.
        aCursorMoveTimer.Start();

        DBG_ASSERT( mpEditView, "EditView missing (NULL pointer)" );
        if (!mpEditView)
            CreateEditView();
        if ( !mpEditView->PostKeyEvent(rKEvt) )
        {
            if ( !SfxViewShell::Current()->KeyInput(rKEvt) )
            {
    // fuert bei F1 (Hilfe) zum Zerstoeren von this!
                Flush();
                if ( aModifyTimer.IsActive() )
                    aModifyTimer.Stop();
                Window::KeyInput(rKEvt);
            }
            else
            {
                //SFX hat evtl. Slot an der View gecallt und dabei (wg. Hack
                //im SFX) den Focus auf die View gesetzt
                SfxViewShell* pVShell = SfxViewShell::Current();
                if ( pVShell && pVShell->ISA(SmViewShell) &&
                     ((SmViewShell*)pVShell)->GetGraphicWindow().HasFocus() )
                {
                    GrabFocus();
                }
            }
        }
        else
        {
            // have doc-shell modified only for formula input/change and not
            // cursor travelling and such things...
            SmDocShell *pDocShell = GetDoc();
            if (pDocShell)
                pDocShell->SetModified( GetEditEngine()->IsModified() );

            aModifyTimer.Start();
        }
    }
    */
}




void EditWindow::Paint(const Rectangle& rRect)
{
    if (!mpEditView)
        CreateEditView();
    mpEditView->Paint(rRect);
}




void EditWindow::CreateEditView (void)
{
    EditEngine* pEditEngine = GetEditEngine();

    //! pEditEngine and mpEditView may be 0.
    //! For example when the program is used by the document-converter
    if (mpEditView==NULL && pEditEngine!=NULL)
    {
        mpEditView = new EditView (pEditEngine, this);
        pEditEngine->InsertView (mpEditView);

        if (mpVerticalScrollBar == NULL)
            mpVerticalScrollBar = new ScrollBar (
                this, WinBits(WB_VSCROLL | WB_DRAG));
        if (mpHorizontalScrollBar == NULL)
            mpHorizontalScrollBar = new ScrollBar (
                this, WinBits(WB_HSCROLL | WB_DRAG));
        if (mpScrollBox == NULL)
            mpScrollBox  = new ScrollBarBox (this);
        mpVerticalScrollBar->SetScrollHdl(LINK(this, EditWindow, ScrollHdl));
        mpHorizontalScrollBar->SetScrollHdl(LINK(this, EditWindow, ScrollHdl));

        mpEditView->SetOutputArea(AdjustScrollBars());

        ESelection eSelection;

        mpEditView->SetSelection(eSelection);
        Update();
        mpEditView->ShowCursor(TRUE, TRUE);

        pEditEngine->SetStatusEventHdl(
            LINK(this, EditWindow, EditStatusHdl));
        SetPointer(mpEditView->GetPointer());

        SetScrollBarRanges();
    }
}




IMPL_LINK( EditWindow, EditStatusHdl, EditStatus *, pStat )
{
    if (!mpEditView)
        return 1;
    else
    {
        SetScrollBarRanges();
        return 0;
    }
}

IMPL_LINK_INLINE_START( EditWindow, ScrollHdl, ScrollBar *, pScrollBar )
{
    DBG_ASSERT(mpEditView, "EditView missing");
    if (mpEditView)
    {
        mpEditView->SetVisArea(Rectangle(Point(mpHorizontalScrollBar->GetThumbPos(),
                                            mpVerticalScrollBar->GetThumbPos()),
                                        mpEditView->GetVisArea().GetSize()));
        mpEditView->Invalidate();
    }
    return 0;
}
IMPL_LINK_INLINE_END( EditWindow, ScrollHdl, ScrollBar *, pScrollBar )

Rectangle EditWindow::AdjustScrollBars()
{
    const Size aOut( GetOutputSizePixel() );
    Point aPoint;
    Rectangle aRect( aPoint, aOut );

    if (mpVerticalScrollBar && mpHorizontalScrollBar && mpScrollBox)
    {
        const long nTmp = GetSettings().GetStyleSettings().GetScrollBarSize();
        Point aPt( aRect.TopRight() ); aPt.X() -= nTmp -1L;
        mpVerticalScrollBar->SetPosSizePixel( aPt, Size(nTmp, aOut.Height() - nTmp));

        aPt = aRect.BottomLeft(); aPt.Y() -= nTmp - 1L;
        mpHorizontalScrollBar->SetPosSizePixel( aPt, Size(aOut.Width() - nTmp, nTmp));

        aPt.X() = mpHorizontalScrollBar->GetSizePixel().Width();
        aPt.Y() = mpVerticalScrollBar->GetSizePixel().Height();
        mpScrollBox->SetPosSizePixel(aPt, Size(nTmp, nTmp ));

        aRect.Right()  = aPt.X() - 2;
        aRect.Bottom() = aPt.Y() - 2;
    }
    return aRect;
}

void EditWindow::SetScrollBarRanges()
{
    EditEngine* pEditEngine = GetEditEngine();
    if (mpEditView != NULL && pEditEngine != NULL)
    {
        if (mpVerticalScrollBar != NULL)
        {
            long nTmp = pEditEngine->GetTextHeight();
            mpVerticalScrollBar->SetRange(Range(0, nTmp));
            mpVerticalScrollBar->SetThumbPos(mpEditView->GetVisArea().Top());
        }
        if (mpHorizontalScrollBar != NULL)
        {
            long nTmp = pEditEngine->GetPaperSize().Width();
            mpHorizontalScrollBar->SetRange(Range(0,nTmp));
            mpHorizontalScrollBar->SetThumbPos(mpEditView->GetVisArea().Left());
        }
    }
}

void EditWindow::InitScrollBars()
{
    if (mpEditView != NULL)
    {
        const Size aOut( mpEditView->GetOutputArea().GetSize() );
        if (mpVerticalScrollBar != NULL)
        {
            mpVerticalScrollBar->SetVisibleSize(aOut.Height());
            mpVerticalScrollBar->SetPageSize(aOut.Height() * 8 / 10);
            mpVerticalScrollBar->SetLineSize(aOut.Height() * 2 / 10);
        }

        if (mpHorizontalScrollBar != NULL)
        {
            mpHorizontalScrollBar->SetVisibleSize(aOut.Width());
            mpHorizontalScrollBar->SetPageSize(aOut.Width() * 8 / 10);
            mpHorizontalScrollBar->SetLineSize(SCROLL_LINE );
        }

        SetScrollBarRanges();

        if (mpVerticalScrollBar != NULL)
            mpVerticalScrollBar->Show();
        if (mpHorizontalScrollBar != NULL)
            mpHorizontalScrollBar->Show();
        if (mpScrollBox != NULL)
            mpScrollBox->Show();
    }
}


XubString EditWindow::GetText()
{
    String aText;
    EditEngine *pEditEngine = GetEditEngine();
    DBG_ASSERT( pEditEngine, "EditEngine missing" );
    if (pEditEngine)
        aText = pEditEngine->GetText( LINEEND_LF );
    return aText;
}


void EditWindow::SetText(const XubString& rText)
{
    EditEngine *pEditEngine = GetEditEngine();
    DBG_ASSERT( pEditEngine, "EditEngine missing" );
    if (pEditEngine  &&  !pEditEngine->IsModified())
    {
        if (!mpEditView)
            CreateEditView();

        ESelection eSelection = mpEditView->GetSelection();

        pEditEngine->SetText(rText);
        pEditEngine->ClearModifyFlag();

        //! Hier die Timer neu zu starten verhindert, dass die Handler für andere
        //! (im Augenblick nicht mehr aktive) Math Tasks aufgerufen werden.
        maModifyTimer.Start();
        maCursorMoveTimer.Start();

        mpEditView->SetSelection(eSelection);
    }
}


void EditWindow::GetFocus()
{
    Window::GetFocus();

    if (!mpEditView)
         CreateEditView();
    if (mpEditEngine != NULL)
        mpEditEngine->SetStatusEventHdl(
            LINK(this, EditWindow, EditStatusHdl));
}


void EditWindow::LoseFocus()
{
    if (mpEditEngine != NULL)
        mpEditEngine->SetStatusEventHdl (Link());

    Window::LoseFocus();
}


BOOL EditWindow::IsAllSelected() const
{
    BOOL bRes = FALSE;
    EditEngine *pEditEngine = ((EditWindow *) this)->GetEditEngine();
    DBG_ASSERT( mpEditView, "NULL pointer" );
    DBG_ASSERT( pEditEngine, "NULL pointer" );
    if (pEditEngine  &&  mpEditView)
    {
        ESelection eSelection( mpEditView->GetSelection() );
        INT32 nParaCnt = pEditEngine->GetParagraphCount();
        if (!(nParaCnt - 1))
        {
            String Text( pEditEngine->GetText( LINEEND_LF ) );
            bRes = !eSelection.nStartPos && (eSelection.nEndPos == Text.Len () - 1);
        }
        else
        {
            bRes = !eSelection.nStartPara && (eSelection.nEndPara == nParaCnt - 1);
        }
    }
    return bRes;
}

void EditWindow::SelectAll()
{
    DBG_ASSERT( mpEditView, "NULL pointer" );
    if (mpEditView)
    {
        // 0xFFFF as last two parameters refers to the end of the text
        mpEditView->SetSelection( ESelection( 0, 0, 0xFFFF, 0xFFFF ) );
    }
}


void EditWindow::MarkError(const Point &rPos)
{
    DBG_ASSERT( mpEditView, "EditView missing" );
    if (mpEditView)
    {
        const int Col = rPos.X();
        const int Row = rPos.Y() - 1;

        mpEditView->SetSelection(ESelection (Row, Col - 1, Row, Col));
        GrabFocus();
    }
}

void EditWindow::SelNextMark()
{
    EditEngine *pEditEngine = GetEditEngine();
    DBG_ASSERT( mpEditView, "NULL pointer" );
    DBG_ASSERT( pEditEngine, "NULL pointer" );
    if (pEditEngine  &&  mpEditView)
    {
        ESelection eSelection = mpEditView->GetSelection();
        USHORT     Pos        = eSelection.nEndPos;
        String     aMark (UniString::CreateFromAscii("<?>"));
        String     aText;
        USHORT     nCounts    = pEditEngine->GetParagraphCount();

        while (eSelection.nEndPara < nCounts)
        {
            aText = pEditEngine->GetText( eSelection.nEndPara );
            Pos   = aText.Search(aMark, Pos);

            if (Pos != STRING_NOTFOUND)
            {
                mpEditView->SetSelection(ESelection (eSelection.nEndPara, Pos, eSelection.nEndPara, Pos + 3));
                break;
            }

            Pos = 0;
            eSelection.nEndPara++;
        }
    }
}

void EditWindow::SelPrevMark()
{
    EditEngine *pEditEngine = GetEditEngine();
    DBG_ASSERT( pEditEngine, "NULL pointer" );
    DBG_ASSERT( mpEditView, "NULL pointer" );
    if (pEditEngine  &&  mpEditView)
    {
        ESelection eSelection = mpEditView->GetSelection();
        USHORT     Pos        = STRING_NOTFOUND;
        xub_StrLen Max        = eSelection.nStartPos;
        String     Text( pEditEngine->GetText( eSelection.nStartPara ) );
        String     aMark (UniString::CreateFromAscii("<?>"));
        USHORT     nCounts    = pEditEngine->GetParagraphCount();

        do
        {
            USHORT Fnd = Text.Search(aMark, 0);

            while ((Fnd < Max) && (Fnd != STRING_NOTFOUND))
            {
                Pos = Fnd;
                Fnd = Text.Search(aMark, Fnd + 1);
            }

            if (Pos == STRING_NOTFOUND)
            {
                eSelection.nStartPara--;
                Text = pEditEngine->GetText( eSelection.nStartPara );
                Max = Text.Len();
            }
        }
        while ((eSelection.nStartPara < nCounts) &&
            (Pos == STRING_NOTFOUND));

        if (Pos != STRING_NOTFOUND)
        {
            mpEditView->SetSelection(ESelection (eSelection.nStartPara, Pos, eSelection.nStartPara, Pos + 3));
        }
    }
}

BOOL EditWindow::HasMark(const String& rText) const
    // returns true iff 'rText' contains a mark
{
    return rText.SearchAscii("<?>", 0) != STRING_NOTFOUND;
}

void EditWindow::MouseMove(const MouseEvent &rEvt)
{
    if (mpEditView)
        mpEditView->MouseMove(rEvt);
}

sal_Int8 EditWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return mpEditView ? /*mpEditView->QueryDrop( rEvt )*/DND_ACTION_NONE: DND_ACTION_NONE;
}

sal_Int8 EditWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    return mpEditView ? /*mpEditView->Drop( rEvt )*/DND_ACTION_NONE : DND_ACTION_NONE;
}

ESelection EditWindow::GetSelection() const
{
    // pointer may be 0 when reloading a document and the old view
    // was already destroyed
    //(DBG_ASSERT( mpEditView, "NULL pointer" );
    ESelection eSel;
    if (mpEditView)
        eSel = mpEditView->GetSelection();
    return eSel;
}

void EditWindow::SetSelection(const ESelection &rSel)
{
    DBG_ASSERT( mpEditView, "NULL pointer" );
    if (mpEditView)
        mpEditView->SetSelection(rSel);
}

BOOL EditWindow::IsEmpty() const
{
    EditEngine *pEditEngine = ((EditWindow *) this)->GetEditEngine();
    return pEditEngine ? pEditEngine->GetTextLen() == 0 : FALSE;
}

BOOL EditWindow::IsSelected() const
{
    return mpEditView ? mpEditView->HasSelection() : FALSE;
}

void EditWindow::Cut()
{
    DBG_ASSERT( mpEditView, "EditView missing" );
    if (mpEditView)
        mpEditView->Cut();
}

void EditWindow::Copy()
{
    DBG_ASSERT( mpEditView, "EditView missing" );
    if (mpEditView)
        mpEditView->Copy();
}

void EditWindow::Paste()
{
    DBG_ASSERT( mpEditView, "EditView missing" );
    if (mpEditView)
        mpEditView->Paste();
}

void EditWindow::Delete()
{
    DBG_ASSERT( mpEditView, "EditView missing" );
    if (mpEditView)
        mpEditView->DeleteSelected();
}

void EditWindow::InsertText(const String& Text)
{
    DBG_ASSERT( mpEditView, "EditView missing" );
    ::vos::OGuard aGuard (::Application::GetSolarMutex());
    if (mpEditView)
        mpEditView->InsertText(Text);
}



} } // end of namespace ::sd::notes
