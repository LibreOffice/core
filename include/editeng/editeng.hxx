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
#ifndef INCLUDED_EDITENG_EDITENG_HXX
#define INCLUDED_EDITENG_EDITENG_HXX

#include <rtl/ref.hxx>
#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#include <rsc/rscsfx.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editengdllapi.h>
#include <i18nlangtag/lang.h>

#include <tools/lineend.hxx>
#include <tools/link.hxx>

#include <editeng/eedata.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <svl/languageoptions.hxx>
#include <functional>

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
class SfxStyleSheetPool;
class SvxSearchItem;
class SvxFieldItem;
class SvxCharSetColorItem;
class SfxUndoAction;
class MapMode;
class Color;
namespace vcl { class Font; }
class KeyEvent;
namespace tools { class PolyPolygon; }
class Size;
class Point;
class Rectangle;
class SvStream;
class OutputDevice;
namespace vcl { class Window; }
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


/** values for:
       SfxItemSet GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs_All );
*/
enum EditEngineAttribs {
    EditEngineAttribs_All,          /// returns all attributes even when theire not set
    EditEngineAttribs_HardAndPara,  /// returns all attributes set on paragraph and on portions
    EditEngineAttribs_OnlyHard      /// returns only attributes hard set on portions
};

/** values for:
       SfxItemSet  GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, sal_uInt8 nFlags = 0xFF ) const;
*/
enum class GetAttribsFlags
{
    NONE         = 0x00,
    STYLESHEET   = 0x01,
    PARAATTRIBS  = 0x02,
    CHARATTRIBS  = 0x04,
    ALL          = 0x07,
};
namespace o3tl
{
    template<> struct typed_flags<GetAttribsFlags> : is_typed_flags<GetAttribsFlags, 0x07> {};
}
template<class T> bool checkSvxFieldData(const SvxFieldData* pData)
{
    return dynamic_cast<const T*>(pData) != nullptr;
}

class SdrObject;
class EDITENG_DLLPUBLIC EditEngine
{
    friend class EditView;
    friend class ImpEditView;
    friend class EditDbg;
    friend class Outliner;
    friend class TextChainingUtils;


public:
    typedef std::vector<EditView*> ViewsType;

private:
    ImpEditEngine*  pImpEditEngine;

