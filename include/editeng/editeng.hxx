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
// MyEDITENG, due to exported EditEng
#ifndef _MyEDITENG_HXX
#define _MyEDITENG_HXX

#include <rtl/ref.hxx>
#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#include <rsc/rscsfx.hxx>
#include <editeng/editdata.hxx>
#include <i18nlangtag/lang.h>
#include "editeng/editengdllapi.h"

#include <tools/rtti.hxx>   // due to typedef TypeId

#include <editeng/eedata.hxx>

namespace com { namespace sun { namespace star {
  namespace linguistic2 {
    class XSpellChecker1;
    class XHyphenator;
  }
  namespace datatransfer {
    class XTransferable;
  }
  namespace lang {
    struct Locale;
  }
}}}

namespace svx {
struct SpellPortion;
typedef std::vector<SpellPortion> SpellPortions;
}

namespace svl { class IUndoManager; }
namespace basegfx { class B2DPolyPolygon; }
namespace editeng {
    struct MisspellRanges;
}

class ImpEditEngine;
class EditView;
class OutputDevice;
class EditUndo;
class SvxFont;
class SfxItemPool;
class SfxStyleSheet;
class String;
class SfxStyleSheetPool;
class SvxSearchItem;
class SvxFieldItem;
class SvxCharSetColorItem;
class SfxUndoAction;
class MapMode;
class Color;
class Font;
class KeyEvent;
class PolyPolygon;
class Size;
class Point;
class Rectangle;
class SvStream;
class Link;
class OutputDevice;
class Window;
class SfxPoolItem;
class SvxNumBulletItem;
class SvxBulletItem;
class SvxLRSpaceItem;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
class SvxNumberFormat;
class FontList;
class SvxFieldData;
class ContentNode;
class ParaPortion;
class EditSelection;
class EditPaM;
class EditLine;
class InternalEditStatus;
class EditSelectionEngine;
class EditDoc;
struct PasteOrDropInfos;
class Range;
struct EPaM;
class DeletedNodeInfo;
class ParaPortionList;

//////////////////////////////////////////////////////////////////////////////

/** values for GetAttribs
*/
const sal_uInt8 EditEngineAttribs_All   = 0;        /// returns all attributes even when theire not set
const sal_uInt8 EditEngineAttribs_HardAndPara = 1;  /// returns all attributes set on paragraph and on portions
const sal_uInt8 EditEngineAttribs_OnlyHard = 2;     /// returns only attributes hard set on portions

#define GETATTRIBS_STYLESHEET   (sal_uInt8)0x01
#define GETATTRIBS_PARAATTRIBS  (sal_uInt8)0x02
#define GETATTRIBS_CHARATTRIBS  (sal_uInt8)0x04
#define GETATTRIBS_ALL          (sal_uInt8)0xFF

class EDITENG_DLLPUBLIC EditEngine
{
    friend class EditView;
    friend class ImpEditView;
    friend class EditDbg;
    friend class Outliner;

public:
    typedef std::vector<EditView*> ViewsType;

private:
    ImpEditEngine*  pImpEditEngine;

