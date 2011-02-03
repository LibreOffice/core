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
#include "precompiled_svtools.hxx"
#include <svtools/textview.hxx>
#include <svtools/texteng.hxx>
#include <textdoc.hxx>
#include <svtools/textdata.hxx>
#include <textdat2.hxx>

#include <svl/undo.hxx>
#include <vcl/cursor.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sound.hxx>
#include <tools/stream.hxx>

#include <sot/formats.hxx>
#include <svl/urlbmk.hxx>

#ifndef _COM_SUN_STAR_TEXT_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#include <cppuhelper/weak.hxx>
#include <vcl/unohelp.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>

#include <vcl/edit.hxx>


#include <sot/exchange.hxx>
#include <sot/formats.hxx>

#include <vos/mutex.hxx>


using namespace ::com::sun::star;

class TETextDataObject :    public ::com::sun::star::datatransfer::XTransferable,
                        public ::cppu::OWeakObject

{
private:
    String          maText;
    SvMemoryStream  maHTMLStream;

public:
                    TETextDataObject( const String& rText );
                    ~TETextDataObject();

    String&         GetText() { return maText; }
    SvMemoryStream& GetHTMLStream() { return maHTMLStream; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::datatransfer::XTransferable
    ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw(::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw(::com::sun::star::uno::RuntimeException);
};

TETextDataObject::TETextDataObject( const String& rText ) : maText( rText )
{
}

TETextDataObject::~TETextDataObject()
{
}

// uno::XInterface
uno::Any TETextDataObject::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType, SAL_STATIC_CAST( datatransfer::XTransferable*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// datatransfer::XTransferable
uno::Any TETextDataObject::getTransferData( const datatransfer::DataFlavor& rFlavor ) throw(datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException)
{
    uno::Any aAny;

    ULONG nT = SotExchange::GetFormat( rFlavor );
    if ( nT == SOT_FORMAT_STRING )
    {
        aAny <<= (::rtl::OUString)GetText();
    }
    else if ( nT == SOT_FORMATSTR_ID_HTML )
    {
        GetHTMLStream().Seek( STREAM_SEEK_TO_END );
        ULONG nLen = GetHTMLStream().Tell();
        GetHTMLStream().Seek(0);

        uno::Sequence< sal_Int8 > aSeq( nLen );
        memcpy( aSeq.getArray(), GetHTMLStream().GetData(), nLen );
        aAny <<= aSeq;
    }
    else
    {
        throw datatransfer::UnsupportedFlavorException();
    }
    return aAny;
}

uno::Sequence< datatransfer::DataFlavor > TETextDataObject::getTransferDataFlavors(  ) throw(uno::RuntimeException)
{
    GetHTMLStream().Seek( STREAM_SEEK_TO_END );
    BOOL bHTML = GetHTMLStream().Tell() > 0;
    uno::Sequence< datatransfer::DataFlavor > aDataFlavors( bHTML ? 2 : 1 );
    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aDataFlavors.getArray()[0] );
    if ( bHTML )
        SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_HTML, aDataFlavors.getArray()[1] );
    return aDataFlavors;
}

sal_Bool TETextDataObject::isDataFlavorSupported( const datatransfer::DataFlavor& rFlavor ) throw(uno::RuntimeException)
{
    ULONG nT = SotExchange::GetFormat( rFlavor );
    return ( nT == SOT_FORMAT_STRING );
}

/*-- 24.06.2004 13:54:36---------------------------------------------------

  -----------------------------------------------------------------------*/
struct ImpTextView
{
    TextEngine*         mpTextEngine;

    Window*             mpWindow;
    TextSelection       maSelection;
    Point               maStartDocPos;
//    TextPaM             maMBDownPaM;

    Cursor*             mpCursor;

    TextDDInfo*         mpDDInfo;

    VirtualDevice*      mpVirtDev;

    SelectionEngine*    mpSelEngine;
    TextSelFunctionSet* mpSelFuncSet;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener > mxDnDListener;

    USHORT              mnTravelXPos;

    BOOL                mbAutoScroll            : 1;
    BOOL                mbInsertMode            : 1;
    BOOL                mbReadOnly              : 1;
    BOOL                mbPaintSelection        : 1;
    BOOL                mbAutoIndent            : 1;
    BOOL                mbHighlightSelection    : 1;
    BOOL                mbCursorEnabled         : 1;
    BOOL                mbClickedInSelection    : 1;
    BOOL                mbSupportProtectAttribute : 1;
    bool                mbCursorAtEndOfLine;
};

// -------------------------------------------------------------------------
// (+) class TextView
// -------------------------------------------------------------------------
TextView::TextView( TextEngine* pEng, Window* pWindow ) :
    mpImpl(new ImpTextView)
{
    pWindow->EnableRTL( FALSE );

    mpImpl->mpWindow = pWindow;
    mpImpl->mpTextEngine = pEng;
    mpImpl->mpVirtDev = NULL;

    mpImpl->mbPaintSelection = TRUE;
    mpImpl->mbAutoScroll = TRUE;
    mpImpl->mbInsertMode = TRUE;
    mpImpl->mbReadOnly = FALSE;
    mpImpl->mbHighlightSelection = FALSE;
    mpImpl->mbAutoIndent = FALSE;
    mpImpl->mbCursorEnabled = TRUE;
    mpImpl->mbClickedInSelection = FALSE;
    mpImpl->mbSupportProtectAttribute = FALSE;
    mpImpl->mbCursorAtEndOfLine = false;
//  mbInSelection = FALSE;

    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;

    mpImpl->mpSelFuncSet = new TextSelFunctionSet( this );
    mpImpl->mpSelEngine = new SelectionEngine( mpImpl->mpWindow, mpImpl->mpSelFuncSet );
    mpImpl->mpSelEngine->SetSelectionMode( RANGE_SELECTION );
    mpImpl->mpSelEngine->EnableDrag( TRUE );

    mpImpl->mpCursor = new Cursor;
    mpImpl->mpCursor->Show();
    pWindow->SetCursor( mpImpl->mpCursor );
    pWindow->SetInputContext( InputContext( pEng->GetFont(), INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT ) );

    if ( pWindow->GetSettings().GetStyleSettings().GetSelectionOptions() & SELECTION_OPTION_INVERT )
        mpImpl->mbHighlightSelection = TRUE;

    pWindow->SetLineColor();

    mpImpl->mpDDInfo = NULL;

    if ( pWindow->GetDragGestureRecognizer().is() )
    {
        vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper( this );
        mpImpl->mxDnDListener = pDnDWrapper;

        uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mpImpl->mxDnDListener, uno::UNO_QUERY );
        pWindow->GetDragGestureRecognizer()->addDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( xDGL, uno::UNO_QUERY );
        pWindow->GetDropTarget()->addDropTargetListener( xDTL );
        pWindow->GetDropTarget()->setActive( sal_True );
        pWindow->GetDropTarget()->setDefaultActions( datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE );
    }
}

TextView::~TextView()
{
    delete mpImpl->mpSelEngine;
    delete mpImpl->mpSelFuncSet;
    delete mpImpl->mpVirtDev;

    if ( mpImpl->mpWindow->GetCursor() == mpImpl->mpCursor )
        mpImpl->mpWindow->SetCursor( 0 );
    delete mpImpl->mpCursor;
    delete mpImpl->mpDDInfo;
    delete mpImpl;
}

void TextView::Invalidate()
{
    mpImpl->mpWindow->Invalidate();
}

void TextView::SetSelection( const TextSelection& rTextSel, BOOL bGotoCursor )
{
    // Falls jemand gerade ein leeres Attribut hinterlassen hat,
    // und dann der Outliner die Selektion manipulitert:
    if ( !mpImpl->maSelection.HasRange() )
        mpImpl->mpTextEngine->CursorMoved( mpImpl->maSelection.GetStart().GetPara() );

    // Wenn nach einem KeyInput die Selection manipuliert wird:
    mpImpl->mpTextEngine->CheckIdleFormatter();

    HideSelection();
    TextSelection aNewSel( rTextSel );
    mpImpl->mpTextEngine->ValidateSelection( aNewSel );
    ImpSetSelection( aNewSel );
    ShowSelection();
    ShowCursor( bGotoCursor );
}

void TextView::SetSelection( const TextSelection& rTextSel )
{
    SetSelection( rTextSel, mpImpl->mbAutoScroll );
}

const TextSelection& TextView::GetSelection() const
{
    return mpImpl->maSelection;
}
TextSelection&      TextView::GetSelection()
{
    return mpImpl->maSelection;
}

void TextView::DeleteSelected()
{
//  HideSelection();

    mpImpl->mpTextEngine->UndoActionStart();
    TextPaM aPaM = mpImpl->mpTextEngine->ImpDeleteText( mpImpl->maSelection );
    mpImpl->mpTextEngine->UndoActionEnd();

    ImpSetSelection( aPaM );
    mpImpl->mpTextEngine->FormatAndUpdate( this );
    ShowCursor();
}

void TextView::ImpPaint( OutputDevice* pOut, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange, TextSelection const* pSelection )
{
    if ( !mpImpl->mbPaintSelection )
        pSelection = NULL;
    else
    {
        // Richtige Hintergrundfarbe einstellen.
        // Ich bekomme leider nicht mit, ob sich diese inzwischen geaendert hat.
        Font aFont = mpImpl->mpTextEngine->GetFont();
        Color aColor = pOut->GetBackground().GetColor();
        aColor.SetTransparency( 0 );
        if ( aColor != aFont.GetFillColor() )
        {
            if( aFont.IsTransparent() )
                aColor = Color( COL_TRANSPARENT );
            aFont.SetFillColor( aColor );
            mpImpl->mpTextEngine->maFont = aFont;
        }
    }

    mpImpl->mpTextEngine->ImpPaint( pOut, rStartPos, pPaintArea, pPaintRange, pSelection );
}

void TextView::Paint( const Rectangle& rRect )
{
    ImpPaint( rRect, FALSE );
}

void TextView::ImpPaint( const Rectangle& rRect, BOOL bUseVirtDev )
{
    if ( !mpImpl->mpTextEngine->GetUpdateMode() || mpImpl->mpTextEngine->IsInUndo() )
        return;

    TextSelection *pDrawSelection = NULL;
    if ( !mpImpl->mbHighlightSelection && mpImpl->maSelection.HasRange() )
        pDrawSelection = &mpImpl->maSelection;

    if ( bUseVirtDev )
    {
        VirtualDevice* pVDev = GetVirtualDevice();

        const Color& rBackgroundColor = mpImpl->mpWindow->GetBackground().GetColor();
        if ( pVDev->GetFillColor() != rBackgroundColor )
            pVDev->SetFillColor( rBackgroundColor );
        if ( pVDev->GetBackground().GetColor() != rBackgroundColor )
            pVDev->SetBackground( rBackgroundColor );

        BOOL bVDevValid = TRUE;
        Size aOutSz( pVDev->GetOutputSizePixel() );
        if ( (  aOutSz.Width() < rRect.GetWidth() ) ||
             (  aOutSz.Height() < rRect.GetHeight() ) )
        {
            bVDevValid = pVDev->SetOutputSizePixel( rRect.GetSize() );
        }
        else
        {
            // Das VirtDev kann bei einem Resize sehr gross werden =>
            // irgendwann mal kleiner machen!
            if ( ( aOutSz.Height() > ( rRect.GetHeight() + 20 ) ) ||
                 ( aOutSz.Width() > ( rRect.GetWidth() + 20 ) ) )
            {
                bVDevValid = pVDev->SetOutputSizePixel( rRect.GetSize() );
            }
            else
            {
                pVDev->Erase();
            }
        }
        if ( !bVDevValid )
        {
            ImpPaint( rRect, FALSE /* ohne VDev */ );
            return;
        }

        Rectangle aTmpRec( Point( 0, 0 ), rRect.GetSize() );

        Point aDocPos( mpImpl->maStartDocPos.X(), mpImpl->maStartDocPos.Y() + rRect.Top() );
        Point aStartPos = ImpGetOutputStartPos( aDocPos );
        ImpPaint( pVDev, aStartPos, &aTmpRec, NULL, pDrawSelection );
        mpImpl->mpWindow->DrawOutDev( rRect.TopLeft(), rRect.GetSize(),
                                Point(0,0), rRect.GetSize(), *pVDev );
//      ShowSelection();
        if ( mpImpl->mbHighlightSelection )
            ImpHighlight( mpImpl->maSelection );
    }
    else
    {
        Point aStartPos = ImpGetOutputStartPos( mpImpl->maStartDocPos );
        ImpPaint( mpImpl->mpWindow, aStartPos, &rRect, NULL, pDrawSelection );

//      ShowSelection();
        if ( mpImpl->mbHighlightSelection )
            ImpHighlight( mpImpl->maSelection );
    }
}

