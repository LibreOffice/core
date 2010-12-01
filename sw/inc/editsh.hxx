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
#include <crsrsh.hxx>   // fuer Basisklasse
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

class SwField;          // fuer Felder
class SwFieldType;
class SwDDEFieldType;
class SwNewDBMgr;

struct SwDocStat;
class SvStringsDtor;
class SvStringsSort;
class SwAutoCompleteWord;

class SwFmtRefMark;

class SwNumRule;        // Numerierung
//class SwNodeNum;      // Numerierung

class SwUndoIds;        // fuer Undo
class SwTxtFmtColl;
class SwGrfNode;
class SwFlyFrmFmt;

class SwFrmFmt;         // fuer GetTxtNodeFmts()
class SwCharFmt;
class SwExtTextInput;
class SwRootFrm;        // fuer CTOR
class Graphic;          // fuer GetGraphic
class GraphicObject;    // fuer GetGraphicObj
class SwFmtINetFmt;     // InsertURL
class SwTable;
class SwTextBlocks;     // fuer GlossaryRW
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

/****************************************************************
 *  zum Abfragen der INet-Attribute fuer den Navigator
 ****************************************************************/
struct SwGetINetAttr
{
    String sText;
    const SwTxtINetFmt& rINetAttr;

    SwGetINetAttr( const String& rTxt, const SwTxtINetFmt& rAttr )
        : sText( rTxt ), rINetAttr( rAttr )
    {}
};
SV_DECL_PTRARR_DEL( SwGetINetAttrs, SwGetINetAttr*, 0, 5 )

/****************************************************************
 *  Typen der Inhaltsformen
 ****************************************************************/
#define CNT_TXT 0x0001
#define CNT_GRF 0x0002
#define CNT_OLE 0x0010
/* Teste einen USHORT auf eine bestimmte Inhaltsform */
#define CNT_HasTxt(USH) ((USH)&CNT_TXT)
#define CNT_HasGrf(USH) ((USH)&CNT_GRF)
#define CNT_HasOLE(USH) ((USH)&CNT_OLE)

class SW_DLLPUBLIC SwEditShell: public SwCrsrShell
{
    static SvxSwAutoFmtFlags* pAutoFmtFlags;

    // fuer die privaten Methoden DelRange und die vom AutoCorrect
    friend class SwAutoFormat;
    friend void _InitCore();
    friend void _FinitCore();
    // fuer die PamCorrAbs/-Rel Methoden
    friend class SwUndo;

    SW_DLLPRIVATE SfxPoolItem& _GetChrFmt( SfxPoolItem& ) const;

    /*
     * liefert einen Pointer auf einen SwGrfNode; dieser wird von
     * GetGraphic() und GetGraphicSize() verwendet.
     */
    SW_DLLPRIVATE SwGrfNode *_GetGrfNode() const ;

    SW_DLLPRIVATE void DeleteSel( SwPaM& rPam, BOOL* pUndo = 0 );

    SW_DLLPRIVATE void _SetSectionAttr( SwSectionFmt& rSectFmt, const SfxItemSet& rSet );

    using ViewShell::UpdateFlds;
    using SwModify::GetInfo;

public:
    // Editieren (immer auf allen selektierten Bereichen)
    void Insert( sal_Unicode, BOOL bOnlyCurrCrsr = FALSE );
    void Insert2( const String &, const bool bForceExpandHints = false );
    void Overwrite( const String & );

    // Ersetz einen selektierten Bereich in einem TextNode mit dem
    // String. Ist fuers Suchen&Ersetzen gedacht.
    // bRegExpRplc - ersetze Tabs (\\t) und setze den gefundenen String
    //               ein ( nicht \& )
    //              z.B.: Fnd: "zzz", Repl: "xx\t\\t..&..\&"
    //                      --> "xx\t<Tab>..zzz..&"
    BOOL Replace( const String& rNewStr, BOOL bRegExpRplc = FALSE );

    // loescht den Inhalt aller Bereiche;
    // werden ganze Nodes selektiert, werden die Nodes geloescht
    long Delete();

    // remove a complete paragraph
    BOOL DelFullPara();

    // change text to Upper/Lower/Hiragana/Katagana/...
    void TransliterateText( sal_uInt32 nType );

    // count words in current selection
    void CountWords( SwDocStat& rStat ) const;

    // loesche den nicht sichtbaren Content aus dem Document, wie z.B.:
    // versteckte Bereiche, versteckte Absaetze
    BOOL RemoveInvisibleContent();

    // replace fields by text - mailmerge support
    BOOL ConvertFieldsToText();
    // set all numbering start points to a fixed value - mailmerge support
    void SetNumberingRestart();

