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

#include <vcl/textview.hxx>
#include <vcl/texteng.hxx>
#include <textdoc.hxx>
#include <vcl/textdata.hxx>
#include <textdat2.hxx>

#include <svl/undo.hxx>
#include <vcl/cursor.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <tools/stream.hxx>

#include <sot/formats.hxx>
#include <svl/urlbmk.hxx>

#include <com/sun/star/i18n/XBreakIterator.hpp>

#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#include <com/sun/star/i18n/WordType.hpp>
#include <cppuhelper/weak.hxx>
#include <vcl/unohelp.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>

#include <vcl/edit.hxx>


#include <sot/exchange.hxx>

#include <osl/mutex.hxx>


using namespace ::com::sun::star;

class TETextDataObject :    public ::com::sun::star::datatransfer::XTransferable,
                        public ::cppu::OWeakObject

{
private:
    OUString        maText;
    SvMemoryStream  maHTMLStream;

public:
                    TETextDataObject( const OUString& rText );
                    ~TETextDataObject();

    OUString&        GetText() { return maText; }
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

TETextDataObject::TETextDataObject( const OUString& rText ) : maText( rText )
{
}

TETextDataObject::~TETextDataObject()
{
}

// uno::XInterface
uno::Any TETextDataObject::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType, (static_cast< datatransfer::XTransferable* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// datatransfer::XTransferable
uno::Any TETextDataObject::getTransferData( const datatransfer::DataFlavor& rFlavor ) throw(datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException)
{
    uno::Any aAny;

    sal_uLong nT = SotExchange::GetFormat( rFlavor );
    if ( nT == SOT_FORMAT_STRING )
    {
        aAny <<= (OUString)GetText();
    }
    else if ( nT == SOT_FORMATSTR_ID_HTML )
    {
        GetHTMLStream().Seek( STREAM_SEEK_TO_END );
        sal_uLong nLen = GetHTMLStream().Tell();
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
    bool bHTML = GetHTMLStream().Tell() > 0;
    uno::Sequence< datatransfer::DataFlavor > aDataFlavors( bHTML ? 2 : 1 );
    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aDataFlavors.getArray()[0] );
    if ( bHTML )
        SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_HTML, aDataFlavors.getArray()[1] );
    return aDataFlavors;
}

sal_Bool TETextDataObject::isDataFlavorSupported( const datatransfer::DataFlavor& rFlavor ) throw(uno::RuntimeException)
{
    sal_uLong nT = SotExchange::GetFormat( rFlavor );
    return ( nT == SOT_FORMAT_STRING );
}

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

    sal_uInt16              mnTravelXPos;

    sal_Bool                mbAutoScroll            : 1;
    sal_Bool                mbInsertMode            : 1;
    sal_Bool                mbReadOnly              : 1;
    sal_Bool                mbPaintSelection        : 1;
    sal_Bool                mbAutoIndent            : 1;
    sal_Bool                mbHighlightSelection    : 1;
    sal_Bool                mbCursorEnabled         : 1;
    sal_Bool                mbClickedInSelection    : 1;
    sal_Bool                mbSupportProtectAttribute : 1;
    bool                mbCursorAtEndOfLine;
};

