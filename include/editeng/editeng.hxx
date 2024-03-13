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

#ifndef INCLUDED_EDITENG_EDITENG_HXX
#define INCLUDED_EDITENG_EDITENG_HXX

#include <memory>
#include <span>
#include <vector>

#include <optional>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#include <sot/formats.hxx>
#include <svl/typedwhich.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editengdllapi.h>
#include <i18nlangtag/lang.h>

#include <tools/lineend.hxx>
#include <tools/degree.hxx>
#include <tools/long.hxx>
#include <tools/fontenum.hxx>
#include <basegfx/tuple/b2dtuple.hxx>

#include <editeng/eedata.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <svl/languageoptions.hxx>
#include <comphelper/errcode.hxx>
#include <functional>

template <typename Arg, typename Ret> class Link;

namespace com::sun::star {
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
}

namespace svx {
struct SpellPortion;
typedef std::vector<SpellPortion> SpellPortions;
}

class SfxUndoManager;
namespace basegfx { class B2DPolyPolygon; }
namespace editeng {
    struct MisspellRanges;
}

class ImpEditEngine;
class EditUndoManager;
class EditView;
class OutputDevice;
class SvxFont;
class SfxItemPool;
class SfxStyleSheet;
class SfxStyleSheetPool;
class SvxSearchItem;
class SvxFieldItem;
class MapMode;
class Color;
namespace vcl { class Font; }
class KeyEvent;
class Size;
class Point;
namespace tools { class Rectangle; }
class SvStream;
namespace vcl { class Window; }
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
class SvxNumberFormat;
class SvxFieldData;
class ContentNode;
class ParaPortion;
class EditSelection;
class EditPaM;
class EditLine;
class InternalEditStatus;
class EditSelectionEngine;
class EditDoc;
class Range;
struct EPaM;
class DeletedNodeInfo;
class ParaPortionList;
enum class CharCompressType;
enum class TransliterationFlags;
class LinkParamNone;

/** values for:
       SfxItemSet GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs::All );
*/
enum class EditEngineAttribs {
    All,          /// returns all attributes even when they are not set
    OnlyHard      /// returns only attributes hard set on portions
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

enum class SetAttribsMode {
    NONE, WholeWord, Edge
};

class EDITENG_DLLPUBLIC EditEngine
{
    friend class EditView;
    friend class ImpEditView;
    friend class Outliner;
    friend class TextChainingUtils;


public:
    typedef std::vector<EditView*> ViewsType;

    SAL_DLLPRIVATE EditSelection InsertText(
        css::uno::Reference<css::datatransfer::XTransferable > const & rxDataObj,
        const OUString& rBaseURL, const EditPaM& rPaM, bool bUseSpecial,
        SotClipboardFormatId format = SotClipboardFormatId::NONE);

    SAL_DLLPRIVATE ImpEditEngine& getImpl() const;

private:
    std::unique_ptr<ImpEditEngine> mpImpEditEngine;

                                       EditEngine( const EditEngine& ) = delete;
                       EditEngine&     operator=( const EditEngine& ) = delete;
    SAL_DLLPRIVATE bool            PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pView, vcl::Window const * pFrameWin );

    SAL_DLLPRIVATE void CheckIdleFormatter();
    SAL_DLLPRIVATE bool IsIdleFormatterActive() const;
    SAL_DLLPRIVATE ParaPortion* FindParaPortion(ContentNode const * pNode);
    SAL_DLLPRIVATE const ParaPortion* FindParaPortion(ContentNode const * pNode) const;
    SAL_DLLPRIVATE const ParaPortion* GetPrevVisPortion(const ParaPortion* pCurPortion) const;

    SAL_DLLPRIVATE css::uno::Reference<css::datatransfer::XTransferable>
            CreateTransferable(const EditSelection& rSelection);

    SAL_DLLPRIVATE EditPaM EndOfWord(const EditPaM& rPaM);

    SAL_DLLPRIVATE EditPaM GetPaM(const Point& aDocPos, bool bSmart = true);

    SAL_DLLPRIVATE EditSelection SelectWord(
        const EditSelection& rCurSelection,
        sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES);

