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
#include <vcl/vclptr.hxx>

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
class Idle;
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
#include <svl/SfxBroadcaster.hxx>
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

enum class TxtAlign { Left, Center, Right };

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

private:
    TextDoc*            mpDoc;
    TEParaPortions*     mpTEParaPortions;
    VclPtr<OutputDevice> mpRefDev;

    TextViews*          mpViews;
    TextView*           mpActiveView;

    TextUndoManager*    mpUndoManager;

    IdleFormatter*      mpIdleFormatter;

    TEIMEInfos*         mpIMEInfos;

    css::lang::Locale   maLocale;
    css::uno::Reference< css::i18n::XBreakIterator > mxBreakIterator;
    css::uno::Reference < css::i18n::XExtendedInputSequenceChecker > mxISC;

    Rectangle           maInvalidRect;

    LocaleDataWrapper*  mpLocaleDataWrapper;

    vcl::Font           maFont;
    Color               maTextColor;

    sal_Int32           mnMaxTextLen;
    long                mnMaxTextWidth;
    long                mnCharHeight;
    long                mnCurTextWidth;
    long                mnCurTextHeight;
    long                mnDefTab;

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

    void                CursorMoved( sal_uInt32 nNode );
    void                TextModified();

    void                ImpInitDoc();
    void                ImpRemoveText();
    TextPaM             ImpDeleteText( const TextSelection& rSel );
    TextPaM             ImpInsertText( const TextSelection& rSel, sal_Unicode c, bool bOverwrite = false );
    TextPaM             ImpInsertText( const TextSelection& rSel, const OUString& rText );
    TextPaM             ImpInsertParaBreak( const TextSelection& rTextSelection );
    TextPaM             ImpInsertParaBreak( const TextPaM& rPaM );
    void                ImpRemoveChars( const TextPaM& rPaM, sal_Int32 nChars );
    TextPaM             ImpConnectParagraphs( sal_uInt32 nLeft, sal_uInt32 nRight );
    void                ImpRemoveParagraph( sal_uInt32 nPara );
    void                ImpInitWritingDirections( sal_uInt32 nPara );
    LocaleDataWrapper*  ImpGetLocaleDataWrapper();

    // to remain compatible in the minor release we copy the above ImpInsertText
    // function and add the extra parameter we need but make sure this function
    // gets not exported. First and second parameter swapped to have a different signatur.
    SAL_DLLPRIVATE TextPaM  ImpInsertText( sal_Unicode c, const TextSelection& rSel, bool bOverwrite = false, bool bIsUserInput = false );
    // some other new functions needed that must not be exported to remain compatible
    SAL_DLLPRIVATE css::uno::Reference< css::i18n::XExtendedInputSequenceChecker > const & GetInputSequenceChecker();
    SAL_DLLPRIVATE bool IsInputSequenceCheckingRequired( sal_Unicode c, const TextSelection& rCurSel ) const;

    // broadcast or adjust selections
    void                ImpParagraphInserted( sal_uInt32 nPara );
    void                ImpParagraphRemoved( sal_uInt32 nPara );
    void                ImpCharsRemoved( sal_uInt32 nPara, sal_Int32 nPos, sal_Int32 nChars );
    void                ImpCharsInserted( sal_uInt32 nPara, sal_Int32 nPos, sal_Int32 nChars );
    void                ImpFormattingParagraph( sal_uInt32 nPara );
    void                ImpTextHeightChanged();
    void                ImpTextFormatted();

    DECL_LINK_TYPED(    IdleFormatHdl, Idle *, void );
    void                CheckIdleFormatter();
    void                IdleFormatAndUpdate( TextView* pCurView = nullptr, sal_uInt16 nMaxTimerRestarts = 5 );

    bool                CreateLines( sal_uInt32 nPara );
    void                CreateAndInsertEmptyLine( sal_uInt32 nPara );
    void                ImpBreakLine( sal_uInt32 nPara, TextLine* pLine, TETextPortion* pPortion, sal_Int32 nPortionStart, long nRemainingWidth );
    sal_uInt16          SplitTextPortion( sal_uInt32 nPara, sal_Int32 nPos );
    void                CreateTextPortions( sal_uInt32 nPara, sal_Int32 nStartPos );
    void                RecalcTextPortion( sal_uInt32 nPara, sal_Int32 nStartPos, sal_Int32 nNewChars );
    void                SeekCursor( sal_uInt32 nNode, sal_Int32 nPos, vcl::Font& rFont, OutputDevice* pOutDev );

    void                FormatDoc();
    void                FormatFullDoc();
    void                FormatAndUpdate( TextView* pCurView = nullptr );
    bool                IsFormatting() const { return mbIsFormatting; }
    void                UpdateViews( TextView* pCurView = nullptr );

    void                ImpPaint( OutputDevice* pOut, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange = nullptr, TextSelection const* pSelection = nullptr );

    bool                IsFormatted() const { return mbFormatted; }

    sal_Int32           GetCharPos( sal_uInt32 nPara, sal_uInt16 nLine, long nDocPosX, bool bSmart = false );
    Rectangle           GetEditCursor( const TextPaM& rPaM, bool bSpecial, bool bPreferPortionStart = false );
    sal_Int32           ImpFindIndex( sal_uInt32 nPortion, const Point& rPosInPara, bool bSmart );
    long                ImpGetPortionXOffset( sal_uInt32 nPara, TextLine* pLine, sal_uInt16 nTextPortion );
    long                ImpGetXPos( sal_uInt32 nPara, TextLine* pLine, sal_Int32 nIndex, bool bPreferPortionStart = false );
    long                ImpGetOutputOffset( sal_uInt32 nPara, TextLine* pLine, sal_Int32 nIndex, sal_Int32 nIndex2 );
    sal_uInt8           ImpGetRightToLeft( sal_uInt32 nPara, sal_Int32 nPos );
    static void         ImpInitLayoutMode( OutputDevice* pOutDev );
    TxtAlign            ImpGetAlign() const;

    long                CalcTextHeight();
    long                CalcParaHeight( sal_uInt32 nParagraph ) const;
    long                CalcTextWidth( sal_uInt32 nPara );
    long                CalcTextWidth( sal_uInt32 nPara, sal_Int32 nPortionStart, sal_Int32 nPortionLen);
    Range               GetInvalidYOffsets( sal_uInt32 nPortion );

    // for Undo/Redo
    void                InsertContent( TextNode* pNode, sal_uInt32 nPara );
    TextPaM             SplitContent( sal_uInt32 nNode, sal_Int32 nSepPos );
    TextPaM             ConnectContents( sal_uInt32 nLeftNode );

    // adjust PaM's and selections that were transferred to the API to a valid range
    void                ValidateSelection( TextSelection& rSel ) const;
    void                ValidatePaM( TextPaM& rPaM ) const;