    // embedded alle lokalen Links (Bereiche/Grafiken)
    USHORT GetLinkUpdMode(BOOL bDocSettings = FALSE) const;
    void SetLinkUpdMode( USHORT nMode );

    // kopiere den Inhalt aller Bereiche an die akt. Cursor-Position
    // in die angegebene Shell
    long Copy( SwEditShell* pDestShell = 0 );

    // fuers Kopieren uebers ClipBoard:
    //  wird Tabelle in Tabelle kopiert, verschiebe aus dieser dann
    //  alle Cursor. Copy und Paste muss aufgrund der FlyFrames in
    //  der FEShell stehen!
    // kopiere alle Selectionen und das Doc
    //JP 21.10.96: und fuer die SVX-Autokorrektur
    BOOL _CopySelToDoc( SwDoc* pInsDoc, SwNodeIndex* pNdInsPos = 0 );

    long SplitNode( BOOL bAutoFormat = FALSE, BOOL bCheckTableStart = TRUE );
    sal_Bool AppendTxtNode();
    void AutoFmtBySplitNode();

    // ist der Cursor in einem INetAttribut, dann wird das komplett
    // geloescht; inclusive des Hinweistextes (wird beim Drag&Drop gebraucht)
    BOOL DelINetAttrWithText();

    // ist der Cursor am Ende einer Zeichenvorlage, an der das DontExpand-Flag
    // noch nicht gesetzt ist, wird dies gesetzt ( => return TRUE; )
    BOOL DontExpandFmt();

    // Anwenden / Entfernen von Attributen
    // liefert Attribute im angeforderten AttributSet. Wenn nicht eindeutig
    // steht im Set ein DONT_CARE !!
    // --> OD 2008-01-16 #newlistlevelattrs#
    // Renaming method to <GetCurAttr(..)> indicating that the attributes at
    // the current cursors are retrieved.
    // Introduce 2nd optional parameter <bMergeIndentValuesOfNumRule>.
    // If <bMergeIndentValuesOfNumRule> == TRUE, the indent attributes of
    // the corresponding list level of an applied list style is merged into
    // the requested item set as a LR-SPACE item, if corresponding node has not
    // its own indent attributes and the position-and-space mode of the list
    // level is SvxNumberFormat::LABEL_ALIGNMENT.
    BOOL GetCurAttr( SfxItemSet& ,
                     const bool bMergeIndentValuesOfNumRule = false ) const;
    // <--
    void SetAttr( const SfxPoolItem&, USHORT nFlags = 0 );
    void SetAttr( const SfxItemSet&, USHORT nFlags = 0 );

    // Setze das Attribut als neues default Attribut im Dokument.
    void SetDefault( const SfxPoolItem& );

    // Erfrage das Default Attribut vom Dokument.
    const SfxPoolItem& GetDefault( USHORT nFmtHint ) const;

    void ResetAttr( const SvUShortsSort* pAttrs = 0 );
    void GCAttr();

    // returns the scripttpye of the selection
    USHORT GetScriptType() const;

    // returns the language at current cursor position
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


    /* Formatcollections, neu */
    /* GetXXXCount() liefert die Anzahl der im Dokument vorhandenen XXX
     * GetXXX(i)     liefert das i-te XXX (ERR_RAISE bei Ueberindizierung!)
     * DelXXX(i)     loescht das i-te XXX (ERR_RAISE bei Ueberindizierung!)
     * GetCurXXX()   liefert das am Cursor oder in den Bereichen
     *               geltende XXX (0, wenn nicht eindeutig!)
     * SetXXX()      setzt am Cursor oder in den Bereichen das XXX
     * MakeXXX()     macht ein XXX, abgeleitet vom pDerivedFrom
     */

    // TXT
    SwTxtFmtColl& GetDfltTxtFmtColl() const;
    USHORT GetTxtFmtCollCount() const;
    SwTxtFmtColl& GetTxtFmtColl( USHORT nTxtFmtColl) const;
    SwTxtFmtColl* GetCurTxtFmtColl() const;
    // --> OD 2007-11-06 #i62675#
    // Add 2nd optional parameter <bResetListAttrs> - see also <SwDoc::SetTxtFmtColl(..)>
    void SetTxtFmtColl( SwTxtFmtColl*,
                        bool bResetListAttrs = false );
    // <--
    SwTxtFmtColl *MakeTxtFmtColl(const String &rFmtCollName,
        SwTxtFmtColl *pDerivedFrom = 0);
    void FillByEx(SwTxtFmtColl*, BOOL bReset = FALSE);
    SwTxtFmtColl* FindTxtFmtCollByName( const String& rName ) const;