TextView::TextView( TextEngine* pEng, Window* pWindow ) :
    mpImpl(new ImpTextView)
{
    pWindow->EnableRTL( sal_False );

    mpImpl->mpWindow = pWindow;
    mpImpl->mpTextEngine = pEng;
    mpImpl->mpVirtDev = NULL;

    mpImpl->mbPaintSelection = sal_True;
    mpImpl->mbAutoScroll = sal_True;
    mpImpl->mbInsertMode = sal_True;
    mpImpl->mbReadOnly = sal_False;
    mpImpl->mbHighlightSelection = sal_False;
    mpImpl->mbAutoIndent = sal_False;
    mpImpl->mbCursorEnabled = sal_True;
    mpImpl->mbClickedInSelection = sal_False;
    mpImpl->mbSupportProtectAttribute = sal_False;
    mpImpl->mbCursorAtEndOfLine = false;
//  mbInSelection = sal_False;

    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;

    mpImpl->mpSelFuncSet = new TextSelFunctionSet( this );
    mpImpl->mpSelEngine = new SelectionEngine( mpImpl->mpWindow, mpImpl->mpSelFuncSet );
    mpImpl->mpSelEngine->SetSelectionMode( RANGE_SELECTION );
    mpImpl->mpSelEngine->EnableDrag( sal_True );

    mpImpl->mpCursor = new Cursor;
    mpImpl->mpCursor->Show();
    pWindow->SetCursor( mpImpl->mpCursor );
    pWindow->SetInputContext( InputContext( pEng->GetFont(), INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT ) );

    if ( pWindow->GetSettings().GetStyleSettings().GetSelectionOptions() & SELECTION_OPTION_INVERT )
        mpImpl->mbHighlightSelection = sal_True;

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

void TextView::SetSelection( const TextSelection& rTextSel, sal_Bool bGotoCursor )
{
    // if someone left an empty attribute and then the Outliner manipulated the selection
    if ( !mpImpl->maSelection.HasRange() )
        mpImpl->mpTextEngine->CursorMoved( mpImpl->maSelection.GetStart().GetPara() );

    // if the selection is manipulated after a KeyInput
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
        // set correct background color;
        // unfortunately we cannot detect if it has changed
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
    ImpPaint( rRect, sal_False );
}

void TextView::ImpPaint( const Rectangle& rRect, sal_Bool bUseVirtDev )
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

        sal_Bool bVDevValid = sal_True;
        Size aOutSz( pVDev->GetOutputSizePixel() );
        if ( (  aOutSz.Width() < rRect.GetWidth() ) ||
             (  aOutSz.Height() < rRect.GetHeight() ) )
        {
            bVDevValid = pVDev->SetOutputSizePixel( rRect.GetSize() );
        }
        else
        {
            // the VirtDev can get very large on Resize =>
            // shrink now and then
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
            ImpPaint( rRect, sal_False /* without VDev */ );
            return;
        }

        Rectangle aTmpRect( Point( 0, 0 ), rRect.GetSize() );

        Point aDocPos( mpImpl->maStartDocPos.X(), mpImpl->maStartDocPos.Y() + rRect.Top() );
        Point aStartPos = ImpGetOutputStartPos( aDocPos );
        ImpPaint( pVDev, aStartPos, &aTmpRect, NULL, pDrawSelection );
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
        sal_uLong nStartPara = aSel.GetStart().GetPara();
        sal_uLong nEndPara = aSel.GetEnd().GetPara();
        for ( sal_uLong nPara = 0; nPara <= nEndPara; nPara++ )
        {
            long nParaHeight = (long)mpImpl->mpTextEngine->CalcParaHeight( nPara );
            if ( ( nPara >= nStartPara ) && ( ( nY + nParaHeight ) > aVisArea.Top() ) )
            {
                TEParaPortion* pTEParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( nPara );
                sal_uInt16 nStartLine = 0;
                sal_uInt16 nEndLine = pTEParaPortion->GetLines().size() -1;
                if ( nPara == nStartPara )
                    nStartLine = pTEParaPortion->GetLineNumber( aSel.GetStart().GetIndex(), sal_False );
                if ( nPara == nEndPara )
                    nEndLine = pTEParaPortion->GetLineNumber( aSel.GetEnd().GetIndex(), sal_True );

                // iterate over all lines
                for ( sal_uInt16 nLine = nStartLine; nLine <= nEndLine; nLine++ )
                {
                    TextLine* pLine = pTEParaPortion->GetLines()[ nLine ];
                    sal_uInt16 nStartIndex = pLine->GetStart();
                    sal_uInt16 nEndIndex = pLine->GetEnd();
                    if ( ( nPara == nStartPara ) && ( nLine == nStartLine ) )
                        nStartIndex = aSel.GetStart().GetIndex();
                    if ( ( nPara == nEndPara ) && ( nLine == nEndLine ) )
                        nEndIndex = aSel.GetEnd().GetIndex();

                    // possible if at the beginning of a wrapped line
                    if ( nEndIndex < nStartIndex )
                        nEndIndex = nStartIndex;

                    Rectangle aTmpRect( mpImpl->mpTextEngine->GetEditCursor( TextPaM( nPara, nStartIndex ), sal_False ) );
                    aTmpRect.Top() += nY;
                    aTmpRect.Bottom() += nY;
                    Point aTopLeft( aTmpRect.TopLeft() );

                    aTmpRect = mpImpl->mpTextEngine->GetEditCursor( TextPaM( nPara, nEndIndex ), sal_True );
                    aTmpRect.Top() += nY;
                    aTmpRect.Bottom() += nY;
                    Point aBottomRight( aTmpRect.BottomRight() );
                    aBottomRight.X()--;

                    // only paint if in the visible region
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
    ImpShowHideSelection( sal_True );
}

void TextView::HideSelection()
{
    ImpShowHideSelection( sal_False );
}

void TextView::ShowSelection( const TextSelection& rRange )
{
    ImpShowHideSelection( sal_True, &rRange );
}

void TextView::ImpShowHideSelection( sal_Bool bShow, const TextSelection* pRange )
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
                sal_Bool bVisCursor = mpImpl->mpCursor->IsVisible();
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

sal_Bool TextView::KeyInput( const KeyEvent& rKeyEvent )
{
    sal_Bool bDone      = sal_True;
    bool bModified  = false;
    bool bMoved     = false;
    sal_Bool bEndKey    = sal_False;    // special CursorPosition
    bool bAllowIdle = true;

    // check mModified;
    // the local bModified is not set e.g. by Cut/Paste, as here
    // the update happens somewhere else
    sal_Bool bWasModified = mpImpl->mpTextEngine->IsModified();
    mpImpl->mpTextEngine->SetModified( sal_False );

    TextSelection aCurSel( mpImpl->maSelection );
    TextSelection aOldSel( aCurSel );

    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();
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

            default:    // might get processed below
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
                    bMoved = true;
                    if ( nCode == KEY_END )
                        bEndKey = sal_True;
                }
                else
                    bDone = sal_False;
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
                    sal_uInt8 nDel = ( nCode == KEY_DELETE ) ? DEL_RIGHT : DEL_LEFT;
                    sal_uInt8 nMode = rKeyEvent.GetKeyCode().IsMod1() ? DELMODE_RESTOFWORD : DELMODE_SIMPLE;
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
                    bModified = true;
                    bAllowIdle = false;
                }
                else
                    bDone = sal_False;
            }
            break;
            case KEY_TAB:
            {
                if ( !mpImpl->mbReadOnly && !rKeyEvent.GetKeyCode().IsShift() &&
                        !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() &&
                        ImplCheckTextLen( OUString('x') ) )
                {
                    aCurSel = mpImpl->mpTextEngine->ImpInsertText( aCurSel, '\t', !IsInsertMode() );
                    bModified = true;
                }
                else
                    bDone = sal_False;
            }
            break;
            case KEY_RETURN:
            {
                // do not swallow Shift-RETURN, as this would disable multi-line entries
                // in dialogs & property editors
                if ( !mpImpl->mbReadOnly && !rKeyEvent.GetKeyCode().IsMod1() &&
                        !rKeyEvent.GetKeyCode().IsMod2() && ImplCheckTextLen( OUString('x') ) )
                {
                    mpImpl->mpTextEngine->UndoActionStart();
                    aCurSel = mpImpl->mpTextEngine->ImpInsertParaBreak( aCurSel );
                    if ( mpImpl->mbAutoIndent )
                    {
                        TextNode* pPrev = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aCurSel.GetEnd().GetPara() - 1 );
                        sal_uInt16 n = 0;
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
                    bModified = true;
                }
                else
                    bDone = sal_False;
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
                    sal_Unicode nCharCode = rKeyEvent.GetCharCode();
                    if ( !mpImpl->mbReadOnly && ImplCheckTextLen( OUString(nCharCode) ) )    // otherwise swallow the character anyway
                    {
                        aCurSel = mpImpl->mpTextEngine->ImpInsertText( nCharCode, aCurSel, !IsInsertMode(), sal_True );
                        bModified = true;
                    }
                }
                else
                    bDone = sal_False;
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
        // Idle-Formatter only if AnyInput
        if ( bAllowIdle && Application::AnyInput( VCL_INPUT_KEYBOARD) )
            mpImpl->mpTextEngine->IdleFormatAndUpdate( this );
        else
            mpImpl->mpTextEngine->FormatAndUpdate( this);
    }
    else if ( bMoved )
    {
        // selection is painted now in ImpMoveCursor
        ImpShowCursor( mpImpl->mbAutoScroll, sal_True, bEndKey );
    }

    if ( mpImpl->mpTextEngine->IsModified() )
        mpImpl->mpTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
    else if ( bWasModified )
        mpImpl->mpTextEngine->SetModified( sal_True );

    return bDone;
}

