/*************************************************************************
 *
 *  $RCSfile: editsh.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-23 20:05:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _EDITSH_HXX
#define _EDITSH_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _SVXSWAFOPT_HXX
#include <svx/swafopt.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>   // fuer Basisklasse
#endif
#ifndef _ITABENUM_HXX
#include <itabenum.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLALTERNATIVES_HPP_
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#endif

class PolyPolygon;
class SwDoc;
class DateTime;
class CommandExtTextInputData;

class SvNumberFormatter;
class SfxPoolItem;
class SfxItemSet;
class SvxLinkManager;
class SvxAutoCorrect;

class SwField;          // fuer Felder
class SwFieldType;
class SwDDEFieldType;
class SwNewDBMgr;

struct SwDocStat;
class SfxDocumentInfo;
class SvStringsDtor;
class SvStringsSort;
class SwAutoCompleteWord;

class SwFmtRefMark;
class SwFmtCol;
class SwNumRule;        // Numerierung
class SwNodeNum;        // Numerierung
class SwUndoIds;        // fuer Undo
class SwTxtFmtColl;
#ifdef USED
class SwGrfFmt;
class SwGrfFmtColl;
#endif
class SwGrfNode;
class SwFlyFrmFmt;

class SwFrmFmt;         // fuer GetTxtNodeFmts()
class SwCharFmt;
class SwExtTextInput;
class SwTxtNode;        // fuer IsNotMakeTxtNode
class SwRootFrm;        // fuer CTOR
class Graphic;          // fuer GetGraphic
class GraphicObject;    // fuer GetGraphicObj
class SwFmtINetFmt;     // InsertURL
class SvInPlaceObjectRef;
class SvInPlaceObject;
class SwTable;
class SwTextBlocks;     // fuer GlossaryRW
class SwBlockExceptList;
class SwFmtFtn;
class SpellCheck;
class SwSection;
class SwSectionFmt;
class SwTOXMarks;
class SwTOXBase;
class SwTOXType;
class SwTableAutoFmt;
class SwPageDesc;
class SchMemChart;
class Font;
class SwTxtINetFmt;
class SwSeqFldList;
class SwGlblDocContent;
class SwGlblDocContents;
class SwRedline;
class SwRedlineData;
class SwFtnInfo;
class SwEndNoteInfo;
class SwLineNumberInfo;
class SwAttrSet;
class SwAuthEntry;

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

class SwEditShell: public SwCrsrShell
{
    static SvxSwAutoFmtFlags* pAutoFmtFlags;

    // fuer die privaten Methoden DelRange und die vom AutoCorrect
    friend class SwAutoFormat;
    friend void _InitCore();
    friend void _FinitCore();
    // fuer die PamCorrAbs/-Rel Methoden
    friend class SwUndo;

    SfxPoolItem& _GetChrFmt( SfxPoolItem& ) const;

    /*
     * liefert einen Pointer auf einen SwGrfNode; dieser wird von
     * GetGraphic() und GetGraphicSize() verwendet.
     */
    SwGrfNode *_GetGrfNode() const ;

    void DeleteSel( SwPaM& rPam, BOOL* pUndo = 0 );

    void _SetSectionAttr( SwSectionFmt& rSectFmt, const SfxItemSet& rSet );

public:
    // Editieren (immer auf allen selektierten Bereichen)
    void Insert( sal_Unicode );
    void Insert( const String &);
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

    // Voellig private Methode. Nur fuer JOE's-Wizzards
    BOOL DelFullPara();

    // loesche den nicht sichtbaren Content aus dem Document, wie z.B.:
    // versteckte Bereiche, versteckte Absaetze
    BOOL RemoveInvisibleContent();

    // embedded alle lokalen Links (Bereiche/Grafiken)
    BOOL EmbedAllLinks();
    USHORT GetLinkUpdMode(BOOL bDocSettings = FALSE) const;
    void SetLinkUpdMode( USHORT nMode );

    // verschiebe den Inhalt aller Bereiche an die akt. Cursor-Position
    long Move();

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
    BOOL GetAttr( SfxItemSet& ) const;
    void SetAttr( const SfxPoolItem&, USHORT nFlags = 0 );
    void SetAttr( const SfxItemSet&, USHORT nFlags = 0 );

    // Setze das Attribut als neues default Attribut im Dokument.
    void SetDefault( const SfxPoolItem& );

    // Erfrage das Default Attribut vom Dokument.
    const SfxPoolItem& GetDefault( USHORT nFmtHint ) const;

    void ResetAttr();
    void GCAttr();

    // returns the scripttpye of the selection
    USHORT GetScriptType() const;

    // FrameFormate
    SwFrmFmt* FindFrmFmtByName( const String& rName ) const;

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
    void SetTxtFmtColl(SwTxtFmtColl*);
    SwTxtFmtColl *MakeTxtFmtColl(const String &rFmtCollName,
        SwTxtFmtColl *pDerivedFrom = 0);
    void FillByEx(SwTxtFmtColl*, BOOL bReset = FALSE);
    SwTxtFmtColl* FindTxtFmtCollByName( const String& rName ) const;
