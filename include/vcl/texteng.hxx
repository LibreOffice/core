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
#ifndef _TEXTENG_HXX
#define _TEXTENG_HXX

#include <vcl/dllapi.h>

class TextDoc;
class TextView;
class TextPaM;
class TextSelection;
class TEParaPortions;
class TextAttrib;
class TextCharAttrib;
class TextUndo;
class TextUndoManager;
class EditSelFunctionSet;
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

#include <com/sun/star/lang/Locale.hpp>
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

typedef std::vector<TextView*> TextViews;

class VCL_DLLPUBLIC TextEngine : public SfxBroadcaster
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

    Rectangle           maInvalidRect;
    Range               maInvalidRange;

    LocaleDataWrapper*  mpLocaleDataWrapper;

    Font                maFont;
    Color               maTextColor;
    sal_uInt16              mnCharHeight;
    sal_uInt16              mnFixCharWidth100;

    sal_uLong               mnMaxTextLen;
    sal_uLong               mnMaxTextWidth;
    sal_uLong               mnCurTextWidth;
    sal_uLong               mnCurTextHeight;
    sal_uLong               mnDefTab;

    TxtAlign            meAlign;

    sal_Bool                mbIsFormatting      : 1;    // semaphore for the Hook's
    sal_Bool                mbFormatted         : 1;
    sal_Bool                mbUpdate            : 1;
    sal_Bool                mbModified          : 1;
    sal_Bool                mbUndoEnabled       : 1;
    sal_Bool                mbIsInUndo          : 1;
    sal_Bool                mbDowning           : 1;
    sal_Bool                mbRightToLeft       : 1;
    sal_Bool                mbHasMultiLineParas : 1;

                        TextEngine( const TextEngine& ) : SfxBroadcaster()  {}
    TextEngine&         operator=( const TextEngine& )      { return *this; }

