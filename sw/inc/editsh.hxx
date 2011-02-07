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

namespace sw {
    class UndoRedoContext;
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
/* Teste einen sal_uInt16 auf eine bestimmte Inhaltsform */
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

    SW_DLLPRIVATE void DeleteSel( SwPaM& rPam, sal_Bool* pUndo = 0 );

    SW_DLLPRIVATE void _SetSectionAttr( SwSectionFmt& rSectFmt, const SfxItemSet& rSet );

    using ViewShell::UpdateFlds;
    using SwModify::GetInfo;

public:
    // Editieren (immer auf allen selektierten Bereichen)
    void Insert( sal_Unicode, sal_Bool bOnlyCurrCrsr = sal_False );
    void Insert2( const String &, const bool bForceExpandHints = false );
    void Overwrite( const String & );

    // Ersetz einen selektierten Bereich in einem TextNode mit dem
    // String. Ist fuers Suchen&Ersetzen gedacht.
    // bRegExpRplc - ersetze Tabs (\\t) und setze den gefundenen String
    //               ein ( nicht \& )
    //              z.B.: Fnd: "zzz", Repl: "xx\t\\t..&..\&"
    //                      --> "xx\t<Tab>..zzz..&"
    sal_Bool Replace( const String& rNewStr, sal_Bool bRegExpRplc = sal_False );

    // loescht den Inhalt aller Bereiche;
    // werden ganze Nodes selektiert, werden die Nodes geloescht
    long Delete();

    // remove a complete paragraph
    sal_Bool DelFullPara();

    // change text to Upper/Lower/Hiragana/Katagana/...
    void TransliterateText( sal_uInt32 nType );

    // count words in current selection
    void CountWords( SwDocStat& rStat ) const;

    // loesche den nicht sichtbaren Content aus dem Document, wie z.B.:
    // versteckte Bereiche, versteckte Absaetze
    sal_Bool RemoveInvisibleContent();

    // replace fields by text - mailmerge support
    sal_Bool ConvertFieldsToText();
    // set all numbering start points to a fixed value - mailmerge support
    void SetNumberingRestart();

    // embedded alle lokalen Links (Bereiche/Grafiken)
    sal_uInt16 GetLinkUpdMode(sal_Bool bDocSettings = sal_False) const;
    void SetLinkUpdMode( sal_uInt16 nMode );

    // kopiere den Inhalt aller Bereiche an die akt. Cursor-Position
    // in die angegebene Shell
    long Copy( SwEditShell* pDestShell = 0 );

    // fuers Kopieren uebers ClipBoard:
    //  wird Tabelle in Tabelle kopiert, verschiebe aus dieser dann
    //  alle Cursor. Copy und Paste muss aufgrund der FlyFrames in
    //  der FEShell stehen!
    // kopiere alle Selectionen und das Doc
    //JP 21.10.96: und fuer die SVX-Autokorrektur
    sal_Bool _CopySelToDoc( SwDoc* pInsDoc, SwNodeIndex* pNdInsPos = 0 );

    long SplitNode( sal_Bool bAutoFormat = sal_False, sal_Bool bCheckTableStart = sal_True );
    sal_Bool AppendTxtNode();
    void AutoFmtBySplitNode();

    // ist der Cursor in einem INetAttribut, dann wird das komplett
    // geloescht; inclusive des Hinweistextes (wird beim Drag&Drop gebraucht)
    sal_Bool DelINetAttrWithText();

    // ist der Cursor am Ende einer Zeichenvorlage, an der das DontExpand-Flag
    // noch nicht gesetzt ist, wird dies gesetzt ( => return sal_True; )
    sal_Bool DontExpandFmt();

    // Anwenden / Entfernen von Attributen
    // liefert Attribute im angeforderten AttributSet. Wenn nicht eindeutig
    // steht im Set ein DONT_CARE !!
    // --> OD 2008-01-16 #newlistlevelattrs#
    // Renaming method to <GetCurAttr(..)> indicating that the attributes at
    // the current cursors are retrieved.
    // Introduce 2nd optional parameter <bMergeIndentValuesOfNumRule>.
    // If <bMergeIndentValuesOfNumRule> == sal_True, the indent attributes of
    // the corresponding list level of an applied list style is merged into
    // the requested item set as a LR-SPACE item, if corresponding node has not
    // its own indent attributes and the position-and-space mode of the list
    // level is SvxNumberFormat::LABEL_ALIGNMENT.
    sal_Bool GetCurAttr( SfxItemSet& ,
                     const bool bMergeIndentValuesOfNumRule = false ) const;
    // <--
    void SetAttr( const SfxPoolItem&, sal_uInt16 nFlags = 0 );
    void SetAttr( const SfxItemSet&, sal_uInt16 nFlags = 0 );

