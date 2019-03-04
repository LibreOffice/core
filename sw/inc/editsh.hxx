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
#ifndef INCLUDED_SW_INC_EDITSH_HXX
#define INCLUDED_SW_INC_EDITSH_HXX

#include "crsrsh.hxx"

#include "charfmt.hxx"
#include "fldupde.hxx"
#include "frmfmt.hxx"
#include "swdllapi.h"
#include "swundo.hxx"
#include "tblenum.hxx"
#include "tox.hxx"

#include <memory>
#include <vector>
#include <set>

namespace tools { class PolyPolygon; }
class SwDoc;
class CommandExtTextInputData;

class SvNumberFormatter;
class SfxPoolItem;
class SfxItemSet;
class SfxWatermarkItem;
class SvxAutoCorrect;

class SwDDEFieldType;
class SwDBManager;

struct SwDocStat;
class SwAutoCompleteWord;

class SwFormatRefMark;

class SwNumRule;

class SwTextFormatColl;
class SwGrfNode;

class Graphic;
class GraphicObject;
class GraphicAttr;
class SwFormatINetFormat;
class SwTable;
class SwTextBlocks;
class SwFormatFootnote;
class SwSection;
class SwSectionData;
class SwSectionFormat;
class SwTableAutoFormat;
class SwPageDesc;
class SwTextINetFormat;
class SwSeqFieldList;
class SwGlblDocContent;
class SwGlblDocContents;
class SwRangeRedline;
class SwRedlineData;
class SwFootnoteInfo;
class SwEndNoteInfo;
class SwLineNumberInfo;
class SwAuthEntry;
class SwRewriter;
class SwView;
struct SwConversionArgs;
struct SvxSwAutoFormatFlags;
struct SwInsertTableOptions;
struct SwDBData;
enum class SvtScriptType;
enum class SfxClassificationPolicyType;
enum class RedlineFlags;
enum class TransliterationFlags;
enum class GraphicType;

namespace com { namespace sun { namespace star { namespace uno {
    template < class > class Sequence;
}}}}
namespace com { namespace sun { namespace star { namespace linguistic2 { class XSpellAlternatives; } } } }
namespace com { namespace sun { namespace star { namespace linguistic2 { struct ProofreadingResult; } } } }

namespace svx{
    struct SpellPortion;
    typedef std::vector<SpellPortion> SpellPortions;
    class ClassificationResult;
}

namespace sfx2{
    class LinkManager;
}

namespace sw {
    class UndoRedoContext;
}

enum class ParaBreakType {
    ToBlank = 0,
    ToOnlyCR = 2
};

 /// For querying the INet-attributes for Navigator.
struct SwGetINetAttr
{
    OUString const sText;
    const SwTextINetFormat& rINetAttr;

    SwGetINetAttr( const OUString& rText, const SwTextINetFormat& rAttr )
        : sText( rText ), rINetAttr( rAttr )
    {}
};
typedef std::vector<SwGetINetAttr> SwGetINetAttrs;

// Types of forms of content.
#define CNT_TXT 0x0001
#define CNT_GRF 0x0002
#define CNT_OLE 0x0010

// Test USHORT for a defined form of content.
#define CNT_HasText(USH) ((USH)&CNT_TXT)
#define CNT_HasGrf(USH) ((USH)&CNT_GRF)
#define CNT_HasOLE(USH) ((USH)&CNT_OLE)

class SW_DLLPUBLIC SwEditShell : public SwCursorShell
{
    static SvxSwAutoFormatFlags* s_pAutoFormatFlags;

    /// For the private methods DelRange and those of AutoCorrect.
    friend class SwAutoFormat;
    friend void InitCore();
    friend void FinitCore();
    /// For the PamCorrAbs/-Rel methods.
    friend class SwUndo;

    /** Returns pointer to a SwGrfNode
     that will be used by GetGraphic() and GetGraphicSize(). */
    SAL_DLLPRIVATE SwGrfNode *GetGrfNode_() const ;

    SAL_DLLPRIVATE void DeleteSel( SwPaM& rPam, bool* pUndo = nullptr );

    SAL_DLLPRIVATE void SetSectionAttr_( SwSectionFormat& rSectFormat, const SfxItemSet& rSet );

    using SwViewShell::UpdateFields;
    using SwModify::GetInfo;

public:
    /// Edit (all selected ranges).
    void Insert( sal_Unicode, bool bOnlyCurrCursor = false );
    void Insert2( const OUString &, const bool bForceExpandHints = false );
    void Overwrite( const OUString & );

    /** Replace a selected range in a TextNode by given string.
     Meant for Search & Replace.
     bRegExpRplc - replace tabs (\\t) and insert found string (not \&).
     E.g.: Fnd: "zzz", Repl: "xx\t\\t..&..\&"
           --> "xx\t<Tab>..zzz..&" */
    bool Replace( const OUString& rNewStr, bool bRegExpRplc );

    /** Delete content of all ranges.
     If whole nodes are selected, these nodes get deleted. */
    bool Delete();

    /// Remove a complete paragraph.
    bool DelFullPara();

    /// Change text to Upper/Lower/Hiragana/Katakana/...
    void TransliterateText( TransliterationFlags nType );

    /// Count words in current selection.
    void CountWords( SwDocStat& rStat ) const;