protected:

    void                CursorMoved( sal_uLong nNode );
    void                TextModified();

    void                ImpInitDoc();
    void                ImpRemoveText();
    TextPaM             ImpDeleteText( const TextSelection& rSel );
    TextPaM             ImpInsertText( const TextSelection& rSel, sal_Unicode c, sal_Bool bOverwrite = sal_False );
    TextPaM             ImpInsertText( const TextSelection& rSel, const OUString& rText );
    TextPaM             ImpInsertParaBreak( const TextSelection& rTextSelection, sal_Bool bKeepEndingAttribs = sal_True );
    TextPaM             ImpInsertParaBreak( const TextPaM& rPaM, sal_Bool bKeepEndingAttribs = sal_True );
    void                ImpRemoveChars( const TextPaM& rPaM, sal_uInt16 nChars, SfxUndoAction* pCurUndo = 0 );
    TextPaM             ImpConnectParagraphs( sal_uLong nLeft, sal_uLong nRight );
    void                ImpRemoveParagraph( sal_uLong nPara );
    void                ImpInitWritingDirections( sal_uLong nPara );
    LocaleDataWrapper*  ImpGetLocaleDataWrapper();

    // to remain compatible in the minor release we copy the above ImpInsertText
    // function and add the extra parameter we need but make sure this function
    // gets not exported. First and seconf parameter swapped to have a different signatur.
    SAL_DLLPRIVATE TextPaM  ImpInsertText( sal_Unicode c, const TextSelection& rSel, sal_Bool bOverwrite = sal_False, sal_Bool bIsUserInput = sal_False );
    // some other new functions needed that must not be exported to remain compatible
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XExtendedInputSequenceChecker > GetInputSequenceChecker() const;
    SAL_DLLPRIVATE sal_Bool IsInputSequenceCheckingRequired( sal_Unicode c, const TextSelection& rCurSel ) const;

    // broadcast or adjust selections
    void                ImpParagraphInserted( sal_uLong nPara );
    void                ImpParagraphRemoved( sal_uLong nPara );
    void                ImpCharsRemoved( sal_uLong nPara, sal_uInt16 nPos, sal_uInt16 nChars );
    void                ImpCharsInserted( sal_uLong nPara, sal_uInt16 nPos, sal_uInt16 nChars );
    void                ImpFormattingParagraph( sal_uLong nPara );
    void                ImpTextHeightChanged();
    void                ImpTextFormatted();

    DECL_LINK(          IdleFormatHdl, void * );
    void                CheckIdleFormatter();
    void                IdleFormatAndUpdate( TextView* pCurView = 0, sal_uInt16 nMaxTimerRestarts = 5 );

    sal_Bool                CreateLines( sal_uLong nPara );
    void                CreateAndInsertEmptyLine( sal_uLong nPara );
    void                ImpBreakLine( sal_uLong nPara, TextLine* pLine, TETextPortion* pPortion, sal_uInt16 nPortionStart, long nRemainingWidth );
    sal_uInt16              SplitTextPortion( sal_uLong nPara, sal_uInt16 nPos );
    void                CreateTextPortions( sal_uLong nPara, sal_uInt16 nStartPos );
    void                RecalcTextPortion( sal_uLong nPara, sal_uInt16 nStartPos, short nNewChars );
    void                SeekCursor( sal_uLong nNode, sal_uInt16 nPos, Font& rFont, OutputDevice* pOutDev );

    void                FormatDoc();
    void                FormatFullDoc();
    void                FormatAndUpdate( TextView* pCurView = 0 );
    sal_Bool                IsFormatting() const { return mbIsFormatting; }
    void                UpdateViews( TextView* pCurView = 0 );

    void                ImpPaint( OutputDevice* pOut, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange = 0, TextSelection const* pSelection = 0 );

    void                UpdateSelections();

    sal_Bool                IsFormatted() const { return mbFormatted; }

    sal_uInt16              GetCharPos( sal_uLong nPara, sal_uInt16 nLine, long nDocPosX, sal_Bool bSmart = sal_False );
    Rectangle           GetEditCursor( const TextPaM& rPaM, sal_Bool bSpecial, sal_Bool bPreferPortionStart = sal_False );
    sal_uInt16              ImpFindIndex( sal_uLong nPortion, const Point& rPosInPara, sal_Bool bSmart );
    long                ImpGetPortionXOffset( sal_uLong nPara, TextLine* pLine, sal_uInt16 nTextPortion );
    long                ImpGetXPos( sal_uLong nPara, TextLine* pLine, sal_uInt16 nIndex, sal_Bool bPreferPortionStart = sal_False );
    long                ImpGetOutputOffset( sal_uLong nPara, TextLine* pLine, sal_uInt16 nIndex, sal_uInt16 nIndex2 );
    sal_uInt8                ImpGetRightToLeft( sal_uLong nPara, sal_uInt16 nPos, sal_uInt16* pStart = NULL, sal_uInt16* pEnd = NULL );
    void                ImpInitLayoutMode( OutputDevice* pOutDev, sal_Bool bDrawingR2LPortion = sal_False );
    TxtAlign            ImpGetAlign() const;

    sal_uLong               CalcTextHeight();
    sal_uLong               CalcParaHeight( sal_uLong nParagraph ) const;
    sal_uLong               CalcTextWidth( sal_uLong nPara );
    sal_uLong               CalcTextWidth( sal_uLong nPara, sal_uInt16 nPortionStart, sal_uInt16 nPortionLen, const Font* pFont = 0 );
    Range               GetInvalidYOffsets( sal_uLong nPortion );

    // for Undo/Redo
    void                InsertContent( TextNode* pNode, sal_uLong nPara );
    TextPaM             SplitContent( sal_uLong nNode, sal_uInt16 nSepPos );
    TextPaM             ConnectContents( sal_uLong nLeftNode );

    // adjust PaM's and selections that were transfered to the API to a valid range
    void                ValidateSelection( TextSelection& rSel ) const;
    void                ValidatePaM( TextPaM& rPaM ) const;

