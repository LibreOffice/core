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

#include <memory>
#include <sal/config.h>

#include <cstddef>
#include <vector>

#include <vcl/dllapi.h>
#include <vcl/vclptr.hxx>
#include <rtl/ustring.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <tools/lineend.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <vcl/font.hxx>
#include <vcl/vclenum.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.hxx>

class TextDoc;
class TextView;
class TextPaM;
class TextSelection;
class TEParaPortions;
class TextAttrib;
class TextCharAttrib;
class TextUndo;
class TextUndoManager;
class IdleFormatter;
class TextNode;
class OutputDevice;
class KeyEvent;
class Timer;
class SfxUndoManager;
class TextLine;
struct TEIMEInfos;

namespace com::sun::star::i18n {
    class XBreakIterator;
    class XExtendedInputSequenceChecker;
}

class LocaleDataWrapper;

typedef std::vector<TextView*> TextViews;

class VCL_DLLPUBLIC TextEngine : public SfxBroadcaster
{
    friend class        TextView;
    friend class        TextSelFunctionSet;
    friend class        ExtTextEngine;

    friend class        TextUndo;
    friend class        TextUndoManager;
    friend class        TextUndoDelPara;
    friend class        TextUndoConnectParas;
    friend class        TextUndoSplitPara;
    friend class        TextUndoInsertChars;
    friend class        TextUndoRemoveChars;

    std::unique_ptr<TextDoc>          mpDoc;
    std::unique_ptr<TEParaPortions>   mpTEParaPortions;
    VclPtr<OutputDevice> mpRefDev;

    std::unique_ptr<TextViews>        mpViews;
    TextView*           mpActiveView;

    std::unique_ptr<TextUndoManager>  mpUndoManager;

    std::unique_ptr<IdleFormatter>    mpIdleFormatter;

    std::unique_ptr<TEIMEInfos> mpIMEInfos;

    css::lang::Locale   maLocale;
    css::uno::Reference< css::i18n::XBreakIterator > mxBreakIterator;
    css::uno::Reference < css::i18n::XExtendedInputSequenceChecker > mxISC;

    tools::Rectangle           maInvalidRect;

    std::unique_ptr<LocaleDataWrapper> mpLocaleDataWrapper;

    vcl::Font           maFont;
    Color               maTextColor;

    sal_Int32           mnMaxTextLen;
    tools::Long                mnMaxTextWidth;
    tools::Long                mnCharHeight;
    tools::Long                mnCurTextWidth;
    tools::Long                mnCurTextHeight;
    tools::Long                mnDefTab;

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
    // gets not exported. First and second parameter swapped to have a different signature.
    SAL_DLLPRIVATE TextPaM  ImpInsertText( sal_Unicode c, const TextSelection& rSel, bool bOverwrite, bool bIsUserInput = false );
    // some other new functions needed that must not be exported to remain compatible
    SAL_DLLPRIVATE css::uno::Reference< css::i18n::XExtendedInputSequenceChecker > const & GetInputSequenceChecker();
    SAL_DLLPRIVATE bool IsInputSequenceCheckingRequired( sal_Unicode c, const TextSelection& rCurSel ) const;

    // broadcast or adjust selections
    void                ImpParagraphInserted( sal_uInt32 nPara );
    void                ImpParagraphRemoved( sal_uInt32 nPara );
    void                ImpCharsRemoved( sal_uInt32 nPara, sal_Int32 nPos, sal_Int32 nChars );
    void                ImpCharsInserted( sal_uInt32 nPara, sal_Int32 nPos, sal_Int32 nChars );

    DECL_LINK(    IdleFormatHdl, Timer *, void );
    void                CheckIdleFormatter();
    void                IdleFormatAndUpdate( TextView* pCurView, sal_uInt16 nMaxTimerRestarts = 5 );

    bool                CreateLines( sal_uInt32 nPara );
    void                CreateAndInsertEmptyLine( sal_uInt32 nPara );
    void                ImpBreakLine( sal_uInt32 nPara, TextLine* pLine, sal_Int32 nPortionStart, tools::Long nRemainingWidth );
    std::size_t         SplitTextPortion( sal_uInt32 nPara, sal_Int32 nPos );
    void                CreateTextPortions( sal_uInt32 nPara, sal_Int32 nStartPos );
    void                RecalcTextPortion( sal_uInt32 nPara, sal_Int32 nStartPos, sal_Int32 nNewChars );
    void                SeekCursor( sal_uInt32 nNode, sal_Int32 nPos, vcl::Font& rFont, OutputDevice* pOutDev );

    void                FormatDoc();
    void                FormatFullDoc();
    void                FormatAndUpdate( TextView* pCurView = nullptr );
    bool                IsFormatting() const { return mbIsFormatting; }
    void                UpdateViews( TextView* pCurView = nullptr );