        // Gebe die "Auto-Collection" mit der Id zurueck. Existiert
        // sie noch nicht, dann erzuege sie
    SwTxtFmtColl* GetTxtCollFromPool( USHORT nId );
        // return das geforderte automatische  Format - Basis-Klasse !
    SwFmt* GetFmtFromPool( USHORT nId );
        // returne die geforderte automatische Seiten-Vorlage
    SwPageDesc* GetPageDescFromPool( USHORT nId );

    // erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird
    BOOL IsUsed( const SwModify& ) const;

        // returne das geforderte automatische Format
    SwFrmFmt* GetFrmFmtFromPool( USHORT nId )
        { return (SwFrmFmt*)SwEditShell::GetFmtFromPool( nId ); }
    SwCharFmt* GetCharFmtFromPool( USHORT nId )
        { return (SwCharFmt*)SwEditShell::GetFmtFromPool( nId ); }

    // Felder
    void Insert2(SwField&, const bool bForceExpandHints = false);
    SwField* GetCurFld() const;

    void UpdateFlds( SwField & );       // ein einzelnes Feld

    USHORT GetFldTypeCount(USHORT nResId = USHRT_MAX, BOOL bUsed = FALSE) const;
    SwFieldType* GetFldType(USHORT nId, USHORT nResId = USHRT_MAX, BOOL bUsed = FALSE) const;
    SwFieldType* GetFldType(USHORT nResId, const String& rName) const;

    void RemoveFldType(USHORT nId, USHORT nResId = USHRT_MAX);
    void RemoveFldType(USHORT nResId, const String& rName);

    void FieldToText( SwFieldType* pType );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    // Datenbankinfo
    SwDBData GetDBData() const;
    const SwDBData& GetDBDesc() const;
    void ChgDBData(const SwDBData& SwDBData);
    void ChangeDBFields( const SvStringsDtor& rOldNames,
                         const String& rNewName );
    void GetAllUsedDB( SvStringsDtor& rDBNameList,
                        SvStringsDtor* pAllDBNames = 0 );

    BOOL IsAnyDatabaseFieldInDoc()const;
    //check whether DB fields point to an available data source and returns it
    BOOL IsFieldDataSourceAvailable(String& rUsedDataSource) const;
    void UpdateExpFlds(BOOL bCloseDB = FALSE);// nur alle ExpressionFelder updaten
    void SetFixFields( BOOL bOnlyTimeDate = FALSE,
                        const DateTime* pNewDateTime = 0 );
    void LockExpFlds();
    void UnlockExpFlds();

    SwFldUpdateFlags GetFldUpdateFlags(BOOL bDocSettings = FALSE) const;
    void SetFldUpdateFlags( SwFldUpdateFlags eFlags );

    // fuer die Evaluierung der DBFelder (neuer DB-Manager)
    SwNewDBMgr* GetNewDBMgr() const;

    SwFieldType* InsertFldType(const SwFieldType &);

    // Aenderungen am Dokument?
    BOOL IsModified() const;
    void SetModified();
    void ResetModified();
    void SetUndoNoResetModified();

    // Dokument - Statistics
    void UpdateDocStat( SwDocStat& rStat );

    // Verzeichnismarke einfuegen loeschen
    void    Insert(const SwTOXMark& rMark);
    void    DeleteTOXMark(SwTOXMark* pMark);

    // Alle Markierungen am aktuellen SPoint ermitteln
    USHORT  GetCurTOXMarks(SwTOXMarks& rMarks) const ;

    // Verzeichnis einfuegen, und bei Bedarf erneuern
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

    // nach einlesen einer Datei alle Verzeichnisse updaten
    void SetUpdateTOX( BOOL bFlag = TRUE );
    BOOL IsUpdateTOX() const;

    // Verzeichnis-Typen verwalten
    USHORT              GetTOXTypeCount(TOXTypes eTyp) const;
    const SwTOXType*    GetTOXType(TOXTypes eTyp, USHORT nId) const;
    void                InsertTOXType(const SwTOXType& rTyp);

    //AutoMark file
    const String&   GetTOIAutoMarkURL() const;
    void            SetTOIAutoMarkURL(const String& rSet);
    void            ApplyAutoMark();

    // Schluessel fuer die Indexverwaltung
    USHORT GetTOIKeys( SwTOIKeyType eTyp, SvStringsSort& rArr ) const;