                    EDITENG_DLLPRIVATE EditEngine( const EditEngine& );
    EDITENG_DLLPRIVATE EditEngine&      operator=( const EditEngine& );
    EDITENG_DLLPRIVATE sal_uInt8        PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pView, Window* pFrameWin = NULL );

    EDITENG_DLLPRIVATE void CursorMoved(ContentNode* pPrevNode);
    EDITENG_DLLPRIVATE void CheckIdleFormatter();
    EDITENG_DLLPRIVATE bool IsIdleFormatterActive() const;
    EDITENG_DLLPRIVATE ParaPortion* FindParaPortion(ContentNode* pNode);
    EDITENG_DLLPRIVATE const ParaPortion* FindParaPortion(ContentNode* pNode) const;
    EDITENG_DLLPRIVATE const ParaPortion* GetPrevVisPortion(const ParaPortion* pCurPortion) const;

    EDITENG_DLLPRIVATE com::sun::star::uno::Reference<
        com::sun::star::datatransfer::XTransferable>
            CreateTransferable(const EditSelection& rSelection);

    EDITENG_DLLPRIVATE EditSelection InsertText(
        com::sun::star::uno::Reference<com::sun::star::datatransfer::XTransferable >& rxDataObj,
        const String& rBaseURL, const EditPaM& rPaM, bool bUseSpecial);

    EDITENG_DLLPRIVATE EditPaM EndOfWord(
        const EditPaM& rPaM, sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES);

    EDITENG_DLLPRIVATE EditPaM GetPaM(const Point& aDocPos, bool bSmart = true);

    EDITENG_DLLPRIVATE EditSelection SelectWord(
        const EditSelection& rCurSelection,
        sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES,
        bool bAcceptStartOfWord = true);

    EDITENG_DLLPRIVATE long GetXPos(
        const ParaPortion* pParaPortion, const EditLine* pLine, sal_uInt16 nIndex, bool bPreferPortionStart = false) const;

    EDITENG_DLLPRIVATE Range GetLineXPosStartEnd(
        const ParaPortion* pParaPortion, const EditLine* pLine) const;

    EDITENG_DLLPRIVATE sal_uInt16 GetOnePixelInRef() const;
    EDITENG_DLLPRIVATE InternalEditStatus& GetInternalEditStatus();

    EDITENG_DLLPRIVATE void HandleBeginPasteOrDrop(PasteOrDropInfos& rInfos);
    EDITENG_DLLPRIVATE void HandleEndPasteOrDrop(PasteOrDropInfos& rInfos);
    EDITENG_DLLPRIVATE bool HasText() const;
    EDITENG_DLLPRIVATE const EditSelectionEngine& GetSelectionEngine() const;
    EDITENG_DLLPRIVATE void SetInSelectionMode(bool b);

protected:


