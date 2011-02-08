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
#ifndef _TEXTENG_HXX
#define _TEXTENG_HXX

#include "svtools/svtdllapi.h"

class TextDoc;
class TextView;
class TextPaM;
class TextSelection;
class TextViews;
class TEParaPortions;
class TextAttrib;
class TextCharAttrib;
class TextUndo;
class TextUndoManager;
class EditSelFunctionSet;
class EditSelEngine;
class IdleFormatter;
class TextNode;
class OutputDevice;
class SfxUndoAction;
class KeyEvent;
class Timer;

namespace svl
{
    class IUndoManager;
}

class TextLine;
class TETextPortion;
#include <svl/brdcst.hxx>
#include <tools/link.hxx>
#include <vcl/font.hxx>
#include <tools/string.hxx>
#include <tools/gen.hxx>

#ifndef _COM_SUN_STAR_LOCALE_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#include <com/sun/star/uno/Reference.hxx>

struct TEIMEInfos;
class SvtCTLOptions;

namespace com {
namespace sun {
namespace star {
namespace i18n {
    class XBreakIterator;
    class XExtendedInputSequenceChecker;
}}}}

class LocaleDataWrapper;

enum TxtAlign { TXTALIGN_LEFT, TXTALIGN_CENTER, TXTALIGN_RIGHT };

class SVT_DLLPUBLIC TextEngine : public SfxBroadcaster
{
    friend class        TextView;
    friend class        TextSelFunctionSet;
    friend class        ExtTextEngine;
    friend class        ExtTextView;

    friend class        TextUndo;
    friend class        TextUndoManager;
    friend class        TextUndoDelPara;
    friend class        TextUndoConnectParas;
    friend class        TextUndoSplitPara;
    friend class        TextUndoInsertChars;
    friend class        TextUndoRemoveChars;
    friend class        TextUndoSetAttribs;

private:
    TextDoc*            mpDoc;
    TEParaPortions*     mpTEParaPortions;
    OutputDevice*       mpRefDev;

    TextViews*          mpViews;
    TextView*           mpActiveView;

    TextUndoManager*    mpUndoManager;

    IdleFormatter*      mpIdleFormatter;

    TEIMEInfos*         mpIMEInfos;

    ::com::sun::star::lang::Locale maLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > mxBreakIterator;

    Rectangle           maInvalidRec;
    Range               maInvalidRange;

    LocaleDataWrapper*  mpLocaleDataWrapper;

    Font                maFont;
    Color               maTextColor;
    USHORT              mnCharHeight;
    USHORT              mnFixCharWidth100;

    ULONG               mnMaxTextLen;
    ULONG               mnMaxTextWidth;
    ULONG               mnCurTextWidth;
    ULONG               mnCurTextHeight;
    ULONG               mnDefTab;

    TxtAlign            meAlign;

    BOOL                mbIsFormatting      : 1;    // Semaphore wegen der Hook's
    BOOL                mbFormatted         : 1;
    BOOL                mbUpdate            : 1;
    BOOL                mbModified          : 1;
    BOOL                mbUndoEnabled       : 1;
    BOOL                mbIsInUndo          : 1;
    BOOL                mbDowning           : 1;
    BOOL                mbRightToLeft       : 1;
    BOOL                mbHasMultiLineParas : 1;

                        TextEngine( const TextEngine& ) : SfxBroadcaster()  {}
    TextEngine&         operator=( const TextEngine& )      { return *this; }

protected:

    void                CursorMoved( ULONG nNode );
    void                TextModified();

    void                ImpInitDoc();
    void                ImpRemoveText();
    TextPaM             ImpDeleteText( const TextSelection& rSel );
    TextPaM             ImpInsertText( const TextSelection& rSel, sal_Unicode c, BOOL bOverwrite = FALSE );
    TextPaM             ImpInsertText( const TextSelection& rSel, const String& rText );
    TextPaM             ImpInsertParaBreak( const TextSelection& rTextSelection, BOOL bKeepEndingAttribs = TRUE );
    TextPaM             ImpInsertParaBreak( const TextPaM& rPaM, BOOL bKeepEndingAttribs = TRUE );
    void                ImpRemoveChars( const TextPaM& rPaM, USHORT nChars, SfxUndoAction* pCurUndo = 0 );
    TextPaM             ImpConnectParagraphs( ULONG nLeft, ULONG nRight );
    void                ImpRemoveParagraph( ULONG nPara );
    void                ImpInitWritingDirections( ULONG nPara );
    LocaleDataWrapper*  ImpGetLocaleDataWrapper();