    void SetOutlineNumRule(const SwNumRule&);
    const SwNumRule* GetOutlineNumRule() const;
    // Gliederung - hoch-/runterstufen
    BOOL OutlineUpDown( short nOffset = 1 );
    // Gliederung - hoch-/runtermoven
    BOOL MoveOutlinePara( short nOffset = 1);
    // Outlines and SubOutline are protected ?
    BOOL IsProtectedOutlinePara() const;

    // Numerierung Aufzaehlunglisten
    // liefert Regelwerk der aktuellen Aufzaehlung (FALSE sonst)
    const SwNumRule* GetCurNumRule() const;

    // setzt, wenn noch keine Numerierung, sonst wird geaendert
    // arbeitet mit alten und neuen Regeln, nur Differenzen aktualisieren
    // --> OD 2008-02-08 #newlistlevelattrs#
    // Add optional parameter <bResetIndentAttrs> (default value FALSE).
    // If <bResetIndentAttrs> equals true, the indent attributes "before text"
    // and "first line indent" are additionally reset at the current selection,
    // if the list style makes use of the new list level attributes.
    // --> OD 2008-03-17 #refactorlists#
    // introduce parameters <bCreateNewList> and <sContinuedListId>
    // <bCreateNewList> indicates, if a new list is created by applying the
    // given list style.
    // If <bCreateNewList> equals FALSE, <sContinuedListId> may contain the
    // list Id of a list, which has to be continued by applying the given list style
    void SetCurNumRule( const SwNumRule&,
                        const bool bCreateNewList /*= false*/,
                        const String sContinuedListId = String(),
                        const bool bResetIndentAttrs = false );
    // <--
    // Absaetze ohne Numerierung, aber mit Einzuegen
    BOOL NoNum();
    // Loeschen, Splitten der Aufzaehlungsliste
    void DelNumRules();
    // Hoch-/Runterstufen
    BOOL NumUpDown( BOOL bDown = TRUE );
    // Hoch-/Runtermoven sowohl innerhalb als auch ausserhalb von Numerierungen
    BOOL MoveParagraph( long nOffset = 1);
    BOOL MoveNumParas( BOOL bUpperLower, BOOL bUpperLeft );
    // No-/Numerierung ueber Delete/Backspace ein/abschalten #115901#
    BOOL NumOrNoNum( BOOL bDelete = FALSE, BOOL bChkStart = TRUE);
    // -> #i23726#
    // --> OD 2008-06-09 #i90078#
    // Remove unused default parameter <nLevel> and <bRelative>.
    // Adjust method name and parameter name
    void ChangeIndentOfAllListLevels( short nDiff );
    // Adjust method name
    void SetIndent(short nIndent, const SwPosition & rPos);
    // <--
    BOOL IsFirstOfNumRule() const;
    BOOL IsFirstOfNumRule(const SwPaM & rPaM) const;
    // <- #i23726#