    SAL_DLLPRIVATE tools::Long GetXPos(ParaPortion const& rParaPortion, EditLine const& rLine, sal_Int32 nIndex, bool bPreferPortionStart = false) const;

    SAL_DLLPRIVATE Range GetLineXPosStartEnd(ParaPortion const& rParaPortion, EditLine const& rLine) const;

    SAL_DLLPRIVATE InternalEditStatus& GetInternalEditStatus();

    SAL_DLLPRIVATE void HandleBeginPasteOrDrop(PasteOrDropInfos& rInfos);
    SAL_DLLPRIVATE void HandleEndPasteOrDrop(PasteOrDropInfos& rInfos);
    SAL_DLLPRIVATE bool HasText() const;
    SAL_DLLPRIVATE const EditSelectionEngine& GetSelectionEngine() const;
    SAL_DLLPRIVATE void SetInSelectionMode(bool b);
    SAL_DLLPRIVATE void ensureDocumentFormatted() const;

public:
    EditEngine(SfxItemPool* pItemPool);
    virtual ~EditEngine();

    const SfxItemSet& GetEmptyItemSet() const;

    void            SetDefTab( sal_uInt16 nDefTab );

    void            SetRefDevice( OutputDevice* pRefDef );
    OutputDevice*   GetRefDevice() const;

    void            SetRefMapMode( const MapMode& rMapMode );
    MapMode const & GetRefMapMode() const;

    /// Change the update mode per bUpdate and potentially trigger FormatAndUpdate.
    /// bRestoring is used for LOK to update cursor visibility, specifically,
    /// when true, it means we are restoring the update mode after internally
    /// disabling it (f.e. during SetText to set/delete default text in Impress).
    /// @return previous value of update
    bool            SetUpdateLayout(bool bUpdate, bool bRestoring = false);
    bool            IsUpdateLayout() const;

    void            SetBackgroundColor( const Color& rColor );
    Color const &   GetBackgroundColor() const;
    Color           GetAutoColor() const;
    void            EnableAutoColor( bool b );
    SAL_DLLPRIVATE void            ForceAutoColor( bool b );
    SAL_DLLPRIVATE bool            IsForceAutoColor() const;

    void            InsertView(EditView* pEditView, size_t nIndex = EE_APPEND);
    EditView*       RemoveView( EditView* pEditView );
    SAL_DLLPRIVATE void            RemoveView(size_t nIndex);
    EditView*       GetView(size_t nIndex = 0) const;
    size_t          GetViewCount() const;
    bool            HasView( EditView* pView ) const;
    SAL_DLLPRIVATE EditView*       GetActiveView() const;
    SAL_DLLPRIVATE void SetActiveView(EditView* pView);

    void            SetPaperSize( const Size& rSize );
    const Size&     GetPaperSize() const;

    void            SetVertical( bool bVertical );
    bool            IsEffectivelyVertical() const;
    SAL_DLLPRIVATE bool            IsTopToBottom() const;
    SAL_DLLPRIVATE bool            GetVertical() const;
    SAL_DLLPRIVATE void            SetRotation(TextRotation nRotation);
    SAL_DLLPRIVATE TextRotation    GetRotation() const;

    SAL_DLLPRIVATE void SetTextColumns(sal_Int16 nColumns, sal_Int32 nSpacing);

    SAL_DLLPRIVATE void            SetFixedCellHeight( bool bUseFixedCellHeight );

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    SAL_DLLPRIVATE EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    SvtScriptType   GetScriptType( const ESelection& rSelection ) const;
    SAL_DLLPRIVATE editeng::LanguageSpan GetLanguage(const EditPaM& rPaM) const;
    editeng::LanguageSpan GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const;

    void            TransliterateText( const ESelection& rSelection, TransliterationFlags nTransliterationMode );
    SAL_DLLPRIVATE EditSelection   TransliterateText( const EditSelection& rSelection, TransliterationFlags nTransliterationMode );

    void            SetAsianCompressionMode( CharCompressType nCompression );

    void            SetKernAsianPunctuation( bool bEnabled );

    void            SetAddExtLeading( bool b );