void TextView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    mpImpl->mbClickedInSelection = sal_False;
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
    mpImpl->mpTextEngine->CheckIdleFormatter();    // for fast typing and MouseButtonDown
    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpImpl->mbClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );

    mpImpl->mpTextEngine->SetActiveView( this );

    mpImpl->mpSelEngine->SelMouseButtonDown( rMouseEvent );

    // mbu 20.01.2005 - SelMouseButtonDown() possibly triggers a 'selection changed'
    // notification. The appropriate handler could change the current selection,
    // which is the case in the MailMerge address block control. To enable select'n'drag
    // we need to reevaluate the selection after the notification has been fired.
    mpImpl->mbClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );

    // special cases
    if ( !rMouseEvent.IsShift() && ( rMouseEvent.GetClicks() >= 2 ) )
    {
        if ( rMouseEvent.IsMod2() )
        {
            HideSelection();
            ImpSetSelection( mpImpl->maSelection.GetEnd() );
            SetCursorAtPoint( rMouseEvent.GetPosPixel() );  // not set by SelectionEngine for MOD2
        }

        if ( rMouseEvent.GetClicks() == 2 )
        {
            // select word
            if ( mpImpl->maSelection.GetEnd().GetIndex() < mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection.GetEnd().GetPara() ) )
            {
                HideSelection();
                TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject(  mpImpl->maSelection.GetEnd().GetPara() );
                uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
                i18n::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                TextSelection aNewSel( mpImpl->maSelection );
                aNewSel.GetStart().GetIndex() = (sal_uInt16)aBoundary.startPos;
                aNewSel.GetEnd().GetIndex() = (sal_uInt16)aBoundary.endPos;
                if(mpImpl->mbSupportProtectAttribute)
                {
                    //expand selection to include all protected content - if there is any
                    const TextCharAttrib* pStartAttr = mpImpl->mpTextEngine->FindCharAttrib(
                                TextPaM(aNewSel.GetStart().GetPara(),
                                (sal_uInt16)aBoundary.startPos),
                                TEXTATTR_PROTECTED );
                    const TextCharAttrib* pEndAttr = mpImpl->mpTextEngine->FindCharAttrib(
                                TextPaM(aNewSel.GetEnd().GetPara(),
                                (sal_uInt16)aBoundary.endPos),
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
                ShowCursor( sal_True, sal_True );
            }
        }
        else if ( rMouseEvent.GetClicks() == 3 )
        {
            // select paragraph
            if ( mpImpl->maSelection.GetStart().GetIndex() || ( mpImpl->maSelection.GetEnd().GetIndex() < mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection.GetEnd().GetPara() ) ) )
            {
                HideSelection();
                TextSelection aNewSel( mpImpl->maSelection );
                aNewSel.GetStart().GetIndex() = 0;
                aNewSel.GetEnd().GetIndex() = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( mpImpl->maSelection.GetEnd().GetPara() )->GetText().Len();
                ImpSetSelection( aNewSel );
                ShowSelection();
                ShowCursor( sal_True, sal_True );
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
    mpImpl->mpTextEngine->CheckIdleFormatter();    // for fast typing and MouseButtonDown
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
        DBG_ASSERT( mpImpl->mpTextEngine->mpIMEInfos, "COMMAND_ENDEXTTEXTINPUT => No Start ?" );
        if( mpImpl->mpTextEngine->mpIMEInfos )
        {
            TEParaPortion* pPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetPara() );
            pPortion->MarkSelectionInvalid( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex(), 0 );

            sal_Bool bInsertMode = !mpImpl->mpTextEngine->mpIMEInfos->bWasCursorOverwrite;

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
        DBG_ASSERT( mpImpl->mpTextEngine->mpIMEInfos, "COMMAND_EXTTEXTINPUT => No Start ?" );
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
                    sal_Int32 nOldIMETextLen = mpImpl->mpTextEngine->mpIMEInfos->nLen;
                    sal_Int32 nNewIMETextLen = pData->GetText().getLength();

                    if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                         ( nNewIMETextLen < mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.getLength() ) )
                    {
                        // restore old characters
                        sal_Int32 nRestore = nOldIMETextLen - nNewIMETextLen;
                        TextPaM aPaM( mpImpl->mpTextEngine->mpIMEInfos->aPos );
                        aPaM.GetIndex() = aPaM.GetIndex() + nNewIMETextLen;
                        mpImpl->mpTextEngine->ImpInsertText( aPaM, mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.copy( nNewIMETextLen, nRestore ) );
                    }
                    else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                              ( nOldIMETextLen < mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.getLength() ) )
                    {
                        // overwrite
                        sal_uInt16 nOverwrite = nNewIMETextLen - nOldIMETextLen;
                        if ( ( nOldIMETextLen + nOverwrite ) > mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.getLength() )
                            nOverwrite = mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.getLength() - nOldIMETextLen;
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
                    mpImpl->mpTextEngine->mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().getLength() );
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

            sal_uInt16 nInputEnd = mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex() + mpImpl->mpTextEngine->mpIMEInfos->nLen;

            if ( !mpImpl->mpTextEngine->IsFormatted() )
                mpImpl->mpTextEngine->FormatDoc();

            TEParaPortion* pParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
            sal_uInt16 nLine = pParaPortion->GetLineNumber( aPaM.GetIndex(), sal_True );
            TextLine* pLine = pParaPortion->GetLines()[ nLine ];
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

void TextView::ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor )
{
    // this setting has more weight
    if ( !mpImpl->mbAutoScroll )
        bGotoCursor = sal_False;
    ImpShowCursor( bGotoCursor, bForceVisCursor, sal_False );
}