    BOOL IsNoNum( BOOL bChkStart = TRUE ) const;
    // returne den Num-Level des Nodes, in dem sich der Point vom
    // Cursor befindet. Return kann sein :
    // - NO_NUMBERING, 0..MAXLEVEL-1, NO_NUMLEVEL .. NO_NUMLEVEL|MAXLEVEL-1
    // --> OD 2008-02-29 #refactorlists# - removed <pHasChilds>
//    BYTE GetNumLevel( BOOL* pHasChilds = 0 ) const;
    BYTE GetNumLevel() const;
    // <--
    // detect highest and lowest level to check moving of outline levels
    void GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower );


    // get Outline level of current paragraph
    int GetCurrentParaOutlineLevel( ) const;// #outlinelevel add by zhaojianwei

    // -> i29560
    BOOL HasNumber() const;
    BOOL HasBullet() const;
    // <- i29560

    String GetUniqueNumRuleName( const String* pChkStr = 0, BOOL bAutoNum = TRUE ) const;
    void ChgNumRuleFmts( const SwNumRule& rRule );
    // setze und erfrage, ob an aktueller PointPos eine Numerierung mit
    // dem StartFlag startet
    void SetNumRuleStart( BOOL bFlag = TRUE );
    BOOL IsNumRuleStart() const;
    void SetNodeNumStart( USHORT nStt );
    // --> OD 2008-02-29 #refactorlists#
    USHORT GetNodeNumStart() const;
    // <--
    BOOL ReplaceNumRule( const String& rOldRule, const String& rNewRule );
    // Searches for a text node with a numbering rule.
    // --> OD 2008-03-18 #refactorlists# - add output parameter <sListId>
    // in case a list style is found, <sListId> holds the list id, to which the
    // text node belongs, which applies the found list style.
    const SwNumRule * SearchNumRule(const bool bForward,
                                    const bool bNum,
                                    const bool bOutline,
                                    int nNonEmptyAllowed,
                                    String& sListId );
    // <--

    // Undo
    // UndoHistory am Dokument pflegen
    // bei Save, SaveAs, Create wird UndoHistory zurueckgesetzt ???
    void DoUndo( BOOL bOn = TRUE );
    BOOL DoesUndo() const;
    void DoGroupUndo( BOOL bUn = TRUE );
    BOOL DoesGroupUndo() const;
    void DelAllUndoObj();

    // macht rueckgaengig:
    // setzt Undoklammerung auf, liefert nUndoId der Klammerung
    SwUndoId StartUndo( SwUndoId eUndoId = UNDO_EMPTY, const SwRewriter * pRewriter = 0 );
    // schliesst Klammerung der nUndoId, nicht vom UI benutzt
    SwUndoId EndUndo( SwUndoId eUndoId = UNDO_EMPTY, const SwRewriter * pRewriter = 0 );
    // liefert die Id der letzten undofaehigen Aktion zurueck
    // fuellt ggf. VARARR mit User-UndoIds
    SwUndoId GetUndoIds( String* pUndoStr = 0, SwUndoIds *pUndoIds = 0) const;
    String GetUndoIdsStr( String* pUndoStr = 0, SwUndoIds *pUndoIds = 0) const;

        // abfragen/setzen der Anzahl von wiederherstellbaren Undo-Actions
    static USHORT GetUndoActionCount();
    static void SetUndoActionCount( USHORT nNew );

    // Redo
    // liefert die Id der letzten Redofaehigen Aktion zurueck
    // fuellt ggf. VARARR mit RedoIds
    SwUndoId GetRedoIds( String* pRedoStr = 0, SwUndoIds *pRedoIds = 0) const;
    String GetRedoIdsStr( String* pRedoStr = 0, SwUndoIds *pRedoIds = 0) const;

    // Repeat
    // liefert die Id der letzten Repeatfaehigen Aktion zurueck
    // fuellt ggf. VARARR mit RedoIds
    SwUndoId GetRepeatIds( String* pRepeatStr = 0, SwUndoIds *pRedoIds = 0) const;
    String GetRepeatIdsStr( String* pRepeatStr = 0,
                            SwUndoIds *pRedoIds = 0) const;

    // 0 letzte Aktion, sonst Aktionen bis zum Start der Klammerung nUndoId
    // mit KillPaMs, ClearMark
    BOOL Undo(SwUndoId nUndoId = UNDO_EMPTY, USHORT nCnt = 1 );
    // wiederholt
    USHORT Repeat( USHORT nCount );
    // wiederholt
    USHORT Redo( USHORT nCnt = 1 );
    // fuer alle Sichten auf dieses Dokument
    void StartAllAction();
    void EndAllAction();

    //Damit Start-/EndActions aufgesetzt werden koennen.
    void CalcLayout();

    // Inhaltsform bestimmen, holen, liefert Type am CurCrsr->SPoint
    USHORT GetCntType() const;
    BOOL HasOtherCnt() const; // gibt es Rahmen, Fussnoten, ...

    /* Anwenden der ViewOptions mit Start-/EndAction */
    inline void ApplyViewOptions( const SwViewOption &rOpt );

    // Text innerhalb der Selektion erfragen
    // Returnwert liefert FALSE, wenn der selektierte Bereich
    // zu gross ist, um in den Stringpuffer kopiert zu werden
    // oder andere Fehler auftreten
    BOOL GetSelectedText( String &rBuf,
                        int nHndlParaBreak = GETSELTXT_PARABRK_TO_BLANK );

    /*
     * liefert eine Graphic, wenn CurCrsr->Point() auf einen
     * SwGrfNode zeigt (und Mark nicht gesetzt ist oder auf die
     * gleiche Graphic zeigt), sonst gibt's was auf die Finger
     */
    // --> OD 2005-02-09 #119353# - robust
    const Graphic* GetGraphic( BOOL bWait = TRUE ) const;
    const GraphicObject* GetGraphicObj() const;
    // <--
    BOOL IsGrfSwapOut( BOOL bOnlyLinked = FALSE ) const;
    USHORT GetGraphicType() const;

    const PolyPolygon *GetGraphicPolygon() const;
    void SetGraphicPolygon( const PolyPolygon *pPoly );

    // If there's an automatic, not manipulated polygon at the selected
    // notxtnode, it has to be deleted, e.g. cause the object has changed.
    void ClearAutomaticContour();

    /*
     * liefert die Groesse einer Graphic in Twips, wenn der Cursor
     * auf einer Graphic steht; BOOL liefert FALSE, wenn s.o.
     */
    BOOL GetGrfSize(Size&) const;
    /*
     * liefert den Namen und den Filter einer Graphic, wenn der Cursor
     * auf einer Graphic steht, sonst gibt's was auf die Finger!
     * Ist ein String-Ptr != 0 dann returne den entsp. Namen
     */
    void GetGrfNms( String* pGrfName, String* pFltName,
                    const SwFlyFrmFmt* = 0 ) const;
    /*
     * erneutes Einlesen, falls Graphic nicht Ok ist. Die
     * aktuelle wird durch die neue ersetzt.
     */
    void ReRead( const String& rGrfName, const String& rFltName,
                  const Graphic* pGraphic = 0,
                  const GraphicObject* pGrafObj = 0 );