    /// Replace fields by text - mailmerge support.
    bool ConvertFieldsToText();

    /// Set all numbering start points to a fixed value - mailmerge support.
    void SetNumberingRestart();

    /// Embeds all local links (ranges/graphics).
    sal_uInt16 GetLinkUpdMode() const;
    void SetLinkUpdMode( sal_uInt16 nMode );

    /// Copy content of all ranges at current position of cursor to given Shell.
    bool Copy( SwEditShell* pDestShell );

    /** For copying via ClipBoard:
       If table is copied into table, move all cursors away from it.
       Copy and Paste must be in FEShell because of FlyFrames!
       Copy all selections to the document. */
    bool CopySelToDoc( SwDoc* pInsDoc );

    void SplitNode( bool bAutoFormat = false, bool bCheckTableStart = true );
    bool AppendTextNode();
    void AutoFormatBySplitNode();

    /** If cursor is in a INetAttribute it will be deleted completely
     including the descriptive text (needed at drag & drop). */
    void DelINetAttrWithText();

    /** If Cursor is at the end of a character style in which the DontExpand-flag
     is not yet set, the latter will be set (==> return TRUE). */
    bool DontExpandFormat();

    /** Apply / remove attributes.
     @return attributes in required AttributeSet.
     When not unambiguous the set has a DONT_CARE !!
     2nd optional parameter <bMergeIndentValuesOfNumRule>.
     If <bMergeIndentValuesOfNumRule> == true, the indent attributes of
     the corresponding list level of an applied list style is merged into
     the requested item set as a LR-SPACE item, if corresponding node has not
     its own indent attributes and the position-and-space mode of the list
     level is SvxNumberFormat::LABEL_ALIGNMENT. */
    bool GetPaMAttr( SwPaM* pPaM, SfxItemSet& ,
                     const bool bMergeIndentValuesOfNumRule = false ) const;
    bool GetCurAttr( SfxItemSet& ,
                     const bool bMergeIndentValuesOfNumRule = false ) const;
    void SetAttrItem( const SfxPoolItem&, SetAttrMode nFlags = SetAttrMode::DEFAULT,
                     const bool bParagraphSetting = false );
    void SetAttrSet( const SfxItemSet&, SetAttrMode nFlags = SetAttrMode::DEFAULT,
                     SwPaM* pCursor = nullptr, const bool bParagraphSetting = false );

    /** Get RES_CHRATR_* items of one type in the current selection.
     * @param nWhich WhichId of the collected items.
     * If parts of the selection have different scripttypes, the items with corresponding WhichIds are also collected.
     * @return a vector of pairs. The pair contains a SfxPoolItem and a SwPaM, in which the item is valid and can be changed. */
    std::vector<std::pair< const SfxPoolItem*, std::unique_ptr<SwPaM>>> GetItemWithPaM( sal_uInt16 nWhich );

    /**
     * Get the paragraph format attribute(s) of the current selection.
     *
     * @see GetPaMParAttr()
     *
     * @param rSet
     * output parameter - the SfxItemSet where the automatic paragraph format attribute(s) will be store.
     * The attributes aren't invalidated or cleared if the function reach the getMaxLookup limit.
     */
    void GetCurParAttr( SfxItemSet& rSet ) const;
    /**
     * Get the paragraph format attribute(s) of the selection(s) described by a SwPaM.
     *
     * @param pPaM
     * input parameter - the selection where to look for the paragraph format.
     *
     * @param rSet
     * output parameter - the SfxItemSet where the automatic paragraph format attribute(s) will be store.
     * The attributes aren't invalidated or cleared if the function reaches the getMaxLookup limit.
     *
     * @return true if the function inspects all the nodes point by the pPaM parameter,
     * false if the function reaches the limit of getMaxLookup number of nodes inspected.
     */
    bool GetPaMParAttr( SwPaM* pPaM, SfxItemSet& rSet ) const;

    /// Set attribute as new default attribute in document.
    void SetDefault( const SfxPoolItem& );

