/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "EditWindow.hxx"

#include "sdmod.hxx"
#include <i18npool/mslangid.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <vcl/scrbar.hxx>
#include <editeng/eeitem.hxx>
#include "sdresid.hxx"
#include <svl/itempool.hxx>
#include <editeng/fhgtitem.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/editstat.hxx>

#define SCROLL_LINE         24

using namespace com::sun::star::accessibility;
using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace sd { namespace notes {

EditWindow::EditWindow (Window* pParentWindow, SfxItemPool* pItemPool)
    : Window (pParentWindow, WinBits()),
      DropTargetHelper(this),
      mpEditView(NULL),
      mpEditEngine(NULL),
      mpHorizontalScrollBar(NULL),
      mpVerticalScrollBar(NULL),
      mpScrollBox(NULL)
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
                            sal_uInt16 &nPara, sal_uInt16 &nPos)
    // returns paragraph number and position of the selections left part
{
    // compare start and end of selection and use the one that comes first
    if (
        (aSel.nStartPara <  aSel.nEndPara) ||
        (aSel.nStartPara == aSel.nEndPara && aSel.nStartPos < aSel.nEndPos)
       )
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
            sal_Int16       nFallbackLang;
            sal_Int16       nLang;
            sal_uInt16      nFontType;
            sal_uInt16      nFontInfoId;
            } aTable[3] =
        {
            // info to get western font to be used
            {   LANGUAGE_ENGLISH_US,    LANGUAGE_NONE,
                DEFAULTFONT_SERIF,      EE_CHAR_FONTINFO },
            // info to get CJK font to be used
            {   LANGUAGE_JAPANESE,      LANGUAGE_NONE,
                DEFAULTFONT_CJK_TEXT,   EE_CHAR_FONTINFO_CJK },
            // info to get CTL font to be used
            {   LANGUAGE_ARABIC_SAUDI_ARABIA,  LANGUAGE_NONE,
                DEFAULTFONT_CTL_TEXT,   EE_CHAR_FONTINFO_CTL }
        };
        aTable[0].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN);
        aTable[1].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN);
        aTable[2].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);
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

        pEditEngine->EnableUndo (sal_True);
        pEditEngine->SetDefTab (sal_uInt16(
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

        //      pEditEngine->SetDefTab( sal_uInt16( GetTextWidth( C2S("XXXX") ) ) );

        sal_uInt16 aFntInfoId[3] = {
                EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK, EE_CHAR_FONTINFO_CTL };
        for (int i = 0;  i < 3;  ++i)
        {
            const SfxPoolItem *pItem = mpEditEngineItemPool->GetPoolDefaultItem(  aFntInfoId[i] );
            if( pItem )
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
    if (mpEditView)
        mpEditView->Command( rCEvt );
    else
        Window::Command (rCEvt);

}
IMPL_LINK_INLINE_START( EditWindow, MenuSelectHdl, Menu *, EMPTYARG )
{
    return 0;
}
IMPL_LINK_INLINE_END( EditWindow, MenuSelectHdl, Menu *, EMPTYARG )

void EditWindow::KeyInput(const KeyEvent& )
{

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
        mpEditView->ShowCursor(sal_True, sal_True);

        pEditEngine->SetStatusEventHdl(
            LINK(this, EditWindow, EditStatusHdl));
        SetPointer(mpEditView->GetPointer());

        SetScrollBarRanges();
    }
}




IMPL_LINK( EditWindow, EditStatusHdl, EditStatus *, EMPTYARG )
{
    if (!mpEditView)
        return 1;
    else
    {
        SetScrollBarRanges();
        return 0;
    }
}

IMPL_LINK_INLINE_START( EditWindow, ScrollHdl, ScrollBar *, EMPTYARG )
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
IMPL_LINK_INLINE_END( EditWindow, ScrollHdl, ScrollBar *, EMPTYARG )

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

        //! Hier die Timer neu zu starten verhindert, dass die Handler f�r andere
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


sal_Bool EditWindow::IsAllSelected() const
{
    sal_Bool bRes = sal_False;
    EditEngine *pEditEngine = ((EditWindow *) this)->GetEditEngine();
    DBG_ASSERT( mpEditView, "NULL pointer" );
    DBG_ASSERT( pEditEngine, "NULL pointer" );
    if (pEditEngine  &&  mpEditView)
    {
        ESelection eSelection( mpEditView->GetSelection() );
        sal_Int32 nParaCnt = pEditEngine->GetParagraphCount();
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

        mpEditView->SetSelection(ESelection ( (sal_uInt16)Row, (sal_uInt16)(Col - 1), (sal_uInt16)Row, (sal_uInt16)Col));
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
        sal_uInt16     Pos        = eSelection.nEndPos;
        String     aMark (UniString::CreateFromAscii("<?>"));
        String     aText;
        sal_uInt16     nCounts    = pEditEngine->GetParagraphCount();

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
        sal_uInt16     Pos        = STRING_NOTFOUND;
        xub_StrLen Max        = eSelection.nStartPos;
        String     Text( pEditEngine->GetText( eSelection.nStartPara ) );
        String     aMark (UniString::CreateFromAscii("<?>"));
        sal_uInt16     nCounts    = pEditEngine->GetParagraphCount();

        do
        {
            sal_uInt16 Fnd = Text.Search(aMark, 0);

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

sal_Bool EditWindow::HasMark(const String& rText) const
    // returns true iff 'rText' contains a mark
{
    return rText.SearchAscii("<?>", 0) != STRING_NOTFOUND;
}

void EditWindow::MouseMove(const MouseEvent &rEvt)
{
    if (mpEditView)
        mpEditView->MouseMove(rEvt);
}

sal_Int8 EditWindow::AcceptDrop( const AcceptDropEvent& )
{
    return mpEditView ? /*mpEditView->QueryDrop( rEvt )*/DND_ACTION_NONE: DND_ACTION_NONE;
}

sal_Int8 EditWindow::ExecuteDrop( const ExecuteDropEvent& )
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

sal_Bool EditWindow::IsEmpty() const
{
    EditEngine *pEditEngine = ((EditWindow *) this)->GetEditEngine();
    return (pEditEngine && (pEditEngine->GetTextLen() == 0)) ? sal_True : sal_False;
}

sal_Bool EditWindow::IsSelected() const
{
    return mpEditView ? mpEditView->HasSelection() : sal_False;
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
    SolarMutexGuard aGuard;
    if (mpEditView)
        mpEditView->InsertText(Text);
}



} } // end of namespace ::sd::notes

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