void TextView::ImpHighlight( const TextSelection& rSel )
{
    TextSelection aSel( rSel );
    aSel.Justify();
    if ( aSel.HasRange() && !mpImpl->mpTextEngine->IsInUndo() && mpImpl->mpTextEngine->GetUpdateMode() )
    {
        mpImpl->mpCursor->Hide();

        DBG_ASSERT( !mpImpl->mpTextEngine->mpIdleFormatter->IsActive(), "ImpHighlight: Not formatted!" );

        Rectangle aVisArea( mpImpl->maStartDocPos, mpImpl->mpWindow->GetOutputSizePixel() );
        long nY = 0;
        ULONG nStartPara = aSel.GetStart().GetPara();
        ULONG nEndPara = aSel.GetEnd().GetPara();
        for ( ULONG nPara = 0; nPara <= nEndPara; nPara++ )
        {
            long nParaHeight = (long)mpImpl->mpTextEngine->CalcParaHeight( nPara );
            if ( ( nPara >= nStartPara ) && ( ( nY + nParaHeight ) > aVisArea.Top() ) )
            {
                TEParaPortion* pTEParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( nPara );
                USHORT nStartLine = 0;
                USHORT nEndLine = pTEParaPortion->GetLines().Count() -1;
                if ( nPara == nStartPara )
                    nStartLine = pTEParaPortion->GetLineNumber( aSel.GetStart().GetIndex(), FALSE );
                if ( nPara == nEndPara )
                    nEndLine = pTEParaPortion->GetLineNumber( aSel.GetEnd().GetIndex(), TRUE );

                // ueber die Zeilen iterieren....
                for ( USHORT nLine = nStartLine; nLine <= nEndLine; nLine++ )
                {
                    TextLine* pLine = pTEParaPortion->GetLines().GetObject( nLine );
                    USHORT nStartIndex = pLine->GetStart();
                    USHORT nEndIndex = pLine->GetEnd();
                    if ( ( nPara == nStartPara ) && ( nLine == nStartLine ) )
                        nStartIndex = aSel.GetStart().GetIndex();
                    if ( ( nPara == nEndPara ) && ( nLine == nEndLine ) )
                        nEndIndex = aSel.GetEnd().GetIndex();

                    // Kann passieren, wenn am Anfang einer umgebrochenen Zeile.
                    if ( nEndIndex < nStartIndex )
                        nEndIndex = nStartIndex;

                    Rectangle aTmpRec( mpImpl->mpTextEngine->GetEditCursor( TextPaM( nPara, nStartIndex ), FALSE ) );
                    aTmpRec.Top() += nY;
                    aTmpRec.Bottom() += nY;
                    Point aTopLeft( aTmpRec.TopLeft() );

                    aTmpRec = mpImpl->mpTextEngine->GetEditCursor( TextPaM( nPara, nEndIndex ), TRUE );
                    aTmpRec.Top() += nY;
                    aTmpRec.Bottom() += nY;
                    Point aBottomRight( aTmpRec.BottomRight() );
                    aBottomRight.X()--;

                    // Nur Painten, wenn im sichtbaren Bereich...
                    if ( ( aTopLeft.X() < aBottomRight.X() ) && ( aBottomRight.Y() >= aVisArea.Top() ) )
                    {
                        Point aPnt1( GetWindowPos( aTopLeft ) );
                        Point aPnt2( GetWindowPos( aBottomRight ) );

                        Rectangle aRect( aPnt1, aPnt2 );
                        mpImpl->mpWindow->Invert( aRect );
                    }
                }
            }
            nY += nParaHeight;

            if ( nY >= aVisArea.Bottom() )
                break;
        }
    }
}

void TextView::ImpSetSelection( const TextSelection& rSelection )
{
    if ( rSelection != mpImpl->maSelection )
    {
        mpImpl->maSelection = rSelection;
        mpImpl->mpTextEngine->Broadcast( TextHint( TEXT_HINT_VIEWSELECTIONCHANGED ) );
    }
}

void TextView::ShowSelection()
{
    ImpShowHideSelection( TRUE );
}

void TextView::HideSelection()
{
    ImpShowHideSelection( FALSE );
}

void TextView::ShowSelection( const TextSelection& rRange )
{
    ImpShowHideSelection( TRUE, &rRange );
}

void TextView::ImpShowHideSelection( BOOL bShow, const TextSelection* pRange )
{
    const TextSelection* pRangeOrSelection = pRange ? pRange : &mpImpl->maSelection;

    if ( pRangeOrSelection->HasRange() )
    {
        if ( mpImpl->mbHighlightSelection )
        {
            ImpHighlight( *pRangeOrSelection );
        }
        else
        {
            if( mpImpl->mpWindow->IsPaintTransparent() )
                mpImpl->mpWindow->Invalidate();
            else
            {
                Rectangle aOutArea( Point( 0, 0 ), mpImpl->mpWindow->GetOutputSizePixel() );
                Point aStartPos( ImpGetOutputStartPos( mpImpl->maStartDocPos ) );
                TextSelection aRange( *pRangeOrSelection );
                aRange.Justify();
                BOOL bVisCursor = mpImpl->mpCursor->IsVisible();
                mpImpl->mpCursor->Hide();
                ImpPaint( mpImpl->mpWindow, aStartPos, &aOutArea, &aRange, bShow ? &mpImpl->maSelection : NULL );
                if ( bVisCursor )
                    mpImpl->mpCursor->Show();
            }
        }
    }
}

VirtualDevice* TextView::GetVirtualDevice()
{
    if ( !mpImpl->mpVirtDev )
    {
        mpImpl->mpVirtDev = new VirtualDevice;
        mpImpl->mpVirtDev->SetLineColor();
    }
    return mpImpl->mpVirtDev;
}

void TextView::EraseVirtualDevice()
{
    delete mpImpl->mpVirtDev;
    mpImpl->mpVirtDev = 0;
}