void TextView::HideCursor()
{
    mpImpl->mpCursor->Hide();
}

void TextView::Scroll( long ndX, long ndY )
{
    DBG_ASSERT( mpImpl->mpTextEngine->IsFormatted(), "Scroll: Not formatted!" );

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
        sal_Bool bVisCursor = mpImpl->mpCursor->IsVisible();
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

        if ( mpImpl->mpTextEngine->HasAttrib( TEXTATTR_HYPERLINK ) )  // then also as HTML
            mpImpl->mpTextEngine->Write( pDataObj->GetHTMLStream(), &mpImpl->maSelection, sal_True );

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
                    OUString aText;
                    aData >>= aText;
                    bool bWasTruncated = false;
                    if( mpImpl->mpTextEngine->GetMaxTextLen() != 0 )
                        bWasTruncated = ImplTruncateNewText( aText );
                    InsertText( aText, sal_False );
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

OUString TextView::GetSelected()
{
    return GetSelected( GetSystemLineEnd() );
}

OUString TextView::GetSelected( LineEnd aSeparator )
{
    return mpImpl->mpTextEngine->GetText( mpImpl->maSelection, aSeparator );
}

void TextView::SetInsertMode( sal_Bool bInsert )
{
    if ( mpImpl->mbInsertMode != bInsert )
    {
        mpImpl->mbInsertMode = bInsert;
        ShowCursor( mpImpl->mbAutoScroll, sal_False );
    }
}

void TextView::SetReadOnly( sal_Bool bReadOnly )
{
    if ( mpImpl->mbReadOnly != bReadOnly )
    {
        mpImpl->mbReadOnly = bReadOnly;
        if ( !mpImpl->mbReadOnly )
            ShowCursor( mpImpl->mbAutoScroll, sal_False );
        else
            HideCursor();

        GetWindow()->SetInputContext( InputContext( mpImpl->mpTextEngine->GetFont(), bReadOnly ? INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT : 0 ) );
    }
}

TextSelection TextView::ImpMoveCursor( const KeyEvent& rKeyEvent )
{
    // normally only needed for Up/Down; but who cares
    mpImpl->mpTextEngine->CheckIdleFormatter();

    TextPaM aPaM( mpImpl->maSelection.GetEnd() );
    TextPaM aOldEnd( aPaM );

    TextDirectionality eTextDirection = TextDirectionality_LeftToRight_TopToBottom;
    if ( mpImpl->mpTextEngine->IsRightToLeft() )
        eTextDirection = TextDirectionality_RightToLeft_TopToBottom;

    KeyEvent aTranslatedKeyEvent = rKeyEvent.LogicalTextDirectionality( eTextDirection );

    bool bCtrl = aTranslatedKeyEvent.GetKeyCode().IsMod1();
    sal_uInt16 nCode = aTranslatedKeyEvent.GetKeyCode().GetCode();

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
        case KEY_LEFT:      aPaM = bCtrl ? CursorWordLeft( aPaM ) : CursorLeft( aPaM, aTranslatedKeyEvent.GetKeyCode().IsMod2() ? (sal_uInt16)i18n::CharacterIteratorMode::SKIPCHARACTER : (sal_uInt16)i18n::CharacterIteratorMode::SKIPCELL );
                            break;
        case KEY_RIGHT:     aPaM = bCtrl ? CursorWordRight( aPaM ) : CursorRight( aPaM, aTranslatedKeyEvent.GetKeyCode().IsMod2() ? (sal_uInt16)i18n::CharacterIteratorMode::SKIPCHARACTER : (sal_uInt16)i18n::CharacterIteratorMode::SKIPCELL );
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

    // might cause a CreateAnchor or Deselection all
    mpImpl->mpSelEngine->CursorPosChanging( bSelect, aTranslatedKeyEvent.GetKeyCode().IsMod1() );

    if ( aOldEnd != aPaM )
    {
        mpImpl->mpTextEngine->CursorMoved( aOldEnd.GetPara() );

        TextSelection aNewSelection( mpImpl->maSelection );
        aNewSelection.GetEnd() = aPaM;
        if ( bSelect )
        {
            // extend the selection
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

void TextView::InsertText( const OUString& rStr, sal_Bool bSelect )
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
    do
    {
        sal_Int32 nChunkLen = nLen > 65534 ? 65534 : nLen;
        OUString aChunk( rStr.copy( nPos, nChunkLen ) );

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
    while( nLen );

    mpImpl->mpTextEngine->UndoActionEnd();

    mpImpl->mpTextEngine->FormatAndUpdate( this );
}

TextPaM TextView::CursorLeft( const TextPaM& rPaM, sal_uInt16 nCharacterIteratorMode )
{
    TextPaM aPaM( rPaM );

    if ( aPaM.GetIndex() )
    {
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        sal_Int32 nCount = 1;
        aPaM.GetIndex() = (sal_uInt16)xBI->previousCharacters( pNode->GetText(), aPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), nCharacterIteratorMode, nCount, nCount );
    }
    else if ( aPaM.GetPara() )
    {
        aPaM.GetPara()--;
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        aPaM.GetIndex() = pNode->GetText().Len();
    }
    return aPaM;
}

TextPaM TextView::CursorRight( const TextPaM& rPaM, sal_uInt16 nCharacterIteratorMode )
{
    TextPaM aPaM( rPaM );

    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
    if ( aPaM.GetIndex() < pNode->GetText().Len() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        sal_Int32 nCount = 1;
        aPaM.GetIndex() = (sal_uInt16)xBI->nextCharacters( pNode->GetText(), aPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), nCharacterIteratorMode, nCount, nCount );
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
        aPaM.GetIndex() = ( aBoundary.startPos != -1 ) ? (sal_uInt16)aBoundary.startPos : 0;
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
        aPaM.GetIndex() = (sal_uInt16)aBoundary.startPos;
    }
    else if ( aPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().Count()-1) )
    {
        aPaM.GetPara()++;
        aPaM.GetIndex() = 0;
    }

    return aPaM;
}

TextPaM TextView::ImpDelete( sal_uInt8 nMode, sal_uInt8 nDelMode )
{
    if ( mpImpl->maSelection.HasRange() )  // only delete selection
        return mpImpl->mpTextEngine->ImpDeleteText( mpImpl->maSelection );

    TextPaM aStartPaM = mpImpl->maSelection.GetStart();
    TextPaM aEndPaM = aStartPaM;
    if ( nMode == DEL_LEFT )
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aEndPaM = CursorLeft( aEndPaM, (sal_uInt16)i18n::CharacterIteratorMode::SKIPCHARACTER );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject(  aEndPaM.GetPara() );
            uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
            i18n::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
            if ( aBoundary.startPos == mpImpl->maSelection.GetEnd().GetIndex() )
                aBoundary = xBI->previousWord( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
            // #i63506# startPos is -1 when the paragraph starts with a tab
            aEndPaM.GetIndex() = (aBoundary.startPos >= 0) ? (sal_uInt16)aBoundary.startPos : 0;
        }
        else    // DELMODE_RESTOFCONTENT
        {
            if ( aEndPaM.GetIndex() != 0 )
                aEndPaM.GetIndex() = 0;
            else if ( aEndPaM.GetPara() )
            {
                // previous paragraph
                aEndPaM.GetPara()--;
                aEndPaM.GetIndex() = 0;
            }
        }
    }
    else
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aEndPaM = CursorRight( aEndPaM, (sal_uInt16)i18n::CharacterIteratorMode::SKIPCELL );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject(  aEndPaM.GetPara() );
            uno::Reference < i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
            i18n::Boundary aBoundary = xBI->nextWord( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
            aEndPaM.GetIndex() = (sal_uInt16)aBoundary.startPos;
        }
        else    // DELMODE_RESTOFCONTENT
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aEndPaM.GetPara() );
            if ( aEndPaM.GetIndex() < pNode->GetText().Len() )
                aEndPaM.GetIndex() = pNode->GetText().Len();
            else if ( aEndPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().Count() - 1 ) )
            {
                // next paragraph
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
        nX = mpImpl->mpTextEngine->GetEditCursor( rPaM, sal_False ).Left();
        mpImpl->mnTravelXPos = (sal_uInt16)nX+1;
    }
    else
        nX = mpImpl->mnTravelXPos;

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    sal_uInt16 nLine = pPPortion->GetLineNumber( rPaM.GetIndex(), sal_False );
    if ( nLine )    // same paragraph
    {
        sal_uInt16 nCharPos = mpImpl->mpTextEngine->GetCharPos( rPaM.GetPara(), nLine-1, nX );
        aPaM.GetIndex() = nCharPos;
        // If we need to go to the end of a line that was wrapped automatically,
        // the cursor ends up at the beginning of the 2nd line
        // Problem: Last character of an automatically wrapped line = Cursor
        TextLine* pLine = pPPortion->GetLines()[ nLine - 1 ];
        if ( aPaM.GetIndex() && ( aPaM.GetIndex() == pLine->GetEnd() ) )
            aPaM.GetIndex()--;
    }
    else if ( rPaM.GetPara() )  // previous paragraph
    {
        aPaM.GetPara()--;
        pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        sal_uInt16 nL = pPPortion->GetLines().size() - 1;
        sal_uInt16 nCharPos = mpImpl->mpTextEngine->GetCharPos( aPaM.GetPara(), nL, nX+1 );
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
        nX = mpImpl->mpTextEngine->GetEditCursor( rPaM, sal_False ).Left();
        mpImpl->mnTravelXPos = (sal_uInt16)nX+1;
    }
    else
        nX = mpImpl->mnTravelXPos;

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    sal_uInt16 nLine = pPPortion->GetLineNumber( rPaM.GetIndex(), sal_False );
    if ( nLine < ( pPPortion->GetLines().size() - 1 ) )
    {
        sal_uInt16 nCharPos = mpImpl->mpTextEngine->GetCharPos( rPaM.GetPara(), nLine+1, nX );
        aPaM.GetIndex() = nCharPos;

        // special case CursorUp
        TextLine* pLine = pPPortion->GetLines()[ nLine + 1 ];
        if ( ( aPaM.GetIndex() == pLine->GetEnd() ) && ( aPaM.GetIndex() > pLine->GetStart() ) && aPaM.GetIndex() < pPPortion->GetNode()->GetText().Len() )
            aPaM.GetIndex()--;
    }
    else if ( rPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().Count() - 1 ) )   // next paragraph
    {
        aPaM.GetPara()++;
        pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        sal_uInt16 nCharPos = mpImpl->mpTextEngine->GetCharPos( aPaM.GetPara(), 0, nX+1 );
        aPaM.GetIndex() = nCharPos;
        TextLine* pLine = pPPortion->GetLines().front();
        if ( ( aPaM.GetIndex() == pLine->GetEnd() ) && ( aPaM.GetIndex() > pLine->GetStart() ) && ( pPPortion->GetLines().size() > 1 ) )
            aPaM.GetIndex()--;
    }

    return aPaM;
}