    // Setze das Attribut als neues default Attribut im Dokument.
    void SetDefault( const SfxPoolItem& );

    // Erfrage das Default Attribut vom Dokument.
    const SfxPoolItem& GetDefault( sal_uInt16 nFmtHint ) const;

    void ResetAttr( const SvUShortsSort* pAttrs = 0 );
    void GCAttr();

    // returns the scripttpye of the selection
    sal_uInt16 GetScriptType() const;

    // returns the language at current cursor position
    sal_uInt16 GetCurLang() const;

    // TABLE
    sal_uInt16 GetTblFrmFmtCount( sal_Bool bUsed = sal_False ) const;
    SwFrmFmt& GetTblFrmFmt(sal_uInt16 nFmt, sal_Bool bUsed = sal_False ) const;
    String GetUniqueTblName() const;

    // CHAR
    sal_uInt16 GetCharFmtCount() const;
    SwCharFmt& GetCharFmt(sal_uInt16 nFmt) const;
    SwCharFmt* GetCurCharFmt() const;
    void FillByEx(SwCharFmt*, sal_Bool bReset = sal_False);
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
    sal_uInt16 GetTxtFmtCollCount() const;
    SwTxtFmtColl& GetTxtFmtColl( sal_uInt16 nTxtFmtColl) const;
    SwTxtFmtColl* GetCurTxtFmtColl() const;
    // --> OD 2007-11-06 #i62675#
    // Add 2nd optional parameter <bResetListAttrs> - see also <SwDoc::SetTxtFmtColl(..)>
    void SetTxtFmtColl( SwTxtFmtColl*,
                        bool bResetListAttrs = false );
    // <--
    SwTxtFmtColl *MakeTxtFmtColl(const String &rFmtCollName,
        SwTxtFmtColl *pDerivedFrom = 0);
    void FillByEx(SwTxtFmtColl*, sal_Bool bReset = sal_False);
    SwTxtFmtColl* FindTxtFmtCollByName( const String& rName ) const;

        // Gebe die "Auto-Collection" mit der Id zurueck. Existiert
        // sie noch nicht, dann erzuege sie
    SwTxtFmtColl* GetTxtCollFromPool( sal_uInt16 nId );
        // return das geforderte automatische  Format - Basis-Klasse !
    SwFmt* GetFmtFromPool( sal_uInt16 nId );
        // returne die geforderte automatische Seiten-Vorlage
    SwPageDesc* GetPageDescFromPool( sal_uInt16 nId );

    // erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird
    sal_Bool IsUsed( const SwModify& ) const;

        // returne das geforderte automatische Format
    SwFrmFmt* GetFrmFmtFromPool( sal_uInt16 nId )
        { return (SwFrmFmt*)SwEditShell::GetFmtFromPool( nId ); }
    SwCharFmt* GetCharFmtFromPool( sal_uInt16 nId )
        { return (SwCharFmt*)SwEditShell::GetFmtFromPool( nId ); }

    // Felder
    void Insert2(SwField&, const bool bForceExpandHints = false);
    SwField* GetCurFld() const;

    void UpdateFlds( SwField & );       // ein einzelnes Feld

    sal_uInt16 GetFldTypeCount(sal_uInt16 nResId = USHRT_MAX, sal_Bool bUsed = sal_False) const;
    SwFieldType* GetFldType(sal_uInt16 nId, sal_uInt16 nResId = USHRT_MAX, sal_Bool bUsed = sal_False) const;
    SwFieldType* GetFldType(sal_uInt16 nResId, const String& rName) const;

    void RemoveFldType(sal_uInt16 nId, sal_uInt16 nResId = USHRT_MAX);
    void RemoveFldType(sal_uInt16 nResId, const String& rName);

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