    void                ImpPaint( OutputDevice* pOut, const Point& rStartPos, tools::Rectangle const* pPaintArea, TextSelection const* pSelection = nullptr );

    bool                IsFormatted() const { return mbFormatted; }

    sal_Int32           GetCharPos( sal_uInt32 nPara, std::vector<TextLine>::size_type nLine, tools::Long nDocPosX );
    tools::Rectangle    GetEditCursor( const TextPaM& rPaM, bool bSpecial, bool bPreferPortionStart = false );
    sal_Int32           ImpFindIndex( sal_uInt32 nPortion, const Point& rPosInPara );
    tools::Long                ImpGetPortionXOffset( sal_uInt32 nPara, TextLine const * pLine, std::size_t nTextPortion );
    tools::Long                ImpGetXPos( sal_uInt32 nPara, TextLine* pLine, sal_Int32 nIndex, bool bPreferPortionStart = false );
    tools::Long                ImpGetOutputOffset( sal_uInt32 nPara, TextLine* pLine, sal_Int32 nIndex, sal_Int32 nIndex2 );
    bool                ImpGetRightToLeft( sal_uInt32 nPara, sal_Int32 nPos );
    static void         ImpInitLayoutMode( OutputDevice* pOutDev );
    TxtAlign            ImpGetAlign() const;

    tools::Long                CalcTextHeight();
    tools::Long                CalcParaHeight( sal_uInt32 nParagraph ) const;
    tools::Long                CalcTextWidth( sal_uInt32 nPara );
    tools::Long                CalcTextWidth( sal_uInt32 nPara, sal_Int32 nPortionStart, sal_Int32 nPortionLen);
    Range               GetInvalidYOffsets( sal_uInt32 nPortion );

    // for Undo/Redo
    void                InsertContent( std::unique_ptr<TextNode> pNode, sal_uInt32 nPara );
    TextPaM             SplitContent( sal_uInt32 nNode, sal_Int32 nSepPos );
    TextPaM             ConnectContents( sal_uInt32 nLeftNode );

    // adjust PaM's and selections that were transferred to the API to a valid range
    void                ValidateSelection( TextSelection& rSel ) const;
    void                ValidatePaM( TextPaM& rPaM ) const;

public:
                        TextEngine();
                        virtual ~TextEngine() override;
                        TextEngine( const TextEngine& ) = delete;
    TextEngine&         operator=( const TextEngine& ) = delete;

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

    void                SetMaxTextWidth( tools::Long nWidth );
    tools::Long                GetMaxTextWidth() const { return mnMaxTextWidth; }

    tools::Long                GetTextHeight() const;
    tools::Long                CalcTextWidth();
    tools::Long                GetCharHeight() const { return mnCharHeight; }

    sal_uInt32          GetParagraphCount() const;
    OUString            GetText( sal_uInt32 nParagraph ) const;
    sal_Int32           GetTextLen( sal_uInt32 nParagraph ) const;
    tools::Long                GetTextHeight( sal_uInt32 nParagraph ) const;

    void                GetTextPortionRange(const TextPaM& rPaM, sal_Int32& nStart, sal_Int32& nEnd);

    sal_uInt16          GetLineCount( sal_uInt32 nParagraph ) const;
    sal_Int32           GetLineLen( sal_uInt32 nParagraph, sal_uInt16 nLine ) const;

    void                SetRightToLeft( bool bR2L );
    bool                IsRightToLeft() const { return mbRightToLeft; }

    bool                HasUndoManager() const { return mpUndoManager != nullptr; }
    SfxUndoManager&     GetUndoManager();
    void                UndoActionStart( sal_uInt16 nId = 0 );
    void                UndoActionEnd();
    void                InsertUndo( std::unique_ptr<TextUndo> pUndo, bool bTryMerge = false );
    bool                IsInUndo() const            { return mbIsInUndo; }
    void                SetIsInUndo( bool bInUndo ) { mbIsInUndo = bInUndo; }
    void                ResetUndo();

    void                EnableUndo( bool bEnable );
    bool                IsUndoEnabled() const           { return mbUndoEnabled; }

    void                SetModified( bool bModified )   { mbModified = bModified; }
    bool                IsModified() const              { return mbModified; }

    bool                Read( SvStream& rInput, const TextSelection* pSel = nullptr );

    void                Write( SvStream& rOutput );

    TextPaM             GetPaM( const Point& rDocPos );
    tools::Rectangle    PaMtoEditCursor( const TextPaM& rPaM, bool bSpecial = false );
    OUString            GetWord( const TextPaM& rCursorPos, TextPaM* pStartOfWord = nullptr, TextPaM* pEndOfWord = nullptr );

    const TextAttrib*       FindAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const;
    const TextCharAttrib*   FindCharAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const;

    void                RemoveAttribs( sal_uInt32 nPara );
    void                SetAttrib( const TextAttrib& rAttr, sal_uInt32 nPara, sal_Int32 nStart, sal_Int32 nEnd );

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