TextPaM TextView::CursorStartOfLine( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    sal_uInt16 nLine = pPPortion->GetLineNumber( aPaM.GetIndex(), sal_False );
    TextLine* pLine = pPPortion->GetLines()[ nLine ];
    aPaM.GetIndex() = pLine->GetStart();

    return aPaM;
}

TextPaM TextView::CursorEndOfLine( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    sal_uInt16 nLine = pPPortion->GetLineNumber( aPaM.GetIndex(), sal_False );
    TextLine* pLine = pPPortion->GetLines()[ nLine ];
    aPaM.GetIndex() = pLine->GetEnd();

    if ( pLine->GetEnd() > pLine->GetStart() )  // empty line
    {
        sal_Unicode cLastChar = pPPortion->GetNode()->GetText().GetChar((sal_uInt16)(aPaM.GetIndex()-1) );
        if ( ( cLastChar == ' ' ) && ( aPaM.GetIndex() != pPPortion->GetNode()->GetText().Len() ) )
        {
            // for a blank in an automatically-wrapped line it is better to stand before it,
            // as the user will intend to stand behind the prior word.
            // If there is a change, special case for Pos1 after End!
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
    sal_uLong nNode = mpImpl->mpTextEngine->mpDoc->GetNodes().Count() - 1;
    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( nNode );
    TextPaM aPaM( nNode, pNode->GetText().Len() );
    return aPaM;
}

TextPaM TextView::PageUp( const TextPaM& rPaM )
{
    Rectangle aRect = mpImpl->mpTextEngine->PaMtoEditCursor( rPaM );
    Point aTopLeft = aRect.TopLeft();
    aTopLeft.Y() -= mpImpl->mpWindow->GetOutputSizePixel().Height() * 9/10;
    aTopLeft.X() += 1;
    if ( aTopLeft.Y() < 0 )
        aTopLeft.Y() = 0;

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aTopLeft );
    return aPaM;
}

TextPaM TextView::PageDown( const TextPaM& rPaM )
{
    Rectangle aRect = mpImpl->mpTextEngine->PaMtoEditCursor( rPaM );
    Point aBottomRight = aRect.BottomRight();
    aBottomRight.Y() += mpImpl->mpWindow->GetOutputSizePixel().Height() * 9/10;
    aBottomRight.X() += 1;
    long nHeight = mpImpl->mpTextEngine->GetTextHeight();
    if ( aBottomRight.Y() > nHeight )
        aBottomRight.Y() = nHeight-1;

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aBottomRight );
    return aPaM;
}

