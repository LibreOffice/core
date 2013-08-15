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
#ifndef _EDITSH_HXX
#define _EDITSH_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <tools/string.hxx>
#include <vcl/font.hxx>
#include <editeng/swafopt.hxx>
#include "swdllapi.h"
#include <crsrsh.hxx>   // For base class.
#include <itabenum.hxx>
#include <swdbdata.hxx>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <fldupde.hxx>
#include <tblenum.hxx>
#include <IMark.hxx>

#include <vector>
#include <set>
#include <swundo.hxx>
#include <svtools/embedhlp.hxx>
#include <boost/ptr_container/ptr_vector.hpp>

class PolyPolygon;
class SwDoc;
class DateTime;
class CommandExtTextInputData;

class SvNumberFormatter;
class SfxPoolItem;
class SfxItemSet;
class SvxAutoCorrect;

class SwField;
class SwFieldType;
class SwDDEFieldType;
class SwNewDBMgr;

struct SwDocStat;
class SwAutoCompleteWord;

class SwFmtRefMark;

class SwNumRule;

class SwTxtFmtColl;
class SwGrfNode;
class SwFlyFrmFmt;

class SwFrmFmt;         ///< For GetTxtNodeFmts().
class SwCharFmt;
class SwExtTextInput;
class SwRootFrm;        ///< For CTOR.
class Graphic;
class GraphicObject;
class SwFmtINetFmt;     ///< InsertURL.
class SwTable;
class SwTextBlocks;     ///< For GlossaryRW.
class SwFmtFtn;
class SwSection;
class SwSectionData;
class SwSectionFmt;
class SwTOXMarks;
class SwTOXBase;
class SwTOXType;
class SwTableAutoFmt;
class SwPageDesc;
class SwTxtINetFmt;
class SwSeqFldList;
class SwGlblDocContent;
class SwGlblDocContents;
class SwRedline;
class SwRedlineData;
class SwFtnInfo;
class SwEndNoteInfo;
class SwLineNumberInfo;
class SwAuthEntry;
class SwRewriter;
struct SwConversionArgs;


namespace com { namespace sun { namespace star { namespace uno {
    template < class > class Sequence;
}}}}

namespace svx{
struct SpellPortion;
typedef std::vector<SpellPortion> SpellPortions;
}

namespace sfx2{
class LinkManager;
}

namespace sw {
    class UndoRedoContext;
}

#define GETSELTXT_PARABRK_TO_BLANK      0
#define GETSELTXT_PARABRK_KEEP          1
#define GETSELTXT_PARABRK_TO_ONLYCR     2


 /// For querying the INet-attributes for Navigator.
struct SwGetINetAttr
{
    String sText;
    const SwTxtINetFmt& rINetAttr;

    SwGetINetAttr( const String& rTxt, const SwTxtINetFmt& rAttr )
        : sText( rTxt ), rINetAttr( rAttr )
    {}
};
typedef boost::ptr_vector<SwGetINetAttr> SwGetINetAttrs;

// Types of forms of content.
#define CNT_TXT 0x0001
#define CNT_GRF 0x0002
#define CNT_OLE 0x0010

// Test USHORT for a defined form of content.
#define CNT_HasTxt(USH) ((USH)&CNT_TXT)
#define CNT_HasGrf(USH) ((USH)&CNT_GRF)
#define CNT_HasOLE(USH) ((USH)&CNT_OLE)

class SW_DLLPUBLIC SwEditShell: public SwCrsrShell
{
    static SvxSwAutoFmtFlags* pAutoFmtFlags;

    /// For the private methods DelRange and those of AutoCorrect.
    friend class SwAutoFormat;
    friend void _InitCore();
    friend void _FinitCore();
    /// For the PamCorrAbs/-Rel methods.
    friend class SwUndo;

    SW_DLLPRIVATE SfxPoolItem& _GetChrFmt( SfxPoolItem& ) const;

    /** Returns pointer to a SwGrfNode
     that will be used by GetGraphic() and GetGraphicSize(). */
    SW_DLLPRIVATE SwGrfNode *_GetGrfNode() const ;

    SW_DLLPRIVATE void DeleteSel( SwPaM& rPam, sal_Bool* pUndo = 0 );

    SW_DLLPRIVATE void _SetSectionAttr( SwSectionFmt& rSectFmt, const SfxItemSet& rSet );

    using ViewShell::UpdateFlds;
    using SwModify::GetInfo;

public:
    /// Edit (all selected ranges).
    void Insert( sal_Unicode, sal_Bool bOnlyCurrCrsr = sal_False );
    void Insert2( const String &, const bool bForceExpandHints = false );
    void Overwrite( const String & );

    /** Replace a selected range in a TextNode by given string.
     Meant for Search & Replace.
     bRegExpRplc - replace tabs (\\t) and insert found string (not \&).
     E.g.: Fnd: "zzz", Repl: "xx\t\\t..&..\&"
           --> "xx\t<Tab>..zzz..&" */
    sal_Bool Replace( const String& rNewStr, sal_Bool bRegExpRplc = sal_False );