    // to remain compatible in the minor release we copy the above ImpInsertText
    // function and add the extra parameter we need but make sure this function
    // gets not exported. First and seconf parameter swapped to have a different signatur.
    SAL_DLLPRIVATE TextPaM  ImpInsertText( sal_Unicode c, const TextSelection& rSel, BOOL bOverwrite = FALSE, BOOL bIsUserInput = sal_False );
    // some other new functions needed that must not be exported to remain compatible
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XExtendedInputSequenceChecker > GetInputSequenceChecker() const;
    SAL_DLLPRIVATE BOOL IsInputSequenceCheckingRequired( sal_Unicode c, const TextSelection& rCurSel ) const;

    // Broadcasten bzw. Selektionen anpassen:
    void                ImpParagraphInserted( ULONG nPara );
    void                ImpParagraphRemoved( ULONG nPara );
    void                ImpCharsRemoved( ULONG nPara, USHORT nPos, USHORT nChars );
    void                ImpCharsInserted( ULONG nPara, USHORT nPos, USHORT nChars );
    void                ImpFormattingParagraph( ULONG nPara );
    void                ImpTextHeightChanged();
    void                ImpTextFormatted();

    DECL_LINK(          IdleFormatHdl, Timer * );
    void                CheckIdleFormatter();
    void                IdleFormatAndUpdate( TextView* pCurView = 0, USHORT nMaxTimerRestarts = 5 );

    BOOL                CreateLines( ULONG nPara );
    void                CreateAndInsertEmptyLine( ULONG nPara );
    void                ImpBreakLine( ULONG nPara, TextLine* pLine, TETextPortion* pPortion, USHORT nPortionStart, long nRemainingWidth );
    USHORT              SplitTextPortion( ULONG nPara, USHORT nPos );
    void                CreateTextPortions( ULONG nPara, USHORT nStartPos );
    void                RecalcTextPortion( ULONG nPara, USHORT nStartPos, short nNewChars );
    void                SeekCursor( ULONG nNode, USHORT nPos, Font& rFont, OutputDevice* pOutDev );

    void                FormatDoc();
    void                FormatFullDoc();
    void                FormatAndUpdate( TextView* pCurView = 0 );
    BOOL                IsFormatting() const { return mbIsFormatting; }
    void                UpdateViews( TextView* pCurView = 0 );
    void                SetUpdateMode( BOOL bUp, TextView* pCurView, BOOL bForceUpdate );

    void                ImpPaint( OutputDevice* pOut, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange = 0, TextSelection const* pSelection = 0 );

    void                UpdateSelections();

    BOOL                IsFormatted() const { return mbFormatted; }

    USHORT              GetCharPos( ULONG nPara, USHORT nLine, long nDocPosX, BOOL bSmart = FALSE );
    Rectangle           GetEditCursor( const TextPaM& rPaM, BOOL bSpecial, BOOL bPreferPortionStart = FALSE );
    USHORT              ImpFindIndex( ULONG nPortion, const Point& rPosInPara, BOOL bSmart );
    long                ImpGetPortionXOffset( ULONG nPara, TextLine* pLine, USHORT nTextPortion );
    long                ImpGetXPos( ULONG nPara, TextLine* pLine, USHORT nIndex, BOOL bPreferPortionStart = FALSE );
    long                ImpGetOutputOffset( ULONG nPara, TextLine* pLine, USHORT nIndex, USHORT nIndex2 );
    BYTE                ImpGetRightToLeft( ULONG nPara, USHORT nPos, USHORT* pStart = NULL, USHORT* pEnd = NULL );
    void                ImpInitLayoutMode( OutputDevice* pOutDev, BOOL bDrawingR2LPortion = FALSE );
    TxtAlign            ImpGetAlign() const;

    ULONG               CalcTextHeight();
    ULONG               CalcParaHeight( ULONG nParagraph ) const;
    ULONG               CalcTextWidth( ULONG nPara );
    ULONG               CalcTextWidth( ULONG nPara, USHORT nPortionStart, USHORT nPortionLen, const Font* pFont = 0 );
    Range               GetInvalidYOffsets( ULONG nPortion );

    // Fuer Undo/Redo
    void                InsertContent( TextNode* pNode, ULONG nPara );
    TextPaM             SplitContent( ULONG nNode, USHORT nSepPos );
    TextPaM             ConnectContents( ULONG nLeftNode );

    // Ans API uebergebene PaM's und Selektionen auf einen gueltigen Bereich einstellen
    void                ValidateSelection( TextSelection& rSel ) const;
    void                ValidatePaM( TextPaM& rPaM ) const;

public:
                        TextEngine();
                        ~TextEngine();

    void                SetText( const String& rStr );
    String              GetText( LineEnd aSeparator = LINEEND_LF ) const;
    String              GetText( const TextSelection& rSel, LineEnd aSeparator = LINEEND_LF ) const;
    String              GetTextLines( LineEnd aSeparator = LINEEND_LF ) const;
    void                ReplaceText(const TextSelection& rSel, const String& rText);