    /// Query default attribute of document.
    const SfxPoolItem& GetDefault( sal_uInt16 nFormatHint ) const;
    template<class T> const T&  GetDefault( TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetDefault(sal_uInt16(nWhich)));
    }

    void ResetAttr( const std::set<sal_uInt16> &attrs = std::set<sal_uInt16>(), SwPaM* pCursor = nullptr );
    void GCAttr();

    /// @return the scripttype of the selection.
    SvtScriptType GetScriptType() const;

    /// @return the language at current cursor position.
    LanguageType GetCurLang() const;

    /// TABLE
    size_t GetTableFrameFormatCount( bool bUsed = false ) const;
    SwFrameFormat& GetTableFrameFormat(size_t nFormat, bool bUsed = false ) const;
    OUString GetUniqueTableName() const;

    /// CHAR
    sal_uInt16 GetCharFormatCount() const;
    SwCharFormat& GetCharFormat(sal_uInt16 nFormat) const;
    SwCharFormat* GetCurCharFormat() const;
    void FillByEx(SwCharFormat*);
    SwCharFormat* MakeCharFormat( const OUString& rName );
    SwCharFormat* FindCharFormatByName( const OUString& rName ) const;

    /* FormatCollections (new) - Explaining the general naming pattern:
     * GetXXXCount() returns the count of xxx in the document.
     * GetXXX(i)     returns i-th xxx (ERR_RAISE if beyond range!).
     * DelXXX(i)     deletes i-th xxx  (ERR_RAISE if beyond range!).
     * GetCurXXX()   returns xxx that is valid at cursor or in ranges.
     *               returns 0, if not unanimous.
     * SetXXX()      sets xxx at cursor or in ranges.
     * MakeXXX()     makes a xxx, derived from pDerivedFrom.
     */

    // TXT
    SwTextFormatColl& GetDfltTextFormatColl() const;
    sal_uInt16 GetTextFormatCollCount() const;
    SwTextFormatColl& GetTextFormatColl( sal_uInt16 nTextFormatColl) const;
    /**
     * Get the named paragraph format of the current selection.
     *
     * @see GetPaMTextFormatColl()
     *
     * @return the named paragraph format of the first node that contains one.
     * Nodes are sort by order of appearance in the selections ;
     * selections are sort by their order of creation
     * (last created selection first, oldest selection at last).
     */
    SwTextFormatColl* GetCurTextFormatColl() const;
    /**
     * Get the named paragraph format of the selection(s) described by a SwPaM.
     *
     * @param pPaM
     * input parameter - the selection where to look for the paragraph format.
     *
     * @return the named paragraph format of the first node that contains one.
     */
    SwTextFormatColl* GetPaMTextFormatColl( SwPaM* pPaM ) const;

    // #i62675#
    /// Add 2nd optional parameter <bResetListAttrs> - see also <SwDoc::SetTextFormatColl(..)>
    void SetTextFormatColl(SwTextFormatColl*, const bool bResetListAttrs = false);
    SwTextFormatColl *MakeTextFormatColl(const OUString &rFormatCollName,
        SwTextFormatColl *pDerivedFrom = nullptr);
    void FillByEx(SwTextFormatColl*);
    SwTextFormatColl* FindTextFormatCollByName( const OUString& rName ) const;

    /// @return "Auto-Collection" with given Id. If it does not exist create it.
    SwTextFormatColl* GetTextCollFromPool( sal_uInt16 nId );

    /// @return required automatic format base class.
    SwFormat* GetFormatFromPool( sal_uInt16 nId );

    /// @return required automatic page style.
    SwPageDesc* GetPageDescFromPool( sal_uInt16 nId );

    /// Query if the paragraph-/character-/frame-/page-style is used.
    bool IsUsed( const SwModify& ) const;

    /// @return required automatic format.
    SwFrameFormat* GetFrameFormatFromPool( sal_uInt16 nId )
        { return static_cast<SwFrameFormat*>(SwEditShell::GetFormatFromPool( nId )); }
    SwCharFormat* GetCharFormatFromPool( sal_uInt16 nId )
        { return static_cast<SwCharFormat*>(SwEditShell::GetFormatFromPool( nId )); }

    void SetClassification(const OUString& rName, SfxClassificationPolicyType eType);
    void ApplyAdvancedClassification(std::vector<svx::ClassificationResult> const & rResult);
    std::vector<svx::ClassificationResult> CollectAdvancedClassification();

    SfxWatermarkItem GetWatermark();
    void SetWatermark(const SfxWatermarkItem& rText);

    /// Sign the paragraph at the cursor.
    void SignParagraph();

    /// Validate the paragraph signatures, if any, of the current text node.
    void ValidateParagraphSignatures(SwTextNode* pNode, bool updateDontRemove);

    /// Validate the current paragraph signatures, if any, at the cursor start.
    void ValidateCurrentParagraphSignatures(bool updateDontRemove);

    /// Validate all paragraph signatures.
    void ValidateAllParagraphSignatures(bool updateDontRemove);

    /// Restore the metadata fields, if missing, from the RDF metadata
    /// and validate the signatures and update the signature metadata fields.
    /// Needed since deleting the metadata field doesn't remove the RDF
    /// and editing docs using software that don't support paragraph signing.
    void RestoreMetadataFieldsAndValidateParagraphSignatures();

    /// Ensure that the classification of the doc is never lower than
    /// the paragraph with the highest classification.
    void ClassifyDocPerHighestParagraphClass();

    /// Apply the classification to the paragraph at cursor.
    void ApplyParagraphClassification(std::vector<svx::ClassificationResult> aResult);
    std::vector<svx::ClassificationResult> CollectParagraphClassification();

    /// Returns true iff the cursor is within a paragraph metadata field.
    /// Currently there are two variants: signature and classification.
    bool IsCursorInParagraphMetadataField() const;

    /// Removes the paragraph metadata field at the current cursor, if any.
    /// Returns true iff a paragraph metadata field was removed.
    /// Currently there are two variants: signature and classification.
    bool RemoveParagraphMetadataFieldAtCursor();

    void Insert2(SwField const &, const bool bForceExpandHints);

    void UpdateOneField(SwField &);   ///< One single field.

    size_t GetFieldTypeCount(SwFieldIds nResId = SwFieldIds::Unknown) const;
    SwFieldType* GetFieldType(size_t nField, SwFieldIds nResId = SwFieldIds::Unknown) const;
    SwFieldType* GetFieldType(SwFieldIds nResId, const OUString& rName) const;

    void RemoveFieldType(size_t nField);
    void RemoveFieldType(SwFieldIds nResId, const OUString& rName);

    void FieldToText( SwFieldType const * pType );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    /// Database information.
    SwDBData const & GetDBData() const;
    const SwDBData& GetDBDesc() const;
    void ChgDBData(const SwDBData& SwDBData);
    void ChangeDBFields( const std::vector<OUString>& rOldNames,
                         const OUString& rNewName );
    void GetAllUsedDB( std::vector<OUString>& rDBNameList,
                       std::vector<OUString> const * pAllDBNames );

    bool IsAnyDatabaseFieldInDoc()const;

    /// Check whether DB fields point to an available data source and returns it.
    bool IsFieldDataSourceAvailable(OUString& rUsedDataSource) const;
    void UpdateExpFields(bool bCloseDB = false);///< only every expression fields update
    void LockExpFields();
    void UnlockExpFields();
    bool IsExpFieldsLocked() const;

    SwFieldUpdateFlags GetFieldUpdateFlags() const;
    void SetFieldUpdateFlags( SwFieldUpdateFlags eFlags );

    /// For evaluation of DB fields (new DB-manager).
    SwDBManager* GetDBManager() const;

    SwFieldType* InsertFieldType(const SwFieldType &);

    /// Changes in document?
    bool IsModified() const;
    void SetModified();
    void ResetModified();
    void SetUndoNoResetModified();

    /// Document - Statistics
    void UpdateDocStat();
    const SwDocStat &GetUpdatedDocStat();

    void    Insert(const SwTOXMark& rMark);

    void    DeleteTOXMark(SwTOXMark const * pMark);

    /// Get all marks at current SPoint.
    void    GetCurTOXMarks(SwTOXMarks& rMarks) const ;

    /// Insert content table. Renew if required.
    void                InsertTableOf(const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet = nullptr);
    void                UpdateTableOf(const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet = nullptr);
    const SwTOXBase*    GetCurTOX() const;
    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, bool bCreate = false );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    static bool         IsTOXBaseReadonly(const SwTOXBase& rTOXBase);
    void                SetTOXBaseReadonly(const SwTOXBase& rTOXBase, bool bReadonly);

    sal_uInt16              GetTOXCount() const;
    const SwTOXBase*    GetTOX( sal_uInt16 nPos ) const;
    bool                DeleteTOX( const SwTOXBase& rTOXBase, bool bDelNodes );

    /// After reading file update all content tables.
    void SetUpdateTOX( bool bFlag );
    bool IsUpdateTOX() const;

    /// Manage types of content tables.
    sal_uInt16              GetTOXTypeCount(TOXTypes eTyp) const;
    const SwTOXType*    GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const;
    void                InsertTOXType(const SwTOXType& rTyp);

    /// AutoMark file
    OUString const & GetTOIAutoMarkURL() const;
    void            SetTOIAutoMarkURL(const OUString& rSet);
    void            ApplyAutoMark();

    /// Key for managing index.
    void GetTOIKeys( SwTOIKeyType eTyp, std::vector<OUString>& rArr ) const;

    void SetOutlineNumRule(const SwNumRule&);
    const SwNumRule* GetOutlineNumRule() const;

    bool OutlineUpDown( short nOffset = 1 );

    bool MoveOutlinePara( SwOutlineNodes::difference_type nOffset );

    bool IsProtectedOutlinePara() const;

    const SwNumRule* GetNumRuleAtCurrCursorPos() const;

    /** Returns the numbering rule found at the paragraphs of the current selection,
       if all paragraphs of the current selection have the same or none numbering rule applied. */
    const SwNumRule* GetNumRuleAtCurrentSelection() const;

    /** Optional parameter <bResetIndentAttrs> (default value false).
        If <bResetIndentAttrs> equals true, the indent attributes "before text"
        and "first line indent" are additionally reset at the current selection,
        if the list style makes use of the new list level attributes.
       Parameters <bCreateNewList> and <sContinuedListId>
        <bCreateNewList> indicates, if a new list is created by applying the given list style.
        If <bCreateNewList> equals false, <sContinuedListId> may contain the
        list Id of a list, which has to be continued by applying the given list style */
    void SetCurNumRule( const SwNumRule&,
                        const bool bCreateNewList /*= false*/,
                        const OUString& sContinuedListId = OUString(),
                        const bool bResetIndentAttrs = false );

    /// Paragraphs without enumeration but with indents.
    void NoNum();

    /// Delete, split enumeration list.
    void DelNumRules();

    void NumUpDown( bool bDown = true );

    bool MoveParagraph( long nOffset = 1);
    bool MoveNumParas( bool bUpperLower, bool bUpperLeft );

    /// Switch on/off of numbering via Delete/Backspace.
    bool NumOrNoNum(bool bDelete = false, bool bChkStart = true);

    // #i23726#
    // #i90078#
    /// Remove unused default parameter <nLevel> and <bRelative>.
    // Adjust method name and parameter name
    void ChangeIndentOfAllListLevels( sal_Int32 nDiff );
    // Adjust method name
    void SetIndent(short nIndent, const SwPosition & rPos);
    bool IsFirstOfNumRuleAtCursorPos() const;

    bool IsNoNum( bool bChkStart = true ) const;

    /** @return Num-Level of the node in which point of cursor is.
     @return values can be: NO_NUMBERING,
     0..MAXLEVEL-1, NO_NUMLEVEL .. NO_NUMLEVEL|MAXLEVEL-1 */
    sal_uInt8 GetNumLevel() const;

    /// Detect highest and lowest level to check moving of outline levels.
    void GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower );

    /// Get Outline level of current paragraph.
    int GetCurrentParaOutlineLevel( ) const;

    // i29560
    bool HasNumber() const;
    bool HasBullet() const;

    bool SelectionHasNumber() const;
    bool SelectionHasBullet() const;

    OUString GetUniqueNumRuleName() const;
    void ChgNumRuleFormats( const SwNumRule& rRule );

    /// Set (and query if) a numbering with StartFlag starts at current PointPos.
    void SetNumRuleStart( bool bFlag, SwPaM* pCursor );
    bool IsNumRuleStart( SwPaM* pPaM = nullptr ) const;
    void SetNodeNumStart( sal_uInt16 nStt );

    sal_uInt16 GetNodeNumStart( SwPaM* pPaM ) const;

    void ReplaceNumRule( const OUString& rOldRule, const OUString& rNewRule );

    /** Searches for a text node with a numbering rule.
     in case a list style is found, <sListId> holds the list id, to which the
     text node belongs, which applies the found list style. */
    const SwNumRule * SearchNumRule(const bool bNum,
                                    OUString& sListId );

    /** Undo.
     Maintain UndoHistory in Document.
     Reset UndoHistory at Save, SaveAs, Create ??? */
    void DoUndo( bool bOn = true );
    bool DoesUndo() const;
    void DoGroupUndo( bool bUn );
    bool DoesGroupUndo() const;
    void DelAllUndoObj();

    /// Undo: set up Undo parenthesis, return nUndoId of this parenthesis.
    SwUndoId StartUndo( SwUndoId eUndoId = SwUndoId::EMPTY, const SwRewriter * pRewriter = nullptr );

    /// Closes parenthesis of nUndoId, not used by UI.
    SwUndoId EndUndo( SwUndoId eUndoId = SwUndoId::EMPTY, const SwRewriter * pRewriter = nullptr );

    bool     GetLastUndoInfo(OUString *const o_pStr,
                             SwUndoId *const o_pId,
                             const SwView* pView = nullptr) const;
    bool     GetFirstRedoInfo(OUString *const o_pStr,
                              SwUndoId *const o_pId,
                              const SwView* pView = nullptr) const;
    SwUndoId GetRepeatInfo(OUString *const o_pStr) const;

    /// is it forbidden to modify cursors via API calls?
    bool CursorsLocked() const;
    /// set selections to those contained in the UndoRedoContext
    /// should only be called by sw::UndoManager!
    void HandleUndoRedoContext(::sw::UndoRedoContext & rContext);

    void Undo(sal_uInt16 const nCount = 1);
    void Redo(sal_uInt16 const nCount = 1);
    void Repeat(sal_uInt16 const nCount);

    /// For all views of this document.
    void StartAllAction();
    void EndAllAction();

    /// To enable set up of StartActions and EndActions.
    virtual void CalcLayout() override;

    /// Determine form of content. Return Type at CurrentCursor->SPoint.
    sal_uInt16 GetCntType() const;

    /// Are there frames, footnotes, etc.
    bool HasOtherCnt() const;

    /// Apply ViewOptions with Start-/EndAction.
    virtual void ApplyViewOptions( const SwViewOption &rOpt ) override;

    /** Query text within selection. */
    void GetSelectedText( OUString &rBuf,
                        ParaBreakType nHndlParaBreak = ParaBreakType::ToBlank );

    /** @return graphic, if CurrentCursor->Point() points to a SwGrfNode
     (and mark is not set or points to the same graphic). */

    const Graphic* GetGraphic( bool bWait = true ) const;
    const GraphicObject* GetGraphicObj() const;
    const GraphicAttr* GetGraphicAttr( GraphicAttr& rGA ) const;

    bool IsLinkedGrfSwapOut() const;
    GraphicType GetGraphicType() const;

    const tools::PolyPolygon *GetGraphicPolygon() const;
    void SetGraphicPolygon( const tools::PolyPolygon *pPoly );

    /** If there's an automatic, not manipulated polygon at the selected
     notxtnode, it has to be deleted, e.g. cause the object has changed. */
    void ClearAutomaticContour();

    /// @return the size of a graphic in Twips if cursor is in a graphic.
    bool GetGrfSize(Size&) const;

    /** @return name and filter of a graphic if the cursor is in a graphic,
     else give a rap on the knuckles!
     If a string-ptr != 0 return the respective name. */
    void GetGrfNms( OUString* pGrfName, OUString* pFltName,
                    const SwFlyFrameFormat* = nullptr ) const;

    /// Re-read if graphic is not ok. Current graphic is replaced by the new one.
    void ReRead( const OUString& rGrfName, const OUString& rFltName,
                  const Graphic* pGraphic = nullptr );

    /// Unique identification of object (for ImageMapDlg).
    void    *GetIMapInventor() const;

    // #i73788#
    /// Remove default parameter, because method always called this default value.
    Graphic GetIMapGraphic() const; ///< @return a graphic for all Flys!
    const SwFlyFrameFormat* FindFlyByName( const OUString& rName ) const;

    /** @return a ClientObject, if CurrentCursor->Point() points to a SwOLENode
     (and mark is neither set not pointint to same ClientObject)
     else give rap on the knuckles. */
    svt::EmbeddedObjectRef&  GetOLEObject() const;

    /// Is there an OLEObject with this name (SwFormat)?
    bool HasOLEObj( const OUString &rName ) const;

    /// @return pointer to the data of the chart in which Cursr is.
    void SetChartName( const OUString &rName );

    /// Update content of all charts for table with given name.
    void UpdateCharts( const OUString &rName );

    OUString GetCurWord();

    /** Glossary from glossary document in current document.
     Styles only if not already existent. */
    void InsertGlossary( SwTextBlocks& rGlossary, const OUString& );

    /** Make current selection glossary and insert into glossary document
     including styles. */
    sal_uInt16 MakeGlossary( SwTextBlocks& rToFill, const OUString& rName,
                         const OUString& rShortName, bool bSaveRelFile,
                         const OUString* pOnlyText );

    /// Save complete content of doc as glossary.
    sal_uInt16 SaveGlossaryDoc( SwTextBlocks& rGlossary, const OUString& rName,
                            const OUString& rShortName,
                            bool bSaveRelFile,
                            bool bOnlyText );

    // Linguistics...
    /// Save selections.
    void HyphStart( SwDocPositions eStart, SwDocPositions eEnd );

    /// restore selections.
    void HyphEnd();
    css::uno::Reference< css::uno::XInterface>
                HyphContinue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    void HyphIgnore();

    /// For Inserting SoftHyphen. Position is offset within the syllabificated word.
    static void InsertSoftHyph( const sal_Int32 nHyphPos );

    const SwTable& InsertTable( const SwInsertTableOptions& rInsTableOpts,  ///< All
                                sal_uInt16 nRows, sal_uInt16 nCols,
                                const SwTableAutoFormat* pTAFormat = nullptr );

    void InsertDDETable( const SwInsertTableOptions& rInsTableOpts,  ///< HeadlineNoBorder
                         SwDDEFieldType* pDDEType,
                         sal_uInt16 nRows, sal_uInt16 nCols  );

    void UpdateTable();
    void SetTableName( SwFrameFormat& rTableFormat, const OUString &rNewName );

    SwFrameFormat *GetTableFormat();
    bool TextToTable( const SwInsertTableOptions& rInsTableOpts,  ///< All
                      sal_Unicode cCh,
                      const SwTableAutoFormat* pTAFormat = nullptr );
    bool TableToText( sal_Unicode cCh );
    bool IsTextToTableAvailable() const;

    bool GetTableBoxFormulaAttrs( SfxItemSet& rSet ) const;
    void SetTableBoxFormulaAttrs( const SfxItemSet& rSet );

    bool IsTableBoxTextFormat() const;
    OUString GetTableBoxText() const;

    TableChgMode GetTableChgMode() const;
    void SetTableChgMode( TableChgMode eMode );

    /// Split table at cursor position.
    void SplitTable( SplitTable_HeadlineOption eMode );

    /** Merge tables.

     Can Merge checks if Prev or Next are possible.
        If pointer pChkNxtPrv is passed possible direction is given. */
    bool CanMergeTable( bool bWithPrev = true, bool* pChkNxtPrv = nullptr ) const;
    bool MergeTable( bool bWithPrev );

    /// Set up InsertDB as table Undo.
    void AppendUndoForInsertFromDB( bool bIsTable );

    /// Functions used for spell checking and text conversion.

    /// Save selections.
    void SpellStart( SwDocPositions eStart, SwDocPositions eEnd,
                     SwDocPositions eCurr, SwConversionArgs *pConvArgs = nullptr );

    /// Restore selections.
    void SpellEnd( SwConversionArgs const *pConvArgs = nullptr, bool bRestoreSelection = true );
    css::uno::Any SpellContinue(
                    sal_uInt16* pPageCnt, sal_uInt16* pPageSt,
                    SwConversionArgs const *pConvArgs );

    /** Spells on a sentence basis - the SpellPortions are needed
     @return false if no error could be found. */
    bool SpellSentence(svx::SpellPortions& rToFill, bool bIsGrammarCheck );

    /// Make SpellIter start with the current sentence when called next time.
    static void PutSpellingToSentenceStart();

    /// Moves the continuation position to the end of the currently checked sentence.
    static void MoveContinuationPosToEndOfCheckedSentence();

    /// Applies a changed sentence.
    void ApplyChangedSentence(const svx::SpellPortions& rNewPortions, bool bRecheck);

    /// Check SwSpellIter data to see if the last sentence got grammar checked.
    static bool HasLastSentenceGotGrammarChecked();

    /// Is text conversion active somewhere else?
    static bool HasConvIter();

    /// Is hyphenation active somewhere else?
    static bool HasHyphIter();

    void HandleCorrectionError(const OUString& aText, SwPosition aPos, sal_Int32 nBegin,
                               sal_Int32 nLen, const Point* pPt,
                               SwRect& rSelectRect);
    css::uno::Reference< css::linguistic2::XSpellAlternatives >
            GetCorrection( const Point* pPt, SwRect& rSelectRect );

    bool GetGrammarCorrection( css::linguistic2::ProofreadingResult /*out*/ &rResult,
            sal_Int32 /*out*/ &rErrorPosInText,
            sal_Int32 /*out*/ &rErrorIndexInResult,
            css::uno::Sequence< OUString > /*out*/ &rSuggestions,
            const Point* pPt, SwRect& rSelectRect );

    static void IgnoreGrammarErrorAt( SwPaM& rErrorPosition );
    void SetLinguRange( SwDocPositions eStart, SwDocPositions eEnd );

    /// @return reference set in document according to given name.
    const SwFormatRefMark* GetRefMark( const OUString& rName ) const;

    /**  @return names of all references set in document.
      If ArrayPointer == 0 then return only whether a RefMark is set in document. */
    sal_uInt16 GetRefMarks( std::vector<OUString>* = nullptr ) const;

    /// Call AutoCorrect
    void AutoCorrect( SvxAutoCorrect& rACorr, bool bInsertMode,
                        sal_Unicode cChar );
    bool GetPrevAutoCorrWord( SvxAutoCorrect const & rACorr, OUString& rWord );

    /// Set our styles according to the respective rules.
    void AutoFormat( const SvxSwAutoFormatFlags* pAFlags );

    static SvxSwAutoFormatFlags* GetAutoFormatFlags();
    static void SetAutoFormatFlags(SvxSwAutoFormatFlags const *);

    /// Calculates selection.
    OUString Calculate();

    bool InsertURL( const SwFormatINetFormat& rFormat, const OUString& rStr,
                    bool bKeepSelection = false );
    void GetINetAttrs( SwGetINetAttrs& rArr );

    OUString GetDropText( const sal_Int32 nChars ) const;
    void   ReplaceDropText( const OUString &rStr, SwPaM* pPaM = nullptr );

    /** May an outline be moved or copied?
     Check whether it's in text body, not in table, and not read-only (move). */
    bool IsOutlineMovable( SwOutlineNodes::size_type nIdx ) const;
    bool IsOutlineCopyable( SwOutlineNodes::size_type nIdx ) const;

    sal_uInt16 GetLineCount();

    /// Query and set footnote-text/number. Set.. to current SSelection!
    bool GetCurFootnote( SwFormatFootnote* pToFillFootnote = nullptr );
    bool SetCurFootnote( const SwFormatFootnote& rFillFootnote );
    bool HasFootnotes( bool bEndNotes = false ) const;

    size_t GetSeqFootnoteList( SwSeqFieldList& rList, bool bEndNotes = false );
    /// @return list of all footnotes and their first portions of text.

    SwSection const* InsertSection(
            SwSectionData & rNewData, SfxItemSet const*const = nullptr );
    bool IsInsRegionAvailable() const;
    const SwSection* GetCurrSection() const;

    /** @return current range like Cet CurrSection(). But this function iterates
     also over frames and catches the range even if the cursor is positioned in
     a footnote the reference of which is in a columned range.
     If bOutOfTab is set, the range comprising the table is searched
     and not an inner one. */
    SwSection* GetAnySection( bool bOutOfTab = false, const Point* pPt = nullptr );

    size_t GetSectionFormatCount() const;
    size_t GetSectionFormatPos(const SwSectionFormat&) const;
    const SwSectionFormat& GetSectionFormat(size_t nFormat) const;
    void DelSectionFormat( size_t nFormat);
    void UpdateSection( size_t const nSect, SwSectionData &,
            SfxItemSet const*const  = nullptr);
    bool IsAnySectionInDoc() const;

    OUString GetUniqueSectionName( const OUString* pChkStr = nullptr ) const;

    /// Set attributes.
    void SetSectionAttr(const SfxItemSet& rSet, SwSectionFormat* pSectFormat = nullptr);

    /** Search inside the cursor selection for full selected sections.
     if any part of section in the selection @return 0.
     if more than one in the selection return the count. */
    sal_uInt16 GetFullSelectedSectionCount() const;

    /** Special insert: Insert a new text node just before or after a section or
     table, if the cursor is positioned at the start/end of said
     section/table. The purpose of the method is to allow users to inert text
     at certain 'impossible' position, e.g. before a table at the document
     start or between to sections. */
    void DoSpecialInsert();
    bool CanSpecialInsert() const;

    /// Optimizing UI.
    void SetNewDoc();

    sfx2::LinkManager& GetLinkManager();
    inline const sfx2::LinkManager& GetLinkManager() const;

    /** Adjust left margin via object bar (similar to adjustment of numerations).
     One can either change the margin "by" adding or subtracting a given
     offset or set it "to" this position @param (bModulus = true). */
    bool IsMoveLeftMargin( bool bRight, bool bModulus = true ) const;
    void MoveLeftMargin( bool bRight, bool bModulus = true );

    /// Query NumberFormatter from document.
          SvNumberFormatter* GetNumberFormatter();
    const SvNumberFormatter* GetNumberFormatter() const
    {   return const_cast<SwEditShell*>(this)->GetNumberFormatter();  }

    /// Interfaces for GlobalDocument.
    bool IsGlobalDoc() const;
    void SetGlblDocSaveLinks( bool bFlag );
    bool IsGlblDocSaveLinks() const;
    void GetGlobalDocContent( SwGlblDocContents& rArr ) const;
    void InsertGlobalDocContent( const SwGlblDocContent& rPos,
                                 SwSectionData & rNew );
    bool InsertGlobalDocContent( const SwGlblDocContent& rPos,
                                 const SwTOXBase& rTOX );
    bool InsertGlobalDocContent( const SwGlblDocContent& rPos );
    void DeleteGlobalDocContent( const SwGlblDocContents& rArr,
                                size_t nPos );
    bool MoveGlobalDocContent( const SwGlblDocContents& rArr ,
                                size_t nFromPos, size_t nToPos,
                                size_t nNewPos );
    void GotoGlobalDocContent( const SwGlblDocContent& rPos );

    /// For Redlining.
    RedlineFlags GetRedlineFlags() const;
    void SetRedlineFlags( RedlineFlags eMode );
    bool IsRedlineOn() const;
    SwRedlineTable::size_type GetRedlineCount() const;
    const SwRangeRedline& GetRedline( SwRedlineTable::size_type nPos ) const;
    bool AcceptRedline( SwRedlineTable::size_type nPos );
    bool RejectRedline( SwRedlineTable::size_type nPos );
    bool AcceptRedlinesInSelection();
    bool RejectRedlinesInSelection();

    /** Search Redline for this Data and @return position in array.
     If not found, return SwRedlineTable::npos. */
    SwRedlineTable::size_type FindRedlineOfData( const SwRedlineData& ) const;

    /// Set comment to Redline at position.
    bool SetRedlineComment( const OUString& rS );
    const SwRangeRedline* GetCurrRedline() const;

    /// Redline attributes have been changed. Updated views.
    void UpdateRedlineAttr();

    /// Compare two documents.
    long CompareDoc( const SwDoc& rDoc );

    /// Merge two documents.
    long MergeDoc( const SwDoc& rDoc );

    /// Footnote attributes global to document.
    const SwFootnoteInfo& GetFootnoteInfo() const;
    void  SetFootnoteInfo(const SwFootnoteInfo& rInfo);
    const SwEndNoteInfo& GetEndNoteInfo() const;
    void  SetEndNoteInfo(const SwEndNoteInfo& rInfo);

    const SwLineNumberInfo &GetLineNumberInfo() const;
    void  SetLineNumberInfo( const SwLineNumberInfo& rInfo);

    /// Labels: Synchronize ranges.
    void SetLabelDoc( bool bFlag );
    bool IsLabelDoc() const;

    /// Interface for TextInputData - (for input of Japanese/Chinese chars.)
    void CreateExtTextInput(LanguageType eInputLanguage);
    OUString DeleteExtTextInput( bool bInsText = true);
    void SetExtTextInputData( const CommandExtTextInputData& );

    /// Returns true iff paragraph signature validation is enabled.
    bool IsParagraphSignatureValidationEnabled() const { return m_bDoParagraphSignatureValidation; }
    /// Enable/Disable paragraph signature validation and return the previous value.
    bool SetParagraphSignatureValidation(const bool bEnable)
    {
        const bool bOldFlag = m_bDoParagraphSignatureValidation;
        m_bDoParagraphSignatureValidation = bEnable;
        return bOldFlag;
    }

    /// Interface for access to AutoComplete-list.
    static SwAutoCompleteWord& GetAutoCompleteWords();

    /** @return a scaling factor of selected text. Used for the rotated
     character attribute dialog. */
    sal_uInt16 GetScalingOfSelectedText() const;

    bool IsNbspRunNext() const { return m_bNbspRunNext; }

    /// Ctor/Dtor.
    SwEditShell( SwDoc&, vcl::Window*, const SwViewOption *pOpt );

    /// Copy-Constructor in disguise.
    SwEditShell( SwEditShell&, vcl::Window* );
    virtual ~SwEditShell() override;