void TextView::ImpShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, sal_Bool bSpecial )
{
    if ( mpImpl->mpTextEngine->IsFormatting() )
        return;
    if ( mpImpl->mpTextEngine->GetUpdateMode() == sal_False )
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
            pParaPortion->GetLineNumber( aPaM.GetIndex(), sal_True ) != pParaPortion->GetLineNumber( aPaM.GetIndex(), sal_False );
    }

    if ( !IsInsertMode() && !mpImpl->maSelection.HasRange() )
    {
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        if ( pNode->GetText().Len() && ( aPaM.GetIndex() < pNode->GetText().Len() ) )
        {
            // If we are behind a portion, and the next portion has other direction, we must change position...
            aEditCursor.Left() = aEditCursor.Right() = mpImpl->mpTextEngine->GetEditCursor( aPaM, sal_False, sal_True ).Left();

            TEParaPortion* pParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );

            sal_uInt16 nTextPortionStart = 0;
            sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, sal_True );
            TETextPortion* pTextPortion = pParaPortion->GetTextPortions()[ nTextPortion ];
            if ( pTextPortion->GetKind() == PORTIONKIND_TAB )
            {
                if ( mpImpl->mpTextEngine->IsRightToLeft() )
                {

                }
                aEditCursor.Right() += pTextPortion->GetWidth();
            }
            else
            {
                TextPaM aNext = CursorRight( TextPaM( aPaM.GetPara(), aPaM.GetIndex() ), (sal_uInt16)i18n::CharacterIteratorMode::SKIPCELL );
                aEditCursor.Right() = mpImpl->mpTextEngine->GetEditCursor( aNext, sal_True ).Left();
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

            // do you want some more?
            aNewStartPos.X() += nMoreX;
        }
        else if ( aEditCursor.Left() <= nVisStartX )
        {
            aNewStartPos.X() -= ( nVisStartX - aEditCursor.Left() );

            // do you want some more?
            aNewStartPos.X() -= nMoreX;
        }

        // X can be wrong for the 'some more' above:
//      sal_uLong nMaxTextWidth = mpImpl->mpTextEngine->GetMaxTextWidth();
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

        // Y should not be further down than needed
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

sal_Bool TextView::SetCursorAtPoint( const Point& rPosPixel )
{
    mpImpl->mpTextEngine->CheckIdleFormatter();

    Point aDocPos = GetDocPos( rPosPixel );

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aDocPos );

    // aTmpNewSel: Diff between old and new; not the new selection
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

    sal_Bool bForceCursor =  mpImpl->mpDDInfo ? sal_False : sal_True; // && !mbInSelection
    ImpShowCursor( mpImpl->mbAutoScroll, bForceCursor, sal_False );
    return sal_True;
}

sal_Bool TextView::IsSelectionAtPoint( const Point& rPosPixel )
{
//  if ( !Rectangle( Point(), mpImpl->mpWindow->GetOutputSizePixel() ).IsInside( rPosPixel ) && !mbInSelection )
//      return sal_False;

    Point aDocPos = GetDocPos( rPosPixel );
    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aDocPos, sal_False );
    // For Hyperlinks D&D also start w/o a selection.
    // BeginDrag is only called, however, if IsSelectionAtPoint()
    // Problem: IsSelectionAtPoint is not called by Command()
    // if before MBDown returned sal_False.
    return ( IsInSelection( aPaM ) ||
            ( /* mpImpl->mpSelEngine->IsInCommand() && */ mpImpl->mpTextEngine->FindAttrib( aPaM, TEXTATTR_HYPERLINK ) ) );
}