public:
                    EditEngine( SfxItemPool* pItemPool );
    virtual         ~EditEngine();

    const SfxItemSet&   GetEmptyItemSet();

    void            SetDefTab( sal_uInt16 nDefTab );

    void            SetRefDevice( OutputDevice* pRefDef );
    OutputDevice*   GetRefDevice() const;

    void            SetRefMapMode( const MapMode& rMapMode );
    MapMode         GetRefMapMode();

    void            SetUpdateMode( sal_Bool bUpdate );
    sal_Bool            GetUpdateMode() const;

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;
    Color           GetAutoColor() const;
    void            EnableAutoColor( sal_Bool b );
    void            ForceAutoColor( sal_Bool b );
    sal_Bool            IsForceAutoColor() const;

    void            InsertView(EditView* pEditView, size_t nIndex = EE_APPEND);
    EditView*       RemoveView( EditView* pEditView );
    EditView*       RemoveView(size_t nIndex = EE_APPEND);
    EditView*       GetView(size_t nIndex = 0) const;
    size_t          GetViewCount() const;
    sal_Bool            HasView( EditView* pView ) const;
    EditView*       GetActiveView() const;
    void SetActiveView(EditView* pView);

    void            SetPaperSize( const Size& rSize );
    const Size&     GetPaperSize() const;

    void            SetVertical( bool bVertical );
    bool            IsVertical() const;

    void            SetFixedCellHeight( sal_Bool bUseFixedCellHeight );

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    sal_uInt16          GetScriptType( const ESelection& rSelection ) const;
    LanguageType    GetLanguage(const EditPaM& rPaM) const;
    LanguageType    GetLanguage( sal_Int32 nPara, sal_uInt16 nPos ) const;

    void            TransliterateText( const ESelection& rSelection, sal_Int32 nTransliterationMode );
    EditSelection   TransliterateText( const EditSelection& rSelection, sal_Int32 nTransliterationMode );

    void            SetAsianCompressionMode( sal_uInt16 nCompression );

    void            SetKernAsianPunctuation( sal_Bool bEnabled );

    void            SetAddExtLeading( sal_Bool b );

    void                SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    void                SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon);
    void                ClearPolygon();

    const Size&     GetMinAutoPaperSize() const;
    void            SetMinAutoPaperSize( const Size& rSz );

    const Size&     GetMaxAutoPaperSize() const;
    void            SetMaxAutoPaperSize( const Size& rSz );

    OUString        GetText( LineEnd eEnd = LINEEND_LF ) const;
    OUString        GetText( const ESelection& rSelection, const LineEnd eEnd = LINEEND_LF ) const;
    sal_uInt32      GetTextLen() const;
    sal_uInt32      GetTextHeight() const;
    sal_uInt32      GetTextHeightNTP() const;
    sal_uInt32      CalcTextWidth();

    OUString        GetText( sal_Int32 nParagraph ) const;
    xub_StrLen      GetTextLen( sal_Int32 nParagraph ) const;
    sal_uInt32      GetTextHeight( sal_Int32 nParagraph ) const;

    sal_Int32       GetParagraphCount() const;

    sal_uInt16      GetLineCount( sal_Int32 nParagraph ) const;
    xub_StrLen      GetLineLen( sal_Int32 nParagraph, sal_uInt16 nLine ) const;
    void            GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_Int32 nParagraph, sal_uInt16 nLine ) const;
    sal_uInt16      GetLineNumberAtIndex( sal_Int32 nPara, sal_uInt16 nIndex ) const;
    sal_uInt32      GetLineHeight( sal_Int32 nParagraph, sal_uInt16 nLine = 0 );
    ParagraphInfos  GetParagraphInfos( sal_Int32 nPara );
    sal_Int32       FindParagraph( long nDocPosY );
    EPosition       FindDocPosition( const Point& rDocPos ) const;
    Rectangle       GetCharacterBounds( const EPosition& rPos ) const;

    OUString        GetWord(sal_Int32 nPara, xub_StrLen nIndex);

    ESelection      GetWord( const ESelection& rSelection, sal_uInt16 nWordType ) const;

    void            Clear();
    void            SetText( const OUString& rStr );

    EditTextObject* CreateTextObject();
    EditTextObject* CreateTextObject( sal_Int32 nPara, sal_Int32 nParas = 1 );
    EditTextObject* CreateTextObject( const ESelection& rESelection );
    void            SetText( const EditTextObject& rTextObject );

    void            RemoveParagraph(sal_Int32 nPara);
    void            InsertParagraph(sal_Int32 nPara, const EditTextObject& rTxtObj);
    void            InsertParagraph(sal_Int32 nPara, const OUString& rText);

    void            SetText(sal_Int32 nPara, const OUString& rText);

    virtual void                SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    virtual const SfxItemSet&   GetParaAttribs( sal_Int32 nPara ) const;

    void            GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    SfxItemSet      GetAttribs( sal_Int32 nPara, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt8 nFlags = 0xFF ) const;
    SfxItemSet      GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib = EditEngineAttribs_All );

    sal_Bool        HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich );

    Font            GetStandardFont( sal_Int32 nPara );
    SvxFont         GetStandardSvxFont( sal_Int32 nPara );

    void            RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich );

    void            ShowParagraph( sal_Int32 nParagraph, sal_Bool bShow = sal_True );

    ::svl::IUndoManager& GetUndoManager();
    ::svl::IUndoManager* SetUndoManager(::svl::IUndoManager* pNew);
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionStart(sal_uInt16 nId, const ESelection& rSel);
    void            UndoActionEnd( sal_uInt16 nId );
    sal_Bool        IsInUndo();

    void            EnableUndo( sal_Bool bEnable );
    sal_Bool            IsUndoEnabled();

    /** returns the value last used for bTryMerge while calling ImpEditEngine::InsertUndo
        This is currently used in a bad but needed hack to get undo actions merged in the
        OutlineView in impress. Do not use it unless you want to sell your soul too! */
    bool            HasTriedMergeOnLastAddUndo() const;

    void            ClearModifyFlag();
    void            SetModified();
    sal_Bool            IsModified() const;

    void            SetModifyHdl( const Link& rLink );
    Link            GetModifyHdl() const;

    sal_Bool            IsInSelectionMode() const;

    void            StripPortions();
    void            GetPortions( sal_Int32 nPara, std::vector<sal_uInt16>& rList );

    long            GetFirstLineStartX( sal_Int32 nParagraph );
    Point           GetDocPosTopLeft( sal_Int32 nParagraph );
    Point           GetDocPos( const Point& rPaperPos ) const;
    sal_Bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder = 0 );

    // StartDocPos corrresponds to VisArea.TopLeft().
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos, sal_Bool bClip );
    void            Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation = 0 );