private:
    SwEditShell(const SwEditShell &) = delete;
    const SwEditShell &operator=(const SwEditShell &) = delete;

    /* TODO: this flag may have to be invalidated / reset to false at various
     * places if it was true and the edit cursor position changes. It's somehow
     * overkill though because it can only be true if a NO-BREAK SPACE was
     * inserted by the last DoAutoCorrect() call (in French language), any
     * subsequent call will reset it anyway and just if the cursor is
     * positioned behind "x :" and the next character inserted is not a space
     * the existing nb-space will be removed. Bear this in mind if that problem
     * arises. */
    bool m_bNbspRunNext;    ///< NO-BREAK SPACE state flag passed to and maintained by SvxAutoCorrect::DoAutoCorrect()
    bool m_bDoParagraphSignatureValidation; ///< Prevent nested calls of ValidateParagraphSignatures.
};

inline const sfx2::LinkManager& SwEditShell::GetLinkManager() const
{   return const_cast<SwEditShell*>(this)->GetLinkManager();  }

 /// Class for automated call of Start- and EndAction().
class SwActContext {
    SwEditShell & m_rShell;
public:
    SwActContext(SwEditShell *pShell);
    ~SwActContext() COVERITY_NOEXCEPT_FALSE;
};

 /// Class for automated call of Start- and EndCursorMove().
class SwMvContext {
    SwEditShell & m_rShell;
public:
    SwMvContext(SwEditShell *pShell);
    ~SwMvContext() COVERITY_NOEXCEPT_FALSE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