sal_Bool TextView::IsInSelection( const TextPaM& rPaM )
{
    TextSelection aSel = mpImpl->maSelection;
    aSel.Justify();

    sal_uLong nStartNode = aSel.GetStart().GetPara();
    sal_uLong nEndNode = aSel.GetEnd().GetPara();
    sal_uLong nCurNode = rPaM.GetPara();

    if ( ( nCurNode > nStartNode ) && ( nCurNode < nEndNode ) )
        return sal_True;

    if ( nStartNode == nEndNode )
    {
        if ( nCurNode == nStartNode )
            if ( ( rPaM.GetIndex() >= aSel.GetStart().GetIndex() ) && ( rPaM.GetIndex() < aSel.GetEnd().GetIndex() ) )
                return sal_True;
    }
    else if ( ( nCurNode == nStartNode ) && ( rPaM.GetIndex() >= aSel.GetStart().GetIndex() ) )
        return sal_True;
    else if ( ( nCurNode == nEndNode ) && ( rPaM.GetIndex() < aSel.GetEnd().GetIndex() ) )
        return sal_True;

    return sal_False;
}

void TextView::ImpHideDDCursor()
{
    if ( mpImpl->mpDDInfo && mpImpl->mpDDInfo->mbVisCursor )
    {
        mpImpl->mpDDInfo->maCursor.Hide();
        mpImpl->mpDDInfo->mbVisCursor = sal_False;
    }
}

void TextView::ImpShowDDCursor()
{
    if ( !mpImpl->mpDDInfo->mbVisCursor )
    {
        Rectangle aCursor = mpImpl->mpTextEngine->PaMtoEditCursor( mpImpl->mpDDInfo->maDropPos, sal_True );
        aCursor.Right()++;
        aCursor.SetPos( GetWindowPos( aCursor.TopLeft() ) );

        mpImpl->mpDDInfo->maCursor.SetWindow( mpImpl->mpWindow );
        mpImpl->mpDDInfo->maCursor.SetPos( aCursor.TopLeft() );
        mpImpl->mpDDInfo->maCursor.SetSize( aCursor.GetSize() );
        mpImpl->mpDDInfo->maCursor.Show();
        mpImpl->mpDDInfo->mbVisCursor = sal_True;
    }
}

void TextView::SetPaintSelection( sal_Bool bPaint )
{
    if ( bPaint != mpImpl->mbPaintSelection )
    {
        mpImpl->mbPaintSelection = bPaint;
        ShowSelection( mpImpl->maSelection );
    }
}

sal_Bool TextView::Read( SvStream& rInput )
{
    sal_Bool bDone = mpImpl->mpTextEngine->Read( rInput, &mpImpl->maSelection );
    ShowCursor();
    return bDone;
}