#ifdef USED
    // GRF
    USHORT GetGrfFmtCollCount() const;
    SwGrfFmtColl& GetGrfFmtColl( USHORT nGrfFmtColl) const;
    void DelGrfFmtColl(USHORT nFmt);
    SwGrfFmtColl* GetCurGrfFmtColl() const;
    void SetGrfFmtColl(SwGrfFmtColl*);
    SwGrfFmtColl *MakeGrfFmtColl(const String &rFmtCollName,
        SwGrfFmtColl *pDerivedFrom = 0);
    SwGrfFmtColl* FindGrfFmtCollByName( const String& rName ) const;
#endif

        // Gebe die "Auto-Collection" mit der Id zurueck. Existiert
        // sie noch nicht, dann erzuege sie
    SwTxtFmtColl* GetTxtCollFromPool( USHORT nId );
        // return das geforderte automatische  Format - Basis-Klasse !
    SwFmt* GetFmtFromPool( USHORT nId );
        // returne die geforderte automatische Seiten-Vorlage
    SwPageDesc* GetPageDescFromPool( USHORT nId );
        // returne die geforderte automatische NumRule
    SwNumRule* GetNumRuleFromPool( USHORT nId );

    // erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird
    BOOL IsUsed( const SwModify& ) const;
    BOOL IsUsed( const SwNumRule& ) const;
        // ist der Name ein Pool-Vorlagen-Name, returne seine ID,
        // sonst USHRT_MAX
    USHORT GetPoolId( const String& rName, SwGetPoolIdFromName eFlag ) const;

    const SvStringsDtor& GetChrFmtNmArray() const;
    const SvStringsDtor& GetHTMLChrFmtNmArray() const;

        // returne das geforderte automatische Format
    SwFrmFmt* GetFrmFmtFromPool( USHORT nId )
        { return (SwFrmFmt*)SwEditShell::GetFmtFromPool( nId ); }
    SwCharFmt* GetCharFmtFromPool( USHORT nId )
        { return (SwCharFmt*)SwEditShell::GetFmtFromPool( nId ); }

    // Felder
    void Insert(SwField&);
    SwField* GetCurFld() const;

    void UpdateFlds( SwField & );       // ein einzelnes Feld

    USHORT GetFldTypeCount(USHORT nResId = USHRT_MAX, BOOL bUsed = FALSE) const;
    SwFieldType* GetFldType(USHORT nId, USHORT nResId = USHRT_MAX, BOOL bUsed = FALSE) const;
    SwFieldType* GetFldType(USHORT nResId, const String& rName) const;

    void RemoveFldType(USHORT nId, USHORT nResId = USHRT_MAX);
    void RemoveFldType(USHORT nResId, const String& rName);
    BOOL RenameUserFields(const String& rOldName, const String& rNewName);

    void FieldToText( SwFieldType* pType );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    // Datenbankinfo
    String GetDBName() const;
    const String& GetDBDesc() const;
    void ChgDBName(const String& rNewName);
    void ChangeDBFields( const SvStringsDtor& rOldNames,
                         const String& rNewName );
    void GetAllUsedDB( SvStringsDtor& rDBNameList,
                        SvStringsDtor* pAllDBNames = 0 );

    void UpdateExpFlds(BOOL bCloseDB = FALSE);// nur alle ExpressionFelder updaten
    void SetFixFields( BOOL bOnlyTimeDate = FALSE,
                        const DateTime* pNewDateTime = 0 );
    void LockExpFlds();
    void UnlockExpFlds();
    BOOL IsExpFldsLocked() const;

    USHORT GetFldUpdateFlags(BOOL bDocSettings = FALSE) const;
    void SetFldUpdateFlags( USHORT eFlags );

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

    // Dokument - Info
    const SfxDocumentInfo* GetInfo() const;

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
    const SwAttrSet&    GetTOXBaseAttrSet(const SwTOXBase& rTOX) const;
    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, BOOL bCreate = FALSE );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    BOOL                IsTOXBaseReadonly(const SwTOXBase& rTOXBase) const;
    void                SetTOXBaseReadonly(const SwTOXBase& rTOXBase, BOOL bReadonly);

    USHORT              GetTOXCount() const;
    const SwTOXBase*    GetTOX( USHORT nPos ) const;
    BOOL                DeleteTOX( const SwTOXBase& rTOXBase, BOOL bDelNodes = FALSE );
    BOOL                DeleteCurTOX();

    String GetUniqueTOXBaseName( const SwTOXType& rType,
                                const String* pChkStr = 0 ) const;

    BOOL SetTOXBaseName(const SwTOXBase& rTOXBase, const String& rName);
    void SetTOXBaseProtection(const SwTOXBase& rTOXBase, BOOL bProtect);

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
    void SetCurNumRule(const SwNumRule&);
    // Absaetze ohne Numerierung, aber mit Einzuegen
    BOOL NoNum();
    // Loeschen, Splitten der Aufzaehlungsliste
    BOOL DelNumRules();
    // Hoch-/Runterstufen
    BOOL NumUpDown( BOOL bDown = TRUE );
    // Hoch-/Runtermoven sowohl innerhalb als auch ausserhalb von Numerierungen
    BOOL MoveParagraph( long nOffset = 1);
    BOOL MoveNumParas( BOOL bUpperLower, BOOL bUpperLeft );
    // No-/Numerierung ueber Delete/Backspace ein/abschalten
    BOOL NumOrNoNum( BOOL bDelete = FALSE, BOOL bChkStart = TRUE,
                        BOOL bOutline = FALSE );
    BOOL IsNoNum( BOOL bChkStart = TRUE, BOOL bOutline = FALSE ) const;
    // returne den Num-Level des Nodes, in dem sich der Point vom
    // Cursor befindet. Return kann sein :
    // - NO_NUMBERING, 0..MAXLEVEL-1, NO_NUMLEVEL .. NO_NUMLEVEL|MAXLEVEL-1
    BYTE GetNumLevel( BOOL* pHasChilds = 0 ) const;
    String GetUniqueNumRuleName( const String* pChkStr = 0, BOOL bAutoNum = TRUE ) const;
    void ChgNumRuleFmts( const SwNumRule& rRule );
    // setze und erfrage, ob an aktueller PointPos eine Numerierung mit
    // dem StartFlag startet
    void SetNumRuleStart( BOOL bFlag = TRUE );
    BOOL IsNumRuleStart() const;
    void SetNodeNumStart( USHORT nStt = USHRT_MAX );
    USHORT IsNodeNumStart() const;
    BOOL ReplaceNumRule( const String& rOldRule, const String& rNewRule );

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
    USHORT StartUndo( USHORT nUndoId = 0 );
    // schliesst Klammerung der nUndoId, nicht vom UI benutzt
    USHORT EndUndo( USHORT nUndoId = 0 );
    // liefert die Id der letzten undofaehigen Aktion zurueck
    // fuellt ggf. VARARR mit User-UndoIds
    USHORT GetUndoIds( String* pUndoStr = 0, SwUndoIds *pUndoIds = 0) const;

        // abfragen/setzen der Anzahl von wiederherstellbaren Undo-Actions
    static USHORT GetUndoActionCount();
    static void SetUndoActionCount( USHORT nNew );

    // Redo
    // liefert die Id der letzten Redofaehigen Aktion zurueck
    // fuellt ggf. VARARR mit RedoIds
    USHORT GetRedoIds( String* pRedoStr = 0, SwUndoIds *pRedoIds = 0) const;

    // Repeat
    // liefert die Id der letzten Repeatfaehigen Aktion zurueck
    // fuellt ggf. VARARR mit RedoIds
    USHORT GetRepeatIds( String* pRepeatStr = 0, SwUndoIds *pRedoIds = 0) const;

    // 0 letzte Aktion, sonst Aktionen bis zum Start der Klammerung nUndoId
    // mit KillPaMs, ClearMark
    BOOL Undo(USHORT nUndoId = 0);
    // wiederholt
    USHORT Repeat( USHORT nCount );
    // wiederholt
    USHORT Redo();