BOOL TextView::KeyInput( const KeyEvent& rKeyEvent )
{
    BOOL bDone      = TRUE;
    BOOL bModified  = FALSE;
    BOOL bMoved     = FALSE;
    BOOL bEndKey    = FALSE;    // spezielle CursorPosition
    BOOL bAllowIdle = TRUE;

    // Um zu pruefen ob durch irgendeine Aktion mModified, das lokale
    // bModified wird z.B. bei Cut/Paste nicht gesetzt, weil dort an anderen
    // Stellen das updaten erfolgt.
    BOOL bWasModified = mpImpl->mpTextEngine->IsModified();
    mpImpl->mpTextEngine->SetModified( FALSE );

    TextSelection aCurSel( mpImpl->maSelection );
    TextSelection aOldSel( aCurSel );

    USHORT nCode = rKeyEvent.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !mpImpl->mbReadOnly )
                    Cut();
            }
            break;
            case KEYFUNC_COPY:
            {
                Copy();
            }
            break;
            case KEYFUNC_PASTE:
            {
                if ( !mpImpl->mbReadOnly )
                    Paste();
            }
            break;
            case KEYFUNC_UNDO:
            {
                if ( !mpImpl->mbReadOnly )
                    Undo();
            }
            break;
            case KEYFUNC_REDO:
            {
                if ( !mpImpl->mbReadOnly )
                    Redo();
            }
            break;

            default:    // wird dann evtl. unten bearbeitet.
                        eFunc = KEYFUNC_DONTKNOW;
        }
    }
    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( nCode )
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case com::sun::star::awt::Key::MOVE_WORD_FORWARD:
            case com::sun::star::awt::Key::SELECT_WORD_FORWARD:
            case com::sun::star::awt::Key::MOVE_WORD_BACKWARD:
            case com::sun::star::awt::Key::SELECT_WORD_BACKWARD:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_LINE:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_LINE:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_DOCUMENT:
            {
                if ( ( !rKeyEvent.GetKeyCode().IsMod2() || ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) )
                      && !( rKeyEvent.GetKeyCode().IsMod1() && ( nCode == KEY_PAGEUP || nCode == KEY_PAGEDOWN ) ) )
                {
                    aCurSel = ImpMoveCursor( rKeyEvent );
                    if ( aCurSel.HasRange() ) {
                        uno::Reference<datatransfer::clipboard::XClipboard> aSelection(GetWindow()->GetPrimarySelection());
                        Copy( aSelection );
                    }
                    bMoved = TRUE;
                    if ( nCode == KEY_END )
                        bEndKey = TRUE;
                }
                else
                    bDone = FALSE;
            }
            break;
            case KEY_BACKSPACE:
            case KEY_DELETE:
            case com::sun::star::awt::Key::DELETE_WORD_BACKWARD:
            case com::sun::star::awt::Key::DELETE_WORD_FORWARD:
            case com::sun::star::awt::Key::DELETE_TO_BEGIN_OF_LINE:
            case com::sun::star::awt::Key::DELETE_TO_END_OF_LINE:
            {
                if ( !mpImpl->mbReadOnly && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    BYTE nDel = ( nCode == KEY_DELETE ) ? DEL_RIGHT : DEL_LEFT;
                    BYTE nMode = rKeyEvent.GetKeyCode().IsMod1() ? DELMODE_RESTOFWORD : DELMODE_SIMPLE;
                    if ( ( nMode == DELMODE_RESTOFWORD ) && rKeyEvent.GetKeyCode().IsShift() )
                        nMode = DELMODE_RESTOFCONTENT;

                    switch( nCode )
                    {
                    case com::sun::star::awt::Key::DELETE_WORD_BACKWARD:
                        nDel = DEL_LEFT;
                        nMode = DELMODE_RESTOFWORD;
                        break;
                    case com::sun::star::awt::Key::DELETE_WORD_FORWARD:
                        nDel = DEL_RIGHT;
                        nMode = DELMODE_RESTOFWORD;
                        break;
                    case com::sun::star::awt::Key::DELETE_TO_BEGIN_OF_LINE:
                        nDel = DEL_LEFT;
                        nMode = DELMODE_RESTOFCONTENT;
                        break;
                    case com::sun::star::awt::Key::DELETE_TO_END_OF_LINE:
                        nDel = DEL_RIGHT;
                        nMode = DELMODE_RESTOFCONTENT;
                        break;
                    default: break;
                    }

                    mpImpl->mpTextEngine->UndoActionStart();
                    if(mpImpl->mbSupportProtectAttribute)
                    {
                        //expand selection to include all protected content - if there is any
                        const TextCharAttrib* pStartAttr = mpImpl->mpTextEngine->FindCharAttrib(
                                    TextPaM(mpImpl->maSelection.GetStart().GetPara(),
                                    mpImpl->maSelection.GetStart().GetIndex()),
                                    TEXTATTR_PROTECTED );
                        const TextCharAttrib* pEndAttr = mpImpl->mpTextEngine->FindCharAttrib(
                                    TextPaM(mpImpl->maSelection.GetEnd().GetPara(),
                                    mpImpl->maSelection.GetEnd().GetIndex()),
                                    TEXTATTR_PROTECTED );
                        if(pStartAttr && pStartAttr->GetStart() < mpImpl->maSelection.GetStart().GetIndex())
                        {
                            mpImpl->maSelection.GetStart().GetIndex() = pStartAttr->GetStart();
                        }
                        if(pEndAttr && pEndAttr->GetEnd() > mpImpl->maSelection.GetEnd().GetIndex())
                        {
                            mpImpl->maSelection.GetEnd().GetIndex() = pEndAttr->GetEnd();
                        }
                    }
                    aCurSel = ImpDelete( nDel, nMode );
                    mpImpl->mpTextEngine->UndoActionEnd();
                    bModified = TRUE;
                    bAllowIdle = FALSE;
                }
                else
                    bDone = FALSE;
            }
            break;
            case KEY_TAB:
            {
                if ( !mpImpl->mbReadOnly && !rKeyEvent.GetKeyCode().IsShift() &&
                        !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() &&
                        ImplCheckTextLen( 'x' ) )
                {
                    aCurSel = mpImpl->mpTextEngine->ImpInsertText( aCurSel, '\t', !IsInsertMode() );
                    bModified = TRUE;
                }
                else
                    bDone = FALSE;
            }
            break;
            case KEY_RETURN:
            {
                // Shift-RETURN darf nicht geschluckt werden, weil dann keine
                // mehrzeilige Eingabe in Dialogen/Property-Editor moeglich.
                if ( !mpImpl->mbReadOnly && !rKeyEvent.GetKeyCode().IsMod1() &&
                        !rKeyEvent.GetKeyCode().IsMod2() && ImplCheckTextLen( 'x' ) )
                {
                    mpImpl->mpTextEngine->UndoActionStart();
                    aCurSel = mpImpl->mpTextEngine->ImpInsertParaBreak( aCurSel );
                    if ( mpImpl->mbAutoIndent )
                    {
                        TextNode* pPrev = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aCurSel.GetEnd().GetPara() - 1 );
                        USHORT n = 0;
                        while ( ( n < pPrev->GetText().Len() ) && (
                                    ( pPrev->GetText().GetChar( n ) == ' ' ) ||
                                    ( pPrev->GetText().GetChar( n ) == '\t' ) ) )
                        {
                            n++;
                        }
                        if ( n )
                            aCurSel = mpImpl->mpTextEngine->ImpInsertText( aCurSel, pPrev->GetText().Copy( 0, n ) );
                    }
                    mpImpl->mpTextEngine->UndoActionEnd();
                    bModified = TRUE;
                }
                else
                    bDone = FALSE;
            }
            break;
            case KEY_INSERT:
            {
                if ( !mpImpl->mbReadOnly )
                    SetInsertMode( !IsInsertMode() );
            }
            break;
            default:
            {
                if ( TextEngine::IsSimpleCharInput( rKeyEvent ) )
                {
                    xub_Unicode nCharCode = rKeyEvent.GetCharCode();
                    if ( !mpImpl->mbReadOnly && ImplCheckTextLen( nCharCode ) )    // sonst trotzdem das Zeichen schlucken...
                    {
                        aCurSel = mpImpl->mpTextEngine->ImpInsertText( nCharCode, aCurSel, !IsInsertMode(), sal_True );
                        bModified = TRUE;
                    }
                }
                else
                    bDone = FALSE;
            }
        }
    }

    if ( aCurSel != aOldSel )   // Check if changed, maybe other method already changed mpImpl->maSelection, don't overwrite that!
        ImpSetSelection( aCurSel );

    mpImpl->mpTextEngine->UpdateSelections();

    if ( ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) )
        mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;

    if ( bModified )
    {
        // Idle-Formatter nur, wenn AnyInput.
        if ( bAllowIdle && Application::AnyInput( INPUT_KEYBOARD) )
            mpImpl->mpTextEngine->IdleFormatAndUpdate( this );
        else
            mpImpl->mpTextEngine->FormatAndUpdate( this);
    }
    else if ( bMoved )
    {
        // Selection wird jetzt gezielt in ImpMoveCursor gemalt.
        ImpShowCursor( mpImpl->mbAutoScroll, TRUE, bEndKey );
    }

    if ( mpImpl->mpTextEngine->IsModified() )
        mpImpl->mpTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
    else if ( bWasModified )
        mpImpl->mpTextEngine->SetModified( TRUE );

    return bDone;
}

void TextView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    mpImpl->mbClickedInSelection = FALSE;
    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpImpl->mpSelEngine->SelMouseButtonUp( rMouseEvent );
    if ( rMouseEvent.IsMiddle() && !IsReadOnly() &&
         ( GetWindow()->GetSettings().GetMouseSettings().GetMiddleButtonAction() == MOUSE_MIDDLE_PASTESELECTION ) )
    {
        uno::Reference<datatransfer::clipboard::XClipboard> aSelection(GetWindow()->GetPrimarySelection());
        Paste( aSelection );
        if ( mpImpl->mpTextEngine->IsModified() )
            mpImpl->mpTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
    }
    else if ( rMouseEvent.IsLeft() && GetSelection().HasRange() )
    {
        uno::Reference<datatransfer::clipboard::XClipboard> aSelection(GetWindow()->GetPrimarySelection());
        Copy( aSelection );
    }
}

void TextView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    mpImpl->mpTextEngine->CheckIdleFormatter();    // Falls schnelles Tippen und MouseButtonDown
    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpImpl->mbClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );

    mpImpl->mpTextEngine->SetActiveView( this );

    mpImpl->mpSelEngine->SelMouseButtonDown( rMouseEvent );

    // mbu 20.01.2005 - SelMouseButtonDown() possibly triggers a 'selection changed'
    // notification. The appropriate handler could change the current selection,
    // which is the case in the MailMerge address block control. To enable select'n'drag
    // we need to reevaluate the selection after the notification has been fired.
    mpImpl->mbClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );

    // Sonderbehandlungen
    if ( !rMouseEvent.IsShift() && ( rMouseEvent.GetClicks() >= 2 ) )
    {
        if ( rMouseEvent.IsMod2() )
        {
            HideSelection();
            ImpSetSelection( mpImpl->maSelection.GetEnd() );
            SetCursorAtPoint( rMouseEvent.GetPosPixel() );  // Wird von SelectionEngine bei MOD2 nicht gesetzt
        }

        if ( rMouseEvent.GetClicks() == 2 )
        {
            // Wort selektieren
            if ( mpImpl->maSelection.GetEnd().GetIndex() < mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection.GetEnd().GetPara() ) )
            {
                HideSelection();
                TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject(  mpImpl->maSelection.GetEnd().GetPara() );
                uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
                i18n::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                TextSelection aNewSel( mpImpl->maSelection );
                aNewSel.GetStart().GetIndex() = (USHORT)aBoundary.startPos;
                aNewSel.GetEnd().GetIndex() = (USHORT)aBoundary.endPos;
                if(mpImpl->mbSupportProtectAttribute)
                {
                    //expand selection to include all protected content - if there is any
                    const TextCharAttrib* pStartAttr = mpImpl->mpTextEngine->FindCharAttrib(
                                TextPaM(aNewSel.GetStart().GetPara(),
                                (USHORT)aBoundary.startPos),
                                TEXTATTR_PROTECTED );
                    const TextCharAttrib* pEndAttr = mpImpl->mpTextEngine->FindCharAttrib(
                                TextPaM(aNewSel.GetEnd().GetPara(),
                                (USHORT)aBoundary.endPos),
                                TEXTATTR_PROTECTED );
                    if(pStartAttr && pStartAttr->GetStart() < aNewSel.GetStart().GetIndex())
                    {
                        aNewSel.GetStart().GetIndex() = pStartAttr->GetStart();
                    }
                    if(pEndAttr && pEndAttr->GetEnd() > aNewSel.GetEnd().GetIndex())
                    {
                        aNewSel.GetEnd().GetIndex() = pEndAttr->GetEnd();
                    }
                }
                ImpSetSelection( aNewSel );
                ShowSelection();
                ShowCursor( TRUE, TRUE );
            }
        }
        else if ( rMouseEvent.GetClicks() == 3 )
        {
            // Absatz selektieren
            if ( mpImpl->maSelection.GetStart().GetIndex() || ( mpImpl->maSelection.GetEnd().GetIndex() < mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection.GetEnd().GetPara() ) ) )
            {
                HideSelection();
                TextSelection aNewSel( mpImpl->maSelection );
                aNewSel.GetStart().GetIndex() = 0;
                aNewSel.GetEnd().GetIndex() = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( mpImpl->maSelection.GetEnd().GetPara() )->GetText().Len();
                ImpSetSelection( aNewSel );
                ShowSelection();
                ShowCursor( TRUE, TRUE );
            }
        }
    }
}


void TextView::MouseMove( const MouseEvent& rMouseEvent )
{
    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpImpl->mpSelEngine->SelMouseMove( rMouseEvent );
}