    sal_Bool IsAnyDatabaseFieldInDoc()const;
    //check whether DB fields point to an available data source and returns it
    sal_Bool IsFieldDataSourceAvailable(String& rUsedDataSource) const;
    void UpdateExpFlds(sal_Bool bCloseDB = sal_False);// nur alle ExpressionFelder updaten
    void SetFixFields( sal_Bool bOnlyTimeDate = sal_False,
                        const DateTime* pNewDateTime = 0 );
    void LockExpFlds();
    void UnlockExpFlds();

    SwFldUpdateFlags GetFldUpdateFlags(sal_Bool bDocSettings = sal_False) const;
    void SetFldUpdateFlags( SwFldUpdateFlags eFlags );

    // fuer die Evaluierung der DBFelder (neuer DB-Manager)
    SwNewDBMgr* GetNewDBMgr() const;

    SwFieldType* InsertFldType(const SwFieldType &);

    // Aenderungen am Dokument?
    sal_Bool IsModified() const;
    void SetModified();
    void ResetModified();
    void SetUndoNoResetModified();

    // Dokument - Statistics
    void UpdateDocStat( SwDocStat& rStat );

    // Verzeichnismarke einfuegen loeschen
    void    Insert(const SwTOXMark& rMark);
    void    DeleteTOXMark(SwTOXMark* pMark);

    // Alle Markierungen am aktuellen SPoint ermitteln
    sal_uInt16  GetCurTOXMarks(SwTOXMarks& rMarks) const ;

    // Verzeichnis einfuegen, und bei Bedarf erneuern
    void                InsertTableOf(const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet = 0);
    sal_Bool                UpdateTableOf(const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet = 0);
    const SwTOXBase*    GetCurTOX() const;
    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, sal_Bool bCreate = sal_False );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    sal_Bool                IsTOXBaseReadonly(const SwTOXBase& rTOXBase) const;
    void                SetTOXBaseReadonly(const SwTOXBase& rTOXBase, sal_Bool bReadonly);

    sal_uInt16              GetTOXCount() const;
    const SwTOXBase*    GetTOX( sal_uInt16 nPos ) const;
    sal_Bool                DeleteTOX( const SwTOXBase& rTOXBase, sal_Bool bDelNodes = sal_False );

    // nach einlesen einer Datei alle Verzeichnisse updaten
    void SetUpdateTOX( sal_Bool bFlag = sal_True );
    sal_Bool IsUpdateTOX() const;

    // Verzeichnis-Typen verwalten
    sal_uInt16              GetTOXTypeCount(TOXTypes eTyp) const;
    const SwTOXType*    GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const;
    void                InsertTOXType(const SwTOXType& rTyp);

    //AutoMark file
    const String&   GetTOIAutoMarkURL() const;
    void            SetTOIAutoMarkURL(const String& rSet);
    void            ApplyAutoMark();

    // Schluessel fuer die Indexverwaltung
    sal_uInt16 GetTOIKeys( SwTOIKeyType eTyp, SvStringsSort& rArr ) const;

    void SetOutlineNumRule(const SwNumRule&);
    const SwNumRule* GetOutlineNumRule() const;
    // Gliederung - hoch-/runterstufen
    sal_Bool OutlineUpDown( short nOffset = 1 );
    // Gliederung - hoch-/runtermoven
    sal_Bool MoveOutlinePara( short nOffset = 1);
    // Outlines and SubOutline are protected ?
    sal_Bool IsProtectedOutlinePara() const;

    // Numerierung Aufzaehlunglisten
    // liefert Regelwerk der aktuellen Aufzaehlung (sal_False sonst)
    const SwNumRule* GetCurNumRule() const;

    // setzt, wenn noch keine Numerierung, sonst wird geaendert
    // arbeitet mit alten und neuen Regeln, nur Differenzen aktualisieren
    // --> OD 2008-02-08 #newlistlevelattrs#
    // Add optional parameter <bResetIndentAttrs> (default value sal_False).
    // If <bResetIndentAttrs> equals true, the indent attributes "before text"
    // and "first line indent" are additionally reset at the current selection,
    // if the list style makes use of the new list level attributes.
    // --> OD 2008-03-17 #refactorlists#
    // introduce parameters <bCreateNewList> and <sContinuedListId>
    // <bCreateNewList> indicates, if a new list is created by applying the
    // given list style.
    // If <bCreateNewList> equals sal_False, <sContinuedListId> may contain the
    // list Id of a list, which has to be continued by applying the given list style
    void SetCurNumRule( const SwNumRule&,
                        const bool bCreateNewList /*= false*/,
                        const String sContinuedListId = String(),
                        const bool bResetIndentAttrs = false );
    // <--
    // Absaetze ohne Numerierung, aber mit Einzuegen
    sal_Bool NoNum();
    // Loeschen, Splitten der Aufzaehlungsliste
    void DelNumRules();
    // Hoch-/Runterstufen
    sal_Bool NumUpDown( sal_Bool bDown = sal_True );
    // Hoch-/Runtermoven sowohl innerhalb als auch ausserhalb von Numerierungen
    sal_Bool MoveParagraph( long nOffset = 1);
    sal_Bool MoveNumParas( sal_Bool bUpperLower, sal_Bool bUpperLeft );
    // No-/Numerierung ueber Delete/Backspace ein/abschalten #115901#
    sal_Bool NumOrNoNum( sal_Bool bDelete = sal_False, sal_Bool bChkStart = sal_True);
    // -> #i23726#
    // --> OD 2008-06-09 #i90078#
    // Remove unused default parameter <nLevel> and <bRelative>.
    // Adjust method name and parameter name
    void ChangeIndentOfAllListLevels( short nDiff );
    // Adjust method name
    void SetIndent(short nIndent, const SwPosition & rPos);
    // <--
    sal_Bool IsFirstOfNumRule() const;
    sal_Bool IsFirstOfNumRule(const SwPaM & rPaM) const;
    // <- #i23726#

    sal_Bool IsNoNum( sal_Bool bChkStart = sal_True ) const;
    // returne den Num-Level des Nodes, in dem sich der Point vom
    // Cursor befindet. Return kann sein :
    // - NO_NUMBERING, 0..MAXLEVEL-1, NO_NUMLEVEL .. NO_NUMLEVEL|MAXLEVEL-1
    // --> OD 2008-02-29 #refactorlists# - removed <pHasChilds>