#ifdef USED
    // Aktionen klammern
    void StartAction();
    void EndAction();
#endif
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
    const Graphic &GetGraphic( BOOL bWait = TRUE ) const;
    const GraphicObject &GetGraphicObj() const;
    BOOL IsGrfSwapOut( BOOL bOnlyLinked = FALSE ) const;
    USHORT GetGraphicType() const;

    const PolyPolygon *GetGraphicPolygon() const;
    void SetGraphicPolygon( const PolyPolygon *pPoly );

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
                  const Graphic* pGraphic = 0 );

    // alternativen Text einer Grafik/OLe-Objectes abfragen/setzen
    const String& GetAlternateText() const;
    void SetAlternateText( const String& rTxt );

    //eindeutige Identifikation des Objektes (fuer ImageMapDlg)
    void    *GetIMapInventor() const;
    Graphic GetIMapGraphic( BOOL bWait = TRUE ) const; //liefert eine Graphic fuer alle Flys!
    const SwFlyFrmFmt* FindFlyByName( const String& rName, BYTE nNdTyp = 0 ) const;

    //liefert ein ClientObject, wenn CurCrsr->Point() auf einen
    //SwOLENode zeigt (und Mark nicht gesetzt ist oder auf das
    //gleiche ClientObject zeigt), sonst gibt's was auf die
    //Finger.
    SvInPlaceObjectRef GetOLEObj() const;
    //Gibt es ein OleObject mit diesem Namen (SwFmt)?
    BOOL HasOLEObj( const String &rName ) const;

    //Liefert den Pointer auf die Daten des Chart, indem sich der Crsr
    //befindet.
    void SetChartName( const String &rName );
    // returne den ChartNamen - vom Crsr oder vom uebergebenen OLE-Object
    // reurnt aEmptyStr wenn nicht gefunden wurde
    const String& GetChartName( SvInPlaceObject* pObj = 0 );
    //Sucht die Tabelle und liefert ein mit den Daten der Tabelle gefuelltes
    //pData. Wenn pData 0 ist wird eines angelegt.
    void UpdateChartData( const String &rName, SchMemChart *&pData );
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
                         BOOL bSaveRelNet = FALSE, const String* pOnlyTxt=0 );
    // speicher den gesamten Inhalt des Docs als Textbaustein
    USHORT SaveGlossaryDoc( SwTextBlocks& rGlossary, const String& rName,
                            const String& rShortName,
                            BOOL bSaveRelFile = FALSE,
                            BOOL bSaveRelNet = FALSE,
                            BOOL bOnlyTxt = FALSE );

    // Linguistik
    // Selektionen sichern
    void HyphStart( SwDocPositions eStart, SwDocPositions eEnde );
    // Selektionen wiederherstellen
    void HyphEnd();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                HyphContinue( USHORT* pPageCnt, USHORT* pPageSt );
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                SpellContinue( USHORT* pPageCnt, USHORT* pPageSt );
    // zu trennendes Wort ignorieren
    void HyphIgnore();

    // zum Einfuegen des SoftHyphens, Position ist der Offset
    // innerhalb des getrennten Wortes.
    void InsertSoftHyph( const xub_StrLen nHyphPos );

    //Tabelle
    const SwTable& InsertTable(USHORT nRows, USHORT nCols,
                                SwHoriOrient = HORI_FULL,
                                USHORT nInsert = ALL_TBL_INS_ATTR,
                                const SwTableAutoFmt* pTAFmt = 0 );
    void InsertDDETable( SwDDEFieldType* pDDEType,
                            USHORT nRows, USHORT nCols,
                            SwHoriOrient eAdj = HORI_FULL,
                            USHORT nInsert = HEADLINE_NO_BORDER );
    void UpdateTable();
    void SetTableName( SwFrmFmt& rTblFmt, const String &rNewName );

    SwFrmFmt *GetTableFmt();
    BOOL TextToTable( sal_Unicode cCh, SwHoriOrient eAdj = HORI_FULL,
                        USHORT nInsert = ALL_TBL_INS_ATTR,
                        const SwTableAutoFmt* pTAFmt = 0 );
    BOOL TableToText( sal_Unicode cCh );
    FASTBOOL IsTextToTableAvailable() const;

    BOOL GetTblBoxFormulaAttrs( SfxItemSet& rSet ) const;
    void SetTblBoxFormulaAttrs( const SfxItemSet& rSet );

    BOOL IsTableBoxTextFormat() const;
    // Change Modus erfragen/setzen
    USHORT GetTblChgMode() const;
    void SetTblChgMode( USHORT eMode );

    // Zellenbreiten ueber Min/Max Berechnung an Tabellenbreite anpassen
    void OptimizeTblBoxWidthMinMax();
    // Tabelle an der Cursor Position aufsplitten
    BOOL SplitTable( USHORT eMode );
    // Tabellen verbinden
    // CanMerge kann feststellen, ob Prev oder Next moeglich ist. Wird
    // der Pointer pChkNxtPrv uebergeben, wird festgestellt in welche
    // Richtung es moeglich ist.
    BOOL CanMergeTable( BOOL bWithPrev = TRUE, BOOL* pChkNxtPrv = 0 ) const;
    BOOL MergeTable( BOOL bWithPrev = TRUE, USHORT nMode = 0 );

    // Selektionen sichern
    void SpellStart( SwDocPositions eStart, SwDocPositions eEnde,
                        SwDocPositions eCurr );
    // Selektionen wiederherstellen
    void SpellEnd();

    // Is spelling active somewhere else?
    BOOL HasSpellIter() const;
    // Is hyphenation active somewhere else?
    BOOL HasHyphIter() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
            GetCorrection( const Point* pPt );

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

    BOOL InsertURL( const SwFmtINetFmt& rFmt, const String& rStr, BOOL bKeepSelection = FALSE );
    USHORT GetINetAttrs( SwGetINetAttrs& rArr );

    //SS Fuer holen/ersetzen DropCap-Inhalt
    String GetDropTxt( const USHORT nChars ) const;
    void   ReplaceDropTxt( const String &rStr );

    // Abfrage von Oultine Informationen:
    USHORT GetOutlineCnt() const;

    BYTE GetOutlineLevel( USHORT nIdx ) const;
    // mit exp. Felder und KapitelNummern
    String GetOutlineText( USHORT nIdx, BOOL bWithNum = TRUE ) const;
    // die Nummer
    const SwNodeNum* GetOutlineNum( USHORT nIdx ) const;

    // darf verschoben werden?
    BOOL IsOutlineMovable( USHORT nIdx ) const;

    USHORT GetLineCount( BOOL bActPos = TRUE );

    // erfrage und setze den Fussnoten-Text/Nummer. Set.. auf akt. SSelection!
    BOOL GetCurFtn( SwFmtFtn* pToFillFtn = 0 );
    BOOL SetCurFtn( const SwFmtFtn& rFillFtn );
    BOOL HasFtns( BOOL bEndNotes = FALSE ) const;