void TextView::Command( const CommandEvent& rCEvt )
{
    mpImpl->mpTextEngine->CheckIdleFormatter();    // Falls schnelles Tippen und MouseButtonDown
    mpImpl->mpTextEngine->SetActiveView( this );

    if ( rCEvt.GetCommand() == COMMAND_STARTEXTTEXTINPUT )
    {
        DeleteSelected();
        delete mpImpl->mpTextEngine->mpIMEInfos;
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( GetSelection().GetEnd().GetPara() );
        mpImpl->mpTextEngine->mpIMEInfos = new TEIMEInfos( GetSelection().GetEnd(), pNode->GetText().Copy( GetSelection().GetEnd().GetIndex() ) );
        mpImpl->mpTextEngine->mpIMEInfos->bWasCursorOverwrite = !IsInsertMode();
    }
    else if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
    {
        DBG_ASSERT( mpImpl->mpTextEngine->mpIMEInfos, "COMMAND_ENDEXTTEXTINPUT => Kein Start ?" );
        if( mpImpl->mpTextEngine->mpIMEInfos )
        {
            TEParaPortion* pPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetPara() );
            pPortion->MarkSelectionInvalid( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex(), 0 );

            BOOL bInsertMode = !mpImpl->mpTextEngine->mpIMEInfos->bWasCursorOverwrite;

            delete mpImpl->mpTextEngine->mpIMEInfos;
            mpImpl->mpTextEngine->mpIMEInfos = NULL;

            mpImpl->mpTextEngine->FormatAndUpdate( this );

            SetInsertMode( bInsertMode );

            if ( mpImpl->mpTextEngine->IsModified() )
                mpImpl->mpTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        DBG_ASSERT( mpImpl->mpTextEngine->mpIMEInfos, "COMMAND_EXTTEXTINPUT => Kein Start ?" );
        if( mpImpl->mpTextEngine->mpIMEInfos )
        {
            const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

            if ( !pData->IsOnlyCursorChanged() )
            {
                TextSelection aSelect( mpImpl->mpTextEngine->mpIMEInfos->aPos );
                aSelect.GetEnd().GetIndex() = aSelect.GetEnd().GetIndex() + mpImpl->mpTextEngine->mpIMEInfos->nLen;
                aSelect = mpImpl->mpTextEngine->ImpDeleteText( aSelect );
                aSelect = mpImpl->mpTextEngine->ImpInsertText( aSelect, pData->GetText() );

                if ( mpImpl->mpTextEngine->mpIMEInfos->bWasCursorOverwrite )
                {
                    USHORT nOldIMETextLen = mpImpl->mpTextEngine->mpIMEInfos->nLen;
                    USHORT nNewIMETextLen = pData->GetText().Len();

                    if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                         ( nNewIMETextLen < mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.Len() ) )
                    {
                        // restore old characters
                        USHORT nRestore = nOldIMETextLen - nNewIMETextLen;
                        TextPaM aPaM( mpImpl->mpTextEngine->mpIMEInfos->aPos );
                        aPaM.GetIndex() = aPaM.GetIndex() + nNewIMETextLen;
                        mpImpl->mpTextEngine->ImpInsertText( aPaM, mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.Copy( nNewIMETextLen, nRestore ) );
                    }
                    else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                              ( nOldIMETextLen < mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.Len() ) )
                    {
                        // overwrite
                        USHORT nOverwrite = nNewIMETextLen - nOldIMETextLen;
                        if ( ( nOldIMETextLen + nOverwrite ) > mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.Len() )
                            nOverwrite = mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.Len() - nOldIMETextLen;
                        DBG_ASSERT( nOverwrite && (nOverwrite < 0xFF00), "IME Overwrite?!" );
                        TextPaM aPaM( mpImpl->mpTextEngine->mpIMEInfos->aPos );
                        aPaM.GetIndex() = aPaM.GetIndex() + nNewIMETextLen;
                        TextSelection aSel( aPaM );
                        aSel.GetEnd().GetIndex() =
                            aSel.GetEnd().GetIndex() + nOverwrite;
                        mpImpl->mpTextEngine->ImpDeleteText( aSel );
                    }
                }

                if ( pData->GetTextAttr() )
                {
                    mpImpl->mpTextEngine->mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().Len() );
                    mpImpl->mpTextEngine->mpIMEInfos->bCursor = pData->IsCursorVisible();
                }
                else
                {
                    mpImpl->mpTextEngine->mpIMEInfos->DestroyAttribs();
                }

                TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetPara() );
                pPPortion->MarkSelectionInvalid( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex(), 0 );
                mpImpl->mpTextEngine->FormatAndUpdate( this );
            }

            TextSelection aNewSel = TextPaM( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetPara(), mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex()+pData->GetCursorPos() );
            SetSelection( aNewSel );
            SetInsertMode( !pData->IsCursorOverwrite() );

            if ( pData->IsCursorVisible() )
                ShowCursor();
            else
                HideCursor();
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_CURSORPOS )
    {
        if ( mpImpl->mpTextEngine->mpIMEInfos && mpImpl->mpTextEngine->mpIMEInfos->nLen )
        {
            TextPaM aPaM( GetSelection().GetEnd() );
            Rectangle aR1 = mpImpl->mpTextEngine->PaMtoEditCursor( aPaM );

            USHORT nInputEnd = mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex() + mpImpl->mpTextEngine->mpIMEInfos->nLen;

            if ( !mpImpl->mpTextEngine->IsFormatted() )
                mpImpl->mpTextEngine->FormatDoc();

            TEParaPortion* pParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
            USHORT nLine = pParaPortion->GetLineNumber( aPaM.GetIndex(), sal_True );
            TextLine* pLine = pParaPortion->GetLines().GetObject( nLine );
            if ( pLine && ( nInputEnd > pLine->GetEnd() ) )
                nInputEnd = pLine->GetEnd();
            Rectangle aR2 = mpImpl->mpTextEngine->PaMtoEditCursor( TextPaM( aPaM.GetPara(), nInputEnd ) );

            long nWidth = aR2.Left()-aR1.Right();
            aR1.Move( -GetStartDocPos().X(), -GetStartDocPos().Y() );
            GetWindow()->SetCursorRect( &aR1, nWidth );
        }
        else
        {
            GetWindow()->SetCursorRect();
        }
    }
    else
    {
        mpImpl->mpSelEngine->Command( rCEvt );
    }
}

void TextView::ShowCursor( BOOL bGotoCursor, BOOL bForceVisCursor )
{
    // Die Einstellung hat mehr Gewicht:
    if ( !mpImpl->mbAutoScroll )
        bGotoCursor = FALSE;
    ImpShowCursor( bGotoCursor, bForceVisCursor, FALSE );
}

void TextView::HideCursor()
{
    mpImpl->mpCursor->Hide();
}

void TextView::Scroll( long ndX, long ndY )
{
    DBG_ASSERT( mpImpl->mpTextEngine->IsFormatted(), "Scroll: Nicht formatiert!" );

    if ( !ndX && !ndY )
        return;

    Point aNewStartPos( mpImpl->maStartDocPos );

    // Vertical:
    aNewStartPos.Y() -= ndY;
    if ( aNewStartPos.Y() < 0 )
        aNewStartPos.Y() = 0;

    // Horizontal:
    aNewStartPos.X() -= ndX;
    if ( aNewStartPos.X() < 0 )
        aNewStartPos.X() = 0;

    long nDiffX = mpImpl->maStartDocPos.X() - aNewStartPos.X();
    long nDiffY = mpImpl->maStartDocPos.Y() - aNewStartPos.Y();

    if ( nDiffX || nDiffY )
    {
        BOOL bVisCursor = mpImpl->mpCursor->IsVisible();
        mpImpl->mpCursor->Hide();
        mpImpl->mpWindow->Update();
        mpImpl->maStartDocPos = aNewStartPos;

        if ( mpImpl->mpTextEngine->IsRightToLeft() )
            nDiffX = -nDiffX;
        mpImpl->mpWindow->Scroll( nDiffX, nDiffY );
        mpImpl->mpWindow->Update();
        mpImpl->mpCursor->SetPos( mpImpl->mpCursor->GetPos() + Point( nDiffX, nDiffY ) );
        if ( bVisCursor && !mpImpl->mbReadOnly )
            mpImpl->mpCursor->Show();
    }

    mpImpl->mpTextEngine->Broadcast( TextHint( TEXT_HINT_VIEWSCROLLED ) );
}

void TextView::Undo()
{
    mpImpl->mpTextEngine->SetActiveView( this );
    mpImpl->mpTextEngine->GetUndoManager().Undo();
}

void TextView::Redo()
{
    mpImpl->mpTextEngine->SetActiveView( this );
    mpImpl->mpTextEngine->GetUndoManager().Redo();
}

void TextView::Cut()
{
    mpImpl->mpTextEngine->UndoActionStart();
    Copy();
    DeleteSelected();
    mpImpl->mpTextEngine->UndoActionEnd();
}

void TextView::Copy( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
{
    if ( rxClipboard.is() )
    {
        TETextDataObject* pDataObj = new TETextDataObject( GetSelected() );

        if ( mpImpl->mpTextEngine->HasAttrib( TEXTATTR_HYPERLINK ) )  // Dann auch als HTML
            mpImpl->mpTextEngine->Write( pDataObj->GetHTMLStream(), &mpImpl->maSelection, TRUE );

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            rxClipboard->setContents( pDataObj, NULL );

            uno::Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( rxClipboard, uno::UNO_QUERY );
            if( xFlushableClipboard.is() )
                xFlushableClipboard->flushClipboard();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );
    }
}

void TextView::Copy()
{
    uno::Reference<datatransfer::clipboard::XClipboard> aClipboard(GetWindow()->GetClipboard());
    Copy( aClipboard );
}

void TextView::Paste( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
{
    if ( rxClipboard.is() )
    {
        uno::Reference< datatransfer::XTransferable > xDataObj;

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            xDataObj = rxClipboard->getContents();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );

        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                try
                {
                    uno::Any aData = xDataObj->getTransferData( aFlavor );
                    ::rtl::OUString aText;
                    aData >>= aText;
                    bool bWasTruncated = false;
                    if( mpImpl->mpTextEngine->GetMaxTextLen() != 0 )
                        bWasTruncated = ImplTruncateNewText( aText );
                    InsertNewText( aText, FALSE );
                    mpImpl->mpTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );

                    if( bWasTruncated )
                        Edit::ShowTruncationWarning( mpImpl->mpWindow );
                }
                catch( const ::com::sun::star::datatransfer::UnsupportedFlavorException& )
                {
                }
            }
        }
    }
}

void TextView::Paste()
{
    uno::Reference<datatransfer::clipboard::XClipboard> aClipboard(GetWindow()->GetClipboard());
    Paste( aClipboard );
}

String TextView::GetSelected()
{
    return GetSelected( GetSystemLineEnd() );
}

String TextView::GetSelected( LineEnd aSeparator )
{
    return mpImpl->mpTextEngine->GetText( mpImpl->maSelection, aSeparator );
}

void TextView::SetInsertMode( BOOL bInsert )
{
    if ( mpImpl->mbInsertMode != bInsert )
    {
        mpImpl->mbInsertMode = bInsert;
        ShowCursor( mpImpl->mbAutoScroll, FALSE );
    }
}

void TextView::SetReadOnly( BOOL bReadOnly )
{
    if ( mpImpl->mbReadOnly != bReadOnly )
    {
        mpImpl->mbReadOnly = bReadOnly;
        if ( !mpImpl->mbReadOnly )
            ShowCursor( mpImpl->mbAutoScroll, FALSE );
        else
            HideCursor();

        GetWindow()->SetInputContext( InputContext( mpImpl->mpTextEngine->GetFont(), bReadOnly ? INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT : 0 ) );
    }
}