public:
                        TextEngine();
                        ~TextEngine();

    void                SetText( const OUString& rStr );
    String              GetText( LineEnd aSeparator = LINEEND_LF ) const;
    String              GetText( const TextSelection& rSel, LineEnd aSeparator = LINEEND_LF ) const;
    String              GetTextLines( LineEnd aSeparator = LINEEND_LF ) const;
    void                ReplaceText(const TextSelection& rSel, const String& rText);

    sal_uLong               GetTextLen( LineEnd aSeparator = LINEEND_LF ) const;
    sal_uLong               GetTextLen( const TextSelection& rSel, LineEnd aSeparator = LINEEND_LF ) const;

    void                SetFont( const Font& rFont );
    const Font&         GetFont() const { return maFont; }

    sal_uInt16              GetDefTab() const;

    void                SetLeftMargin( sal_uInt16 n );
    sal_uInt16              GetLeftMargin() const;

    void                SetUpdateMode( sal_Bool bUpdate );
    sal_Bool                GetUpdateMode() const { return mbUpdate; }

    sal_uInt16              GetViewCount() const;
    TextView*           GetView( sal_uInt16 nView ) const;
    void                InsertView( TextView* pTextView );
    void                RemoveView( TextView* pTextView );
    TextView*           GetActiveView() const;
    void                SetActiveView( TextView* pView );

    void                SetMaxTextLen( sal_uLong nLen );
    sal_uLong               GetMaxTextLen() const { return mnMaxTextLen; }

    void                SetMaxTextWidth( sal_uLong nWidth );
    sal_uLong               GetMaxTextWidth() const { return mnMaxTextWidth; }

    sal_uLong               GetTextHeight() const;
    sal_uLong               CalcTextWidth();
    sal_uInt16              GetCharHeight() const { return mnCharHeight; }

    sal_uLong               GetParagraphCount() const;
    String              GetText( sal_uLong nParagraph ) const;
    sal_uInt16              GetTextLen( sal_uLong nParagraph ) const;
    sal_uLong               GetTextHeight( sal_uLong nParagraph ) const;

    sal_uInt16              GetLineCount( sal_uLong nParagraph ) const;
    sal_uInt16              GetLineLen( sal_uLong nParagraph, sal_uInt16 nLine ) const;

    void                SetRightToLeft( sal_Bool bR2L );
    sal_Bool                IsRightToLeft() const { return mbRightToLeft; }

    sal_Bool            HasUndoManager() const { return mpUndoManager ? sal_True : sal_False; }
    ::svl::IUndoManager&
                        GetUndoManager();
    void                UndoActionStart( sal_uInt16 nId = 0 );
    void                UndoActionEnd();
    void                InsertUndo( TextUndo* pUndo, sal_Bool bTryMerge = sal_False );
    sal_Bool            IsInUndo()                  { return mbIsInUndo; }
    void                SetIsInUndo( sal_Bool bInUndo ) { mbIsInUndo = bInUndo; }
    void                ResetUndo();

    void                EnableUndo( sal_Bool bEnable );
    sal_Bool                IsUndoEnabled()             { return mbUndoEnabled; }

    void                SetModified( sal_Bool bModified )   { mbModified = bModified; }
    sal_Bool                IsModified() const              { return mbModified; }

    sal_Bool                Read( SvStream& rInput, const TextSelection* pSel = NULL );

    sal_Bool                Write( SvStream& rOutput, const TextSelection* pSel = NULL, sal_Bool bHTML = sal_False );

    TextPaM             GetPaM( const Point& rDocPos, sal_Bool bSmart = sal_True );
    Rectangle           PaMtoEditCursor( const TextPaM& rPaM, sal_Bool bSpecial = sal_False );
    String              GetWord( const TextPaM& rCursorPos, TextPaM* pStartOfWord = 0 );

    sal_Bool                    HasAttrib( sal_uInt16 nWhich ) const;
    const TextAttrib*       FindAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const;
    const TextCharAttrib*   FindCharAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const;

    void                RemoveAttribs( sal_uLong nPara, sal_uInt16 nWhich, sal_Bool bIdleFormatAndUpdate );
    void                RemoveAttrib( sal_uLong nPara, const TextCharAttrib& rAttrib );
    void                RemoveAttribs( sal_uLong nPara, sal_Bool bIdleFormatAndUpdate = sal_True );
    void                SetAttrib( const TextAttrib& rAttr, sal_uLong nPara, sal_uInt16 nStart, sal_uInt16 nEnd, sal_Bool bIdleFormatAndUpdate = sal_True );

    TxtAlign            GetTextAlign() const { return meAlign; }
    void                SetTextAlign( TxtAlign eAlign );

    void                Draw( OutputDevice* pDev, const Point& rPos );

    void                            SetLocale( const ::com::sun::star::lang::Locale& rLocale );
    ::com::sun::star::lang::Locale  GetLocale();
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > GetBreakIterator();

    static sal_Bool         DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static sal_Bool         IsSimpleCharInput( const KeyEvent& rKeyEvent );
};

#endif // _TEXTENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