bool TextView::ImplTruncateNewText( OUString& rNewText ) const
{
    bool bTruncated = false;

    if( rNewText.getLength() > 65534 ) // limit to String API
    {
        rNewText = rNewText.copy( 0, 65534 );
        bTruncated = true;
    }

    sal_uLong nMaxLen = mpImpl->mpTextEngine->GetMaxTextLen();
    // 0 means unlimited, there is just the String API limit handled above
    if( nMaxLen != 0 )
    {
        sal_uLong nCurLen = mpImpl->mpTextEngine->GetTextLen();

        sal_uInt32 nNewLen = rNewText.getLength();
        if ( nCurLen + nNewLen > nMaxLen )
        {
            // see how much text will be replaced
            sal_uLong nSelLen = mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection );
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

sal_Bool TextView::ImplCheckTextLen( const OUString& rNewText )
{
    sal_Bool bOK = sal_True;
    if ( mpImpl->mpTextEngine->GetMaxTextLen() )
    {
        sal_uLong n = mpImpl->mpTextEngine->GetTextLen();
        n += rNewText.getLength();
        if ( n > mpImpl->mpTextEngine->GetMaxTextLen() )
        {
            // calculate how much text is being deleted
            n -= mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection );
            if ( n > mpImpl->mpTextEngine->GetMaxTextLen() )
                bOK = sal_False;
        }
    }
    return bOK;
}

void TextView::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpImpl->mbClickedInSelection )
    {
        SolarMutexGuard aVclGuard;

        DBG_ASSERT( mpImpl->maSelection.HasRange(), "TextView::dragGestureRecognized: mpImpl->mbClickedInSelection, but no selection?" );

        delete mpImpl->mpDDInfo;
        mpImpl->mpDDInfo = new TextDDInfo;
        mpImpl->mpDDInfo->mbStarterOfDD = sal_True;

        TETextDataObject* pDataObj = new TETextDataObject( GetSelected() );

        if ( mpImpl->mpTextEngine->HasAttrib( TEXTATTR_HYPERLINK ) )  // then also as HTML
            mpImpl->mpTextEngine->Write( pDataObj->GetHTMLStream(), &mpImpl->maSelection, sal_True );


        /*
        // D&D of a Hyperlink
        // TODO: Better would be to store MBDownPaM in MBDown,
        // but this would be incompatible => change later
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
    SolarMutexGuard aVclGuard;

    sal_Bool bChanges = sal_False;
    if ( !mpImpl->mbReadOnly && mpImpl->mpDDInfo )
    {
        ImpHideDDCursor();

        // Data for deleting after DROP_MOVE:
        TextSelection aPrevSel( mpImpl->maSelection );
        aPrevSel.Justify();
        sal_uLong nPrevParaCount = mpImpl->mpTextEngine->GetParagraphCount();
        sal_uInt16 nPrevStartParaLen = mpImpl->mpTextEngine->GetTextLen( aPrevSel.GetStart().GetPara() );

        sal_Bool bStarterOfDD = sal_False;
        for ( sal_uInt16 nView = mpImpl->mpTextEngine->GetViewCount(); nView && !bStarterOfDD; )
            bStarterOfDD = mpImpl->mpTextEngine->GetView( --nView )->mpImpl->mpDDInfo ? mpImpl->mpTextEngine->GetView( nView )->mpImpl->mpDDInfo->mbStarterOfDD : sal_False;

        HideSelection();
        ImpSetSelection( mpImpl->mpDDInfo->maDropPos );

        mpImpl->mpTextEngine->UndoActionStart();

        OUString aText;
        uno::Reference< datatransfer::XTransferable > xDataObj = rDTDE.Transferable;
        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                OUString aOUString;
                aData >>= aOUString;
                aText = convertLineEnd(aOUString, LINEEND_LF);
            }
        }

        if ( !aText.isEmpty() && ( aText[ aText.getLength()-1 ] == LINE_SEP ) )
            aText = aText.replaceAt( aText.getLength()-1, 1, "" );

        TextPaM aTempStart = mpImpl->maSelection.GetStart();
        if ( ImplCheckTextLen( aText ) )
            ImpSetSelection( mpImpl->mpTextEngine->ImpInsertText( mpImpl->mpDDInfo->maDropPos, aText ) );
        if(mpImpl->mbSupportProtectAttribute)
        {
            mpImpl->mpTextEngine->SetAttrib( TextAttribProtect(),
                aTempStart.GetPara(),
                aTempStart.GetIndex(),
                mpImpl->maSelection.GetEnd().GetIndex(), sal_False );
        }

        if ( aPrevSel.HasRange() &&
                !mpImpl->mbSupportProtectAttribute && // don't remove currently selected element
                (( rDTDE.DropAction & datatransfer::dnd::DNDConstants::ACTION_MOVE ) || !bStarterOfDD) )
        {
            // adjust selection if necessary
            if ( ( mpImpl->mpDDInfo->maDropPos.GetPara() < aPrevSel.GetStart().GetPara() ) ||
                 ( ( mpImpl->mpDDInfo->maDropPos.GetPara() == aPrevSel.GetStart().GetPara() )
                        && ( mpImpl->mpDDInfo->maDropPos.GetIndex() < aPrevSel.GetStart().GetIndex() ) ) )
            {
                sal_uLong nNewParasBeforeSelection =
                    mpImpl->mpTextEngine->GetParagraphCount() -    nPrevParaCount;

                aPrevSel.GetStart().GetPara() += nNewParasBeforeSelection;
                aPrevSel.GetEnd().GetPara() += nNewParasBeforeSelection;

                if ( mpImpl->mpDDInfo->maDropPos.GetPara() == aPrevSel.GetStart().GetPara() )
                {
                    sal_uInt16 nNewChars =
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
                // adjust current selection
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
    SolarMutexGuard aVclGuard;
    ImpHideDDCursor();
}

void TextView::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    if ( !mpImpl->mpDDInfo )
        mpImpl->mpDDInfo = new TextDDInfo;

    TextPaM aPrevDropPos = mpImpl->mpDDInfo->maDropPos;
    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );
    Point aDocPos = GetDocPos( aMousePos );
    mpImpl->mpDDInfo->maDropPos = mpImpl->mpTextEngine->GetPaM( aDocPos );

    bool bProtected = false;
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
        // delete old Cursor
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
    // Window Position => Document Position

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
    // Document Position => Window Position

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
        nLineNo = pPPortion->GetLineNumber( aPaM.GetIndex(), sal_False );
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

void TextSelFunctionSet::BeginDrag()
{
}

void TextSelFunctionSet::CreateAnchor()
{
//  TextSelection aSel( mpView->GetSelection() );
//  aSel.GetStart() = aSel.GetEnd();
//  mpView->SetSelection( aSel );

    // may not be followed by ShowCursor
    mpView->HideSelection();
    mpView->ImpSetSelection( mpView->mpImpl->maSelection.GetEnd() );
}

sal_Bool TextSelFunctionSet::SetCursorAtPoint( const Point& rPointPixel, sal_Bool )
{
    return mpView->SetCursorAtPoint( rPointPixel );
}

sal_Bool TextSelFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    return mpView->IsSelectionAtPoint( rPointPixel );
}

void TextSelFunctionSet::DeselectAll()
{
    CreateAnchor();
}

void TextSelFunctionSet::DeselectAtPoint( const Point& )
{
    // only for multiple selection
}

void TextSelFunctionSet::DestroyAnchor()
{
    // only for multiple selection
}
TextEngine*         TextView::GetTextEngine() const
{ return mpImpl->mpTextEngine; }
Window*             TextView::GetWindow() const
{ return mpImpl->mpWindow; }
void                TextView::EnableCursor( sal_Bool bEnable )
{ mpImpl->mbCursorEnabled = bEnable; }
sal_Bool                TextView::IsCursorEnabled() const
{ return mpImpl->mbCursorEnabled; }
void                TextView::SetStartDocPos( const Point& rPos )
{ mpImpl->maStartDocPos = rPos; }
const Point&        TextView::GetStartDocPos() const
{ return mpImpl->maStartDocPos; }
void                TextView::SetAutoIndentMode( sal_Bool bAutoIndent )
{ mpImpl->mbAutoIndent = bAutoIndent; }
sal_Bool                TextView::IsReadOnly() const
{ return mpImpl->mbReadOnly; }
void                TextView::SetAutoScroll( sal_Bool bAutoScroll )
{ mpImpl->mbAutoScroll = bAutoScroll; }
sal_Bool                TextView::IsAutoScroll() const
{ return mpImpl->mbAutoScroll; }
sal_Bool                TextView::HasSelection() const
{ return mpImpl->maSelection.HasRange(); }
sal_Bool                TextView::IsInsertMode() const
{ return mpImpl->mbInsertMode; }
void                TextView::SupportProtectAttribute(sal_Bool bSupport)
{ mpImpl->mbSupportProtectAttribute = bSupport;}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