    /** Delete content of all ranges.
     If whole nodes are selected, these nodes get deleted. */
    long Delete();

    /// Remove a complete paragraph.
    sal_Bool DelFullPara();

    /// Change text to Upper/Lower/Hiragana/Katagana/...
    void TransliterateText( sal_uInt32 nType );

    /// Count words in current selection.
    void CountWords( SwDocStat& rStat ) const;

    /** Delete non-visible content in the document.
     E.g. hidden ranges, hidden paragraphs. */
    sal_Bool RemoveInvisibleContent();

    /// Replace fields by text - mailmerge support.
    bool ConvertFieldsToText();

    /// Set all numbering start points to a fixed value - mailmerge support.
    void SetNumberingRestart();

    /// Embedds all local links (ranges/graphics).
    sal_uInt16 GetLinkUpdMode(sal_Bool bDocSettings = sal_False) const;
    void SetLinkUpdMode( sal_uInt16 nMode );

    /// Copy content of all ranges at current position of cursor to given Shell.
    long Copy( SwEditShell* pDestShell = 0 );

    /** For copying via ClipBoard:
       If table is copied into table, move all cursors away from it.
       Copy and Paste must be in FEShell because of FlyFrames!
       Copy all selections and the document. */
    sal_Bool _CopySelToDoc( SwDoc* pInsDoc, SwNodeIndex* pNdInsPos = 0 );

    long SplitNode( sal_Bool bAutoFormat = sal_False, sal_Bool bCheckTableStart = sal_True );
    sal_Bool AppendTxtNode();
    void AutoFmtBySplitNode();

    /** If cursor is in a INetAttribute it will be deleted completely
     including the descriptive text (needed at drag & drop). */
    sal_Bool DelINetAttrWithText();

    /** If Cursor is at the end of a character style in which the DontExpand-flag
     is not yet set, the latter will be set (==> return TRUE). */
    bool DontExpandFmt();

    /** Apply / remove attributes.
     @return attributs in required AttributeSet.
     When not unambiguous the set has a DONT_CARE !!
     2nd optional parameter <bMergeIndentValuesOfNumRule>.
     If <bMergeIndentValuesOfNumRule> == sal_True, the indent attributes of
     the corresponding list level of an applied list style is merged into
     the requested item set as a LR-SPACE item, if corresponding node has not
     its own indent attributes and the position-and-space mode of the list
     level is SvxNumberFormat::LABEL_ALIGNMENT. */
    sal_Bool GetPaMAttr( SwPaM* pPaM, SfxItemSet& ,
                     const bool bMergeIndentValuesOfNumRule = false ) const;
    sal_Bool GetCurAttr( SfxItemSet& ,
                     const bool bMergeIndentValuesOfNumRule = false ) const;
    void SetAttr( const SfxPoolItem&, sal_uInt16 nFlags = 0 );
    void SetAttr( const SfxItemSet&, sal_uInt16 nFlags = 0, SwPaM* pCrsr = NULL );

    /**
     * Get the paragraph format attribute(s) of the current selection.
     *
     * @see GetPaMParAttr()
     *
     * @param rSet
     * output parameter - the SfxItemSet where the automatic paragraph format attribut(s) will be store.
     * The attributes aren't invalidated or cleared if the function reach the getMaxLookup limite.
     *
     * @return true if the function inspect all the nodes point by the pPaM parameter,
     * false if the function reach the limit of getMaxLookup number of nodes inspected.
     */
    sal_Bool GetCurParAttr( SfxItemSet& rSet ) const;
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
    sal_Bool GetPaMParAttr( SwPaM* pPaM, SfxItemSet& rSet ) const;

    /// Set attribute as new default attribute in document.
    void SetDefault( const SfxPoolItem& );

    /// Query default attribute of document.
    const SfxPoolItem& GetDefault( sal_uInt16 nFmtHint ) const;

    void ResetAttr( const std::set<sal_uInt16> &attrs = std::set<sal_uInt16>(), SwPaM* pCrsr = NULL );
    void GCAttr();

    /// @return the scripttpye of the selection.
    sal_uInt16 GetScriptType() const;

    /// @return the language at current cursor position.
    sal_uInt16 GetCurLang() const;

    /// TABLE
    sal_uInt16 GetTblFrmFmtCount( bool bUsed = false ) const;
    SwFrmFmt& GetTblFrmFmt(sal_uInt16 nFmt, bool bUsed = false ) const;
    String GetUniqueTblName() const;

    /// CHAR
    sal_uInt16 GetCharFmtCount() const;
    SwCharFmt& GetCharFmt(sal_uInt16 nFmt) const;
    SwCharFmt* GetCurCharFmt() const;
    void FillByEx(SwCharFmt*, sal_Bool bReset = sal_False);
    SwCharFmt* MakeCharFmt( const String& rName, SwCharFmt* pDerivedFrom = 0 );
    SwCharFmt* FindCharFmtByName( const String& rName ) const;


