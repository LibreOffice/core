/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _EDITSH_HXX
#define _EDITSH_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <tools/string.hxx>
#include <svl/svarray.hxx>
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
#include <swundo.hxx>
#include <svtools/embedhlp.hxx>

class PolyPolygon;
class SwDoc;
class DateTime;
class CommandExtTextInputData;

class SvUShortsSort;
class SvNumberFormatter;
class SfxPoolItem;
class SfxItemSet;
class SvxAutoCorrect;

class SwField;
class SwFieldType;
class SwDDEFieldType;
class SwNewDBMgr;

struct SwDocStat;
class SvStringsDtor;
class SvStringsSort;
class SwAutoCompleteWord;

class SwFmtRefMark;

class SwNumRule;

class SwUndoIds;
class SwTxtFmtColl;
class SwGrfNode;
class SwFlyFrmFmt;

class SwFrmFmt;         // For GetTxtNodeFmts().
class SwCharFmt;
class SwExtTextInput;
class SwRootFrm;        // For CTOR.
class Graphic;
class GraphicObject;
class SwFmtINetFmt;     // InsertURL.
class SwTable;
class SwTextBlocks;     // For GlossaryRW.
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

#define GETSELTXT_PARABRK_TO_BLANK      0
#define GETSELTXT_PARABRK_KEEP          1
#define GETSELTXT_PARABRK_TO_ONLYCR     2


// For querying the INet-attributes for Navigator.
struct SwGetINetAttr
{
    String sText;
    const SwTxtINetFmt& rINetAttr;

    SwGetINetAttr( const String& rTxt, const SwTxtINetFmt& rAttr )
        : sText( rTxt ), rINetAttr( rAttr )
    {}
};
SV_DECL_PTRARR_DEL( SwGetINetAttrs, SwGetINetAttr*, 0, 5 )

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

    // For the private methods DelRange and those of AutoCorrect.
    friend class SwAutoFormat;
    friend void _InitCore();
    friend void _FinitCore();
    // For the PamCorrAbs/-Rel methods.
    friend class SwUndo;

    SW_DLLPRIVATE SfxPoolItem& _GetChrFmt( SfxPoolItem& ) const;

    // Returns pointer to a SwGrfNode
    // that will be used by GetGraphic() and GetGraphicSize().
    SW_DLLPRIVATE SwGrfNode *_GetGrfNode() const ;

    SW_DLLPRIVATE void DeleteSel( SwPaM& rPam, BOOL* pUndo = 0 );

    SW_DLLPRIVATE void _SetSectionAttr( SwSectionFmt& rSectFmt, const SfxItemSet& rSet );

    using ViewShell::UpdateFlds;
    using SwModify::GetInfo;