//    // alternativen Text einer Grafik/OLe-Objectes abfragen/setzen
//    const String& GetAlternateText() const;
//    void SetAlternateText( const String& rTxt );

    //eindeutige Identifikation des Objektes (fuer ImageMapDlg)
    void    *GetIMapInventor() const;
    // --> OD 2007-03-01 #i73788#
    // remove default parameter, because method always called this default value
    Graphic GetIMapGraphic() const; //liefert eine Graphic fuer alle Flys!
    // <--
    const SwFlyFrmFmt* FindFlyByName( const String& rName, BYTE nNdTyp = 0 ) const;

    //liefert ein ClientObject, wenn CurCrsr->Point() auf einen
    //SwOLENode zeigt (und Mark nicht gesetzt ist oder auf das
    //gleiche ClientObject zeigt), sonst gibt's was auf die
    //Finger.
    svt::EmbeddedObjectRef&  GetOLEObject() const;
    //Gibt es ein OleObject mit diesem Namen (SwFmt)?
    BOOL HasOLEObj( const String &rName ) const;

    //Liefert den Pointer auf die Daten des Chart, indem sich der Crsr
    //befindet.
    void SetChartName( const String &rName );

    //Updaten der Inhalte aller Charts zu der Tabelle mit dem angegeben Namen
    void UpdateCharts( const String &rName );

    //  aktuelles Wort erfragen
    String GetCurWord();

    // Textbaustein aus dem Textbausteindokument in
    // das aktuelle Dokument, Vorlagen nur wenn es nicht schon gibt
    void InsertGlossary( SwTextBlocks& rGlossary, const String& );
    // aktuelle Selektion zum Textbaustein machen und ins
    // Textbausteindokument einfuegen, einschliesslich Vorlagen
    USHORT MakeGlossary( SwTextBlocks& rToFill, const String& rName,
                         const String& rShortName, BOOL bSaveRelFile = FALSE,
                         const String* pOnlyTxt=0 );
    // speicher den gesamten Inhalt des Docs als Textbaustein
    USHORT SaveGlossaryDoc( SwTextBlocks& rGlossary, const String& rName,
                            const String& rShortName,
                            BOOL bSaveRelFile = FALSE,
                            BOOL bOnlyTxt = FALSE );

    // Linguistik
    // Selektionen sichern
    void HyphStart( SwDocPositions eStart, SwDocPositions eEnde );
    // Selektionen wiederherstellen
    void HyphEnd();
    com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                HyphContinue( USHORT* pPageCnt, USHORT* pPageSt );
    // zu trennendes Wort ignorieren
    void HyphIgnore();

    // zum Einfuegen des SoftHyphens, Position ist der Offset
    // innerhalb des getrennten Wortes.
    void InsertSoftHyph( const xub_StrLen nHyphPos );

    //Tabelle
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

    // Change Modus erfragen/setzen
    TblChgMode GetTblChgMode() const;
    void SetTblChgMode( TblChgMode eMode );

    // Tabelle an der Cursor Position aufsplitten
    BOOL SplitTable( USHORT eMode );
    // Tabellen verbinden
    // CanMerge kann feststellen, ob Prev oder Next moeglich ist. Wird
    // der Pointer pChkNxtPrv uebergeben, wird festgestellt in welche
    // Richtung es moeglich ist.
    BOOL CanMergeTable( BOOL bWithPrev = TRUE, BOOL* pChkNxtPrv = 0 ) const;
    BOOL MergeTable( BOOL bWithPrev = TRUE, USHORT nMode = 0 );
        // setze das InsertDB als Tabelle Undo auf:
    void AppendUndoForInsertFromDB( BOOL bIsTable );

    /*
        functions used for spell checking and text conversion
    */

    // Selektionen sichern
    void SpellStart( SwDocPositions eStart, SwDocPositions eEnde,
                     SwDocPositions eCurr, SwConversionArgs *pConvArgs = 0 );
    // Selektionen wiederherstellen
    void SpellEnd( SwConversionArgs *pConvArgs = 0, bool bRestoreSelection = true );
    ::com::sun::star::uno::Any SpellContinue(
                    USHORT* pPageCnt, USHORT* pPageSt,
                    SwConversionArgs *pConvArgs = 0 );

    // spells on a sentence basis - the SpellPortions are needed
    // returns false if no error could be found
    bool SpellSentence(::svx::SpellPortions& rToFill, bool bIsGrammarCheck );
    // make SpellIter start with the current sentence when called next time
    void PutSpellingToSentenceStart();
    // moves the continuation position to the end of the currently checked sentence
    void MoveContinuationPosToEndOfCheckedSentence();
    //applies a changed sentence
    void ApplyChangedSentence(const ::svx::SpellPortions& rNewPortions, bool bRecheck);


    // check SwSpellIter data to see if the last sentence got grammar checked
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

    // returne zum Namen die im Doc gesetzte Referenz
    const SwFmtRefMark* GetRefMark( const String& rName ) const;
    // returne die Namen aller im Doc gesetzten Referenzen
    //  Ist der ArrayPointer 0 dann returne nur, ob im Doc. eine RefMark
    //  gesetzt ist
    USHORT GetRefMarks( SvStringsDtor* = 0 ) const;

    // rufe die Autokorrektur auf
    void AutoCorrect( SvxAutoCorrect& rACorr, BOOL bInsertMode = TRUE,
                        sal_Unicode cChar = ' ' );
    BOOL GetPrevAutoCorrWord( SvxAutoCorrect& rACorr, String& rWord );

    // dann setze nach entsprechenden Regeln unsere Vorlagen
    void AutoFormat( const SvxSwAutoFmtFlags* pAFlags = 0 );

    static SvxSwAutoFmtFlags* GetAutoFmtFlags();
    static void SetAutoFmtFlags(SvxSwAutoFmtFlags *);

    // errechnet die Selektion
    String Calculate();

    BOOL InsertURL( const SwFmtINetFmt& rFmt, const String& rStr,
                    BOOL bKeepSelection = FALSE );
    USHORT GetINetAttrs( SwGetINetAttrs& rArr );

    //SS Fuer holen/ersetzen DropCap-Inhalt
    String GetDropTxt( const USHORT nChars ) const;
    void   ReplaceDropTxt( const String &rStr );

    // may an outline be moved or copied?
    // Check whether it's in text body, not in table, and not read-only (move)
    BOOL IsOutlineMovable( USHORT nIdx ) const;
    BOOL IsOutlineCopyable( USHORT nIdx ) const;

    USHORT GetLineCount( BOOL bActPos = TRUE );

    // erfrage und setze den Fussnoten-Text/Nummer. Set.. auf akt. SSelection!
    BOOL GetCurFtn( SwFmtFtn* pToFillFtn = 0 );
    bool SetCurFtn( const SwFmtFtn& rFillFtn );
    bool HasFtns( bool bEndNotes = false ) const;