    /* FormatCollections (new) - Explaining the general naming pattern:
     * GetXXXCount() returns the count of xxx in the document.
     * GetXXX(i)     returns i-th xxx (ERR_RAISE if beyond range!).
     * DelXXX(i)     deletes i-th xxx  (ERR_RAISE if beyond range!).
     * GetCurXXX()   returns xxx that is valid at cursor or in ranges.
     *               returns 0, if not unanimuous.
     * SetXXX()      sets xxx at cursor or in ranges.
     * MakeXXX()     makes a xxx, derived from pDerivedFrom.
     */

    // TXT
    SwTxtFmtColl& GetDfltTxtFmtColl() const;
    sal_uInt16 GetTxtFmtCollCount() const;
    SwTxtFmtColl& GetTxtFmtColl( sal_uInt16 nTxtFmtColl) const;
    /**
     * Get the named paragraph format of the current selection.
     *
     * @see GetPaMTxtFmtColl()
     *
     * @return the named paragraph format of the first node that contains one.
     * Nodes are sort by order of appearance in the selections ;
     * selections are sort by their order of creation
     * (last created selection first, oldest selection at last).
     */
    SwTxtFmtColl* GetCurTxtFmtColl() const;
    /**
     * Get the named paragraph format of the selection(s) described by a SwPaM.
     *
     * @param pPaM
     * input parameter - the selection where to look for the paragraph format.
     *
     * @return the named paragraph format of the first node that contains one.
     */
    SwTxtFmtColl* GetPaMTxtFmtColl( SwPaM* pPaM ) const;

    // #i62675#
    /// Add 2nd optional parameter <bResetListAttrs> - see also <SwDoc::SetTxtFmtColl(..)>
    void SetTxtFmtColl( SwTxtFmtColl*,
                        bool bResetListAttrs = false );
    SwTxtFmtColl *MakeTxtFmtColl(const String &rFmtCollName,
        SwTxtFmtColl *pDerivedFrom = 0);
    void FillByEx(SwTxtFmtColl*, sal_Bool bReset = sal_False);
    SwTxtFmtColl* FindTxtFmtCollByName( const String& rName ) const;

    /// @return "Auto-Collection" with given Id. If it does not exist create it.
    SwTxtFmtColl* GetTxtCollFromPool( sal_uInt16 nId );

    /// @return required automatic format base class.
    SwFmt* GetFmtFromPool( sal_uInt16 nId );

    /// @return required automatic page style.
    SwPageDesc* GetPageDescFromPool( sal_uInt16 nId );

    /// Query if the paragraph-/character-/frame-/page-style is used.
    bool IsUsed( const SwModify& ) const;

    /// @return required automatic format.
    SwFrmFmt* GetFrmFmtFromPool( sal_uInt16 nId )
        { return (SwFrmFmt*)SwEditShell::GetFmtFromPool( nId ); }
    SwCharFmt* GetCharFmtFromPool( sal_uInt16 nId )
        { return (SwCharFmt*)SwEditShell::GetFmtFromPool( nId ); }

    void Insert2(SwField&, const bool bForceExpandHints = false);
    SwField* GetCurFld() const;

    void UpdateFlds( SwField & );   ///< One single field.

    sal_uInt16 GetFldTypeCount(sal_uInt16 nResId = USHRT_MAX, sal_Bool bUsed = sal_False) const;
    SwFieldType* GetFldType(sal_uInt16 nId, sal_uInt16 nResId = USHRT_MAX, sal_Bool bUsed = sal_False) const;
    SwFieldType* GetFldType(sal_uInt16 nResId, const String& rName) const;

    void RemoveFldType(sal_uInt16 nId, sal_uInt16 nResId = USHRT_MAX);
    void RemoveFldType(sal_uInt16 nResId, const String& rName);

    void FieldToText( SwFieldType* pType );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    /// Database information.
    SwDBData GetDBData() const;
    const SwDBData& GetDBDesc() const;
    void ChgDBData(const SwDBData& SwDBData);
    void ChangeDBFields( const std::vector<String>& rOldNames,
                         const String& rNewName );
    void GetAllUsedDB( std::vector<String>& rDBNameList,
                       std::vector<String>* pAllDBNames = 0 );

    sal_Bool IsAnyDatabaseFieldInDoc()const;

    /// Check whether DB fields point to an available data source and returns it.
    sal_Bool IsFieldDataSourceAvailable(String& rUsedDataSource) const;
    void UpdateExpFlds(sal_Bool bCloseDB = sal_False);///< only every expression fields update
    void LockExpFlds();
    void UnlockExpFlds();

    SwFldUpdateFlags GetFldUpdateFlags(sal_Bool bDocSettings = sal_False) const;
    void SetFldUpdateFlags( SwFldUpdateFlags eFlags );

    /// For evaluation of DB fields (new DB-manager).
    SwNewDBMgr* GetNewDBMgr() const;

    SwFieldType* InsertFldType(const SwFieldType &);