    SAL_DLLPRIVATE void            SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    SAL_DLLPRIVATE void            SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon);
    SAL_DLLPRIVATE void            ClearPolygon();

    SAL_DLLPRIVATE const Size&     GetMinAutoPaperSize() const;
    SAL_DLLPRIVATE void            SetMinAutoPaperSize( const Size& rSz );

    SAL_DLLPRIVATE const Size&     GetMaxAutoPaperSize() const;
    SAL_DLLPRIVATE void            SetMaxAutoPaperSize( const Size& rSz );

    SAL_DLLPRIVATE void SetMinColumnWrapHeight(tools::Long nVal);

    OUString        GetText( LineEnd eEnd = LINEEND_LF ) const;
    OUString        GetText( const ESelection& rSelection ) const;
    sal_Int32       GetTextLen() const;
    sal_uInt32      GetTextHeight() const;
    SAL_DLLPRIVATE sal_uInt32      GetTextHeightNTP() const;
    sal_uInt32      CalcTextWidth();

    OUString        GetText( sal_Int32 nParagraph ) const;
    sal_Int32       GetTextLen( sal_Int32 nParagraph ) const;
    sal_uInt32      GetTextHeight( sal_Int32 nParagraph ) const;

    sal_Int32       GetParagraphCount() const;

    sal_Int32       GetLineCount( sal_Int32 nParagraph ) const;
    sal_Int32       GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const;
    void            GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const;
    sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const;
    SAL_DLLPRIVATE sal_uInt32      GetLineHeight( sal_Int32 nParagraph );
    SAL_DLLPRIVATE tools::Rectangle GetParaBounds( sal_Int32 nPara );
    SAL_DLLPRIVATE ParagraphInfos  GetParagraphInfos( sal_Int32 nPara );
    SAL_DLLPRIVATE sal_Int32       FindParagraph( tools::Long nDocPosY );
    EPosition       FindDocPosition( const Point& rDocPos ) const;
    tools::Rectangle       GetCharacterBounds( const EPosition& rPos ) const;

    OUString        GetWord(sal_Int32 nPara, sal_Int32 nIndex);

    ESelection      GetWord( const ESelection& rSelection, sal_uInt16 nWordType ) const;

    void            Clear();
    void            SetText( const OUString& rStr );

    std::unique_ptr<EditTextObject> CreateTextObject();
    SAL_DLLPRIVATE std::unique_ptr<EditTextObject> GetEmptyTextObject();
    std::unique_ptr<EditTextObject> CreateTextObject( sal_Int32 nPara, sal_Int32 nParas = 1 );
    std::unique_ptr<EditTextObject> CreateTextObject( const ESelection& rESelection );
    void            SetText( const EditTextObject& rTextObject );

    SAL_DLLPRIVATE void            RemoveParagraph(sal_Int32 nPara);
    SAL_DLLPRIVATE void            InsertParagraph(sal_Int32 nPara, const EditTextObject& rTxtObj, const bool bAppend = false);
    void            InsertParagraph(sal_Int32 nPara, const OUString& rText);

    void            SetText(sal_Int32 nPara, const OUString& rText);

    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    const SfxItemSet&   GetParaAttribs( sal_Int32 nPara ) const;

    /// Set attributes from rSet an all characters of nPara.
    SAL_DLLPRIVATE void SetCharAttribs(sal_Int32 nPara, const SfxItemSet& rSet);
    void            GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    SfxItemSet      GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags = GetAttribsFlags::ALL ) const;
    SfxItemSet      GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs::All );

    bool            HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    template<class T>
    const T&            GetParaAttrib( sal_Int32 nPara, TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetParaAttrib(nPara, sal_uInt16(nWhich)));
    }

    vcl::Font       GetStandardFont( sal_Int32 nPara );
    SAL_DLLPRIVATE SvxFont         GetStandardSvxFont( sal_Int32 nPara );

    void            RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich );

    SAL_DLLPRIVATE void            ShowParagraph( sal_Int32 nParagraph, bool bShow );

    EditUndoManager& GetUndoManager();
    EditUndoManager* SetUndoManager(EditUndoManager* pNew);
    void            UndoActionStart( sal_uInt16 nId );
    SAL_DLLPRIVATE void            UndoActionStart(sal_uInt16 nId, const ESelection& rSel);
    void            UndoActionEnd();
    SAL_DLLPRIVATE bool            IsInUndo() const;

    void            EnableUndo( bool bEnable );
    bool            IsUndoEnabled() const;

    /** returns the value last used for bTryMerge while calling ImpEditEngine::InsertUndo
        This is currently used in a bad but needed hack to get undo actions merged in the
        OutlineView in impress. Do not use it unless you want to sell your soul too! */
    bool            HasTriedMergeOnLastAddUndo() const;

    void            ClearModifyFlag();
    void            SetModified();
    bool            IsModified() const;

    void            SetModifyHdl( const Link<LinkParamNone*,void>& rLink );

    bool            IsInSelectionMode() const;

    SAL_DLLPRIVATE void            StripPortions();
    void            GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList );

    SAL_DLLPRIVATE tools::Long            GetFirstLineStartX( sal_Int32 nParagraph );
    Point           GetDocPosTopLeft( sal_Int32 nParagraph );
    SAL_DLLPRIVATE Point           GetDocPos( const Point& rPaperPos ) const;
    SAL_DLLPRIVATE bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder );

    // StartDocPos corresponds to VisArea.TopLeft().
    SAL_DLLPRIVATE void            Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect );
    SAL_DLLPRIVATE void            Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos );
    void            Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos, bool bClip );
    void            Draw( OutputDevice& rOutDev, const Point& rStartPos, Degree10 nOrientation = 0_deg10 );

    ErrCode         Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat, SvKeyValueIterator* pHTTPHeaderAttrs = nullptr );
    void            Write( SvStream& rOutput, EETextFormat );

    void            SetStatusEventHdl( const Link<EditStatus&,void>& rLink );
    SAL_DLLPRIVATE Link<EditStatus&,void> const & GetStatusEventHdl() const;

    void            SetNotifyHdl( const Link<EENotify&,void>& rLink );
    Link<EENotify&,void> const & GetNotifyHdl() const;

    void            SetRtfImportHdl( const Link<RtfImportInfo&,void>& rLink );
    const Link<RtfImportInfo&,void>& GetRtfImportHdl() const;

    void            SetHtmlImportHdl( const Link<HtmlImportInfo&,void>& rLink );
    const Link<HtmlImportInfo&,void>& GetHtmlImportHdl() const;

    // Do not evaluate font formatting => For Outliner
    bool            IsFlatMode() const;
    SAL_DLLPRIVATE void            SetFlatMode( bool bFlat );

    void            SetSingleLine( bool bValue );

    void            SetControlWord( EEControlBits nWord );
    EEControlBits   GetControlWord() const;

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    SAL_DLLPRIVATE void            QuickMarkInvalid( const ESelection& rSel );
    void            QuickFormatDoc( bool bFull = false );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );
    void            QuickInsertText(const OUString& rText, const ESelection& rSel);
    void            QuickDelete( const ESelection& rSel );
    SAL_DLLPRIVATE void            QuickMarkToBeRepainted( sal_Int32 nPara );

    SAL_DLLPRIVATE void setGlobalScale(double fFontScaleX, double fFontScaleY, double fSpacingScaleX, double fSpacingScaleY);

    SAL_DLLPRIVATE void getGlobalSpacingScale(double& rX, double& rY) const;
    basegfx::B2DTuple getGlobalSpacingScale() const;
    SAL_DLLPRIVATE void getGlobalFontScale(double& rX, double& rY) const;
    basegfx::B2DTuple getGlobalFontScale() const;

    SAL_DLLPRIVATE void setRoundFontSizeToPt(bool bRound);

    void            SetEditTextObjectPool( SfxItemPool* pPool );
    SfxItemPool*    GetEditTextObjectPool() const;

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    SAL_DLLPRIVATE void SetStyleSheet(const EditSelection& aSel, SfxStyleSheet* pStyle);
    void                 SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle );
    SAL_DLLPRIVATE const SfxStyleSheet* GetStyleSheet( sal_Int32 nPara ) const;
    SfxStyleSheet* GetStyleSheet( sal_Int32 nPara );

    void            SetWordDelimiters( const OUString& rDelimiters );
    const OUString& GetWordDelimiters() const;

    void            EraseVirtualDevice();

    void            SetSpeller( css::uno::Reference<
                            css::linguistic2::XSpellChecker1 > const &xSpeller );
    SAL_DLLPRIVATE css::uno::Reference<
        css::linguistic2::XSpellChecker1 > const &
                    GetSpeller();
    void            SetHyphenator( css::uno::Reference<
                            css::linguistic2::XHyphenator > const & xHyph );

    void GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const;
    void SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges );

    static void     SetForbiddenCharsTable(const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars);

    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    bool            HasOnlineSpellErrors() const;
    void            CompleteOnlineSpelling();

    SAL_DLLPRIVATE bool            ShouldCreateBigTextObject() const;

    // For fast Pre-Test without view:
    EESpellState    HasSpellErrors();
    void ClearSpellErrors();
    SAL_DLLPRIVATE bool            HasText( const SvxSearchItem& rSearchItem );

    //spell and return a sentence
    bool            SpellSentence(EditView const & rEditView, svx::SpellPortions& rToFill );
    // put spell position to start of current sentence
    SAL_DLLPRIVATE void            PutSpellingToSentenceStart( EditView const & rEditView );
    //applies a changed sentence
    void            ApplyChangedSentence(EditView const & rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck );

    // for text conversion (see also HasSpellErrors)
    bool            HasConvertibleTextPortion( LanguageType nLang );
    virtual bool    ConvertNextDocument();

    bool            UpdateFields();
    SAL_DLLPRIVATE bool            UpdateFieldsOnly();
    void            RemoveFields( const std::function<bool ( const SvxFieldData* )>& isFieldData = [] (const SvxFieldData* ){return true;} );

    sal_uInt16      GetFieldCount( sal_Int32 nPara ) const;
    EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const;

    bool            IsRightToLeft( sal_Int32 nPara ) const;

    css::uno::Reference<css::datatransfer::XTransferable> CreateTransferable(const ESelection& rSelection);

    // MT: Can't create new virtual functions like for ParagraphInserted/Deleted, must be compatible in SRC638, change later...
    SAL_DLLPRIVATE void            SetBeginMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink );
    SAL_DLLPRIVATE void            SetEndMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink );
    SAL_DLLPRIVATE void            SetBeginPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink );
    SAL_DLLPRIVATE void            SetEndPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink );

    virtual void    PaintingFirstLine(sal_Int32 nPara, const Point& rStartPos, const Point& rOrigin, Degree10 nOrientation, OutputDevice& rOutDev);
    virtual void    ParagraphInserted( sal_Int32 nNewParagraph );
    virtual void    ParagraphDeleted( sal_Int32 nDeletedParagraph );
    virtual void    ParagraphConnected( sal_Int32 nLeftParagraph, sal_Int32 nRightParagraph );
    virtual void    ParaAttribsChanged( sal_Int32 nParagraph );
    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );
    SAL_DLLPRIVATE void            ParagraphHeightChanged( sal_Int32 nPara );

    virtual void DrawingText( const Point& rStartPos, const OUString& rText,
                              sal_Int32 nTextStart, sal_Int32 nTextLen,
                              std::span<const sal_Int32> pDXArray,
                              std::span<const sal_Bool> pKashidaArray,
                              const SvxFont& rFont,
                              sal_Int32 nPara, sal_uInt8 nRightToLeft,
                              const EEngineData::WrongSpellVector* pWrongSpellVector,
                              const SvxFieldData* pFieldData,
                              bool bEndOfLine,
                              bool bEndOfParagraph,
                              const css::lang::Locale* pLocale,
                              const Color& rOverlineColor,
                              const Color& rTextLineColor);

    virtual void DrawingTab( const Point& rStartPos, tools::Long nWidth, const OUString& rChar,
                             const SvxFont& rFont, sal_Int32 nPara, sal_uInt8 nRightToLeft,
                             bool bEndOfLine,
                             bool bEndOfParagraph,
                             const Color& rOverlineColor,
                             const Color& rTextLineColor);
    virtual OUString  GetUndoComment( sal_uInt16 nUndoId ) const;
    virtual bool    SpellNextDocument();
    /** @return true, when click was consumed. false otherwise. */
    virtual bool    FieldClicked( const SvxFieldItem& rField );
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rTxtColor, std::optional<Color>& rFldColor, std::optional<FontLineStyle>& rFldLineStyle );

    // override this if access to bullet information needs to be provided
    virtual const SvxNumberFormat * GetNumberFormat( sal_Int32 nPara ) const;

    virtual tools::Rectangle GetBulletArea( sal_Int32 nPara );

    static rtl::Reference<SfxItemPool> CreatePool();
    static SfxItemPool& GetGlobalItemPool();
    static bool     DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static bool     DoesKeyMoveCursor( const KeyEvent& rKeyEvent );
    static bool     IsSimpleCharInput( const KeyEvent& rKeyEvent );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const vcl::Font& rFont );
    SAL_DLLPRIVATE static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const SvxFont& rFont );
    SAL_DLLPRIVATE static vcl::Font CreateFontFromItemSet( const SfxItemSet& rItemSet, SvtScriptType nScriptType );
    SAL_DLLPRIVATE static SvxFont  CreateSvxFontFromItemSet( const SfxItemSet& rItemSet );
    SAL_DLLPRIVATE static bool     IsPrintable( sal_Unicode c ) { return ( ( c >= 32 ) && ( c != 127 ) ); }
    static bool     HasValidData( const css::uno::Reference< css::datatransfer::XTransferable >& rTransferable );
    /** sets a link that is called at the beginning of a drag operation at an edit view */
    SAL_DLLPRIVATE void            SetBeginDropHdl( const Link<EditView*,void>& rLink );
    SAL_DLLPRIVATE Link<EditView*,void> const & GetBeginDropHdl() const;

    /** sets a link that is called at the end of a drag operation at an edit view */
    SAL_DLLPRIVATE void            SetEndDropHdl( const Link<EditView*,void>& rLink );
    SAL_DLLPRIVATE Link<EditView*,void> const & GetEndDropHdl() const;

    /// specifies if auto-correction should capitalize the first word or not (default is on)
    void            SetFirstWordCapitalization( bool bCapitalize );

    /** specifies if auto-correction should replace a leading single quotation
        mark (apostrophe) or not (default is on) */
    void            SetReplaceLeadingSingleQuotationMark( bool bReplace );

    SAL_DLLPRIVATE EditDoc& GetEditDoc();
    SAL_DLLPRIVATE const EditDoc& GetEditDoc() const;
    void dumpAsXmlEditDoc(xmlTextWriterPtr pWriter) const;

    SAL_DLLPRIVATE ParaPortionList& GetParaPortions();
    SAL_DLLPRIVATE const ParaPortionList& GetParaPortions() const;

    SAL_DLLPRIVATE bool IsFormatted() const;
    SAL_DLLPRIVATE bool IsHtmlImportHandlerSet() const;
    SAL_DLLPRIVATE bool IsRtfImportHandlerSet() const;
    SAL_DLLPRIVATE bool IsImportRTFStyleSheetsSet() const;

    SAL_DLLPRIVATE void CallRtfImportHandler(RtfImportInfo& rInfo);
    SAL_DLLPRIVATE void CallHtmlImportHandler(HtmlImportInfo& rInfo);

    SAL_DLLPRIVATE void ParaAttribsToCharAttribs(ContentNode* pNode);

    SAL_DLLPRIVATE EditPaM CreateEditPaM(const EPaM& rEPaM);
    SAL_DLLPRIVATE EditPaM ConnectParagraphs(
        ContentNode* pLeft, ContentNode* pRight, bool bBackward);

    SAL_DLLPRIVATE EditPaM InsertField(const EditSelection& rEditSelection, const SvxFieldItem& rFld);
    SAL_DLLPRIVATE EditPaM InsertText(const EditSelection& aCurEditSelection, const OUString& rStr);
    SAL_DLLPRIVATE EditSelection InsertText(const EditTextObject& rTextObject, const EditSelection& rSel);
    SAL_DLLPRIVATE EditPaM InsertParaBreak(const EditSelection& rEditSelection);
    SAL_DLLPRIVATE EditPaM InsertLineBreak(const EditSelection& rEditSelection);

    SAL_DLLPRIVATE EditPaM CursorLeft(
        const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = css::i18n::CharacterIteratorMode::SKIPCELL);
    SAL_DLLPRIVATE EditPaM CursorRight(
        const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = css::i18n::CharacterIteratorMode::SKIPCELL);

    SAL_DLLPRIVATE void SeekCursor(ContentNode* pNode, sal_Int32 nPos, SvxFont& rFont);

    SAL_DLLPRIVATE EditPaM DeleteSelection(const EditSelection& rSel);

    SAL_DLLPRIVATE ESelection CreateESelection(const EditSelection& rSel) const;
    SAL_DLLPRIVATE EditSelection CreateSelection(const ESelection& rSel);

    SAL_DLLPRIVATE const SfxItemSet& GetBaseParaAttribs(sal_Int32 nPara) const;
    SAL_DLLPRIVATE void SetParaAttribsOnly(sal_Int32 nPara, const SfxItemSet& rSet);
    SAL_DLLPRIVATE void SetAttribs(const EditSelection& rSel, const SfxItemSet& rSet, SetAttribsMode nSpecial = SetAttribsMode::NONE);

    SAL_DLLPRIVATE OUString GetSelected(const EditSelection& rSel) const;
    SAL_DLLPRIVATE EditPaM DeleteSelected(const EditSelection& rSel);

    SAL_DLLPRIVATE SvtScriptType GetScriptType(const EditSelection& rSel) const;

    SAL_DLLPRIVATE void RemoveParaPortion(sal_Int32 nNode);

    SAL_DLLPRIVATE void SetCallParaInsertedOrDeleted(bool b);
    SAL_DLLPRIVATE bool IsCallParaInsertedOrDeleted() const;

    SAL_DLLPRIVATE void AppendDeletedNodeInfo(DeletedNodeInfo* pInfo);
    SAL_DLLPRIVATE void UpdateSelections();

    SAL_DLLPRIVATE void InsertContent(std::unique_ptr<ContentNode> pNode, sal_Int32 nPos);
    SAL_DLLPRIVATE EditPaM SplitContent(sal_Int32 nNode, sal_Int32 nSepPos);
    SAL_DLLPRIVATE EditPaM ConnectContents(sal_Int32 nLeftNode, bool bBackward);

    SAL_DLLPRIVATE void InsertFeature(const EditSelection& rEditSelection, const SfxPoolItem& rItem);

    SAL_DLLPRIVATE EditSelection MoveParagraphs(const Range& rParagraphs, sal_Int32 nNewPos);

    void RemoveCharAttribs(sal_Int32 nPara, sal_uInt16 nWhich = 0, bool bRemoveFeatures = false);
    SAL_DLLPRIVATE void RemoveCharAttribs(const EditSelection& rSel, bool bRemoveParaAttribs, sal_uInt16 nWhich);
    SAL_DLLPRIVATE void RemoveCharAttribs(const EditSelection& rSel, EERemoveParaAttribsMode eMode, sal_uInt16 nWhich);

    SAL_DLLPRIVATE ViewsType& GetEditViews();
    SAL_DLLPRIVATE const ViewsType& GetEditViews() const;

    SAL_DLLPRIVATE void SetUndoMode(bool b);
    void FormatAndLayout(EditView* pCurView, bool bCalledFromUndo = false);

    SAL_DLLPRIVATE void Undo(EditView* pView);
    SAL_DLLPRIVATE void Redo(EditView* pView);

    SAL_DLLPRIVATE sal_Int32 GetOverflowingParaNum() const;
    SAL_DLLPRIVATE sal_Int32 GetOverflowingLineNum() const;
    SAL_DLLPRIVATE void ClearOverflowingParaNum();
    SAL_DLLPRIVATE bool IsPageOverflow();

    // tdf#132288  By default inserting an attribute beside another that is of
    // the same type expands the original instead of inserting another. But the
    // spell check dialog doesn't want that behaviour
    void DisableAttributeExpanding();

    // Optimization, if set, formatting will be done only for text lines that fit
    // in given paper size and exceeding lines will be ignored.
    void EnableSkipOutsideFormat(bool set);

    void SetLOKSpecialPaperSize(const Size& rSize);
    const Size& GetLOKSpecialPaperSize() const;

#ifdef DBG_UTIL
    SAL_DLLPRIVATE static void DumpData(const EditEngine* pEE, bool bInfoBox);
#endif
};

#endif // INCLUDED_EDITENG_EDITENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