//z.Zt nicht benoetigt  USHORT GetFtnCnt( BOOL bEndNotes = FALSE ) const;
        // gebe Liste aller Fussnoten und deren Anfangstexte
    USHORT GetSeqFtnList( SwSeqFldList& rList, BOOL bEndNotes = FALSE );

    // SS fuer Bereiche
    const SwSection* InsertSection( const SwSection& rNew,
                                    const SfxItemSet* = 0 );
    BOOL IsInsRegionAvailable() const;
    const SwSection* GetCurrSection() const;
    // liefert wie GetCurrSection() den aktuellen Bereich, allerdings geht diese Funktion
    // ueber die Frames und erwischt dabei auch den Bereich, wenn der Cursor in einer
    // Fussnote steht, deren Referenz in einem spaltigen Bereich steckt.
    // Wenn man bOutOfTab setzt, wird der Bereich gesucht,
    // der die Tabelle umfasst, nicht etwa ein innerer.
    const SwSection* GetAnySection( BOOL bOutOfTab = FALSE ) const;

    USHORT GetSectionFmtCount() const;
    USHORT GetSectionFmtPos( const SwSectionFmt& ) const;
    const SwSectionFmt& GetSectionFmt(USHORT nFmt) const;
    void DelSectionFmt( USHORT nFmt );
    void ChgSection( USHORT nSect, const SwSection&, const SfxItemSet* = 0 );
    BOOL IsAnySectionInDoc( BOOL bChkReadOnly = FALSE,
                            BOOL bChkHidden = FALSE,
                            BOOL BChkTOX = FALSE ) const;
        // Passwort fuer geschuetzte Bereiche erfragen/setzen
    void ChgSectionPasswd( const String& sNew );
    const String& GetSectionPasswd() const;
    String GetUniqueSectionName( const String* pChkStr = 0 ) const;

    //Attribute setzen
    void SetSectionAttr(const SfxItemSet& rSet, SwSectionFmt* pSectFmt = 0);

    // search inside the cursor selection for full selected sections.
    // if any part of section in the selection return 0.
    // if more than one in the selection return the count
    USHORT GetFullSelectedSectionCount() const;

    BOOL AppendNodeInSection();
    int CanInsertNodeAtEndOfSection() const;

    // Optimierung UI
    void SetNewDoc(BOOL bNew = TRUE);

          SvxLinkManager& GetLinkManager();
    inline const SvxLinkManager& GetLinkManager() const;

    // linken Rand ueber Objectleiste einstellen (aenhlich dem Stufen von
    // Numerierungen), optional kann man "um" den Offset stufen oder "auf"
    // die Position gestuft werden (bModulus = TRUE)
    BOOL IsMoveLeftMargin( BOOL bRight = TRUE, BOOL bModulus = TRUE ) const;
    void MoveLeftMargin( BOOL bRight = TRUE, BOOL bModulus = TRUE );

    // Numberformatter vom Doc erfragen
          SvNumberFormatter* GetNumberFormatter();
    const SvNumberFormatter* GetNumberFormatter() const
    {   return ((SwEditShell*)this)->GetNumberFormatter();  }

    // Extrakt fuellen
    void Summary( SwDoc* pExtDoc, BYTE nLevel, BYTE nPara, BOOL bImpress );

    // Schnitstellen fuers GlobalDokument
    void SetGlobalDoc( BOOL bFlag = TRUE );
    BOOL IsGlobalDoc() const;
    void SetGlblDocSaveLinks( BOOL bFlag = TRUE );
    BOOL IsGlblDocSaveLinks() const;
    USHORT GetGlobalDocContent( SwGlblDocContents& rArr ) const;
    BOOL InsertGlobalDocContent( const SwGlblDocContent& rPos,
                                 const SwSection& rNew );
    BOOL InsertGlobalDocContent( const SwGlblDocContent& rPos,
                                 const SwTOXBase& rTOX );
    BOOL InsertGlobalDocContent( const SwGlblDocContent& rPos );
    BOOL DeleteGlobalDocContent( const SwGlblDocContents& rArr,
                                USHORT nPos );
    BOOL MoveGlobalDocContent( const SwGlblDocContents& rArr ,
                                USHORT nFromPos, USHORT nToPos,
                                USHORT nNewPos );
    BOOL GotoGlobalDocContent( const SwGlblDocContent& rPos );
    // erzeuge Anhand der vorgebenen Collection Teildokumente
    // falls keine angegeben ist, nehme die Kapitelvorlage der 1. Ebene
    BOOL GenerateGlobalDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );
    BOOL GenerateHTMLDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );

    // alles fuers Redlining
    USHORT GetRedlineMode() const;
    void SetRedlineMode( USHORT eMode );
    BOOL IsRedlineOn() const;
    USHORT GetRedlineCount() const;
    const SwRedline& GetRedline( USHORT nPos ) const;
    BOOL AcceptRedline( USHORT nPos );
    BOOL AcceptRedline();
    BOOL RejectRedline( USHORT nPos );
    BOOL RejectRedline();
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
    SwExtTextInput* CreateExtTextInput();
    void DeleteExtTextInput( SwExtTextInput* pDel = 0, BOOL bInsText = TRUE );
    SwExtTextInput* GetExtTextInput() const;
    void SetExtTextInputData( const CommandExtTextInputData& );

    // Schnistelle fuer den Zugriff auf die AutoComplete-Liste
    static SwAutoCompleteWord& GetAutoCompleteWords();

    // ctor/dtor
    SwEditShell( SwDoc&, Window*,
                 SwRootFrm* = 0, const SwViewOption *pOpt = 0 );
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

inline const SvxLinkManager& SwEditShell::GetLinkManager() const
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