TextSelection TextView::ImpMoveCursor( const KeyEvent& rKeyEvent )
{
    // Eigentlich nur bei Up/Down noetig, aber was solls.
    mpImpl->mpTextEngine->CheckIdleFormatter();

    TextPaM aPaM( mpImpl->maSelection.GetEnd() );
    TextPaM aOldEnd( aPaM );

    TextDirectionality eTextDirection = TextDirectionality_LeftToRight_TopToBottom;
    if ( mpImpl->mpTextEngine->IsRightToLeft() )
        eTextDirection = TextDirectionality_RightToLeft_TopToBottom;

    KeyEvent aTranslatedKeyEvent = rKeyEvent.LogicalTextDirectionality( eTextDirection );

    BOOL bCtrl = aTranslatedKeyEvent.GetKeyCode().IsMod1() ? TRUE : FALSE;
    USHORT nCode = aTranslatedKeyEvent.GetKeyCode().GetCode();

    bool bSelect = aTranslatedKeyEvent.GetKeyCode().IsShift();
    switch ( nCode )
    {
        case KEY_UP:        aPaM = CursorUp( aPaM );
                            break;
        case KEY_DOWN:      aPaM = CursorDown( aPaM );
                            break;
        case KEY_HOME:      aPaM = bCtrl ? CursorStartOfDoc() : CursorStartOfLine( aPaM );
                            break;
        case KEY_END:       aPaM = bCtrl ? CursorEndOfDoc() : CursorEndOfLine( aPaM );
                            break;
        case KEY_PAGEUP:    aPaM = bCtrl ? CursorStartOfDoc() : PageUp( aPaM );
                            break;
        case KEY_PAGEDOWN:  aPaM = bCtrl ? CursorEndOfDoc() : PageDown( aPaM );
                            break;
        case KEY_LEFT:      aPaM = bCtrl ? CursorWordLeft( aPaM ) : CursorLeft( aPaM, aTranslatedKeyEvent.GetKeyCode().IsMod2() ? (USHORT)i18n::CharacterIteratorMode::SKIPCHARACTER : (USHORT)i18n::CharacterIteratorMode::SKIPCELL );
                            break;
        case KEY_RIGHT:     aPaM = bCtrl ? CursorWordRight( aPaM ) : CursorRight( aPaM, aTranslatedKeyEvent.GetKeyCode().IsMod2() ? (USHORT)i18n::CharacterIteratorMode::SKIPCHARACTER : (USHORT)i18n::CharacterIteratorMode::SKIPCELL );
                            break;
        case com::sun::star::awt::Key::SELECT_WORD_FORWARD:
                            bSelect = true; // fallthrough intentional
        case com::sun::star::awt::Key::MOVE_WORD_FORWARD:
                            aPaM = CursorWordRight( aPaM );
                            break;
        case com::sun::star::awt::Key::SELECT_WORD_BACKWARD:
                            bSelect = true; // fallthrough intentional
        case com::sun::star::awt::Key::MOVE_WORD_BACKWARD:
                            aPaM = CursorWordLeft( aPaM );
                            break;
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_LINE:
                            bSelect = true; // fallthrough intentional
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_LINE:
                            aPaM = CursorStartOfLine( aPaM );
                            break;
        case com::sun::star::awt::Key::SELECT_TO_END_OF_LINE:
                            bSelect = true; // fallthrough intentional
        case com::sun::star::awt::Key::MOVE_TO_END_OF_LINE:
                            aPaM = CursorEndOfLine( aPaM );
                            break;
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
                            bSelect = true; // falltthrough intentional
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
                            aPaM = CursorStartOfParagraph( aPaM );
                            break;
        case com::sun::star::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
                            bSelect = true; // falltthrough intentional
        case com::sun::star::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
                            aPaM = CursorEndOfParagraph( aPaM );
                            break;
        case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
                            bSelect = true; // falltthrough intentional
        case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
                            aPaM = CursorStartOfDoc();
                            break;
        case com::sun::star::awt::Key::SELECT_TO_END_OF_DOCUMENT:
                            bSelect = true; // falltthrough intentional
        case com::sun::star::awt::Key::MOVE_TO_END_OF_DOCUMENT:
                            aPaM = CursorEndOfDoc();
                            break;
    }

    // Bewirkt evtl. ein CreateAnchor oder Deselection all
    mpImpl->mpSelEngine->CursorPosChanging( bSelect, aTranslatedKeyEvent.GetKeyCode().IsMod1() );

    if ( aOldEnd != aPaM )
    {
        mpImpl->mpTextEngine->CursorMoved( aOldEnd.GetPara() );


        TextSelection aOldSelection( mpImpl->maSelection );
        TextSelection aNewSelection( mpImpl->maSelection );
        aNewSelection.GetEnd() = aPaM;
        if ( bSelect )
        {
            // Dann wird die Selektion erweitert...
            ImpSetSelection( aNewSelection );
            ShowSelection( TextSelection( aOldEnd, aPaM ) );
        }
        else
        {
            aNewSelection.GetStart() = aPaM;
            ImpSetSelection( aNewSelection );
        }
    }

    return mpImpl->maSelection;
}

void TextView::InsertText( const XubString& rStr, BOOL bSelect )
{
    InsertNewText( rStr, bSelect );
}

void TextView::InsertNewText( const rtl::OUString& rStr, BOOL bSelect )
{
//  HideSelection();
    mpImpl->mpTextEngine->UndoActionStart();

    /* #i87633#
    break inserted text into chunks that fit into the underlying String
    based API (which has a maximum length of 65534 elements

    note: this will of course still cause problems for lines longer than those
    65534 elements, but those cases will hopefully be few.
    In the long run someone should switch the TextEngine to OUString instead of String
    */
    sal_Int32 nLen = rStr.getLength();
    sal_Int32 nPos = 0;
    while( nLen )
    {
        sal_Int32 nChunkLen = nLen > 65534 ? 65534 : nLen;
        String aChunk( rStr.copy( nPos, nChunkLen ) );

        TextSelection aNewSel( mpImpl->maSelection );

        TextPaM aPaM = mpImpl->mpTextEngine->ImpInsertText( mpImpl->maSelection, aChunk );

        if ( bSelect )
        {
            aNewSel.Justify();
            aNewSel.GetEnd() = aPaM;
        }
        else
        {
            aNewSel = aPaM;
        }

        ImpSetSelection( aNewSel );
        nLen -= nChunkLen;
        nPos += nChunkLen;
    }
    mpImpl->mpTextEngine->UndoActionEnd();

    mpImpl->mpTextEngine->FormatAndUpdate( this );
}

/*
void TextView::InsertText( const XubString& rStr, BOOL bSelect )
{
//  HideSelection();

    TextSelection aNewSel( mpImpl->maSelection );

    mpImpl->mpTextEngine->UndoActionStart();
    TextPaM aPaM = mpImpl->mpTextEngine->ImpInsertText( mpImpl->maSelection, rStr );
    mpImpl->mpTextEngine->UndoActionEnd();

    if ( bSelect )
    {
        aNewSel.Justify();
        aNewSel.GetEnd() = aPaM;
    }
    else
    {
        aNewSel = aPaM;
    }

    ImpSetSelection( aNewSel );

    mpImpl->mpTextEngine->FormatAndUpdate( this );
}
*/

// OLD
TextPaM TextView::CursorLeft( const TextPaM& rPaM, BOOL bWordMode )
{
    return bWordMode ? CursorWordLeft( rPaM ) : CursorLeft( rPaM, (USHORT)i18n::CharacterIteratorMode::SKIPCELL );

    // Remove (USHORT) typecasts in this file when removing this method!
}

// OLD
TextPaM TextView::CursorRight( const TextPaM& rPaM, BOOL bWordMode )
{
    return bWordMode ? CursorWordRight( rPaM ) : CursorRight( rPaM, (USHORT)i18n::CharacterIteratorMode::SKIPCELL );

    // Remove (USHORT) typecasts in this file when removing this method!
}

TextPaM TextView::CursorLeft( const TextPaM& rPaM, USHORT nCharacterIteratorMode )
{
    TextPaM aPaM( rPaM );

    if ( aPaM.GetIndex() )
    {
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        sal_Int32 nCount = 1;
        aPaM.GetIndex() = (USHORT)xBI->previousCharacters( pNode->GetText(), aPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), nCharacterIteratorMode, nCount, nCount );
    }
    else if ( aPaM.GetPara() )
    {
        aPaM.GetPara()--;
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        aPaM.GetIndex() = pNode->GetText().Len();
    }
    return aPaM;
}

TextPaM TextView::CursorRight( const TextPaM& rPaM, USHORT nCharacterIteratorMode )
{
    TextPaM aPaM( rPaM );

    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
    if ( aPaM.GetIndex() < pNode->GetText().Len() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        sal_Int32 nCount = 1;
        aPaM.GetIndex() = (USHORT)xBI->nextCharacters( pNode->GetText(), aPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), nCharacterIteratorMode, nCount, nCount );
    }
    else if ( aPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().Count()-1) )
    {
        aPaM.GetPara()++;
        aPaM.GetIndex() = 0;
    }

    return aPaM;
}


TextPaM TextView::CursorWordLeft( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    if ( aPaM.GetIndex() )
    {
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        i18n::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), rPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
        if ( aBoundary.startPos >= rPaM.GetIndex() )
            aBoundary = xBI->previousWord( pNode->GetText(), rPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
        aPaM.GetIndex() = ( aBoundary.startPos != -1 ) ? (USHORT)aBoundary.startPos : 0;
    }
    else if ( aPaM.GetPara() )
    {
        aPaM.GetPara()--;
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        aPaM.GetIndex() = pNode->GetText().Len();
    }
    return aPaM;
}


TextPaM TextView::CursorWordRight( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
    if ( aPaM.GetIndex() < pNode->GetText().Len() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        i18n::Boundary aBoundary = xBI->nextWord(  pNode->GetText(), aPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
        aPaM.GetIndex() = (USHORT)aBoundary.startPos;
    }
    else if ( aPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().Count()-1) )
    {
        aPaM.GetPara()++;
        aPaM.GetIndex() = 0;
    }

    return aPaM;
}

TextPaM TextView::ImpDelete( BYTE nMode, BYTE nDelMode )
{
    if ( mpImpl->maSelection.HasRange() )  // dann nur Sel. loeschen
        return mpImpl->mpTextEngine->ImpDeleteText( mpImpl->maSelection );

    TextPaM aStartPaM = mpImpl->maSelection.GetStart();
    TextPaM aEndPaM = aStartPaM;
    if ( nMode == DEL_LEFT )
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aEndPaM = CursorLeft( aEndPaM, (USHORT)i18n::CharacterIteratorMode::SKIPCHARACTER );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject(  aEndPaM.GetPara() );
            uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
            i18n::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
            if ( aBoundary.startPos == mpImpl->maSelection.GetEnd().GetIndex() )
                aBoundary = xBI->previousWord( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
            // #i63506# startPos is -1 when the paragraph starts with a tab
            aEndPaM.GetIndex() = (aBoundary.startPos >= 0) ? (USHORT)aBoundary.startPos : 0;
        }
        else    // DELMODE_RESTOFCONTENT
        {
            if ( aEndPaM.GetIndex() != 0 )
                aEndPaM.GetIndex() = 0;
            else if ( aEndPaM.GetPara() )
            {
                // Absatz davor
                aEndPaM.GetPara()--;
                aEndPaM.GetIndex() = 0;
            }
        }
    }
    else
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aEndPaM = CursorRight( aEndPaM, (USHORT)i18n::CharacterIteratorMode::SKIPCELL );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject(  aEndPaM.GetPara() );
            uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
            i18n::Boundary aBoundary = xBI->nextWord( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
            aEndPaM.GetIndex() = (USHORT)aBoundary.startPos;
        }
        else    // DELMODE_RESTOFCONTENT
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aEndPaM.GetPara() );
            if ( aEndPaM.GetIndex() < pNode->GetText().Len() )
                aEndPaM.GetIndex() = pNode->GetText().Len();
            else if ( aEndPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().Count() - 1 ) )
            {
                // Absatz danach
                aEndPaM.GetPara()++;
                TextNode* pNextNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aEndPaM.GetPara() );
                aEndPaM.GetIndex() = pNextNode->GetText().Len();
            }
        }
    }

    return mpImpl->mpTextEngine->ImpDeleteText( TextSelection( aStartPaM, aEndPaM ) );
}