//  sal_uInt32: Error code of the stream.
        sal_uLong               Read( SvStream& rInput, const String& rBaseURL, EETextFormat, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );
    sal_uLong       Write( SvStream& rOutput, EETextFormat );

    void            SetStatusEventHdl( const Link& rLink );
    Link            GetStatusEventHdl() const;

    void            SetNotifyHdl( const Link& rLink );
    Link            GetNotifyHdl() const;

    void            SetImportHdl( const Link& rLink );
    Link            GetImportHdl() const;

    // Do not evaluate font formatting => For Outliner
    sal_Bool            IsFlatMode() const;
    void            SetFlatMode( sal_Bool bFlat );

    void            SetControlWord( sal_uInt32 nWord );
    sal_uInt32      GetControlWord() const;

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickRemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich = 0 );
    void            QuickMarkInvalid( const ESelection& rSel );
    void            QuickFormatDoc( sal_Bool bFull = sal_False );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );
    void            QuickInsertText(const OUString& rText, const ESelection& rSel);
    void            QuickDelete( const ESelection& rSel );
    void            QuickMarkToBeRepainted( sal_Int32 nPara );

    void            SetGlobalCharStretching( sal_uInt16 nX = 100, sal_uInt16 nY = 100 );
    void            GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const;

    void            SetEditTextObjectPool( SfxItemPool* pPool );
    SfxItemPool*    GetEditTextObjectPool() const;

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    void SetStyleSheet(const EditSelection& aSel, SfxStyleSheet* pStyle);
    void                 SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle );
    const SfxStyleSheet* GetStyleSheet( sal_Int32 nPara ) const;
    SfxStyleSheet* GetStyleSheet( sal_Int32 nPara );

    void            SetWordDelimiters( const OUString& rDelimiters );
    OUString        GetWordDelimiters() const;

    void            EraseVirtualDevice();

    void            SetSpeller( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XSpellChecker1 > &xSpeller );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
                    GetSpeller();
    void            SetHyphenator( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XHyphenator >& xHyph );

    void GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const;
    void SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges );

    void            SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );

    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    sal_Bool            HasOnlineSpellErrors() const;
    void            CompleteOnlineSpelling();

    sal_Bool            ShouldCreateBigTextObject() const;

    // For fast Pre-Test without view:
    EESpellState    HasSpellErrors();
    void ClearSpellErrors();
    sal_Bool            HasText( const SvxSearchItem& rSearchItem );

    //initialize sentence spelling
    void            StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc);
    //spell and return a sentence
    bool            SpellSentence(EditView& rEditView, ::svx::SpellPortions& rToFill, bool bIsGrammarChecking );
    // put spell position to start of current sentence
    void            PutSpellingToSentenceStart( EditView& rEditView );
    //applies a changed sentence
    void            ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions, bool bRecheck );

    // for text conversion (see also HasSpellErrors)
    sal_Bool        HasConvertibleTextPortion( LanguageType nLang );
    virtual sal_Bool    ConvertNextDocument();

    bool UpdateFields();
    bool UpdateFieldsOnly();
    void            RemoveFields( sal_Bool bKeepFieldText, TypeId aType = NULL );

    sal_uInt16      GetFieldCount( sal_Int32 nPara ) const;
    EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const;

    sal_Bool        IsRightToLeft( sal_Int32 nPara ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >
                    CreateTransferable( const ESelection& rSelection ) const;

    // MT: Can't create new virtual functions like for ParagraphInserted/Deleted, musst be compatible in SRC638, change later...
    void            SetBeginMovingParagraphsHdl( const Link& rLink );
    void            SetEndMovingParagraphsHdl( const Link& rLink );
    void            SetBeginPasteOrDropHdl( const Link& rLink );
    void            SetEndPasteOrDropHdl( const Link& rLink );

    virtual void    PaintingFirstLine( sal_Int32 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );
    virtual void    ParagraphInserted( sal_Int32 nNewParagraph );
    virtual void    ParagraphDeleted( sal_Int32 nDeletedParagraph );
    virtual void    ParagraphConnected( sal_Int32 nLeftParagraph, sal_Int32 nRightParagraph );
    virtual void    ParaAttribsChanged( sal_Int32 nParagraph );
    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );
    virtual void    ParagraphHeightChanged( sal_Int32 nPara );

    virtual void DrawingText(
        const Point& rStartPos, const OUString& rText, sal_uInt16 nTextStart, sal_uInt16 nTextLen, const sal_Int32* pDXArray,
        const SvxFont& rFont, sal_Int32 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft,
        const EEngineData::WrongSpellVector* pWrongSpellVector,
        const SvxFieldData* pFieldData,
        bool bEndOfLine,
        bool bEndOfParagraph,
        bool bEndOfBullet,
        const ::com::sun::star::lang::Locale* pLocale,
        const Color& rOverlineColor,
        const Color& rTextLineColor);

    virtual void DrawingTab(
        const Point& rStartPos, long nWidth, const String& rChar,
        const SvxFont& rFont, sal_Int32 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft,
        bool bEndOfLine,
        bool bEndOfParagraph,
        const Color& rOverlineColor,
        const Color& rTextLineColor);
    virtual OUString  GetUndoComment( sal_uInt16 nUndoId ) const;
    virtual sal_Bool    FormattingParagraph( sal_Int32 nPara );
    virtual sal_Bool    SpellNextDocument();
    virtual void    FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, xub_StrLen nPos );
    virtual void    FieldSelected( const SvxFieldItem& rField, sal_Int32 nPara, xub_StrLen nPos );
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, xub_StrLen nPos, Color*& rTxtColor, Color*& rFldColor );

    // to be overloaded if access to bullet information needs to be provided
    virtual const SvxNumberFormat * GetNumberFormat( sal_Int32 nPara ) const;

    virtual Rectangle GetBulletArea( sal_Int32 nPara );

    static SfxItemPool* CreatePool( sal_Bool bLoadRefCounts = sal_True );
    static SfxItemPool& GetGlobalItemPool();
    static sal_Bool     DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static sal_Bool     DoesKeyMoveCursor( const KeyEvent& rKeyEvent );
    static sal_Bool     IsSimpleCharInput( const KeyEvent& rKeyEvent );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const Font& rFont );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const SvxFont& rFont );
    static Font     CreateFontFromItemSet( const SfxItemSet& rItemSet, sal_uInt16 nScriptType );
    static SvxFont  CreateSvxFontFromItemSet( const SfxItemSet& rItemSet );
    static sal_Bool     IsPrintable( sal_Unicode c ) { return ( ( c >= 32 ) && ( c != 127 ) ); }
    static sal_Bool     HasValidData( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rTransferable );

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link& rLink );
    Link            GetBeginDropHdl() const;

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link& rLink );
    Link            GetEndDropHdl() const;

    /// specifies if auto-correction should capitalize the first word or not (default is on)
    void            SetFirstWordCapitalization( sal_Bool bCapitalize );

    EditDoc& GetEditDoc();
    const EditDoc& GetEditDoc() const;

    ParaPortionList& GetParaPortions();
    const ParaPortionList& GetParaPortions() const;

    bool IsFormatted() const;
    bool IsImportHandlerSet() const;
    bool IsImportRTFStyleSheetsSet() const;

    void CallImportHandler(ImportInfo& rInfo);

    void ParaAttribsToCharAttribs(ContentNode* pNode);

    EditPaM CreateEditPaM(const EPaM& rEPaM);
    EditPaM ConnectParagraphs(
        ContentNode* pLeft, ContentNode* pRight, bool bBackward = false);

    EditPaM InsertField(const EditSelection& rEditSelection, const SvxFieldItem& rFld);
    EditPaM InsertText(const EditSelection& aCurEditSelection, const String& rStr);
    EditSelection InsertText(const EditTextObject& rTextObject, const EditSelection& rSel);
    EditPaM InsertParaBreak(
        const EditSelection& rEditSelection, bool bKeepEndingAttribs = true);
    EditPaM InsertLineBreak(const EditSelection& rEditSelection);

    EditPaM CursorLeft(
        const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = com::sun::star::i18n::CharacterIteratorMode::SKIPCELL);
    EditPaM CursorRight(
        const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = com::sun::star::i18n::CharacterIteratorMode::SKIPCELL);

    void SeekCursor(
        ContentNode* pNode, sal_uInt16 nPos, SvxFont& rFont, OutputDevice* pOut = NULL, sal_uInt16 nIgnoreWhich = 0);

    EditPaM DeleteSelection(const EditSelection& rSel);

    ESelection CreateESelection(const EditSelection& rSel);
    EditSelection CreateSelection(const ESelection& rSel);

    const SfxItemSet& GetBaseParaAttribs(sal_Int32 nPara) const;
    void SetParaAttribsOnly(sal_Int32 nPara, const SfxItemSet& rSet);
    void SetAttribs(const EditSelection& rSel, const SfxItemSet& rSet, sal_uInt8 nSpecial = 0);

    String GetSelected(const EditSelection& rSel, const LineEnd eParaSep = LINEEND_LF) const;
    EditPaM DeleteSelected(const EditSelection& rSel);

    sal_uInt16 GetScriptType(const EditSelection& rSel) const;

    void RemoveParaPortion(sal_Int32 nNode);

    void SetCallParaInsertedOrDeleted(bool b);
    bool IsCallParaInsertedOrDeleted() const;

    void AppendDeletedNodeInfo(DeletedNodeInfo* pInfo);
    void UpdateSelections();

    void InsertContent(ContentNode* pNode, sal_Int32 nPos);
    EditPaM SplitContent(sal_Int32 nNode, sal_uInt16 nSepPos);
    EditPaM ConnectContents(sal_Int32 nLeftNode, bool bBackward);

    EditPaM InsertFeature(const EditSelection& rEditSelection, const SfxPoolItem& rItem);

    EditSelection MoveParagraphs(const Range& rParagraphs, sal_Int32 nNewPos, EditView* pCurView);

    void RemoveCharAttribs(sal_Int32 nPara, sal_uInt16 nWhich = 0, bool bRemoveFeatures = false);
    void RemoveCharAttribs(const EditSelection& rSel, bool bRemoveParaAttribs, sal_uInt16 nWhich = 0);

    ViewsType& GetEditViews();
    const ViewsType& GetEditViews() const;

    void SetUndoMode(bool b);
    void FormatAndUpdate(EditView* pCurView = NULL);

    bool Undo(EditView* pView);
    bool Redo(EditView* pView);
};

#endif // _MyEDITENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
