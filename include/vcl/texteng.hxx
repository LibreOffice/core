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
#ifndef INCLUDED_VCL_TEXTENG_HXX
#define INCLUDED_VCL_TEXTENG_HXX

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
#include <rtl/ustring.hxx>
#include <svl/brdcst.hxx>
#include <tools/lineend.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/font.hxx>

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
    css::uno::Reference < css::i18n::XExtendedInputSequenceChecker > mxISC;

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

    bool                mbIsFormatting      : 1;    // semaphore for the Hook's
    bool                mbFormatted         : 1;
    bool                mbUpdate            : 1;
    bool                mbModified          : 1;
    bool                mbUndoEnabled       : 1;
    bool                mbIsInUndo          : 1;
    bool                mbDowning           : 1;
    bool                mbRightToLeft       : 1;
    bool                mbHasMultiLineParas : 1;

                        TextEngine( const TextEngine& ) : SfxBroadcaster()  {}
    TextEngine&         operator=( const TextEngine& )      { return *this; }

protected:

    void                CursorMoved( sal_uLong nNode );
    void                TextModified();

    void                ImpInitDoc();
    void                ImpRemoveText();
    TextPaM             ImpDeleteText( const TextSelection& rSel );
    TextPaM             ImpInsertText( const TextSelection& rSel, sal_Unicode c, bool bOverwrite = false );
    TextPaM             ImpInsertText( const TextSelection& rSel, const OUString& rText );
    TextPaM             ImpInsertParaBreak( const TextSelection& rTextSelection, bool bKeepEndingAttribs = true );
    TextPaM             ImpInsertParaBreak( const TextPaM& rPaM, bool bKeepEndingAttribs = true );
    void                ImpRemoveChars( const TextPaM& rPaM, sal_uInt16 nChars, SfxUndoAction* pCurUndo = 0 );
    TextPaM             ImpConnectParagraphs( sal_uLong nLeft, sal_uLong nRight );
    void                ImpRemoveParagraph( sal_uLong nPara );
    void                ImpInitWritingDirections( sal_uLong nPara );
    LocaleDataWrapper*  ImpGetLocaleDataWrapper();

    // to remain compatible in the minor release we copy the above ImpInsertText
    // function and add the extra parameter we need but make sure this function
    // gets not exported. First and seconf parameter swapped to have a different signatur.
    SAL_DLLPRIVATE TextPaM  ImpInsertText( sal_Unicode c, const TextSelection& rSel, bool bOverwrite = false, bool bIsUserInput = false );
    // some other new functions needed that must not be exported to remain compatible
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XExtendedInputSequenceChecker > GetInputSequenceChecker();
    SAL_DLLPRIVATE bool IsInputSequenceCheckingRequired( sal_Unicode c, const TextSelection& rCurSel ) const;

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

    bool                CreateLines( sal_uLong nPara );
    void                CreateAndInsertEmptyLine( sal_uLong nPara );
    void                ImpBreakLine( sal_uLong nPara, TextLine* pLine, TETextPortion* pPortion, sal_uInt16 nPortionStart, long nRemainingWidth );
    sal_uInt16              SplitTextPortion( sal_uLong nPara, sal_uInt16 nPos );
    void                CreateTextPortions( sal_uLong nPara, sal_uInt16 nStartPos );
    void                RecalcTextPortion( sal_uLong nPara, sal_uInt16 nStartPos, short nNewChars );
    void                SeekCursor( sal_uLong nNode, sal_uInt16 nPos, Font& rFont, OutputDevice* pOutDev );

    void                FormatDoc();
    void                FormatFullDoc();
    void                FormatAndUpdate( TextView* pCurView = 0 );
    bool                IsFormatting() const { return mbIsFormatting; }
    void                UpdateViews( TextView* pCurView = 0 );

    void                ImpPaint( OutputDevice* pOut, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange = 0, TextSelection const* pSelection = 0 );

    void                UpdateSelections();

    bool                IsFormatted() const { return mbFormatted; }

    sal_uInt16              GetCharPos( sal_uLong nPara, sal_uInt16 nLine, long nDocPosX, bool bSmart = false );
    Rectangle           GetEditCursor( const TextPaM& rPaM, bool bSpecial, bool bPreferPortionStart = false );
    sal_uInt16              ImpFindIndex( sal_uLong nPortion, const Point& rPosInPara, bool bSmart );
    long                ImpGetPortionXOffset( sal_uLong nPara, TextLine* pLine, sal_uInt16 nTextPortion );
    long                ImpGetXPos( sal_uLong nPara, TextLine* pLine, sal_uInt16 nIndex, bool bPreferPortionStart = false );
    long                ImpGetOutputOffset( sal_uLong nPara, TextLine* pLine, sal_uInt16 nIndex, sal_uInt16 nIndex2 );
    sal_uInt8                ImpGetRightToLeft( sal_uLong nPara, sal_uInt16 nPos, sal_uInt16* pStart = NULL, sal_uInt16* pEnd = NULL );
    void                ImpInitLayoutMode( OutputDevice* pOutDev, bool bDrawingR2LPortion = false );
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
    OUString            GetText( LineEnd aSeparator = LINEEND_LF ) const;
    OUString            GetText( const TextSelection& rSel, LineEnd aSeparator = LINEEND_LF ) const;
    OUString            GetTextLines( LineEnd aSeparator = LINEEND_LF ) const;
    void                ReplaceText(const TextSelection& rSel, const OUString& rText);

    sal_uLong               GetTextLen( LineEnd aSeparator = LINEEND_LF ) const;
    sal_uLong               GetTextLen( const TextSelection& rSel, LineEnd aSeparator = LINEEND_LF ) const;

    void                SetFont( const Font& rFont );
    const Font&         GetFont() const { return maFont; }

    sal_uInt16              GetDefTab() const;

    void                SetLeftMargin( sal_uInt16 n );
    sal_uInt16          GetLeftMargin() const;

    void                SetUpdateMode( bool bUpdate );
    bool            GetUpdateMode() const { return mbUpdate; }

    sal_uInt16          GetViewCount() const;
    TextView*           GetView( sal_uInt16 nView ) const;
    void                InsertView( TextView* pTextView );
    void                RemoveView( TextView* pTextView );
    TextView*           GetActiveView() const;
    void                SetActiveView( TextView* pView );

    void                SetMaxTextLen( sal_uLong nLen );
    sal_uLong           GetMaxTextLen() const { return mnMaxTextLen; }

    void                SetMaxTextWidth( sal_uLong nWidth );
    sal_uLong           GetMaxTextWidth() const { return mnMaxTextWidth; }

    sal_uLong           GetTextHeight() const;
    sal_uLong           CalcTextWidth();
    sal_uInt16          GetCharHeight() const { return mnCharHeight; }

    sal_uLong           GetParagraphCount() const;
    OUString            GetText( sal_uLong nParagraph ) const;
    sal_uInt16          GetTextLen( sal_uLong nParagraph ) const;
    sal_uLong           GetTextHeight( sal_uLong nParagraph ) const;

    sal_uInt16          GetLineCount( sal_uLong nParagraph ) const;
    sal_uInt16          GetLineLen( sal_uLong nParagraph, sal_uInt16 nLine ) const;

    void                SetRightToLeft( bool bR2L );
    bool            IsRightToLeft() const { return mbRightToLeft; }

    bool            HasUndoManager() const { return mpUndoManager ? true : false; }
    ::svl::IUndoManager&
                        GetUndoManager();
    void                UndoActionStart( sal_uInt16 nId = 0 );
    void                UndoActionEnd();
    void                InsertUndo( TextUndo* pUndo, bool bTryMerge = false );
    bool            IsInUndo()                  { return mbIsInUndo; }
    void                SetIsInUndo( bool bInUndo ) { mbIsInUndo = bInUndo; }
    void                ResetUndo();

    void                EnableUndo( bool bEnable );
    bool            IsUndoEnabled()             { return mbUndoEnabled; }

    void                SetModified( bool bModified )   { mbModified = bModified; }
    bool            IsModified() const              { return mbModified; }

    bool            Read( SvStream& rInput, const TextSelection* pSel = NULL );

    bool            Write( SvStream& rOutput, const TextSelection* pSel = NULL, bool bHTML = false );

    TextPaM             GetPaM( const Point& rDocPos, bool bSmart = true );
    Rectangle           PaMtoEditCursor( const TextPaM& rPaM, bool bSpecial = false );
    OUString            GetWord( const TextPaM& rCursorPos, TextPaM* pStartOfWord = 0 );

    bool            HasAttrib( sal_uInt16 nWhich ) const;
    const TextAttrib*       FindAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const;
    const TextCharAttrib*   FindCharAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const;

    void                RemoveAttribs( sal_uLong nPara, sal_uInt16 nWhich, bool bIdleFormatAndUpdate );
    void                RemoveAttrib( sal_uLong nPara, const TextCharAttrib& rAttrib );
    void                RemoveAttribs( sal_uLong nPara, bool bIdleFormatAndUpdate = true );
    void                SetAttrib( const TextAttrib& rAttr, sal_uLong nPara, sal_uInt16 nStart, sal_uInt16 nEnd, bool bIdleFormatAndUpdate = true );

    TxtAlign            GetTextAlign() const { return meAlign; }
    void                SetTextAlign( TxtAlign eAlign );

    void                Draw( OutputDevice* pDev, const Point& rPos );

    void                            SetLocale( const ::com::sun::star::lang::Locale& rLocale );
    ::com::sun::star::lang::Locale  GetLocale();
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > GetBreakIterator();

    static bool     DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static bool     IsSimpleCharInput( const KeyEvent& rKeyEvent );

    Color               GetTextColor() const { return maTextColor; }
};

#endif // INCLUDED_VCL_TEXTENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