                                       EditEngine( const EditEngine& ) = delete;
                       EditEngine&     operator=( const EditEngine& ) = delete;
    EDITENG_DLLPRIVATE bool            PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pView, vcl::Window* pFrameWin = nullptr );

    EDITENG_DLLPRIVATE void CursorMoved(ContentNode* pPrevNode);
    EDITENG_DLLPRIVATE void CheckIdleFormatter();
    EDITENG_DLLPRIVATE bool IsIdleFormatterActive() const;
    EDITENG_DLLPRIVATE ParaPortion* FindParaPortion(ContentNode* pNode);
    EDITENG_DLLPRIVATE const ParaPortion* FindParaPortion(ContentNode* pNode) const;
    EDITENG_DLLPRIVATE const ParaPortion* GetPrevVisPortion(const ParaPortion* pCurPortion) const;

    EDITENG_DLLPRIVATE css::uno::Reference<
        css::datatransfer::XTransferable>
            CreateTransferable(const EditSelection& rSelection);

    EDITENG_DLLPRIVATE EditSelection InsertText(
        css::uno::Reference<css::datatransfer::XTransferable >& rxDataObj,
        const OUString& rBaseURL, const EditPaM& rPaM, bool bUseSpecial);

    EDITENG_DLLPRIVATE EditPaM EndOfWord(const EditPaM& rPaM);

    EDITENG_DLLPRIVATE EditPaM GetPaM(const Point& aDocPos, bool bSmart = true);

    EDITENG_DLLPRIVATE EditSelection SelectWord(
        const EditSelection& rCurSelection,
        sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES,
        bool bAcceptStartOfWord = true);

    EDITENG_DLLPRIVATE long GetXPos(
        const ParaPortion* pParaPortion, const EditLine* pLine, sal_Int32 nIndex, bool bPreferPortionStart = false) const;

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

    void            SetUpdateMode( bool bUpdate );
    bool            GetUpdateMode() const;

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;
    Color           GetAutoColor() const;
    void            EnableAutoColor( bool b );
    void            ForceAutoColor( bool b );
    bool            IsForceAutoColor() const;

    void            InsertView(EditView* pEditView, size_t nIndex = EE_APPEND);
    EditView*       RemoveView( EditView* pEditView );
    void            RemoveView(size_t nIndex = EE_APPEND);
    EditView*       GetView(size_t nIndex = 0) const;
    size_t          GetViewCount() const;
    bool            HasView( EditView* pView ) const;
    EditView*       GetActiveView() const;
    void SetActiveView(EditView* pView);

    void            SetPaperSize( const Size& rSize );
    const Size&     GetPaperSize() const;

    void            SetVertical( bool bVertical );
    bool            IsVertical() const;

    void            SetFixedCellHeight( bool bUseFixedCellHeight );

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    SvtScriptType   GetScriptType( const ESelection& rSelection ) const;
    LanguageType    GetLanguage(const EditPaM& rPaM) const;
    LanguageType    GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const;

    void            TransliterateText( const ESelection& rSelection, sal_Int32 nTransliterationMode );
    EditSelection   TransliterateText( const EditSelection& rSelection, sal_Int32 nTransliterationMode );

    void            SetAsianCompressionMode( sal_uInt16 nCompression );

    void            SetKernAsianPunctuation( bool bEnabled );

    void            SetAddExtLeading( bool b );

    void            SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    void            SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon);
    void            ClearPolygon();

    const Size&     GetMinAutoPaperSize() const;
    void            SetMinAutoPaperSize( const Size& rSz );

    const Size&     GetMaxAutoPaperSize() const;
    void            SetMaxAutoPaperSize( const Size& rSz );

    OUString        GetText( LineEnd eEnd = LINEEND_LF ) const;
    OUString        GetText( const ESelection& rSelection ) const;
    sal_uInt32      GetTextLen() const;
    sal_uInt32      GetTextHeight() const;
    sal_uInt32      GetTextHeightNTP() const;
    sal_uInt32      CalcTextWidth();

    OUString        GetText( sal_Int32 nParagraph ) const;
    sal_Int32       GetTextLen( sal_Int32 nParagraph ) const;
    sal_uInt32      GetTextHeight( sal_Int32 nParagraph ) const;

    sal_Int32       GetParagraphCount() const;

    sal_Int32       GetLineCount( sal_Int32 nParagraph ) const;
    sal_Int32       GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const;
    void            GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const;
    sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const;
    sal_uInt32      GetLineHeight( sal_Int32 nParagraph, sal_Int32 nLine = 0 );
    ParagraphInfos  GetParagraphInfos( sal_Int32 nPara );
    sal_Int32       FindParagraph( long nDocPosY );
    EPosition       FindDocPosition( const Point& rDocPos ) const;
    Rectangle       GetCharacterBounds( const EPosition& rPos ) const;

    OUString        GetWord(sal_Int32 nPara, sal_Int32 nIndex);

    ESelection      GetWord( const ESelection& rSelection, sal_uInt16 nWordType ) const;

    void            Clear();
    void            SetText( const OUString& rStr );

    EditTextObject* CreateTextObject();
    EditTextObject* GetEmptyTextObject() const;
    EditTextObject* CreateTextObject( sal_Int32 nPara, sal_Int32 nParas = 1 );
    EditTextObject* CreateTextObject( const ESelection& rESelection );
    void            SetText( const EditTextObject& rTextObject );

    void            RemoveParagraph(sal_Int32 nPara);
    void            InsertParagraph(sal_Int32 nPara, const EditTextObject& rTxtObj);
    void            InsertParagraph(sal_Int32 nPara, const OUString& rText);

    void            SetText(sal_Int32 nPara, const OUString& rText);

    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    const SfxItemSet&   GetParaAttribs( sal_Int32 nPara ) const;

    void            GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    SfxItemSet      GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags = GetAttribsFlags::ALL ) const;
    SfxItemSet      GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs_All );

    bool            HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich );

    vcl::Font       GetStandardFont( sal_Int32 nPara );
    SvxFont         GetStandardSvxFont( sal_Int32 nPara );

    void            RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich );

    void            ShowParagraph( sal_Int32 nParagraph, bool bShow = true );

    ::svl::IUndoManager& GetUndoManager();
    ::svl::IUndoManager* SetUndoManager(::svl::IUndoManager* pNew);
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionStart(sal_uInt16 nId, const ESelection& rSel);
    void            UndoActionEnd( sal_uInt16 nId );
    bool            IsInUndo();

    void            EnableUndo( bool bEnable );
    bool            IsUndoEnabled();

    /** returns the value last used for bTryMerge while calling ImpEditEngine::InsertUndo
        This is currently used in a bad but needed hack to get undo actions merged in the
        OutlineView in impress. Do not use it unless you want to sell your soul too! */
    bool            HasTriedMergeOnLastAddUndo() const;

    void            ClearModifyFlag();
    void            SetModified();
    bool            IsModified() const;

    void            SetModifyHdl( const Link<LinkParamNone*,void>& rLink );
    Link<LinkParamNone*,void> GetModifyHdl() const;

    bool            IsInSelectionMode() const;

    void            StripPortions();
    void            GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList );

    long            GetFirstLineStartX( sal_Int32 nParagraph );
    Point           GetDocPosTopLeft( sal_Int32 nParagraph );
    Point           GetDocPos( const Point& rPaperPos ) const;
    bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder = 0 );

    // StartDocPos corresponds to VisArea.TopLeft().
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos, bool bClip );
    void            Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation = 0 );