//z.Zt nicht benoetigt  USHORT GetFtnCnt( BOOL bEndNotes = FALSE ) const;
        // gebe Liste aller Fussnoten und deren Anfangstexte
    USHORT GetSeqFtnList( SwSeqFldList& rList, bool bEndNotes = false );

    SwSection const* InsertSection(
            SwSectionData & rNewData, SfxItemSet const*const = 0 );
    BOOL IsInsRegionAvailable() const;
    const SwSection* GetCurrSection() const;
    // liefert wie GetCurrSection() den aktuellen Bereich, allerdings geht diese Funktion
    // ueber die Frames und erwischt dabei auch den Bereich, wenn der Cursor in einer
    // Fussnote steht, deren Referenz in einem spaltigen Bereich steckt.
    // Wenn man bOutOfTab setzt, wird der Bereich gesucht,
    // der die Tabelle umfasst, nicht etwa ein innerer.
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

    //Attribute setzen
    void SetSectionAttr(const SfxItemSet& rSet, SwSectionFmt* pSectFmt = 0);

    // search inside the cursor selection for full selected sections.
    // if any part of section in the selection return 0.
    // if more than one in the selection return the count
    USHORT GetFullSelectedSectionCount() const;

    // special insert: Insert a new text node just before or after a section or
    // table, if the cursor is positioned at the start/end of said
    // section/table. The purpose of the method is to allow users to inert text
    // at certain 'impossible' position, e.g. before a table at the document
    // start or between to sections.
    bool DoSpecialInsert();
    bool CanSpecialInsert() const;

    // Optimierung UI
    void SetNewDoc(BOOL bNew = TRUE);

    sfx2::LinkManager& GetLinkManager();
    inline const sfx2::LinkManager& GetLinkManager() const;

    // linken Rand ueber Objectleiste einstellen (aenhlich dem Stufen von
    // Numerierungen), optional kann man "um" den Offset stufen oder "auf"
    // die Position gestuft werden (bModulus = TRUE)
    BOOL IsMoveLeftMargin( BOOL bRight = TRUE, BOOL bModulus = TRUE ) const;
    void MoveLeftMargin( BOOL bRight = TRUE, BOOL bModulus = TRUE );

    // Numberformatter vom Doc erfragen
          SvNumberFormatter* GetNumberFormatter();
    const SvNumberFormatter* GetNumberFormatter() const
    {   return ((SwEditShell*)this)->GetNumberFormatter();  }

    // Schnitstellen fuers GlobalDokument
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

    // alles fuers Redlining
    USHORT GetRedlineMode() const;
    void SetRedlineMode( USHORT eMode );
    BOOL IsRedlineOn() const;
    USHORT GetRedlineCount() const;
    const SwRedline& GetRedline( USHORT nPos ) const;
    BOOL AcceptRedline( USHORT nPos );
    BOOL RejectRedline( USHORT nPos );
    // suche das Redline zu diesem Data und returne die Pos im Array
    // USHRT_MAX wird returnt, falls nicht vorhanden
    USHORT FindRedlineOfData( const SwRedlineData& ) const;

    // Kommentar am Redline an der Position setzen
    BOOL SetRedlineComment( const String& rS );
    const SwRedline* GetCurrRedline() const;

    // Redline Anzeigeattribute wurden geaendert, Views updaten
    void UpdateRedlineAttr();

    //  vergleiche zwei Dokument mit einander
    long CompareDoc( const SwDoc& rDoc );
    // merge zweier Dokumente
    long MergeDoc( const SwDoc& rDoc );

    // Dokumentglobale Fussnoteneigenschaften
    const SwFtnInfo& GetFtnInfo() const;
    void  SetFtnInfo(const SwFtnInfo& rInfo);
    const SwEndNoteInfo& GetEndNoteInfo() const;
    void  SetEndNoteInfo(const SwEndNoteInfo& rInfo);

    //Einstellungen fuer Zeilennummierung
    const SwLineNumberInfo &GetLineNumberInfo() const;
    void  SetLineNumberInfo( const SwLineNumberInfo& rInfo);

    // Etiketten: Bereiche synchronisieren
    void SetLabelDoc( BOOL bFlag = TRUE );
    BOOL IsLabelDoc() const;

    // Schnittstelle fuer die TextInputDaten - ( fuer die Texteingabe
    // von japanischen/chinesischen Zeichen)
    SwExtTextInput* CreateExtTextInput(LanguageType eInputLanguage);
    String DeleteExtTextInput( SwExtTextInput* pDel = 0, BOOL bInsText = TRUE);