public:
                        TextEngine();
                        virtual ~TextEngine();

    void                SetText( const OUString& rStr );
    OUString            GetText( LineEnd aSeparator = LINEEND_LF ) const;
    OUString            GetText( const TextSelection& rSel, LineEnd aSeparator = LINEEND_LF ) const;
    OUString            GetTextLines( LineEnd aSeparator = LINEEND_LF ) const;
    void                ReplaceText(const TextSelection& rSel, const OUString& rText);

    sal_Int32           GetTextLen() const;
    sal_Int32           GetTextLen( const TextSelection& rSel ) const;

    void                SetFont( const vcl::Font& rFont );
    const vcl::Font&    GetFont() const { return maFont; }

    void                SetLeftMargin( sal_uInt16 n );

    void                SetUpdateMode( bool bUpdate );
    bool                GetUpdateMode() const { return mbUpdate; }

    sal_uInt16          GetViewCount() const;
    TextView*           GetView( sal_uInt16 nView ) const;
    void                InsertView( TextView* pTextView );
    void                RemoveView( TextView* pTextView );
    TextView*           GetActiveView() const { return mpActiveView;}
    void                SetActiveView( TextView* pView );

    void                SetMaxTextLen( sal_Int32 nLen );
    sal_Int32           GetMaxTextLen() const { return mnMaxTextLen; }

    void                SetMaxTextWidth( long nWidth );
    long                GetMaxTextWidth() const { return mnMaxTextWidth; }

    long                GetTextHeight() const;
    long                CalcTextWidth();
    long                GetCharHeight() const { return mnCharHeight; }

    sal_uInt32          GetParagraphCount() const;
    OUString            GetText( sal_uInt32 nParagraph ) const;
    sal_Int32           GetTextLen( sal_uInt32 nParagraph ) const;
    long                GetTextHeight( sal_uInt32 nParagraph ) const;

    sal_uInt16          GetLineCount( sal_uInt32 nParagraph ) const;
    sal_Int32           GetLineLen( sal_uInt32 nParagraph, sal_uInt16 nLine ) const;

    void                SetRightToLeft( bool bR2L );
    bool                IsRightToLeft() const { return mbRightToLeft; }

    bool                HasUndoManager() const { return mpUndoManager != nullptr; }
    ::svl::IUndoManager&
                        GetUndoManager();
    void                UndoActionStart( sal_uInt16 nId = 0 );
    void                UndoActionEnd();
    void                InsertUndo( TextUndo* pUndo, bool bTryMerge = false );
    bool                IsInUndo()                  { return mbIsInUndo; }
    void                SetIsInUndo( bool bInUndo ) { mbIsInUndo = bInUndo; }
    void                ResetUndo();

    void                EnableUndo( bool bEnable );
    bool                IsUndoEnabled()             { return mbUndoEnabled; }

    void                SetModified( bool bModified )   { mbModified = bModified; }
    bool                IsModified() const              { return mbModified; }

    bool                Read( SvStream& rInput, const TextSelection* pSel = nullptr );

    bool                Write( SvStream& rOutput, const TextSelection* pSel = nullptr, bool bHTML = false );

    TextPaM             GetPaM( const Point& rDocPos, bool bSmart = true );
    Rectangle           PaMtoEditCursor( const TextPaM& rPaM, bool bSpecial = false );
    OUString            GetWord( const TextPaM& rCursorPos, TextPaM* pStartOfWord = nullptr );

    bool                HasAttrib( sal_uInt16 nWhich ) const;
    const TextAttrib*       FindAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const;
    const TextCharAttrib*   FindCharAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const;

    void                RemoveAttribs( sal_uInt32 nPara, sal_uInt16 nWhich );
    void                RemoveAttrib( sal_uInt32 nPara, const TextCharAttrib& rAttrib );
    void                RemoveAttribs( sal_uInt32 nPara );
    void                SetAttrib( const TextAttrib& rAttr, sal_uInt32 nPara, sal_Int32 nStart, sal_Int32 nEnd, bool bIdleFormatAndUpdate = true );

    TxtAlign            GetTextAlign() const { return meAlign; }
    void                SetTextAlign( TxtAlign eAlign );

    void                Draw( OutputDevice* pDev, const Point& rPos );

    void                SetLocale( const css::lang::Locale& rLocale );
    css::lang::Locale const & GetLocale();
    css::uno::Reference< css::i18n::XBreakIterator > const & GetBreakIterator();

    static bool         DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static bool         IsSimpleCharInput( const KeyEvent& rKeyEvent );

    const Color&        GetTextColor() const { return maTextColor; }
};

#endif // INCLUDED_VCL_TEXTENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