//  sal_uInt32: Error code of the stream.
    sal_uLong       Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat, SvKeyValueIterator* pHTTPHeaderAttrs = nullptr );
    void            Write( SvStream& rOutput, EETextFormat );

    void            SetStatusEventHdl( const Link<EditStatus&,void>& rLink );
    Link<EditStatus&,void> GetStatusEventHdl() const;

    void            SetNotifyHdl( const Link<EENotify&,void>& rLink );
    Link<EENotify&,void>  GetNotifyHdl() const;

    void            SetImportHdl( const Link<ImportInfo&,void>& rLink );
    Link<ImportInfo&,void> GetImportHdl() const;

    // Do not evaluate font formatting => For Outliner
    bool            IsFlatMode() const;
    void            SetFlatMode( bool bFlat );

    void            SetControlWord( EEControlBits nWord );
    EEControlBits   GetControlWord() const;

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickMarkInvalid( const ESelection& rSel );
    void            QuickFormatDoc( bool bFull = false );
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

    void            SetSpeller( css::uno::Reference<
                            css::linguistic2::XSpellChecker1 > &xSpeller );
    css::uno::Reference<
        css::linguistic2::XSpellChecker1 >
                    GetSpeller();
    void            SetHyphenator( css::uno::Reference<
                            css::linguistic2::XHyphenator >& xHyph );

    void GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const;
    void SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges );

    static void     SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );

    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    bool            HasOnlineSpellErrors() const;
    void            CompleteOnlineSpelling();

    bool            ShouldCreateBigTextObject() const;

    // For fast Pre-Test without view:
    EESpellState    HasSpellErrors();
    void ClearSpellErrors();
    bool            HasText( const SvxSearchItem& rSearchItem );

    //spell and return a sentence
    bool            SpellSentence(EditView& rEditView, svx::SpellPortions& rToFill, bool bIsGrammarChecking );
    // put spell position to start of current sentence
    void            PutSpellingToSentenceStart( EditView& rEditView );
    //applies a changed sentence
    void            ApplyChangedSentence(EditView& rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck );

    // for text conversion (see also HasSpellErrors)
    bool            HasConvertibleTextPortion( LanguageType nLang );
    virtual bool    ConvertNextDocument();

    bool UpdateFields();
    bool UpdateFieldsOnly();
    void            RemoveFields( bool bKeepFieldText, std::function<bool ( const SvxFieldData* )> isFieldData = [] (const SvxFieldData* ){return true;} );

    sal_uInt16      GetFieldCount( sal_Int32 nPara ) const;
    EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const;

    bool            IsRightToLeft( sal_Int32 nPara ) const;

    css::uno::Reference< css::datatransfer::XTransferable >
                    CreateTransferable( const ESelection& rSelection ) const;

    // MT: Can't create new virtual functions like for ParagraphInserted/Deleted, musst be compatible in SRC638, change later...
    void            SetBeginMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink );
    void            SetEndMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink );
    void            SetBeginPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink );
    void            SetEndPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink );

    virtual void    PaintingFirstLine( sal_Int32 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );
    virtual void    ParagraphInserted( sal_Int32 nNewParagraph );
    virtual void    ParagraphDeleted( sal_Int32 nDeletedParagraph );
    virtual void    ParagraphConnected( sal_Int32 nLeftParagraph, sal_Int32 nRightParagraph );
    virtual void    ParaAttribsChanged( sal_Int32 nParagraph );
    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );
    void            ParagraphHeightChanged( sal_Int32 nPara );

    virtual void DrawingText( const Point& rStartPos, const OUString& rText,
                              sal_Int32 nTextStart, sal_Int32 nTextLen,
                              const long* pDXArray, const SvxFont& rFont,
                              sal_Int32 nPara, sal_uInt8 nRightToLeft,
                              const EEngineData::WrongSpellVector* pWrongSpellVector,
                              const SvxFieldData* pFieldData,
                              bool bEndOfLine,
                              bool bEndOfParagraph,
                              bool bEndOfBullet,
                              const css::lang::Locale* pLocale,
                              const Color& rOverlineColor,
                              const Color& rTextLineColor);

    virtual void DrawingTab( const Point& rStartPos, long nWidth, const OUString& rChar,
                             const SvxFont& rFont, sal_Int32 nPara, sal_uInt8 nRightToLeft,
                             bool bEndOfLine,
                             bool bEndOfParagraph,
                             const Color& rOverlineColor,
                             const Color& rTextLineColor);
    virtual OUString  GetUndoComment( sal_uInt16 nUndoId ) const;
    virtual bool    SpellNextDocument();
    virtual void    FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos );
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rTxtColor, Color*& rFldColor );

    // override this if access to bullet information needs to be provided
    virtual const SvxNumberFormat * GetNumberFormat( sal_Int32 nPara ) const;

    virtual Rectangle GetBulletArea( sal_Int32 nPara );

    static SfxItemPool* CreatePool( bool bLoadRefCounts = true );
    static SfxItemPool& GetGlobalItemPool();
    static bool     DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static bool     DoesKeyMoveCursor( const KeyEvent& rKeyEvent );
    static bool     IsSimpleCharInput( const KeyEvent& rKeyEvent );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const vcl::Font& rFont );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const SvxFont& rFont );
    static vcl::Font CreateFontFromItemSet( const SfxItemSet& rItemSet, SvtScriptType nScriptType );
    static SvxFont  CreateSvxFontFromItemSet( const SfxItemSet& rItemSet );
    static bool     IsPrintable( sal_Unicode c ) { return ( ( c >= 32 ) && ( c != 127 ) ); }
    static bool     HasValidData( const css::uno::Reference< css::datatransfer::XTransferable >& rTransferable );
    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link<EditView*,void>& rLink );
    Link<EditView*,void> GetBeginDropHdl() const;

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link<EditView*,void>& rLink );
    Link<EditView*,void> GetEndDropHdl() const;

    /// specifies if auto-correction should capitalize the first word or not (default is on)
    void            SetFirstWordCapitalization( bool bCapitalize );

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
    EditPaM InsertText(const EditSelection& aCurEditSelection, const OUString& rStr);
    EditSelection InsertText(const EditTextObject& rTextObject, const EditSelection& rSel);
    EditPaM InsertParaBreak(const EditSelection& rEditSelection);
    EditPaM InsertLineBreak(const EditSelection& rEditSelection);

    EditPaM CursorLeft(
        const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = css::i18n::CharacterIteratorMode::SKIPCELL);
    EditPaM CursorRight(
        const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = css::i18n::CharacterIteratorMode::SKIPCELL);

    void SeekCursor(
        ContentNode* pNode, sal_Int32 nPos, SvxFont& rFont, OutputDevice* pOut = nullptr);

    EditPaM DeleteSelection(const EditSelection& rSel);

    ESelection CreateESelection(const EditSelection& rSel);
    EditSelection CreateSelection(const ESelection& rSel);

    const SfxItemSet& GetBaseParaAttribs(sal_Int32 nPara) const;
    void SetParaAttribsOnly(sal_Int32 nPara, const SfxItemSet& rSet);
    void SetAttribs(const EditSelection& rSel, const SfxItemSet& rSet, sal_uInt8 nSpecial = 0);

    OUString GetSelected(const EditSelection& rSel) const;
    EditPaM DeleteSelected(const EditSelection& rSel);

    SvtScriptType GetScriptType(const EditSelection& rSel) const;

    void RemoveParaPortion(sal_Int32 nNode);

    void SetCallParaInsertedOrDeleted(bool b);
    bool IsCallParaInsertedOrDeleted() const;

    void AppendDeletedNodeInfo(DeletedNodeInfo* pInfo);
    void UpdateSelections();

    void InsertContent(ContentNode* pNode, sal_Int32 nPos);
    EditPaM SplitContent(sal_Int32 nNode, sal_Int32 nSepPos);
    EditPaM ConnectContents(sal_Int32 nLeftNode, bool bBackward);

    void InsertFeature(const EditSelection& rEditSelection, const SfxPoolItem& rItem);

    EditSelection MoveParagraphs(const Range& rParagraphs, sal_Int32 nNewPos, EditView* pCurView);

    void RemoveCharAttribs(sal_Int32 nPara, sal_uInt16 nWhich = 0, bool bRemoveFeatures = false);
    void RemoveCharAttribs(const EditSelection& rSel, bool bRemoveParaAttribs, sal_uInt16 nWhich = 0);

    ViewsType& GetEditViews();
    const ViewsType& GetEditViews() const;

    void SetUndoMode(bool b);
    void FormatAndUpdate(EditView* pCurView = nullptr);

    void Undo(EditView* pView);
    void Redo(EditView* pView);

    sal_Int32 GetOverflowingParaNum() const;
    sal_Int32 GetOverflowingLineNum() const;
    void ClearOverflowingParaNum();
    bool IsPageOverflow();
};

#endif // INCLUDED_EDITENG_EDITENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