//  SwExtTextInput* GetExtTextInput() const;
    void SetExtTextInputData( const CommandExtTextInputData& );

    // Schnistelle fuer den Zugriff auf die AutoComplete-Liste
    static SwAutoCompleteWord& GetAutoCompleteWords();

    // returns a scaling factor of selected text. Used for the rotated
    // character attribut dialog.
    USHORT GetScalingOfSelectedText() const;

    // ctor/dtor
    SwEditShell( SwDoc&, Window*, const SwViewOption *pOpt = 0 );
    // verkleideter Copy-Constructor
    SwEditShell( SwEditShell&, Window* );
    virtual ~SwEditShell();

private:
    // fuer METWARE:
    // es wird nicht kopiert und nicht zugewiesen
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

/*
 * Klasse fuer den automatisierten Aufruf von Start- und
 * EndAction();
 */
class SwActKontext {
    SwEditShell *pSh;
public:
    SwActKontext(SwEditShell *pShell);
    ~SwActKontext();
};

#define ACT_KONTEXT(x)  SwActKontext _aActKontext_(x)

/*
 * Klasse fuer den automatisierten Aufruf von Start- und
 * EndCrsrMove();
 */
class SwMvKontext {
    SwEditShell *pSh;
public:
    SwMvKontext(SwEditShell *pShell );
    ~SwMvKontext();
};

#define MV_KONTEXT(x)   SwMvKontext _aMvKontext_(x)



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