    ULONG               GetTextLen( LineEnd aSeparator = LINEEND_LF ) const;
    ULONG               GetTextLen( const TextSelection& rSel, LineEnd aSeparator = LINEEND_LF ) const;

    void                SetFont( const Font& rFont );
    const Font&         GetFont() const { return maFont; }

    void                SetDefTab( USHORT nDefTab );
    USHORT              GetDefTab() const;

    void                SetLeftMargin( USHORT n );
    USHORT              GetLeftMargin() const;

    void                SetUpdateMode( BOOL bUpdate );
    BOOL                GetUpdateMode() const { return mbUpdate; }

    USHORT              GetViewCount() const;
    TextView*           GetView( USHORT nView ) const;
    void                InsertView( TextView* pTextView );
    void                RemoveView( TextView* pTextView );
    TextView*           GetActiveView() const;
    void                SetActiveView( TextView* pView );

    void                SetMaxTextLen( ULONG nLen );
    ULONG               GetMaxTextLen() const { return mnMaxTextLen; }

    void                SetMaxTextWidth( ULONG nWidth );
    ULONG               GetMaxTextWidth() const { return mnMaxTextWidth; }

    ULONG               GetTextHeight() const;
    ULONG               CalcTextWidth();
    USHORT              GetCharHeight() const { return mnCharHeight; }

    ULONG               GetParagraphCount() const;
    String              GetText( ULONG nParagraph ) const;
    USHORT              GetTextLen( ULONG nParagraph ) const;
    ULONG               GetTextHeight( ULONG nParagraph ) const;

    USHORT              GetLineCount( ULONG nParagraph ) const;
    USHORT              GetLineLen( ULONG nParagraph, USHORT nLine ) const;

    void                SetRightToLeft( BOOL bR2L );
    BOOL                IsRightToLeft() const { return mbRightToLeft; }

    BOOL                HasUndoManager() const { return mpUndoManager ? TRUE : FALSE; }
    ::svl::IUndoManager&
                        GetUndoManager();
    void                UndoActionStart( USHORT nId = 0 );
    void                UndoActionEnd();
    void                InsertUndo( TextUndo* pUndo, BOOL bTryMerge = FALSE );
    BOOL                IsInUndo()                  { return mbIsInUndo; }
    void                SetIsInUndo( BOOL bInUndo ) { mbIsInUndo = bInUndo; }
    void                ResetUndo();

    void                EnableUndo( BOOL bEnable );
    BOOL                IsUndoEnabled()             { return mbUndoEnabled; }

    void                SetModified( BOOL bModified )   { mbModified = bModified; }
    BOOL                IsModified() const              { return mbModified; }

    BOOL                Read( SvStream& rInput, const TextSelection* pSel = NULL );

    BOOL                Write( SvStream& rOutput, const TextSelection* pSel = NULL, BOOL bHTML = FALSE );

    TextPaM             GetPaM( const Point& rDocPos, BOOL bSmart = TRUE );
    Rectangle           PaMtoEditCursor( const TextPaM& rPaM, BOOL bSpecial = FALSE );
    String              GetWord( const TextPaM& rCursorPos, TextPaM* pStartOfWord = 0 );

    BOOL                    HasAttrib( USHORT nWhich ) const;
    const TextAttrib*       FindAttrib( const TextPaM& rPaM, USHORT nWhich ) const;
    const TextCharAttrib*   FindCharAttrib( const TextPaM& rPaM, USHORT nWhich ) const;

    void                RemoveAttribs( ULONG nPara, USHORT nWhich, BOOL bIdleFormatAndUpdate );
    void                RemoveAttrib( ULONG nPara, const TextCharAttrib& rAttrib );
    void                RemoveAttribs( ULONG nPara, BOOL bIdleFormatAndUpdate = TRUE );
    void                SetAttrib( const TextAttrib& rAttr, ULONG nPara, USHORT nStart, USHORT nEnd, BOOL bIdleFormatAndUpdate = TRUE );

    TxtAlign            GetTextAlign() const { return meAlign; }
    void                SetTextAlign( TxtAlign eAlign );

    void                Draw( OutputDevice* pDev, const Point& rPos );

    void                            SetLocale( const ::com::sun::star::lang::Locale& rLocale );
    ::com::sun::star::lang::Locale  GetLocale();
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > GetBreakIterator();

    static BOOL         DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static BOOL         DoesKeyMoveCursor( const KeyEvent& rKeyEvent );
    static BOOL         IsSimpleCharInput( const KeyEvent& rKeyEvent );
};

#endif // _TEXTENG_HXX