TextPaM TextView::CursorUp( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    long nX;
    if ( mpImpl->mnTravelXPos == TRAVEL_X_DONTKNOW )
    {
        nX = mpImpl->mpTextEngine->GetEditCursor( rPaM, FALSE ).Left();
        mpImpl->mnTravelXPos = (USHORT)nX+1;
    }
    else
        nX = mpImpl->mnTravelXPos;

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    USHORT nLine = pPPortion->GetLineNumber( rPaM.GetIndex(), FALSE );
    if ( nLine )    // gleicher Absatz
    {
        USHORT nCharPos = mpImpl->mpTextEngine->GetCharPos( rPaM.GetPara(), nLine-1, nX );
        aPaM.GetIndex() = nCharPos;
        // Wenn davor eine autom.Umgebrochene Zeile, und ich muss genau an das
        // Ende dieser Zeile, landet der Cursor in der aktuellen Zeile am Anfang
        // Siehe Problem: Letztes Zeichen einer autom.umgebr. Zeile = Cursor
        TextLine* pLine = pPPortion->GetLines().GetObject( nLine - 1 );
        if ( aPaM.GetIndex() && ( aPaM.GetIndex() == pLine->GetEnd() ) )
            aPaM.GetIndex()--;
    }
    else if ( rPaM.GetPara() )  // vorheriger Absatz
    {
        aPaM.GetPara()--;
        pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        USHORT nL = pPPortion->GetLines().Count() - 1;
        USHORT nCharPos = mpImpl->mpTextEngine->GetCharPos( aPaM.GetPara(), nL, nX+1 );
        aPaM.GetIndex() = nCharPos;
    }

    return aPaM;
}

TextPaM TextView::CursorDown( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    long nX;
    if ( mpImpl->mnTravelXPos == TRAVEL_X_DONTKNOW )
    {
        nX = mpImpl->mpTextEngine->GetEditCursor( rPaM, FALSE ).Left();
        mpImpl->mnTravelXPos = (USHORT)nX+1;
    }
    else
        nX = mpImpl->mnTravelXPos;

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    USHORT nLine = pPPortion->GetLineNumber( rPaM.GetIndex(), FALSE );
    if ( nLine < ( pPPortion->GetLines().Count() - 1 ) )
    {
        USHORT nCharPos = mpImpl->mpTextEngine->GetCharPos( rPaM.GetPara(), nLine+1, nX );
        aPaM.GetIndex() = nCharPos;

        // Sonderbehandlung siehe CursorUp...
        TextLine* pLine = pPPortion->GetLines().GetObject( nLine + 1 );
        if ( ( aPaM.GetIndex() == pLine->GetEnd() ) && ( aPaM.GetIndex() > pLine->GetStart() ) && aPaM.GetIndex() < pPPortion->GetNode()->GetText().Len() )
            aPaM.GetIndex()--;
    }
    else if ( rPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().Count() - 1 ) )   // naechster Absatz
    {
        aPaM.GetPara()++;
        pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        USHORT nCharPos = mpImpl->mpTextEngine->GetCharPos( aPaM.GetPara(), 0, nX+1 );
        aPaM.GetIndex() = nCharPos;
        TextLine* pLine = pPPortion->GetLines().GetObject( 0 );
        if ( ( aPaM.GetIndex() == pLine->GetEnd() ) && ( aPaM.GetIndex() > pLine->GetStart() ) && ( pPPortion->GetLines().Count() > 1 ) )
            aPaM.GetIndex()--;
    }

    return aPaM;
}

TextPaM TextView::CursorStartOfLine( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    USHORT nLine = pPPortion->GetLineNumber( aPaM.GetIndex(), FALSE );
    TextLine* pLine = pPPortion->GetLines().GetObject( nLine );
    aPaM.GetIndex() = pLine->GetStart();

    return aPaM;
}

TextPaM TextView::CursorEndOfLine( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    USHORT nLine = pPPortion->GetLineNumber( aPaM.GetIndex(), FALSE );
    TextLine* pLine = pPPortion->GetLines().GetObject( nLine );
    aPaM.GetIndex() = pLine->GetEnd();

    if ( pLine->GetEnd() > pLine->GetStart() )  // Leerzeile
    {
        xub_Unicode cLastChar = pPPortion->GetNode()->GetText().GetChar((USHORT)(aPaM.GetIndex()-1) );
        if ( ( cLastChar == ' ' ) && ( aPaM.GetIndex() != pPPortion->GetNode()->GetText().Len() ) )
        {
            // Bei einem Blank in einer autom. umgebrochenen Zeile macht es Sinn,
            // davor zu stehen, da der Anwender hinter das Wort will.
            // Wenn diese geaendert wird, Sonderbehandlung fuer Pos1 nach End!
            aPaM.GetIndex()--;
        }
    }
    return aPaM;
}

TextPaM TextView::CursorStartOfParagraph( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );
    aPaM.GetIndex() = 0;
    return aPaM;
}

TextPaM TextView::CursorEndOfParagraph( const TextPaM& rPaM )
{
    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( rPaM.GetPara() );
    TextPaM aPaM( rPaM );
    aPaM.GetIndex() = pNode->GetText().Len();
    return aPaM;
}

TextPaM TextView::CursorStartOfDoc()
{
    TextPaM aPaM( 0, 0 );
    return aPaM;
}

TextPaM TextView::CursorEndOfDoc()
{
    ULONG nNode = mpImpl->mpTextEngine->mpDoc->GetNodes().Count() - 1;
    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( nNode );
    TextPaM aPaM( nNode, pNode->GetText().Len() );
    return aPaM;
}

TextPaM TextView::PageUp( const TextPaM& rPaM )
{
    Rectangle aRec = mpImpl->mpTextEngine->PaMtoEditCursor( rPaM );
    Point aTopLeft = aRec.TopLeft();
    aTopLeft.Y() -= mpImpl->mpWindow->GetOutputSizePixel().Height() * 9/10;
    aTopLeft.X() += 1;
    if ( aTopLeft.Y() < 0 )
        aTopLeft.Y() = 0;

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aTopLeft );
    return aPaM;
}

TextPaM TextView::PageDown( const TextPaM& rPaM )
{
    Rectangle aRec = mpImpl->mpTextEngine->PaMtoEditCursor( rPaM );
    Point aBottomRight = aRec.BottomRight();
    aBottomRight.Y() += mpImpl->mpWindow->GetOutputSizePixel().Height() * 9/10;
    aBottomRight.X() += 1;
    long nHeight = mpImpl->mpTextEngine->GetTextHeight();
    if ( aBottomRight.Y() > nHeight )
        aBottomRight.Y() = nHeight-1;

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aBottomRight );
    return aPaM;
}

void TextView::ImpShowCursor( BOOL bGotoCursor, BOOL bForceVisCursor, BOOL bSpecial )
{
    if ( mpImpl->mpTextEngine->IsFormatting() )
        return;
    if ( mpImpl->mpTextEngine->GetUpdateMode() == FALSE )
        return;
    if ( mpImpl->mpTextEngine->IsInUndo() )
        return;

    mpImpl->mpTextEngine->CheckIdleFormatter();
    if ( !mpImpl->mpTextEngine->IsFormatted() )
        mpImpl->mpTextEngine->FormatAndUpdate( this );


    TextPaM aPaM( mpImpl->maSelection.GetEnd() );
    Rectangle aEditCursor = mpImpl->mpTextEngine->PaMtoEditCursor( aPaM, bSpecial );

    // Remember that we placed the cursor behind the last character of a line
    mpImpl->mbCursorAtEndOfLine = false;
    if( bSpecial )
    {
        TEParaPortion* pParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        mpImpl->mbCursorAtEndOfLine =
            pParaPortion->GetLineNumber( aPaM.GetIndex(), TRUE ) != pParaPortion->GetLineNumber( aPaM.GetIndex(), FALSE );
    }

    if ( !IsInsertMode() && !mpImpl->maSelection.HasRange() )
    {
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        if ( pNode->GetText().Len() && ( aPaM.GetIndex() < pNode->GetText().Len() ) )
        {
            // If we are behind a portion, and the next portion has other direction, we must change position...
            aEditCursor.Left() = aEditCursor.Right() = mpImpl->mpTextEngine->GetEditCursor( aPaM, FALSE, TRUE ).Left();

            TEParaPortion* pParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );

            USHORT nTextPortionStart = 0;
            USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, TRUE );
            TETextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
            if ( pTextPortion->GetKind() == PORTIONKIND_TAB )
            {
                if ( mpImpl->mpTextEngine->IsRightToLeft() )
                {

                }
                aEditCursor.Right() += pTextPortion->GetWidth();
            }
            else
            {
                TextPaM aNext = CursorRight( TextPaM( aPaM.GetPara(), aPaM.GetIndex() ), (USHORT)i18n::CharacterIteratorMode::SKIPCELL );
                aEditCursor.Right() = mpImpl->mpTextEngine->GetEditCursor( aNext, TRUE ).Left();
            }
        }
    }

    Size aOutSz = mpImpl->mpWindow->GetOutputSizePixel();
    if ( aEditCursor.GetHeight() > aOutSz.Height() )
        aEditCursor.Bottom() = aEditCursor.Top() + aOutSz.Height() - 1;

    aEditCursor.Left() -= 1;

    if ( bGotoCursor
        // #i81283# protext maStartDocPos against initialization problems
        && aOutSz.Width() && aOutSz.Height()
    )
    {
        long nVisStartY = mpImpl->maStartDocPos.Y();
        long nVisEndY = mpImpl->maStartDocPos.Y() + aOutSz.Height();
        long nVisStartX = mpImpl->maStartDocPos.X();
        long nVisEndX = mpImpl->maStartDocPos.X() + aOutSz.Width();
        long nMoreX = aOutSz.Width() / 4;

        Point aNewStartPos( mpImpl->maStartDocPos );

        if ( aEditCursor.Bottom() > nVisEndY )
        {
            aNewStartPos.Y() += ( aEditCursor.Bottom() - nVisEndY );
        }
        else if ( aEditCursor.Top() < nVisStartY )
        {
            aNewStartPos.Y() -= ( nVisStartY - aEditCursor.Top() );
        }

        if ( aEditCursor.Right() >= nVisEndX )
        {
            aNewStartPos.X() += ( aEditCursor.Right() - nVisEndX );

            // Darfs ein bischen mehr sein?
            aNewStartPos.X() += nMoreX;
        }
        else if ( aEditCursor.Left() <= nVisStartX )
        {
            aNewStartPos.X() -= ( nVisStartX - aEditCursor.Left() );

            // Darfs ein bischen mehr sein?
            aNewStartPos.X() -= nMoreX;
        }

        // X kann durch das 'bischen mehr' falsch sein:
//      ULONG nMaxTextWidth = mpImpl->mpTextEngine->GetMaxTextWidth();
//      if ( !nMaxTextWidth || ( nMaxTextWidth > 0x7FFFFFFF ) )
//          nMaxTextWidth = 0x7FFFFFFF;
//      long nMaxX = (long)nMaxTextWidth - aOutSz.Width();
        long nMaxX = mpImpl->mpTextEngine->CalcTextWidth() - aOutSz.Width();
        if ( nMaxX < 0 )
            nMaxX = 0;

        if ( aNewStartPos.X() < 0 )
            aNewStartPos.X() = 0;
        else if ( aNewStartPos.X() > nMaxX )
            aNewStartPos.X() = nMaxX;

        // Y sollte nicht weiter unten als noetig liegen:
        long nYMax = mpImpl->mpTextEngine->GetTextHeight() - aOutSz.Height();
        if ( nYMax < 0 )
            nYMax = 0;
        if ( aNewStartPos.Y() > nYMax )
            aNewStartPos.Y() = nYMax;

        if ( aNewStartPos != mpImpl->maStartDocPos )
            Scroll( -(aNewStartPos.X() - mpImpl->maStartDocPos.X()), -(aNewStartPos.Y() - mpImpl->maStartDocPos.Y()) );
    }

    if ( aEditCursor.Right() < aEditCursor.Left() )
    {
        long n = aEditCursor.Left();
        aEditCursor.Left() = aEditCursor.Right();
        aEditCursor.Right() = n;
    }

    Point aPoint( GetWindowPos( !mpImpl->mpTextEngine->IsRightToLeft() ? aEditCursor.TopLeft() : aEditCursor.TopRight() ) );
    mpImpl->mpCursor->SetPos( aPoint );
    mpImpl->mpCursor->SetSize( aEditCursor.GetSize() );
    if ( bForceVisCursor && mpImpl->mbCursorEnabled )
        mpImpl->mpCursor->Show();
}