public:
    // Edit (all selected ranges).
    void Insert( sal_Unicode, BOOL bOnlyCurrCrsr = FALSE );
    void Insert2( const String &, const bool bForceExpandHints = false );
    void Overwrite( const String & );

    // Replace a selected range in a TextNode by given string.
    // Meant for Search & Replace.
    // bRegExpRplc - replace tabs (\\t) and insert found string (not \&).
    // E.g.: Fnd: "zzz", Repl: "xx\t\\t..&..\&"
    //       --> "xx\t<Tab>..zzz..&"
    BOOL Replace( const String& rNewStr, BOOL bRegExpRplc = FALSE );

    // Delete content of all ranges.
    // If whole nodes are selected, these nodes get deleted.
    long Delete();

    // Remove a complete paragraph.
    BOOL DelFullPara();

    // Change text to Upper/Lower/Hiragana/Katagana/...
    void TransliterateText( sal_uInt32 nType );

    // Count words in current selection.
    void CountWords( SwDocStat& rStat ) const;

    // Delete non-visible content in the document.
    // E.g. hidden ranges, hidden paragraphs.
    BOOL RemoveInvisibleContent();

    // Replace fields by text - mailmerge support.
    BOOL ConvertFieldsToText();

    // Set all numbering start points to a fixed value - mailmerge support.
    void SetNumberingRestart();

    // Embedds all local links (ranges/graphics).
    USHORT GetLinkUpdMode(BOOL bDocSettings = FALSE) const;
    void SetLinkUpdMode( USHORT nMode );

    // Copy content of all ranges at current position of cursor to given Shell.
    long Copy( SwEditShell* pDestShell = 0 );

    // For copying via ClipBoard:
    //   If table is copied into table, move all cursors away from it.
    //   Copy and Paste must be in FEShell because of FlyFrames!
    //   Copy all selections and the document.
    BOOL _CopySelToDoc( SwDoc* pInsDoc, SwNodeIndex* pNdInsPos = 0 );

    long SplitNode( BOOL bAutoFormat = FALSE, BOOL bCheckTableStart = TRUE );
    sal_Bool AppendTxtNode();
    void AutoFmtBySplitNode();

    // If cursor is in a INetAttribute it will be deleted completely
    // including the descriptive text (needed at drag & drop).
    BOOL DelINetAttrWithText();

    // If Cursor is at the end of a character style in which the DontExpand-flag
       is not yet set, the latter will be set (==> return TRUE).
    BOOL DontExpandFmt();

    // Apply / remove attributes.
    // Returns attributs in required AttributeSet.
    // When not unambiguous the set has a DONT_CARE !!
    // 2nd optional parameter <bMergeIndentValuesOfNumRule>.
    // If <bMergeIndentValuesOfNumRule> == TRUE, the indent attributes of
    // the corresponding list level of an applied list style is merged into
    // the requested item set as a LR-SPACE item, if corresponding node has not
    // its own indent attributes and the position-and-space mode of the list
    // level is SvxNumberFormat::LABEL_ALIGNMENT.
    BOOL GetCurAttr( SfxItemSet& ,
                     const bool bMergeIndentValuesOfNumRule = false ) const;
    void SetAttr( const SfxPoolItem&, USHORT nFlags = 0 );
    void SetAttr( const SfxItemSet&, USHORT nFlags = 0 );

    // Set attribute as new default attribute in document.
    void SetDefault( const SfxPoolItem& );

    // Query default attribute of document.
    const SfxPoolItem& GetDefault( USHORT nFmtHint ) const;

    void ResetAttr( const SvUShortsSort* pAttrs = 0 );
    void GCAttr();

    // Returns the scripttpye of the selection.
    USHORT GetScriptType() const;

    // Returns the language at current cursor position.
    USHORT GetCurLang() const;

    // TABLE
    USHORT GetTblFrmFmtCount( BOOL bUsed = FALSE ) const;
    SwFrmFmt& GetTblFrmFmt(USHORT nFmt, BOOL bUsed = FALSE ) const;
    String GetUniqueTblName() const;

    // CHAR
    USHORT GetCharFmtCount() const;
    SwCharFmt& GetCharFmt(USHORT nFmt) const;
    SwCharFmt* GetCurCharFmt() const;
    void FillByEx(SwCharFmt*, BOOL bReset = FALSE);
    SwCharFmt* MakeCharFmt( const String& rName, SwCharFmt* pDerivedFrom = 0 );
    SwCharFmt* FindCharFmtByName( const String& rName ) const;


    /* FormatCollections (new) - Explaining the general naming pattern:
     * GetXXXCount() returns the count of xxx in the document.
     * GetXXX(i)     returns i-th xxx (ERR_RAISE if beyond range!).
     * DelXXX(i)     delets i-th xxx  (ERR_RAISE if beyond range!).
     * GetCurXXX()   returns xxx that is valid at cursor or in ranges.
     *               returns 0, if not unanimuous.
     * SetXXX()      sets xxx at cursor or in ranges.
     * MakeXXX()     makes a xxx, derived from pDerivedFrom.
     */

    // TXT
    SwTxtFmtColl& GetDfltTxtFmtColl() const;
    USHORT GetTxtFmtCollCount() const;
    SwTxtFmtColl& GetTxtFmtColl( USHORT nTxtFmtColl) const;
    SwTxtFmtColl* GetCurTxtFmtColl() const;

    // #i62675#
    // Add 2nd optional parameter <bResetListAttrs> - see also <SwDoc::SetTxtFmtColl(..)>
    void SetTxtFmtColl( SwTxtFmtColl*,
                        bool bResetListAttrs = false );
    SwTxtFmtColl *MakeTxtFmtColl(const String &rFmtCollName,
        SwTxtFmtColl *pDerivedFrom = 0);
    void FillByEx(SwTxtFmtColl*, BOOL bReset = FALSE);
    SwTxtFmtColl* FindTxtFmtCollByName( const String& rName ) const;

    // Return "Auto-Collection" with given Id. If it does not exist create it.
    SwTxtFmtColl* GetTxtCollFromPool( USHORT nId );

    // Return required automatic format base class.
    SwFmt* GetFmtFromPool( USHORT nId );

    // Return required automatic page style.
    SwPageDesc* GetPageDescFromPool( USHORT nId );

    // Query if the paragraph-/character-/frame-/page-style is used.
    BOOL IsUsed( const SwModify& ) const;

    // Return required automatic format.
    SwFrmFmt* GetFrmFmtFromPool( USHORT nId )
        { return (SwFrmFmt*)SwEditShell::GetFmtFromPool( nId ); }
    SwCharFmt* GetCharFmtFromPool( USHORT nId )
        { return (SwCharFmt*)SwEditShell::GetFmtFromPool( nId ); }

    void Insert2(SwField&, const bool bForceExpandHints = false);
    SwField* GetCurFld() const;

    void UpdateFlds( SwField & );   // One single field.

    USHORT GetFldTypeCount(USHORT nResId = USHRT_MAX, BOOL bUsed = FALSE) const;
    SwFieldType* GetFldType(USHORT nId, USHORT nResId = USHRT_MAX, BOOL bUsed = FALSE) const;
    SwFieldType* GetFldType(USHORT nResId, const String& rName) const;

    void RemoveFldType(USHORT nId, USHORT nResId = USHRT_MAX);
    void RemoveFldType(USHORT nResId, const String& rName);

    void FieldToText( SwFieldType* pType );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    // Database information.
    SwDBData GetDBData() const;
    const SwDBData& GetDBDesc() const;
    void ChgDBData(const SwDBData& SwDBData);
    void ChangeDBFields( const SvStringsDtor& rOldNames,
                         const String& rNewName );
    void GetAllUsedDB( SvStringsDtor& rDBNameList,
                        SvStringsDtor* pAllDBNames = 0 );

    BOOL IsAnyDatabaseFieldInDoc()const;

    // Check whether DB fields point to an available data source and returns it.
    BOOL IsFieldDataSourceAvailable(String& rUsedDataSource) const;
    void UpdateExpFlds(BOOL bCloseDB = FALSE);// nur alle ExpressionFelder updaten
    void SetFixFields( BOOL bOnlyTimeDate = FALSE,
                        const DateTime* pNewDateTime = 0 );
    void LockExpFlds();
    void UnlockExpFlds();

    SwFldUpdateFlags GetFldUpdateFlags(BOOL bDocSettings = FALSE) const;
    void SetFldUpdateFlags( SwFldUpdateFlags eFlags );

    // For evaluation of DB fields (new DB-manager).
    SwNewDBMgr* GetNewDBMgr() const;

    SwFieldType* InsertFldType(const SwFieldType &);

    // Changes in document?
    BOOL IsModified() const;
    void SetModified();
    void ResetModified();
    void SetUndoNoResetModified();

    // Document - Statistics
    void UpdateDocStat( SwDocStat& rStat );

    void    Insert(const SwTOXMark& rMark);

    void    DeleteTOXMark(SwTOXMark* pMark);

    // Get all marks at current SPoint.
    USHORT  GetCurTOXMarks(SwTOXMarks& rMarks) const ;

    // Insert content table. Renew if required.
    void                InsertTableOf(const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet = 0);
    BOOL                UpdateTableOf(const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet = 0);
    const SwTOXBase*    GetCurTOX() const;
    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, BOOL bCreate = FALSE );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    BOOL                IsTOXBaseReadonly(const SwTOXBase& rTOXBase) const;
    void                SetTOXBaseReadonly(const SwTOXBase& rTOXBase, BOOL bReadonly);

    USHORT              GetTOXCount() const;
    const SwTOXBase*    GetTOX( USHORT nPos ) const;
    BOOL                DeleteTOX( const SwTOXBase& rTOXBase, BOOL bDelNodes = FALSE );

    // After reading file update all content tables.
    void SetUpdateTOX( BOOL bFlag = TRUE );
    BOOL IsUpdateTOX() const;

    // Manage types of content tables.
    USHORT              GetTOXTypeCount(TOXTypes eTyp) const;
    const SwTOXType*    GetTOXType(TOXTypes eTyp, USHORT nId) const;
    void                InsertTOXType(const SwTOXType& rTyp);

    // AutoMark file
    const String&   GetTOIAutoMarkURL() const;
    void            SetTOIAutoMarkURL(const String& rSet);
    void            ApplyAutoMark();

    // Key for managing index.
    USHORT GetTOIKeys( SwTOIKeyType eTyp, SvStringsSort& rArr ) const;

    void SetOutlineNumRule(const SwNumRule&);
    const SwNumRule* GetOutlineNumRule() const;

    BOOL OutlineUpDown( short nOffset = 1 );

    BOOL MoveOutlinePara( short nOffset = 1);

    BOOL IsProtectedOutlinePara() const;

    // Returns numbering rulse pf current enumeration list (else FALSE).
    const SwNumRule* GetCurNumRule() const;

    // If there is not already a numbering, set one, else change.
    // Works with old and new rules. Update only differences.
    // Add optional parameter <bResetIndentAttrs> (default value FALSE).
    // If <bResetIndentAttrs> equals true, the indent attributes "before text"
    // and "first line indent" are additionally reset at the current selection,
    // if the list style makes use of the new list level attributes.
    // introduce parameters <bCreateNewList> and <sContinuedListId>
    // <bCreateNewList> indicates, if a new list is created by applying the
    // given list style.
    // If <bCreateNewList> equals FALSE, <sContinuedListId> may contain the
    // list Id of a list, which has to be continued by applying the given list style.
    void SetCurNumRule( const SwNumRule&,
                        const bool bCreateNewList /*= false*/,
                        const String sContinuedListId = String(),
                        const bool bResetIndentAttrs = false );

    // Paragraphs without enumeration but with indents.
    BOOL NoNum();

    // Delete, split enumeration list.
    void DelNumRules();

    BOOL NumUpDown( BOOL bDown = TRUE );

    BOOL MoveParagraph( long nOffset = 1);
    BOOL MoveNumParas( BOOL bUpperLower, BOOL bUpperLeft );

    // Switch on/off of numbering via Delete/Backspace.
    BOOL NumOrNoNum( BOOL bDelete = FALSE, BOOL bChkStart = TRUE);

    // #i23726#
    // #i90078#
    // Remove unused default parameter <nLevel> and <bRelative>.
    // Adjust method name and parameter name
    void ChangeIndentOfAllListLevels( short nDiff );
    // Adjust method name
    void SetIndent(short nIndent, const SwPosition & rPos);
    BOOL IsFirstOfNumRule() const;
    BOOL IsFirstOfNumRule(const SwPaM & rPaM) const;

    BOOL IsNoNum( BOOL bChkStart = TRUE ) const;

    // Return Num-Level of the node in which point of cursor is.
    // Return values can be: NO_NUMBERING,
    // 0..MAXLEVEL-1, NO_NUMLEVEL .. NO_NUMLEVEL|MAXLEVEL-1
    BYTE GetNumLevel() const;

    // Detect highest and lowest level to check moving of outline levels.
    void GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower );


    // Get Outline level of current paragraph.
    int GetCurrentParaOutlineLevel( ) const;// #outlinelevel add by zhaojianwei

    // i29560
    BOOL HasNumber() const;
    BOOL HasBullet() const;

    String GetUniqueNumRuleName( const String* pChkStr = 0, BOOL bAutoNum = TRUE ) const;
    void ChgNumRuleFmts( const SwNumRule& rRule );

    // Set (and query if) a numbering with StartFlag starts at current PointPos.
    void SetNumRuleStart( BOOL bFlag = TRUE );
    BOOL IsNumRuleStart() const;
    void SetNodeNumStart( USHORT nStt );

    USHORT GetNodeNumStart() const;

    BOOL ReplaceNumRule( const String& rOldRule, const String& rNewRule );

    // Searches for a text node with a numbering rule.
    // in case a list style is found, <sListId> holds the list id, to which the
    // text node belongs, which applies the found list style.
    const SwNumRule * SearchNumRule(const bool bForward,
                                    const bool bNum,
                                    const bool bOutline,
                                    int nNonEmptyAllowed,
                                    String& sListId );

    // Undo.
    // Maintain UndoHistory in Document.
    // Reset UndoHistory at Save, SaveAs, Create ???
    void DoUndo( BOOL bOn = TRUE );
    BOOL DoesUndo() const;
    void DoGroupUndo( BOOL bUn = TRUE );
    BOOL DoesGroupUndo() const;
    void DelAllUndoObj();

    // Undo: set up Undo parenthesis, return nUndoId of this parenthesis.
    SwUndoId StartUndo( SwUndoId eUndoId = UNDO_EMPTY, const SwRewriter * pRewriter = 0 );

    // Closes parenthesis of nUndoId, not used by UI.
    SwUndoId EndUndo( SwUndoId eUndoId = UNDO_EMPTY, const SwRewriter * pRewriter = 0 );

    // Returns Id of last undo-able action. Fills VARARR with User-UndoIds if required.
    SwUndoId GetUndoIds( String* pUndoStr = 0, SwUndoIds *pUndoIds = 0) const;
    String GetUndoIdsStr( String* pUndoStr = 0, SwUndoIds *pUndoIds = 0) const;

    // Get count of possible undo-actions.
    static USHORT GetUndoActionCount();

    // Query count of possible undo-actions.
    static void SetUndoActionCount( USHORT nNew );

    // Redo.
    // Returns Id of las undo-able action. Fills VARARR with RedoIds if required.
    SwUndoId GetRedoIds( String* pRedoStr = 0, SwUndoIds *pRedoIds = 0) const;
    String GetRedoIdsStr( String* pRedoStr = 0, SwUndoIds *pRedoIds = 0) const;

    // Repeat.
    // Returns Id of last repeatable action. Fills VARARR with RedoIds if required.
    SwUndoId GetRepeatIds( String* pRepeatStr = 0, SwUndoIds *pRedoIds = 0) const;
    String GetRepeatIdsStr( String* pRepeatStr = 0,
                            SwUndoIds *pRedoIds = 0) const;


    // 0 if last action, else actions until start of parenthesis nUndoId
    // with  KillPaMs, ClearMark.
    BOOL Undo(SwUndoId nUndoId = UNDO_EMPTY, USHORT nCnt = 1 );

    USHORT Repeat( USHORT nCount );

    USHORT Redo( USHORT nCnt = 1 );

    // For all views of this document.
    void StartAllAction();
    void EndAllAction();

    // To enable set up of StartActions and EndActions.
    void CalcLayout();

    // Determine form of content. Return Type at CurCrsr->SPoint.
    USHORT GetCntType() const;

    // Are there frames, footnotes, etc.
    BOOL HasOtherCnt() const;

    // Apply ViewOptions with Start-/EndAction.
    inline void ApplyViewOptions( const SwViewOption &rOpt );

    // Query text within selection.
    // Returns FALSE, if selected range is too large to be copied
    // into string buffer or if other errors occur.
    BOOL GetSelectedText( String &rBuf,
                        int nHndlParaBreak = GETSELTXT_PARABRK_TO_BLANK );

    // Returns graphic, if CurCrsr->Point() points to a SwGrfNode
    // (and mark is not set or points to the same graphic).

    const Graphic* GetGraphic( BOOL bWait = TRUE ) const;
    const GraphicObject* GetGraphicObj() const;

    BOOL IsGrfSwapOut( BOOL bOnlyLinked = FALSE ) const;
    USHORT GetGraphicType() const;

    const PolyPolygon *GetGraphicPolygon() const;
    void SetGraphicPolygon( const PolyPolygon *pPoly );

    // If there's an automatic, not manipulated polygon at the selected
        notxtnode, it has to be deleted, e.g. cause the object has changed.
    void ClearAutomaticContour();

    // Returns the size of a graphic in Twips if cursor is in a graphic.
    BOOL GetGrfSize(Size&) const;

    // Returns name and filter of a graphic if the cursor is in a graphic,
    // else give a rap on the knuckles!
    // If a string-ptr != 0 return the respective name.
    void GetGrfNms( String* pGrfName, String* pFltName,
                    const SwFlyFrmFmt* = 0 ) const;

    // Re-read if graphic is not ok. Current graphic is replaced by the new one.
    void ReRead( const String& rGrfName, const String& rFltName,
                  const Graphic* pGraphic = 0,
                  const GraphicObject* pGrafObj = 0 );

    // Unique identification of object (for ImageMapDlg).
    void    *GetIMapInventor() const;

    // #i73788#
    // Remove default parameter, because method always called this default value.
    Graphic GetIMapGraphic() const; // Returns a graphic for all Flys!

    const SwFlyFrmFmt* FindFlyByName( const String& rName, BYTE nNdTyp = 0 ) const;

    // Returns a ClientObject, if CurCrsr->Point() points to a SwOLENode
    // (and mark is neither set not pointint to same ClientObject)
    // else give rap on the knuckles.
    svt::EmbeddedObjectRef&  GetOLEObject() const;

    // Is there an OLEObject with this name (SwFmt)?
    BOOL HasOLEObj( const String &rName ) const;

    // Returns pointer to the data of the chart in which Cursr is.
    void SetChartName( const String &rName );

    // Update content of all charts for table with given name.
    void UpdateCharts( const String &rName );

    String GetCurWord();

    // Glossary from glossary document in current document.
    // Styles only if not already existent.
    void InsertGlossary( SwTextBlocks& rGlossary, const String& );

    // Make current selection glossary and insert into glossary document
    // including styles.
    USHORT MakeGlossary( SwTextBlocks& rToFill, const String& rName,
                         const String& rShortName, BOOL bSaveRelFile = FALSE,
                         const String* pOnlyTxt=0 );

    // Save complete content of doc as glossary.
    USHORT SaveGlossaryDoc( SwTextBlocks& rGlossary, const String& rName,
                            const String& rShortName,
                            BOOL bSaveRelFile = FALSE,
                            BOOL bOnlyTxt = FALSE );

    // Linguistics...
    // Save selections.
    void HyphStart( SwDocPositions eStart, SwDocPositions eEnde );

    // restore selections.
    void HyphEnd();
    com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                HyphContinue( USHORT* pPageCnt, USHORT* pPageSt );

    void HyphIgnore();

    // For Inserting SoftHyphen. Position is offset within the syllabificated word.
    void InsertSoftHyph( const xub_StrLen nHyphPos );

    const SwTable& InsertTable( const SwInsertTableOptions& rInsTblOpts,  // ALL_TBL_INS_ATTR
                                USHORT nRows, USHORT nCols,
                                sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL,
                                const SwTableAutoFmt* pTAFmt = 0 );

    void InsertDDETable( const SwInsertTableOptions& rInsTblOpts,  // HEADLINE_NO_BORDER
                         SwDDEFieldType* pDDEType,
                         USHORT nRows, USHORT nCols,
                         sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL );

    void UpdateTable();
    void SetTableName( SwFrmFmt& rTblFmt, const String &rNewName );

    SwFrmFmt *GetTableFmt();
    BOOL TextToTable( const SwInsertTableOptions& rInsTblOpts,  //ALL_TBL_INS_ATTR
                      sal_Unicode cCh,
                      sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL,
                      const SwTableAutoFmt* pTAFmt = 0 );
    BOOL TableToText( sal_Unicode cCh );
    BOOL IsTextToTableAvailable() const;

    BOOL GetTblBoxFormulaAttrs( SfxItemSet& rSet ) const;
    void SetTblBoxFormulaAttrs( const SfxItemSet& rSet );

    BOOL IsTableBoxTextFormat() const;
    String GetTableBoxText() const;

    TblChgMode GetTblChgMode() const;
    void SetTblChgMode( TblChgMode eMode );

    // Split table at cursor position.
    BOOL SplitTable( USHORT eMode );

    // Merge tables.

    // Can Merge checks if Prev or Next are possible.
    //    If pointer pChkNxtPrv is passed possible direction is given.
    BOOL CanMergeTable( BOOL bWithPrev = TRUE, BOOL* pChkNxtPrv = 0 ) const;
    BOOL MergeTable( BOOL bWithPrev = TRUE, USHORT nMode = 0 );

    // Set up InsertDB as table Undo.
    void AppendUndoForInsertFromDB( BOOL bIsTable );

    // Functions used for spell checking and text conversion.

    // Save selections.
    void SpellStart( SwDocPositions eStart, SwDocPositions eEnde,
                     SwDocPositions eCurr, SwConversionArgs *pConvArgs = 0 );

    // Restore selections.
    void SpellEnd( SwConversionArgs *pConvArgs = 0, bool bRestoreSelection = true );
    ::com::sun::star::uno::Any SpellContinue(
                    USHORT* pPageCnt, USHORT* pPageSt,
                    SwConversionArgs *pConvArgs = 0 );

    // Spells on a sentence basis - the SpellPortions are needed
    // returns false if no error could be found.
    bool SpellSentence(::svx::SpellPortions& rToFill, bool bIsGrammarCheck );

    // Make SpellIter start with the current sentence when called next time.
    void PutSpellingToSentenceStart();

    // Moves the continuation position to the end of the currently checked sentence.
    void MoveContinuationPosToEndOfCheckedSentence();

    // Applies a changed sentence.
    void ApplyChangedSentence(const ::svx::SpellPortions& rNewPortions, bool bRecheck);

    // Check SwSpellIter data to see if the last sentence got grammar checked.
    bool HasLastSentenceGotGrammarChecked() const;

    // Is text conversion active somewhere else?
    BOOL HasConvIter() const;

    // Is hyphenation active somewhere else?
    BOOL HasHyphIter() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
            GetCorrection( const Point* pPt, SwRect& rSelectRect );


    bool GetGrammarCorrection( ::com::sun::star::linguistic2::ProofreadingResult /*out*/ &rResult,
            sal_Int32 /*out*/ &rErrorPosInText,
            sal_Int32 /*out*/ &rErrorIndexInResult,
            ::com::sun::star::uno::Sequence< rtl::OUString > /*out*/ &rSuggestions,
            const Point* pPt, SwRect& rSelectRect );

    void IgnoreGrammarErrorAt( SwPaM& rErrorPosition );
    void SetLinguRange( SwDocPositions eStart, SwDocPositions eEnde );

    // Return reference set in document according to given name.
    const SwFmtRefMark* GetRefMark( const String& rName ) const;

    //  Return names of all references set in document.
    //  If ArrayPointer == 0 then return only whether a RefMark is set in document.
    USHORT GetRefMarks( SvStringsDtor* = 0 ) const;

    // Call AutoCorrect
    void AutoCorrect( SvxAutoCorrect& rACorr, BOOL bInsertMode = TRUE,
                        sal_Unicode cChar = ' ' );
    BOOL GetPrevAutoCorrWord( SvxAutoCorrect& rACorr, String& rWord );

    // Set our styles according to the respective rules.
    void AutoFormat( const SvxSwAutoFmtFlags* pAFlags = 0 );

    static SvxSwAutoFmtFlags* GetAutoFmtFlags();
    static void SetAutoFmtFlags(SvxSwAutoFmtFlags *);

    // Calculates selection.
    String Calculate();

    BOOL InsertURL( const SwFmtINetFmt& rFmt, const String& rStr,
                    BOOL bKeepSelection = FALSE );
    USHORT GetINetAttrs( SwGetINetAttrs& rArr );

    String GetDropTxt( const USHORT nChars ) const;
    void   ReplaceDropTxt( const String &rStr );

    // May an outline be moved or copied?
    // Check whether it's in text body, not in table, and not read-only (move).
    BOOL IsOutlineMovable( USHORT nIdx ) const;
    BOOL IsOutlineCopyable( USHORT nIdx ) const;

    USHORT GetLineCount( BOOL bActPos = TRUE );

    // Query and set footnote-text/number. Set.. to current SSelection!
    BOOL GetCurFtn( SwFmtFtn* pToFillFtn = 0 );
    bool SetCurFtn( const SwFmtFtn& rFillFtn );
    bool HasFtns( bool bEndNotes = false ) const;

    // USHORT GetFtnCnt( BOOL bEndNotes = FALSE ) const; //Currently not required.

    // Return list of all footnotes and their first portions of text.
    USHORT GetSeqFtnList( SwSeqFldList& rList, bool bEndNotes = false );

    SwSection const* InsertSection(
            SwSectionData & rNewData, SfxItemSet const*const = 0 );
    BOOL IsInsRegionAvailable() const;
    const SwSection* GetCurrSection() const;

    // Returns current range like Cet CurrSection(). But this function iterates
    // also over frames and catches the range even if the cursor is positioned in
    // a footnote the reference of which is in a columned range.
    // If bOutOfTab is set, the range comprising the table is searched
    // and not an inner one.
    const SwSection* GetAnySection( BOOL bOutOfTab = FALSE, const Point* pPt = 0 ) const;

    USHORT GetSectionFmtCount() const;
    USHORT GetSectionFmtPos( const SwSectionFmt& ) const;
    const SwSectionFmt& GetSectionFmt(USHORT nFmt) const;
    void DelSectionFmt( USHORT nFmt );
    void UpdateSection(sal_uInt16 const nSect, SwSectionData &,
            SfxItemSet const*const  = 0);
    BOOL IsAnySectionInDoc( BOOL bChkReadOnly = FALSE,
                            BOOL bChkHidden = FALSE,
                            BOOL BChkTOX = FALSE ) const;

    String GetUniqueSectionName( const String* pChkStr = 0 ) const;

    // Set attributes.
    void SetSectionAttr(const SfxItemSet& rSet, SwSectionFmt* pSectFmt = 0);

    // Search inside the cursor selection for full selected sections.
    // if any part of section in the selection return 0.
    // if more than one in the selection return the count.
    USHORT GetFullSelectedSectionCount() const;

    // Special insert: Insert a new text node just before or after a section or
    // table, if the cursor is positioned at the start/end of said
    // section/table. The purpose of the method is to allow users to inert text
    // at certain 'impossible' position, e.g. before a table at the document
    // start or between to sections.
    bool DoSpecialInsert();
    bool CanSpecialInsert() const;

    // Optimizing UI.
    void SetNewDoc(BOOL bNew = TRUE);

    sfx2::LinkManager& GetLinkManager();
    inline const sfx2::LinkManager& GetLinkManager() const;

    // linken Rand ueber Objectleiste einstellen (aenhlich dem Stufen von
    // Numerierungen), optional kann man "um" den Offset stufen oder "auf"
    // die Position gestuft werden (bModulus = TRUE)
    BOOL IsMoveLeftMargin( BOOL bRight = TRUE, BOOL bModulus = TRUE ) const;
    void MoveLeftMargin( BOOL bRight = TRUE, BOOL bModulus = TRUE );

    // Query NumberFormater from document.
          SvNumberFormatter* GetNumberFormatter();
    const SvNumberFormatter* GetNumberFormatter() const
    {   return ((SwEditShell*)this)->GetNumberFormatter();  }

    // Interfaces for GlobalDocument.
    BOOL IsGlobalDoc() const;
    void SetGlblDocSaveLinks( BOOL bFlag = TRUE );
    BOOL IsGlblDocSaveLinks() const;
    USHORT GetGlobalDocContent( SwGlblDocContents& rArr ) const;
    BOOL InsertGlobalDocContent( const SwGlblDocContent& rPos,
                                 SwSectionData & rNew );
    BOOL InsertGlobalDocContent( const SwGlblDocContent& rPos,
                                 const SwTOXBase& rTOX );
    BOOL InsertGlobalDocContent( const SwGlblDocContent& rPos );
    BOOL DeleteGlobalDocContent( const SwGlblDocContents& rArr,
                                USHORT nPos );
    BOOL MoveGlobalDocContent( const SwGlblDocContents& rArr ,
                                USHORT nFromPos, USHORT nToPos,
                                USHORT nNewPos );
    BOOL GotoGlobalDocContent( const SwGlblDocContent& rPos );

    // For Redlining.
    USHORT GetRedlineMode() const;
    void SetRedlineMode( USHORT eMode );
    BOOL IsRedlineOn() const;
    USHORT GetRedlineCount() const;
    const SwRedline& GetRedline( USHORT nPos ) const;
    BOOL AcceptRedline( USHORT nPos );
    BOOL RejectRedline( USHORT nPos );


    // Search Redline for this Data and return position in array.
    // If not found, return USHRT_MAX.
    USHORT FindRedlineOfData( const SwRedlineData& ) const;

    // Set comment to Redline at position.
    BOOL SetRedlineComment( const String& rS );
    const SwRedline* GetCurrRedline() const;

    // Redline attributes have been changed. Updated views.
    void UpdateRedlineAttr();

    // Compare two documents.
    long CompareDoc( const SwDoc& rDoc );

    // Merge two documents.
    long MergeDoc( const SwDoc& rDoc );

    // Footnote attributes global to document.
    const SwFtnInfo& GetFtnInfo() const;
    void  SetFtnInfo(const SwFtnInfo& rInfo);
    const SwEndNoteInfo& GetEndNoteInfo() const;
    void  SetEndNoteInfo(const SwEndNoteInfo& rInfo);

    const SwLineNumberInfo &GetLineNumberInfo() const;
    void  SetLineNumberInfo( const SwLineNumberInfo& rInfo);

    // Labels: Synchronize ranges.
    void SetLabelDoc( BOOL bFlag = TRUE );
    BOOL IsLabelDoc() const;

    // Interface for TextInputData - (for input of Japanese/Chinese chars.)
    SwExtTextInput* CreateExtTextInput(LanguageType eInputLanguage);
    String DeleteExtTextInput( SwExtTextInput* pDel = 0, BOOL bInsText = TRUE);
    void SetExtTextInputData( const CommandExtTextInputData& );

    // Interface for access to AutoComplete-list.
    static SwAutoCompleteWord& GetAutoCompleteWords();

    // Returns a scaling factor of selected text. Used for the rotated
    // character attribut dialog.
    USHORT GetScalingOfSelectedText() const;

    // Ctor/Dtor.
    SwEditShell( SwDoc&, Window*, const SwViewOption *pOpt = 0 );

    // Copy-Constructor in disguise.
    SwEditShell( SwEditShell&, Window* );
    virtual ~SwEditShell();

private:
    // For METWARE: no copying and no assignment.
    SwEditShell(const SwEditShell &);
    const SwEditShell &operator=(const SwEditShell &);
};

inline void SwEditShell::ApplyViewOptions( const SwViewOption &rOpt )
{
    SwCrsrShell::StartAction();
    ViewShell::ApplyViewOptions( rOpt );
    SwEditShell::EndAction();
}

inline const sfx2::LinkManager& SwEditShell::GetLinkManager() const
{   return ((SwEditShell*)this)->GetLinkManager();  }

// Class for automated call of Start- and EndAction().
class SwActKontext {
    SwEditShell *pSh;
public:
    SwActKontext(SwEditShell *pShell);
    ~SwActKontext();
};

#define ACT_KONTEXT(x)  SwActKontext _aActKontext_(x)

// Class for automated call of Start- and EndCrsrMove().
class SwMvKontext {
    SwEditShell *pSh;
public:
    SwMvKontext(SwEditShell *pShell );
    ~SwMvKontext();
};

#define MV_KONTEXT(x)   SwMvKontext _aMvKontext_(x)



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