    /// Changes in document?
    sal_Bool IsModified() const;
    void SetModified();
    void ResetModified();
    void SetUndoNoResetModified();

    /// Document - Statistics
    void UpdateDocStat();
    const SwDocStat &GetUpdatedDocStat();

    void    Insert(const SwTOXMark& rMark);

    void    DeleteTOXMark(SwTOXMark* pMark);

    /// Get all marks at current SPoint.
    sal_uInt16  GetCurTOXMarks(SwTOXMarks& rMarks) const ;

    /// Insert content table. Renew if required.
    void                InsertTableOf(const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet = 0);
    sal_Bool                UpdateTableOf(const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet = 0);
    const SwTOXBase*    GetCurTOX() const;
    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, bool bCreate = false );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    sal_Bool                IsTOXBaseReadonly(const SwTOXBase& rTOXBase) const;
    void                SetTOXBaseReadonly(const SwTOXBase& rTOXBase, sal_Bool bReadonly);

    sal_uInt16              GetTOXCount() const;
    const SwTOXBase*    GetTOX( sal_uInt16 nPos ) const;
    bool                DeleteTOX( const SwTOXBase& rTOXBase, bool bDelNodes = false );

    /// After reading file update all content tables.
    void SetUpdateTOX( sal_Bool bFlag = sal_True );
    sal_Bool IsUpdateTOX() const;

    /// Manage types of content tables.
    sal_uInt16              GetTOXTypeCount(TOXTypes eTyp) const;
    const SwTOXType*    GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const;
    void                InsertTOXType(const SwTOXType& rTyp);

    /// AutoMark file
    const String&   GetTOIAutoMarkURL() const;
    void            SetTOIAutoMarkURL(const String& rSet);
    void            ApplyAutoMark();

    /// Key for managing index.
    sal_uInt16 GetTOIKeys( SwTOIKeyType eTyp, std::vector<String>& rArr ) const;

    void SetOutlineNumRule(const SwNumRule&);
    const SwNumRule* GetOutlineNumRule() const;

    bool OutlineUpDown( short nOffset = 1 );

    bool MoveOutlinePara( short nOffset = 1);

    sal_Bool IsProtectedOutlinePara() const;

    /// @return numbering rulse pf current enumeration list (else FALSE).
    const SwNumRule* GetCurNumRule() const;

    /** If there is not already a numbering, set one, else change.
     Works with old and new rules. Update only differences.
     Add optional parameter @param <bResetIndentAttrs> (default value sal_False).
     If @param <bResetIndentAttrs> equals true, the indent attributes "before text"
     and "first line indent" are additionally reset at the current selection,
     if the list style makes use of the new list level attributes.
     introduce parameters @param <bCreateNewList> and @param <sContinuedListId>
     @param <bCreateNewList> indicates, if a new list is created by applying the
     given list style.
     If @param <bCreateNewList> equals sal_False, @param <sContinuedListId> may contain the
     list Id of a list, which has to be continued by applying the given list style. */
    void SetCurNumRule( const SwNumRule&,
                        const bool bCreateNewList /*= false*/,
                        const String sContinuedListId = String(),
                        const bool bResetIndentAttrs = false );

    /// Paragraphs without enumeration but with indents.
    bool NoNum();

    /// Delete, split enumeration list.
    void DelNumRules();

    bool NumUpDown( bool bDown = true );

    bool MoveParagraph( long nOffset = 1);
    bool MoveNumParas( bool bUpperLower, bool bUpperLeft );

    /// Switch on/off of numbering via Delete/Backspace.
    bool NumOrNoNum( sal_Bool bDelete = sal_False, bool bChkStart = true);

    // #i23726#
    // #i90078#
    /// Remove unused default parameter <nLevel> and <bRelative>.
    // Adjust method name and parameter name
    void ChangeIndentOfAllListLevels( short nDiff );
    // Adjust method name
    void SetIndent(short nIndent, const SwPosition & rPos);
    bool IsFirstOfNumRule() const;
    bool IsFirstOfNumRule(const SwPaM & rPaM) const;

    sal_Bool IsNoNum( sal_Bool bChkStart = sal_True ) const;

    /** @return Num-Level of the node in which point of cursor is.
     @return values can be: NO_NUMBERING,
     0..MAXLEVEL-1, NO_NUMLEVEL .. NO_NUMLEVEL|MAXLEVEL-1 */
    sal_uInt8 GetNumLevel() const;

    /// Detect highest and lowest level to check moving of outline levels.
    void GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower );


    /// Get Outline level of current paragraph.
    int GetCurrentParaOutlineLevel( ) const;// #outlinelevel add by zhaojianwei

    // i29560
    sal_Bool HasNumber() const;
    sal_Bool HasBullet() const;

    sal_Bool SelectionHasNumber() const;
    sal_Bool SelectionHasBullet() const;

    String GetUniqueNumRuleName( const String* pChkStr = 0, bool bAutoNum = true ) const;
    void ChgNumRuleFmts( const SwNumRule& rRule );

    /// Set (and query if) a numbering with StartFlag starts at current PointPos.
    void SetNumRuleStart( sal_Bool bFlag = sal_True, SwPaM* pCrsr = NULL );
    sal_Bool IsNumRuleStart( SwPaM* pPaM = NULL ) const;
    void SetNodeNumStart( sal_uInt16 nStt, SwPaM* = NULL );

    sal_uInt16 GetNodeNumStart( SwPaM* pPaM = NULL ) const;

    bool ReplaceNumRule( const String& rOldRule, const String& rNewRule );

    /** Searches for a text node with a numbering rule.
     in case a list style is found, <sListId> holds the list id, to which the
     text node belongs, which applies the found list style. */
    const SwNumRule * SearchNumRule(const bool bForward,
                                    const bool bNum,
                                    const bool bOutline,
                                    int nNonEmptyAllowed,
                                    String& sListId );

    /** Undo.
     Maintain UndoHistory in Document.
     Reset UndoHistory at Save, SaveAs, Create ??? */
    void DoUndo( sal_Bool bOn = sal_True );
    sal_Bool DoesUndo() const;
    void DoGroupUndo( sal_Bool bUn = sal_True );
    sal_Bool DoesGroupUndo() const;
    void DelAllUndoObj();

    /// Undo: set up Undo parenthesis, return nUndoId of this parenthesis.
    SwUndoId StartUndo( SwUndoId eUndoId = UNDO_EMPTY, const SwRewriter * pRewriter = 0 );

    /// Closes parenthesis of nUndoId, not used by UI.
    SwUndoId EndUndo( SwUndoId eUndoId = UNDO_EMPTY, const SwRewriter * pRewriter = 0 );

    bool     GetLastUndoInfo(OUString *const o_pStr,
                             SwUndoId *const o_pId) const;
    bool     GetFirstRedoInfo(OUString *const o_pStr) const;
    SwUndoId GetRepeatInfo(OUString *const o_pStr) const;

    /// is it forbidden to modify cursors via API calls?
    bool CursorsLocked() const;
    /// set selections to those contained in the UndoRedoContext
    /// should only be called by sw::UndoManager!
    void HandleUndoRedoContext(::sw::UndoRedoContext & rContext);

    bool Undo(sal_uInt16 const nCount = 1);
    bool Redo(sal_uInt16 const nCount = 1);
    bool Repeat(sal_uInt16 const nCount);


    /// For all views of this document.
    void StartAllAction();
    void EndAllAction();

    /// To enable set up of StartActions and EndActions.
    void CalcLayout();

    /// Determine form of content. Return Type at CurCrsr->SPoint.
    sal_uInt16 GetCntType() const;

    /// Are there frames, footnotes, etc.
    sal_Bool HasOtherCnt() const;

    /// Apply ViewOptions with Start-/EndAction.
    virtual void ApplyViewOptions( const SwViewOption &rOpt );

    /** Query text within selection.
     @returns FALSE, if selected range is too large to be copied
     into string buffer or if other errors occur. */
    sal_Bool GetSelectedText( String &rBuf,
                        int nHndlParaBreak = GETSELTXT_PARABRK_TO_BLANK );

    /** @return graphic, if CurCrsr->Point() points to a SwGrfNode
     (and mark is not set or points to the same graphic). */

    const Graphic* GetGraphic( sal_Bool bWait = sal_True ) const;
    const GraphicObject* GetGraphicObj() const;

    sal_Bool IsGrfSwapOut( sal_Bool bOnlyLinked = sal_False ) const;
    sal_uInt16 GetGraphicType() const;

    const PolyPolygon *GetGraphicPolygon() const;
    void SetGraphicPolygon( const PolyPolygon *pPoly );

    /** If there's an automatic, not manipulated polygon at the selected
     notxtnode, it has to be deleted, e.g. cause the object has changed. */
    void ClearAutomaticContour();

    /// @return the size of a graphic in Twips if cursor is in a graphic.
    sal_Bool GetGrfSize(Size&) const;

    /** @return name and filter of a graphic if the cursor is in a graphic,
     else give a rap on the knuckles!
     If a string-ptr != 0 return the respective name. */
    void GetGrfNms( String* pGrfName, String* pFltName,
                    const SwFlyFrmFmt* = 0 ) const;

    /// Re-read if graphic is not ok. Current graphic is replaced by the new one.
    void ReRead( const String& rGrfName, const String& rFltName,
                  const Graphic* pGraphic = 0,
                  const GraphicObject* pGrafObj = 0 );

    /// Unique identification of object (for ImageMapDlg).
    void    *GetIMapInventor() const;

    // #i73788#
    /// Remove default parameter, because method always called this default value.
    Graphic GetIMapGraphic() const; ///< @return a graphic for all Flys!
    const SwFlyFrmFmt* FindFlyByName( const String& rName, sal_uInt8 nNdTyp = 0 ) const;

    /** @return a ClientObject, if CurCrsr->Point() points to a SwOLENode
     (and mark is neither set not pointint to same ClientObject)
     else give rap on the knuckles. */
    svt::EmbeddedObjectRef&  GetOLEObject() const;

    /// Is there an OLEObject with this name (SwFmt)?
    sal_Bool HasOLEObj( const String &rName ) const;

    /// @return pointer to the data of the chart in which Cursr is.
    void SetChartName( const String &rName );

    /// Update content of all charts for table with given name.
    void UpdateCharts( const String &rName );

    String GetCurWord();

    /** Glossary from glossary document in current document.
     Styles only if not already existent. */
    void InsertGlossary( SwTextBlocks& rGlossary, const String& );

    /** Make current selection glossary and insert into glossary document
     including styles. */
    sal_uInt16 MakeGlossary( SwTextBlocks& rToFill, const String& rName,
                         const String& rShortName, sal_Bool bSaveRelFile = sal_False,
                         const String* pOnlyTxt=0 );

    /// Save complete content of doc as glossary.
    sal_uInt16 SaveGlossaryDoc( SwTextBlocks& rGlossary, const String& rName,
                            const String& rShortName,
                            sal_Bool bSaveRelFile = sal_False,
                            sal_Bool bOnlyTxt = sal_False );

    // Linguistics...
    /// Save selections.
    void HyphStart( SwDocPositions eStart, SwDocPositions eEnde );

    /// restore selections.
    void HyphEnd();
    com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                HyphContinue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    void HyphIgnore();

    /// For Inserting SoftHyphen. Position is offset within the syllabificated word.
    void InsertSoftHyph( const xub_StrLen nHyphPos );

    const SwTable& InsertTable( const SwInsertTableOptions& rInsTblOpts,  ///< ALL_TBL_INS_ATTR
                                sal_uInt16 nRows, sal_uInt16 nCols,
                                sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL,
                                const SwTableAutoFmt* pTAFmt = 0 );

    void InsertDDETable( const SwInsertTableOptions& rInsTblOpts,  ///< HEADLINE_NO_BORDER
                         SwDDEFieldType* pDDEType,
                         sal_uInt16 nRows, sal_uInt16 nCols,
                         sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL );

    void UpdateTable();
    void SetTableName( SwFrmFmt& rTblFmt, const String &rNewName );

    SwFrmFmt *GetTableFmt();
    sal_Bool TextToTable( const SwInsertTableOptions& rInsTblOpts,  ///< ALL_TBL_INS_ATTR
                      sal_Unicode cCh,
                      sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL,
                      const SwTableAutoFmt* pTAFmt = 0 );
    sal_Bool TableToText( sal_Unicode cCh );
    sal_Bool IsTextToTableAvailable() const;

    sal_Bool GetTblBoxFormulaAttrs( SfxItemSet& rSet ) const;
    void SetTblBoxFormulaAttrs( const SfxItemSet& rSet );

    sal_Bool IsTableBoxTextFormat() const;
    String GetTableBoxText() const;

    TblChgMode GetTblChgMode() const;
    void SetTblChgMode( TblChgMode eMode );

    /// Split table at cursor position.
    sal_Bool SplitTable( sal_uInt16 eMode );

    /** Merge tables.

     Can Merge checks if Prev or Next are possible.
        If pointer pChkNxtPrv is passed possible direction is given. */
    sal_Bool CanMergeTable( sal_Bool bWithPrev = sal_True, sal_Bool* pChkNxtPrv = 0 ) const;
    sal_Bool MergeTable( sal_Bool bWithPrev = sal_True, sal_uInt16 nMode = 0 );

    /// Set up InsertDB as table Undo.
    void AppendUndoForInsertFromDB( sal_Bool bIsTable );

    /// Functions used for spell checking and text conversion.

    /// Save selections.
    void SpellStart( SwDocPositions eStart, SwDocPositions eEnde,
                     SwDocPositions eCurr, SwConversionArgs *pConvArgs = 0 );

    /// Restore selections.
    void SpellEnd( SwConversionArgs *pConvArgs = 0, bool bRestoreSelection = true );
    ::com::sun::star::uno::Any SpellContinue(
                    sal_uInt16* pPageCnt, sal_uInt16* pPageSt,
                    SwConversionArgs *pConvArgs = 0 );

    /** Spells on a sentence basis - the SpellPortions are needed
     @return false if no error could be found. */
    bool SpellSentence(::svx::SpellPortions& rToFill, bool bIsGrammarCheck );

    /// Make SpellIter start with the current sentence when called next time.
    void PutSpellingToSentenceStart();

    /// Moves the continuation position to the end of the currently checked sentence.
    void MoveContinuationPosToEndOfCheckedSentence();

    /// Applies a changed sentence.
    void ApplyChangedSentence(const ::svx::SpellPortions& rNewPortions, bool bRecheck);

    /// Check SwSpellIter data to see if the last sentence got grammar checked.
    bool HasLastSentenceGotGrammarChecked() const;

    /// Is text conversion active somewhere else?
    sal_Bool HasConvIter() const;

    /// Is hyphenation active somewhere else?
    sal_Bool HasHyphIter() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
            GetCorrection( const Point* pPt, SwRect& rSelectRect );


    bool GetGrammarCorrection( ::com::sun::star::linguistic2::ProofreadingResult /*out*/ &rResult,
            sal_Int32 /*out*/ &rErrorPosInText,
            sal_Int32 /*out*/ &rErrorIndexInResult,
            ::com::sun::star::uno::Sequence< OUString > /*out*/ &rSuggestions,
            const Point* pPt, SwRect& rSelectRect );

    void IgnoreGrammarErrorAt( SwPaM& rErrorPosition );
    void SetLinguRange( SwDocPositions eStart, SwDocPositions eEnde );

    /// @return reference set in document according to given name.
    const SwFmtRefMark* GetRefMark( const String& rName ) const;

    /**  @return names of all references set in document.
      If ArrayPointer == 0 then return only whether a RefMark is set in document. */
    sal_uInt16 GetRefMarks( std::vector<OUString>* = 0 ) const;

    /// Call AutoCorrect
    void AutoCorrect( SvxAutoCorrect& rACorr, sal_Bool bInsertMode = sal_True,
                        sal_Unicode cChar = ' ' );
    sal_Bool GetPrevAutoCorrWord( SvxAutoCorrect& rACorr, String& rWord );

    /// Set our styles according to the respective rules.
    void AutoFormat( const SvxSwAutoFmtFlags* pAFlags = 0 );

    static SvxSwAutoFmtFlags* GetAutoFmtFlags();
    static void SetAutoFmtFlags(SvxSwAutoFmtFlags *);

    /// Calculates selection.
    String Calculate();

    sal_Bool InsertURL( const SwFmtINetFmt& rFmt, const OUString& rStr,
                    sal_Bool bKeepSelection = sal_False );
    sal_uInt16 GetINetAttrs( SwGetINetAttrs& rArr );

    String GetDropTxt( const sal_uInt16 nChars ) const;
    void   ReplaceDropTxt( const String &rStr, SwPaM* pPaM = NULL );

    /** May an outline be moved or copied?
     Check whether it's in text body, not in table, and not read-only (move). */
    sal_Bool IsOutlineMovable( sal_uInt16 nIdx ) const;
    sal_Bool IsOutlineCopyable( sal_uInt16 nIdx ) const;

    sal_uInt16 GetLineCount( sal_Bool bActPos = sal_True );

    /// Query and set footnote-text/number. Set.. to current SSelection!
    sal_Bool GetCurFtn( SwFmtFtn* pToFillFtn = 0 );
    bool SetCurFtn( const SwFmtFtn& rFillFtn );
    bool HasFtns( bool bEndNotes = false ) const;

    sal_uInt16 GetSeqFtnList( SwSeqFldList& rList, bool bEndNotes = false );
    /// @return list of all footnotes and their first portions of text.

    SwSection const* InsertSection(
            SwSectionData & rNewData, SfxItemSet const*const = 0 );
    sal_Bool IsInsRegionAvailable() const;
    const SwSection* GetCurrSection() const;

    /** @return current range like Cet CurrSection(). But this function iterates
     also over frames and catches the range even if the cursor is positioned in
     a footnote the reference of which is in a columned range.
     If bOutOfTab is set, the range comprising the table is searched
     and not an inner one. */
    const SwSection* GetAnySection( sal_Bool bOutOfTab = sal_False, const Point* pPt = 0 ) const;

    sal_uInt16 GetSectionFmtCount() const;
    sal_uInt16 GetSectionFmtPos( const SwSectionFmt& ) const;
    const SwSectionFmt& GetSectionFmt(sal_uInt16 nFmt) const;
    void DelSectionFmt( sal_uInt16 nFmt );
    void UpdateSection(sal_uInt16 const nSect, SwSectionData &,
            SfxItemSet const*const  = 0);
    sal_Bool IsAnySectionInDoc( sal_Bool bChkReadOnly = sal_False,
                            sal_Bool bChkHidden = sal_False,
                            sal_Bool BChkTOX = sal_False ) const;

    OUString GetUniqueSectionName( const OUString* pChkStr = 0 ) const;

    /// Set attributes.
    void SetSectionAttr(const SfxItemSet& rSet, SwSectionFmt* pSectFmt = 0);

    /** Search inside the cursor selection for full selected sections.
     if any part of section in the selection @return 0.
     if more than one in the selection return the count. */
    sal_uInt16 GetFullSelectedSectionCount() const;

    /** Special insert: Insert a new text node just before or after a section or
     table, if the cursor is positioned at the start/end of said
     section/table. The purpose of the method is to allow users to inert text
     at certain 'impossible' position, e.g. before a table at the document
     start or between to sections. */
    bool DoSpecialInsert();
    bool CanSpecialInsert() const;

    /// Optimizing UI.
    void SetNewDoc(sal_Bool bNew = sal_True);

    sfx2::LinkManager& GetLinkManager();
    inline const sfx2::LinkManager& GetLinkManager() const;

    /** Adjust left margin via object bar (similar to adjustment of numerations).
     One can either change the margin "by" adding or substracting a given
     offset or set it "to" this position @param (bModulus = true). */
    bool IsMoveLeftMargin( bool bRight = true, bool bModulus = true ) const;
    void MoveLeftMargin( bool bRight = true, bool bModulus = true );

    /// Query NumberFormater from document.
          SvNumberFormatter* GetNumberFormatter();
    const SvNumberFormatter* GetNumberFormatter() const
    {   return ((SwEditShell*)this)->GetNumberFormatter();  }

    /// Interfaces for GlobalDocument.
    sal_Bool IsGlobalDoc() const;
    void SetGlblDocSaveLinks( sal_Bool bFlag = sal_True );
    sal_Bool IsGlblDocSaveLinks() const;
    sal_uInt16 GetGlobalDocContent( SwGlblDocContents& rArr ) const;
    sal_Bool InsertGlobalDocContent( const SwGlblDocContent& rPos,
                                 SwSectionData & rNew );
    sal_Bool InsertGlobalDocContent( const SwGlblDocContent& rPos,
                                 const SwTOXBase& rTOX );
    sal_Bool InsertGlobalDocContent( const SwGlblDocContent& rPos );
    sal_Bool DeleteGlobalDocContent( const SwGlblDocContents& rArr,
                                sal_uInt16 nPos );
    sal_Bool MoveGlobalDocContent( const SwGlblDocContents& rArr ,
                                sal_uInt16 nFromPos, sal_uInt16 nToPos,
                                sal_uInt16 nNewPos );
    sal_Bool GotoGlobalDocContent( const SwGlblDocContent& rPos );

    /// For Redlining.
    sal_uInt16 GetRedlineMode() const;
    void SetRedlineMode( sal_uInt16 eMode );
    sal_Bool IsRedlineOn() const;
    sal_uInt16 GetRedlineCount() const;
    const SwRedline& GetRedline( sal_uInt16 nPos ) const;
    sal_Bool AcceptRedline( sal_uInt16 nPos );
    sal_Bool RejectRedline( sal_uInt16 nPos );


    /** Search Redline for this Data and @return position in array.
     If not found, return USHRT_MAX. */
    sal_uInt16 FindRedlineOfData( const SwRedlineData& ) const;


    /// Set comment to Redline at position.
    sal_Bool SetRedlineComment( const String& rS );
    const SwRedline* GetCurrRedline() const;

    /// Redline attributes have been changed. Updated views.
    void UpdateRedlineAttr();

    /// Compare two documents.
    long CompareDoc( const SwDoc& rDoc );

    /// Merge two documents.
    long MergeDoc( const SwDoc& rDoc );

    /// Footnote attributes global to document.
    const SwFtnInfo& GetFtnInfo() const;
    void  SetFtnInfo(const SwFtnInfo& rInfo);
    const SwEndNoteInfo& GetEndNoteInfo() const;
    void  SetEndNoteInfo(const SwEndNoteInfo& rInfo);

    const SwLineNumberInfo &GetLineNumberInfo() const;
    void  SetLineNumberInfo( const SwLineNumberInfo& rInfo);

    /// Labels: Synchronize ranges.
    void SetLabelDoc( sal_Bool bFlag = sal_True );
    sal_Bool IsLabelDoc() const;

    /// Interface for TextInputData - (for input of Japanese/Chinese chars.)
    SwExtTextInput* CreateExtTextInput(LanguageType eInputLanguage);
    String DeleteExtTextInput( SwExtTextInput* pDel = 0, sal_Bool bInsText = sal_True);
    void SetExtTextInputData( const CommandExtTextInputData& );

    /// Interface for access to AutoComplete-list.
    static SwAutoCompleteWord& GetAutoCompleteWords();

    /** @return a scaling factor of selected text. Used for the rotated
     character attribut dialog. */
    sal_uInt16 GetScalingOfSelectedText() const;

    /// Ctor/Dtor.
    SwEditShell( SwDoc&, Window*, const SwViewOption *pOpt = 0 );

    /// Copy-Constructor in disguise.
    SwEditShell( SwEditShell&, Window* );
    virtual ~SwEditShell();

private:
    /// For METWARE: no copying and no assignment.
    SwEditShell(const SwEditShell &);
    const SwEditShell &operator=(const SwEditShell &);
};

inline const sfx2::LinkManager& SwEditShell::GetLinkManager() const
{   return ((SwEditShell*)this)->GetLinkManager();  }

 /// Class for automated call of Start- and EndAction().
class SwActContext {
    SwEditShell *pSh;
public:
    SwActContext(SwEditShell *pShell);
    ~SwActContext();
};

 /// Class for automated call of Start- and EndCrsrMove().
class SwMvContext {
    SwEditShell *pSh;
public:
    SwMvContext(SwEditShell *pShell);
    ~SwMvContext();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