BOOL TextView::SetCursorAtPoint( const Point& rPosPixel )
{
    mpImpl->mpTextEngine->CheckIdleFormatter();

    Point aDocPos = GetDocPos( rPosPixel );

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aDocPos );

    // aTmpNewSel: Diff zwischen alt und neu, nicht die neue Selektion
    TextSelection aTmpNewSel( mpImpl->maSelection.GetEnd(), aPaM );
    TextSelection aNewSel( mpImpl->maSelection );
    aNewSel.GetEnd() = aPaM;

    if ( !mpImpl->mpSelEngine->HasAnchor() )
    {
        if ( mpImpl->maSelection.GetStart() != aPaM )
            mpImpl->mpTextEngine->CursorMoved( mpImpl->maSelection.GetStart().GetPara() );
        aNewSel.GetStart() = aPaM;
        ImpSetSelection( aNewSel );
    }
    else
    {
        ImpSetSelection( aNewSel );
        ShowSelection( aTmpNewSel );
    }

    BOOL bForceCursor =  mpImpl->mpDDInfo ? FALSE : TRUE; // && !mbInSelection
    ImpShowCursor( mpImpl->mbAutoScroll, bForceCursor, FALSE );
    return TRUE;
}

BOOL TextView::IsSelectionAtPoint( const Point& rPosPixel )
{
//  if ( !Rectangle( Point(), mpImpl->mpWindow->GetOutputSizePixel() ).IsInside( rPosPixel ) && !mbInSelection )
//      return FALSE;

    Point aDocPos = GetDocPos( rPosPixel );
    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aDocPos, FALSE );
    // Bei Hyperlinks D&D auch ohne Selektion starten.
    // BeginDrag wird aber nur gerufen, wenn IsSelectionAtPoint()
    // Problem: IsSelectionAtPoint wird bei Command() nicht gerufen,
    // wenn vorher im MBDown schon FALSE returnt wurde.
    return ( IsInSelection( aPaM ) ||
            ( /* mpImpl->mpSelEngine->IsInCommand() && */ mpImpl->mpTextEngine->FindAttrib( aPaM, TEXTATTR_HYPERLINK ) ) );
}

BOOL TextView::IsInSelection( const TextPaM& rPaM )
{
    TextSelection aSel = mpImpl->maSelection;
    aSel.Justify();

    ULONG nStartNode = aSel.GetStart().GetPara();
    ULONG nEndNode = aSel.GetEnd().GetPara();
    ULONG nCurNode = rPaM.GetPara();

    if ( ( nCurNode > nStartNode ) && ( nCurNode < nEndNode ) )
        return TRUE;

    if ( nStartNode == nEndNode )
    {
        if ( nCurNode == nStartNode )
            if ( ( rPaM.GetIndex() >= aSel.GetStart().GetIndex() ) && ( rPaM.GetIndex() < aSel.GetEnd().GetIndex() ) )
                return TRUE;
    }
    else if ( ( nCurNode == nStartNode ) && ( rPaM.GetIndex() >= aSel.GetStart().GetIndex() ) )
        return TRUE;
    else if ( ( nCurNode == nEndNode ) && ( rPaM.GetIndex() < aSel.GetEnd().GetIndex() ) )
        return TRUE;

    return FALSE;
}

void TextView::ImpHideDDCursor()
{
    if ( mpImpl->mpDDInfo && mpImpl->mpDDInfo->mbVisCursor )
    {
        mpImpl->mpDDInfo->maCursor.Hide();
        mpImpl->mpDDInfo->mbVisCursor = FALSE;
    }
}

void TextView::ImpShowDDCursor()
{
    if ( !mpImpl->mpDDInfo->mbVisCursor )
    {
        Rectangle aCursor = mpImpl->mpTextEngine->PaMtoEditCursor( mpImpl->mpDDInfo->maDropPos, TRUE );
        aCursor.Right()++;
        aCursor.SetPos( GetWindowPos( aCursor.TopLeft() ) );

        mpImpl->mpDDInfo->maCursor.SetWindow( mpImpl->mpWindow );
        mpImpl->mpDDInfo->maCursor.SetPos( aCursor.TopLeft() );
        mpImpl->mpDDInfo->maCursor.SetSize( aCursor.GetSize() );
        mpImpl->mpDDInfo->maCursor.Show();
        mpImpl->mpDDInfo->mbVisCursor = TRUE;
    }
}

void TextView::SetPaintSelection( BOOL bPaint )
{
    if ( bPaint != mpImpl->mbPaintSelection )
    {
        mpImpl->mbPaintSelection = bPaint;
        ShowSelection( mpImpl->maSelection );
    }
}

void TextView::SetHighlightSelection( BOOL bSelectByHighlight )
{
    if ( bSelectByHighlight != mpImpl->mbHighlightSelection )
    {
        // Falls umschalten zwischendurch moeglich...
        mpImpl->mbHighlightSelection = bSelectByHighlight;
    }
}

BOOL TextView::Read( SvStream& rInput )
{
    BOOL bDone = mpImpl->mpTextEngine->Read( rInput, &mpImpl->maSelection );
    ShowCursor();
    return bDone;
}

BOOL TextView::Write( SvStream& rOutput )
{
    return mpImpl->mpTextEngine->Read( rOutput, &mpImpl->maSelection );
}

bool TextView::ImplTruncateNewText( rtl::OUString& rNewText ) const
{
    bool bTruncated = false;

    if( rNewText.getLength() > 65534 ) // limit to String API
    {
        rNewText = rNewText.copy( 0, 65534 );
        bTruncated = true;
    }

    ULONG nMaxLen = mpImpl->mpTextEngine->GetMaxTextLen();
    // 0 means unlimited, there is just the String API limit handled above
    if( nMaxLen != 0 )
    {
        ULONG nCurLen = mpImpl->mpTextEngine->GetTextLen();

        sal_uInt32 nNewLen = rNewText.getLength();
        if ( nCurLen + nNewLen > nMaxLen )
        {
            // see how much text will be replaced
            ULONG nSelLen = mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection );
            if ( nCurLen + nNewLen - nSelLen > nMaxLen )
            {
                sal_uInt32 nTruncatedLen = static_cast<sal_uInt32>(nMaxLen - (nCurLen - nSelLen));
                rNewText = rNewText.copy( 0, nTruncatedLen );
                bTruncated = true;
            }
        }
    }
    return bTruncated;
}

BOOL TextView::ImplCheckTextLen( const String& rNewText )
{
    BOOL bOK = TRUE;
    if ( mpImpl->mpTextEngine->GetMaxTextLen() )
    {
        ULONG n = mpImpl->mpTextEngine->GetTextLen();
        n += rNewText.Len();
        if ( n > mpImpl->mpTextEngine->GetMaxTextLen() )
        {
            // nur dann noch ermitteln, wie viel Text geloescht wird
            n -= mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection );
            if ( n > mpImpl->mpTextEngine->GetMaxTextLen() )
            {
                // Beep hat hier eigentlich nichts verloren, sondern lieber ein Hdl,
                // aber so funktioniert es wenigstens in ME, BasicIDE, SourceView
                Sound::Beep();
                bOK = FALSE;
            }
        }
    }
    return bOK;
}

void TextView::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpImpl->mbClickedInSelection )
    {
        vos::OGuard aVclGuard( Application::GetSolarMutex() );

        DBG_ASSERT( mpImpl->maSelection.HasRange(), "TextView::dragGestureRecognized: mpImpl->mbClickedInSelection, but no selection?" );

        delete mpImpl->mpDDInfo;
        mpImpl->mpDDInfo = new TextDDInfo;
        mpImpl->mpDDInfo->mbStarterOfDD = TRUE;

        TETextDataObject* pDataObj = new TETextDataObject( GetSelected() );

        if ( mpImpl->mpTextEngine->HasAttrib( TEXTATTR_HYPERLINK ) )  // Dann auch als HTML
            mpImpl->mpTextEngine->Write( pDataObj->GetHTMLStream(), &mpImpl->maSelection, TRUE );


        /*
        // D&D eines Hyperlinks.
        // Besser waere es im MBDown sich den MBDownPaM zu merken,
        // ist dann aber inkompatibel => spaeter mal umstellen.
        TextPaM aPaM( mpImpl->mpTextEngine->GetPaM( GetDocPos( GetWindow()->GetPointerPosPixel() ) ) );
        const TextCharAttrib* pAttr = mpImpl->mpTextEngine->FindCharAttrib( aPaM, TEXTATTR_HYPERLINK );
        if ( pAttr )
        {
            aSel = aPaM;
            aSel.GetStart().GetIndex() = pAttr->GetStart();
            aSel.GetEnd().GetIndex() = pAttr->GetEnd();

            const TextAttribHyperLink& rLink = (const TextAttribHyperLink&)pAttr->GetAttr();
            String aText( rLink.GetDescription() );
            if ( !aText.Len() )
                aText = mpImpl->mpTextEngine->GetText( aSel );
            INetBookmark aBookmark( rLink.GetURL(), aText );
            aBookmark.CopyDragServer();
        }
        */

        mpImpl->mpCursor->Hide();

        sal_Int8 nActions = datatransfer::dnd::DNDConstants::ACTION_COPY;
        if ( !IsReadOnly() )
            nActions |= datatransfer::dnd::DNDConstants::ACTION_MOVE;
        rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, pDataObj, mpImpl->mxDnDListener );
    }
}

void TextView::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    ImpHideDDCursor();
    delete mpImpl->mpDDInfo;
    mpImpl->mpDDInfo = NULL;
}