//    sal_uInt8 GetNumLevel( sal_Bool* pHasChilds = 0 ) const;
    sal_uInt8 GetNumLevel() const;
    // <--
    // detect highest and lowest level to check moving of outline levels
    void GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower );


    // get Outline level of current paragraph
    int GetCurrentParaOutlineLevel( ) const;// #outlinelevel add by zhaojianwei

    // -> i29560
    sal_Bool HasNumber() const;
    sal_Bool HasBullet() const;
    // <- i29560

    String GetUniqueNumRuleName( const String* pChkStr = 0, sal_Bool bAutoNum = sal_True ) const;
    void ChgNumRuleFmts( const SwNumRule& rRule );
    // setze und erfrage, ob an aktueller PointPos eine Numerierung mit
    // dem StartFlag startet
    void SetNumRuleStart( sal_Bool bFlag = sal_True );
    sal_Bool IsNumRuleStart() const;
    void SetNodeNumStart( sal_uInt16 nStt );
    // --> OD 2008-02-29 #refactorlists#
    sal_uInt16 GetNodeNumStart() const;
    // <--
    sal_Bool ReplaceNumRule( const String& rOldRule, const String& rNewRule );
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
    void DoUndo( sal_Bool bOn = sal_True );
    sal_Bool DoesUndo() const;
    void DoGroupUndo( sal_Bool bUn = sal_True );
    sal_Bool DoesGroupUndo() const;
    void DelAllUndoObj();

    // macht rueckgaengig:
    // setzt Undoklammerung auf, liefert nUndoId der Klammerung
    SwUndoId StartUndo( SwUndoId eUndoId = UNDO_EMPTY, const SwRewriter * pRewriter = 0 );
    // schliesst Klammerung der nUndoId, nicht vom UI benutzt
    SwUndoId EndUndo( SwUndoId eUndoId = UNDO_EMPTY, const SwRewriter * pRewriter = 0 );

    bool     GetLastUndoInfo(::rtl::OUString *const o_pStr,
                             SwUndoId *const o_pId) const;
    bool     GetFirstRedoInfo(::rtl::OUString *const o_pStr) const;
    SwUndoId GetRepeatInfo(::rtl::OUString *const o_pStr) const;

    /// is it forbidden to modify cursors via API calls?
    bool CursorsLocked() const;
    /// set selections to those contained in the UndoRedoContext
    /// should only be called by sw::UndoManager!
    void HandleUndoRedoContext(::sw::UndoRedoContext & rContext);

    bool Undo(sal_uInt16 const nCount = 1);
    bool Redo(sal_uInt16 const nCount = 1);
    bool Repeat(sal_uInt16 const nCount);

    // fuer alle Sichten auf dieses Dokument
    void StartAllAction();
    void EndAllAction();

    //Damit Start-/EndActions aufgesetzt werden koennen.
    void CalcLayout();

    // Inhaltsform bestimmen, holen, liefert Type am CurCrsr->SPoint
    sal_uInt16 GetCntType() const;
    sal_Bool HasOtherCnt() const; // gibt es Rahmen, Fussnoten, ...

    /* Anwenden der ViewOptions mit Start-/EndAction */
    inline void ApplyViewOptions( const SwViewOption &rOpt );

    // Text innerhalb der Selektion erfragen
    // Returnwert liefert sal_False, wenn der selektierte Bereich
    // zu gross ist, um in den Stringpuffer kopiert zu werden
    // oder andere Fehler auftreten
    sal_Bool GetSelectedText( String &rBuf,
                        int nHndlParaBreak = GETSELTXT_PARABRK_TO_BLANK );

    /*
     * liefert eine Graphic, wenn CurCrsr->Point() auf einen
     * SwGrfNode zeigt (und Mark nicht gesetzt ist oder auf die
     * gleiche Graphic zeigt), sonst gibt's was auf die Finger
     */
    // --> OD 2005-02-09 #119353# - robust
    const Graphic* GetGraphic( sal_Bool bWait = sal_True ) const;
    const GraphicObject* GetGraphicObj() const;
    // <--
    sal_Bool IsGrfSwapOut( sal_Bool bOnlyLinked = sal_False ) const;
    sal_uInt16 GetGraphicType() const;

    const PolyPolygon *GetGraphicPolygon() const;
    void SetGraphicPolygon( const PolyPolygon *pPoly );

    // If there's an automatic, not manipulated polygon at the selected
    // notxtnode, it has to be deleted, e.g. cause the object has changed.
    void ClearAutomaticContour();

    /*
     * liefert die Groesse einer Graphic in Twips, wenn der Cursor
     * auf einer Graphic steht; sal_Bool liefert sal_False, wenn s.o.
     */
    sal_Bool GetGrfSize(Size&) const;
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
    const SwFlyFrmFmt* FindFlyByName( const String& rName, sal_uInt8 nNdTyp = 0 ) const;

    //liefert ein ClientObject, wenn CurCrsr->Point() auf einen
    //SwOLENode zeigt (und Mark nicht gesetzt ist oder auf das
    //gleiche ClientObject zeigt), sonst gibt's was auf die
    //Finger.
    svt::EmbeddedObjectRef&  GetOLEObject() const;
    //Gibt es ein OleObject mit diesem Namen (SwFmt)?
    sal_Bool HasOLEObj( const String &rName ) const;

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
    sal_uInt16 MakeGlossary( SwTextBlocks& rToFill, const String& rName,
                         const String& rShortName, sal_Bool bSaveRelFile = sal_False,
                         const String* pOnlyTxt=0 );
    // speicher den gesamten Inhalt des Docs als Textbaustein
    sal_uInt16 SaveGlossaryDoc( SwTextBlocks& rGlossary, const String& rName,
                            const String& rShortName,
                            sal_Bool bSaveRelFile = sal_False,
                            sal_Bool bOnlyTxt = sal_False );

    // Linguistik
    // Selektionen sichern
    void HyphStart( SwDocPositions eStart, SwDocPositions eEnde );
    // Selektionen wiederherstellen
    void HyphEnd();
    com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                HyphContinue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );
    // zu trennendes Wort ignorieren
    void HyphIgnore();

    // zum Einfuegen des SoftHyphens, Position ist der Offset
    // innerhalb des getrennten Wortes.
    void InsertSoftHyph( const xub_StrLen nHyphPos );

    //Tabelle
    const SwTable& InsertTable( const SwInsertTableOptions& rInsTblOpts,  // ALL_TBL_INS_ATTR
                                sal_uInt16 nRows, sal_uInt16 nCols,
                                sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL,
                                const SwTableAutoFmt* pTAFmt = 0 );

    void InsertDDETable( const SwInsertTableOptions& rInsTblOpts,  // HEADLINE_NO_BORDER
                         SwDDEFieldType* pDDEType,
                         sal_uInt16 nRows, sal_uInt16 nCols,
                         sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL );

    void UpdateTable();
    void SetTableName( SwFrmFmt& rTblFmt, const String &rNewName );

    SwFrmFmt *GetTableFmt();
    sal_Bool TextToTable( const SwInsertTableOptions& rInsTblOpts,  //ALL_TBL_INS_ATTR
                      sal_Unicode cCh,
                      sal_Int16 eAdj = com::sun::star::text::HoriOrientation::FULL,
                      const SwTableAutoFmt* pTAFmt = 0 );
    sal_Bool TableToText( sal_Unicode cCh );
    sal_Bool IsTextToTableAvailable() const;

    sal_Bool GetTblBoxFormulaAttrs( SfxItemSet& rSet ) const;
    void SetTblBoxFormulaAttrs( const SfxItemSet& rSet );

    sal_Bool IsTableBoxTextFormat() const;
    String GetTableBoxText() const;

    // Change Modus erfragen/setzen
    TblChgMode GetTblChgMode() const;
    void SetTblChgMode( TblChgMode eMode );

    // Tabelle an der Cursor Position aufsplitten
    sal_Bool SplitTable( sal_uInt16 eMode );
    // Tabellen verbinden
    // CanMerge kann feststellen, ob Prev oder Next moeglich ist. Wird
    // der Pointer pChkNxtPrv uebergeben, wird festgestellt in welche
    // Richtung es moeglich ist.
    sal_Bool CanMergeTable( sal_Bool bWithPrev = sal_True, sal_Bool* pChkNxtPrv = 0 ) const;
    sal_Bool MergeTable( sal_Bool bWithPrev = sal_True, sal_uInt16 nMode = 0 );
        // setze das InsertDB als Tabelle Undo auf:
    void AppendUndoForInsertFromDB( sal_Bool bIsTable );

    /*
        functions used for spell checking and text conversion
    */

    // Selektionen sichern
    void SpellStart( SwDocPositions eStart, SwDocPositions eEnde,
                     SwDocPositions eCurr, SwConversionArgs *pConvArgs = 0 );
    // Selektionen wiederherstellen
    void SpellEnd( SwConversionArgs *pConvArgs = 0, bool bRestoreSelection = true );
    ::com::sun::star::uno::Any SpellContinue(
                    sal_uInt16* pPageCnt, sal_uInt16* pPageSt,
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
    sal_Bool HasConvIter() const;
    // Is hyphenation active somewhere else?
    sal_Bool HasHyphIter() const;

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
    sal_uInt16 GetRefMarks( SvStringsDtor* = 0 ) const;

    // rufe die Autokorrektur auf
    void AutoCorrect( SvxAutoCorrect& rACorr, sal_Bool bInsertMode = sal_True,
                        sal_Unicode cChar = ' ' );
    sal_Bool GetPrevAutoCorrWord( SvxAutoCorrect& rACorr, String& rWord );

    // dann setze nach entsprechenden Regeln unsere Vorlagen
    void AutoFormat( const SvxSwAutoFmtFlags* pAFlags = 0 );

    static SvxSwAutoFmtFlags* GetAutoFmtFlags();
    static void SetAutoFmtFlags(SvxSwAutoFmtFlags *);

    // errechnet die Selektion
    String Calculate();

    sal_Bool InsertURL( const SwFmtINetFmt& rFmt, const String& rStr,
                    sal_Bool bKeepSelection = sal_False );
    sal_uInt16 GetINetAttrs( SwGetINetAttrs& rArr );

    //SS Fuer holen/ersetzen DropCap-Inhalt
    String GetDropTxt( const sal_uInt16 nChars ) const;
    void   ReplaceDropTxt( const String &rStr );

    // may an outline be moved or copied?
    // Check whether it's in text body, not in table, and not read-only (move)
    sal_Bool IsOutlineMovable( sal_uInt16 nIdx ) const;
    sal_Bool IsOutlineCopyable( sal_uInt16 nIdx ) const;

    sal_uInt16 GetLineCount( sal_Bool bActPos = sal_True );

    // erfrage und setze den Fussnoten-Text/Nummer. Set.. auf akt. SSelection!
    sal_Bool GetCurFtn( SwFmtFtn* pToFillFtn = 0 );
    bool SetCurFtn( const SwFmtFtn& rFillFtn );
    bool HasFtns( bool bEndNotes = false ) const;
//z.Zt nicht benoetigt  sal_uInt16 GetFtnCnt( sal_Bool bEndNotes = sal_False ) const;
        // gebe Liste aller Fussnoten und deren Anfangstexte
    sal_uInt16 GetSeqFtnList( SwSeqFldList& rList, bool bEndNotes = false );

    SwSection const* InsertSection(
            SwSectionData & rNewData, SfxItemSet const*const = 0 );
    sal_Bool IsInsRegionAvailable() const;
    const SwSection* GetCurrSection() const;
    // liefert wie GetCurrSection() den aktuellen Bereich, allerdings geht diese Funktion
    // ueber die Frames und erwischt dabei auch den Bereich, wenn der Cursor in einer
    // Fussnote steht, deren Referenz in einem spaltigen Bereich steckt.
    // Wenn man bOutOfTab setzt, wird der Bereich gesucht,
    // der die Tabelle umfasst, nicht etwa ein innerer.
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

    String GetUniqueSectionName( const String* pChkStr = 0 ) const;

    //Attribute setzen
    void SetSectionAttr(const SfxItemSet& rSet, SwSectionFmt* pSectFmt = 0);

    // search inside the cursor selection for full selected sections.
    // if any part of section in the selection return 0.
    // if more than one in the selection return the count
    sal_uInt16 GetFullSelectedSectionCount() const;

    // special insert: Insert a new text node just before or after a section or
    // table, if the cursor is positioned at the start/end of said
    // section/table. The purpose of the method is to allow users to inert text
    // at certain 'impossible' position, e.g. before a table at the document
    // start or between to sections.
    bool DoSpecialInsert();
    bool CanSpecialInsert() const;

    // Optimierung UI
    void SetNewDoc(sal_Bool bNew = sal_True);

    sfx2::LinkManager& GetLinkManager();
    inline const sfx2::LinkManager& GetLinkManager() const;

    // linken Rand ueber Objectleiste einstellen (aenhlich dem Stufen von
    // Numerierungen), optional kann man "um" den Offset stufen oder "auf"
    // die Position gestuft werden (bModulus = sal_True)
    sal_Bool IsMoveLeftMargin( sal_Bool bRight = sal_True, sal_Bool bModulus = sal_True ) const;
    void MoveLeftMargin( sal_Bool bRight = sal_True, sal_Bool bModulus = sal_True );

    // Numberformatter vom Doc erfragen
          SvNumberFormatter* GetNumberFormatter();
    const SvNumberFormatter* GetNumberFormatter() const
    {   return ((SwEditShell*)this)->GetNumberFormatter();  }

    // Schnitstellen fuers GlobalDokument
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

    // alles fuers Redlining
    sal_uInt16 GetRedlineMode() const;
    void SetRedlineMode( sal_uInt16 eMode );
    sal_Bool IsRedlineOn() const;
    sal_uInt16 GetRedlineCount() const;
    const SwRedline& GetRedline( sal_uInt16 nPos ) const;
    sal_Bool AcceptRedline( sal_uInt16 nPos );
    sal_Bool RejectRedline( sal_uInt16 nPos );
    // suche das Redline zu diesem Data und returne die Pos im Array
    // USHRT_MAX wird returnt, falls nicht vorhanden
    sal_uInt16 FindRedlineOfData( const SwRedlineData& ) const;

    // Kommentar am Redline an der Position setzen
    sal_Bool SetRedlineComment( const String& rS );
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
    void SetLabelDoc( sal_Bool bFlag = sal_True );
    sal_Bool IsLabelDoc() const;

    // Schnittstelle fuer die TextInputDaten - ( fuer die Texteingabe
    // von japanischen/chinesischen Zeichen)
    SwExtTextInput* CreateExtTextInput(LanguageType eInputLanguage);
    String DeleteExtTextInput( SwExtTextInput* pDel = 0, sal_Bool bInsText = sal_True);
//  SwExtTextInput* GetExtTextInput() const;
    void SetExtTextInputData( const CommandExtTextInputData& );

    // Schnistelle fuer den Zugriff auf die AutoComplete-Liste
    static SwAutoCompleteWord& GetAutoCompleteWords();

    // returns a scaling factor of selected text. Used for the rotated
    // character attribut dialog.
    sal_uInt16 GetScalingOfSelectedText() const;

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