void TextView::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    BOOL bChanges = FALSE;
    if ( !mpImpl->mbReadOnly && mpImpl->mpDDInfo )
    {
        ImpHideDDCursor();

        // Daten fuer das loeschen nach einem DROP_MOVE:
        TextSelection aPrevSel( mpImpl->maSelection );
        aPrevSel.Justify();
        ULONG nPrevParaCount = mpImpl->mpTextEngine->GetParagraphCount();
        USHORT nPrevStartParaLen = mpImpl->mpTextEngine->GetTextLen( aPrevSel.GetStart().GetPara() );

        BOOL bStarterOfDD = FALSE;
        for ( USHORT nView = mpImpl->mpTextEngine->GetViewCount(); nView && !bStarterOfDD; )
            bStarterOfDD = mpImpl->mpTextEngine->GetView( --nView )->mpImpl->mpDDInfo ? mpImpl->mpTextEngine->GetView( nView )->mpImpl->mpDDInfo->mbStarterOfDD : FALSE;

        HideSelection();
        ImpSetSelection( mpImpl->mpDDInfo->maDropPos );

        mpImpl->mpTextEngine->UndoActionStart();

        String aText;
        uno::Reference< datatransfer::XTransferable > xDataObj = rDTDE.Transferable;
        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                ::rtl::OUString aOUString;
                aData >>= aOUString;
                aText = aOUString;
                aText.ConvertLineEnd( LINEEND_LF );
            }
        }

        if ( aText.Len() && ( aText.GetChar( aText.Len()-1 ) == LINE_SEP ) )
            aText.Erase( aText.Len()-1 );

        TextPaM aTempStart = mpImpl->maSelection.GetStart();
        if ( ImplCheckTextLen( aText ) )
            ImpSetSelection( mpImpl->mpTextEngine->ImpInsertText( mpImpl->mpDDInfo->maDropPos, aText ) );
        if(mpImpl->mbSupportProtectAttribute)
        {
            mpImpl->mpTextEngine->SetAttrib( TextAttribProtect(),
                aTempStart.GetPara(),
                aTempStart.GetIndex(),
                mpImpl->maSelection.GetEnd().GetIndex(), FALSE );
        }

        if ( aPrevSel.HasRange() &&
                !mpImpl->mbSupportProtectAttribute && // don't remove currently selected element
                (( rDTDE.DropAction & datatransfer::dnd::DNDConstants::ACTION_MOVE ) || !bStarterOfDD) )
        {
            // ggf. Selection anpasssen:
            if ( ( mpImpl->mpDDInfo->maDropPos.GetPara() < aPrevSel.GetStart().GetPara() ) ||
                 ( ( mpImpl->mpDDInfo->maDropPos.GetPara() == aPrevSel.GetStart().GetPara() )
                        && ( mpImpl->mpDDInfo->maDropPos.GetIndex() < aPrevSel.GetStart().GetIndex() ) ) )
            {
                ULONG nNewParasBeforeSelection =
                    mpImpl->mpTextEngine->GetParagraphCount() -    nPrevParaCount;

                aPrevSel.GetStart().GetPara() += nNewParasBeforeSelection;
                aPrevSel.GetEnd().GetPara() += nNewParasBeforeSelection;

                if ( mpImpl->mpDDInfo->maDropPos.GetPara() == aPrevSel.GetStart().GetPara() )
                {
                    USHORT nNewChars =
                        mpImpl->mpTextEngine->GetTextLen( aPrevSel.GetStart().GetPara() ) - nPrevStartParaLen;

                    aPrevSel.GetStart().GetIndex() =
                        aPrevSel.GetStart().GetIndex() + nNewChars;
                    if ( aPrevSel.GetStart().GetPara() == aPrevSel.GetEnd().GetPara() )
                        aPrevSel.GetEnd().GetIndex() =
                            aPrevSel.GetEnd().GetIndex() + nNewChars;
                }
            }
            else
            {
                // aktuelle Selektion anpassen
                TextPaM aPaM = mpImpl->maSelection.GetStart();
                aPaM.GetPara() -= ( aPrevSel.GetEnd().GetPara() - aPrevSel.GetStart().GetPara() );
                if ( aPrevSel.GetEnd().GetPara() == mpImpl->mpDDInfo->maDropPos.GetPara() )
                {
                    aPaM.GetIndex() =
                        aPaM.GetIndex() - aPrevSel.GetEnd().GetIndex();
                    if ( aPrevSel.GetStart().GetPara() == mpImpl->mpDDInfo->maDropPos.GetPara() )
                        aPaM.GetIndex() =
                            aPaM.GetIndex() + aPrevSel.GetStart().GetIndex();
                }
                ImpSetSelection( aPaM );

            }
            mpImpl->mpTextEngine->ImpDeleteText( aPrevSel );
        }

        mpImpl->mpTextEngine->UndoActionEnd();

        delete mpImpl->mpDDInfo;
        mpImpl->mpDDInfo = 0;

        mpImpl->mpTextEngine->FormatAndUpdate( this );

        mpImpl->mpTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
    }
    rDTDE.Context->dropComplete( bChanges );
}

void TextView::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
}

void TextView::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );
    ImpHideDDCursor();
}

void TextView::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    if ( !mpImpl->mpDDInfo )
        mpImpl->mpDDInfo = new TextDDInfo;

    TextPaM aPrevDropPos = mpImpl->mpDDInfo->maDropPos;
    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );
    Point aDocPos = GetDocPos( aMousePos );
    mpImpl->mpDDInfo->maDropPos = mpImpl->mpTextEngine->GetPaM( aDocPos );

/*
    Size aOutSize = mpImpl->mpWindow->GetOutputSizePixel();
    if ( ( aMousePos.X() < 0 ) || ( aMousePos.X() > aOutSize.Width() ) ||
         ( aMousePos.Y() < 0 ) || ( aMousePos.Y() > aOutSize.Height() ) )
    {
        // Scroll?
        // No, I will not receive events for this...
    }
*/

    sal_Bool bProtected = sal_False;
    if(mpImpl->mbSupportProtectAttribute)
    {
        const TextCharAttrib* pStartAttr = mpImpl->mpTextEngine->FindCharAttrib(
                    mpImpl->mpDDInfo->maDropPos,
                    TEXTATTR_PROTECTED );
        bProtected = pStartAttr != 0 &&
                pStartAttr->GetStart() != mpImpl->mpDDInfo->maDropPos.GetIndex() &&
                pStartAttr->GetEnd() != mpImpl->mpDDInfo->maDropPos.GetIndex();
    }
    // Don't drop in selection or in read only engine
    if ( IsReadOnly() || IsInSelection( mpImpl->mpDDInfo->maDropPos ) || bProtected)
    {
        ImpHideDDCursor();
        rDTDE.Context->rejectDrag();
    }
    else
    {
        // Alten Cursor wegzeichnen...
        if ( !mpImpl->mpDDInfo->mbVisCursor || ( aPrevDropPos != mpImpl->mpDDInfo->maDropPos ) )
        {
            ImpHideDDCursor();
            ImpShowDDCursor();
        }
        rDTDE.Context->acceptDrag( rDTDE.DropAction );
    }
}

Point TextView::ImpGetOutputStartPos( const Point& rStartDocPos ) const
{
    Point aStartPos( -rStartDocPos.X(), -rStartDocPos.Y() );
    if ( mpImpl->mpTextEngine->IsRightToLeft() )
    {
        Size aSz = mpImpl->mpWindow->GetOutputSizePixel();
        aStartPos.X() = rStartDocPos.X() + aSz.Width() - 1; // -1: Start is 0
    }
    return aStartPos;
}

Point TextView::GetDocPos( const Point& rWindowPos ) const
{
    // Fensterposition => Dokumentposition

    Point aPoint;

    aPoint.Y() = rWindowPos.Y() + mpImpl->maStartDocPos.Y();

    if ( !mpImpl->mpTextEngine->IsRightToLeft() )
    {
        aPoint.X() = rWindowPos.X() + mpImpl->maStartDocPos.X();
    }
    else
    {
        Size aSz = mpImpl->mpWindow->GetOutputSizePixel();
        aPoint.X() = ( aSz.Width() - 1 ) - rWindowPos.X() + mpImpl->maStartDocPos.X();
    }

    return aPoint;
}

Point TextView::GetWindowPos( const Point& rDocPos ) const
{
    // Dokumentposition => Fensterposition

    Point aPoint;

    aPoint.Y() = rDocPos.Y() - mpImpl->maStartDocPos.Y();

    if ( !mpImpl->mpTextEngine->IsRightToLeft() )
    {
        aPoint.X() = rDocPos.X() - mpImpl->maStartDocPos.X();
    }
    else
    {
        Size aSz = mpImpl->mpWindow->GetOutputSizePixel();
        aPoint.X() = ( aSz.Width() - 1 ) - ( rDocPos.X() - mpImpl->maStartDocPos.X() );
    }

    return aPoint;
}

sal_Int32 TextView::GetLineNumberOfCursorInSelection() const
{
 // PROGRESS
    sal_Int32 nLineNo = -1;
    if( mpImpl->mbCursorEnabled )
    {
        TextPaM aPaM = GetSelection().GetEnd();
        TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        nLineNo = pPPortion->GetLineNumber( aPaM.GetIndex(), FALSE );
        if( mpImpl->mbCursorAtEndOfLine )
            --nLineNo;
    }
    return nLineNo;
}


// -------------------------------------------------------------------------
// (+) class TextSelFunctionSet
// -------------------------------------------------------------------------
TextSelFunctionSet::TextSelFunctionSet( TextView* pView )
{
    mpView = pView;
}

void __EXPORT TextSelFunctionSet::BeginDrag()
{
}

void __EXPORT TextSelFunctionSet::CreateAnchor()
{
//  TextSelection aSel( mpView->GetSelection() );
//  aSel.GetStart() = aSel.GetEnd();
//  mpView->SetSelection( aSel );

    // Es darf kein ShowCursor folgen:
    mpView->HideSelection();
    mpView->ImpSetSelection( mpView->mpImpl->maSelection.GetEnd() );
}

BOOL __EXPORT TextSelFunctionSet::SetCursorAtPoint( const Point& rPointPixel, BOOL )
{
    return mpView->SetCursorAtPoint( rPointPixel );
}

BOOL __EXPORT TextSelFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    return mpView->IsSelectionAtPoint( rPointPixel );
}

void __EXPORT TextSelFunctionSet::DeselectAll()
{
    CreateAnchor();
}

void __EXPORT TextSelFunctionSet::DeselectAtPoint( const Point& )
{
    // Nur bei Mehrfachselektion
}

void __EXPORT TextSelFunctionSet::DestroyAnchor()
{
    // Nur bei Mehrfachselektion
}
TextEngine*         TextView::GetTextEngine() const
{ return mpImpl->mpTextEngine; }
Window*             TextView::GetWindow() const
{ return mpImpl->mpWindow; }
void                TextView::EnableCursor( BOOL bEnable )
{ mpImpl->mbCursorEnabled = bEnable; }
BOOL                TextView::IsCursorEnabled() const
{ return mpImpl->mbCursorEnabled; }
void                TextView::SetStartDocPos( const Point& rPos )
{ mpImpl->maStartDocPos = rPos; }
const Point&        TextView::GetStartDocPos() const
{ return mpImpl->maStartDocPos; }
void                TextView::SetAutoIndentMode( BOOL bAutoIndent )
{ mpImpl->mbAutoIndent = bAutoIndent; }
BOOL                TextView::IsAutoIndentMode() const
{ return mpImpl->mbAutoIndent; }
BOOL                TextView::IsReadOnly() const
{ return mpImpl->mbReadOnly; }
void                TextView::SetAutoScroll( BOOL bAutoScroll )
{ mpImpl->mbAutoScroll = bAutoScroll; }
BOOL                TextView::IsAutoScroll() const
{ return mpImpl->mbAutoScroll; }
BOOL                TextView::IsPaintSelection() const
{ return mpImpl->mbPaintSelection; }
BOOL                TextView::IsHighlightSelection() const
{ return mpImpl->mbHighlightSelection; }
BOOL                TextView::HasSelection() const
{ return mpImpl->maSelection.HasRange(); }
BOOL                TextView::IsInsertMode() const
{ return mpImpl->mbInsertMode; }
void                TextView::SupportProtectAttribute(sal_Bool bSupport)
{ mpImpl->mbSupportProtectAttribute = bSupport;}

