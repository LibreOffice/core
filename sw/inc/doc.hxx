/*************************************************************************
 *
 *  $RCSfile: doc.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-05 12:04:44 $
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
#ifndef _DOC_HXX
#define _DOC_HXX

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _NDARR_HXX
#include <ndarr.hxx>
#endif
#ifndef _SWATRSET_HXX
#include <swatrset.hxx>
#endif
#ifndef _TOXE_HXX
#include <toxe.hxx>             // enums
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>         // enums
#endif
#ifndef _BKMRKE_HXX
#include <bkmrke.hxx>
#endif
#ifndef _FLYENUM_HXX
#include <flyenum.hxx>
#endif
#ifndef _ITABENUM_HXX
#include <itabenum.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic/XSpellChecker1.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic/XHyphenatedWord.hpp>
#endif

class SwExtTextInput;
class DateTime;
class EditFieldInfo;
class JobSetup;
class Color;
class KeyCode;
class Outliner;
class OutputDevice;
class Point;
class SbxArray;
class SdrModel;
class SdrObject;
class SdrUndoAction;
class SfxDocumentInfo;
class SfxPrinter;
class SvData;
class SvEmbeddedObjectRef;
class SvNumberFormatter;
class SvPersist;
class SvPseudoObject;
class SvStorage;
class SvStrings;
class SvStringsSort;
class SvUShorts;
class SvUShortsSort;
class SvxAutoCorrDoc;
class SvxLinkManager;
class SvxMacro;
class SvxMacroTableDtor;
class SvxBorderLine;
class SwAutoCompleteWord;
class SwAutoCorrExceptWord;
class SwBookmark;
class SwBookmarks;
class SwCalc;
class SwCellFrm;
class SwCharFmt;
class SwCharFmts;
class SwClient;
class SwConditionTxtFmtColl;
class SwCrsrShell;
class SwCursor;
class SwDBNameInfField;
class SwDocShell;
class SwDocUpdtFld;
class SwDrawFrmFmt;
class SwDrawView;
class SwEditShell;
class SwFieldType;
class SwFldTypes;
class SwFlyFrm;
class SwFlyFrmFmt;
class SwFmt;
class SwFmtCol;
class SwFmtINetFmt;
class SwFmtRefMark;
class SwFrmFmt;
class SwFrmFmts;
class SwFtnIdxs;
class SwFtnInfo;
class SwEndNoteInfo;
class SwGrfFmtColl;
class SwGrfFmtColls;
class SwLineNumberInfo;
class SwNewDBMgr;
class SwNoTxtNode;
class SwNodeIndex;
class SwNodeRange;
class SwNumRule;
class SwNumRuleTbl;
class SwPageDesc;
class SwPosFlyFrms;
class SwPagePreViewPrtData;
class SwRedline;
class SwRedlineTbl;
class SwRootFrm;
class SwSectionFmt;
class SwSectionFmts;
class SwSelBoxes;
class SwSpzFrmFmts;
class SwTOXBase;
class SwTOXBaseSection;
class SwTOXMark;
class SwTOXMarks;
class SwTOXType;
class SwTOXTypes;
class SwTabCols;
class SwTable;
class SwTableAutoFmt;
class SwTableBox;
class SwTableBoxFmt;
class SwTableFmt;
class SwTableLineFmt;
class SwTableNode;
class SwTextBlocks;
class SwTxtFld;
class SwTxtFmtColl;
class SwTxtFmtColls;
class SwURLStateChanged;
class SwUndo;
class SwUndoIds;
class SwUndoIter;
class SwUndos;
class SwUnoCrsr;
class SwUnoCrsrTbl;
class ViewShell;
class _SetGetExpFld;
class SwDrawContact;
class SwLayouter;
class SdrView;
class SdrMarkList;
class SwAuthEntry;
class SwUnoCallBack;
struct SwCallMouseEvent;
struct SwDocStat;
struct SwHash;
struct SwSortOptions;
struct SwDefTOXBase_Impl;

//PageDescriptor-Schnittstelle, Array hier wegen inlines.
typedef SwPageDesc* SwPageDescPtr;
SV_DECL_PTRARR_DEL( SwPageDescs, SwPageDescPtr, 4, 4 );

enum SwMoveFlags
{
   DOC_MOVEDEFAULT      = 0x00,
   DOC_MOVEALLFLYS      = 0x01,
   DOC_CREATEUNDOOBJ    = 0x02,
   DOC_MOVEREDLINES     = 0x04
};

#define DUMMY_PARASPACEMAX          0x04
#define DUMMY_PARASPACEMAX_AT_PAGES 0x20


#define SW_HYPH_ERROR       0
#define SW_HYPH_OK          1
#define SW_HYPH_ABORTED     2
#define SW_HYPH_CONTINUE    3

class SwDoc
{
    friend class SwReader;    // fuers Undo von Insert(SwDoc)
    friend class SwSwgReader; // Zugriff auf bDtor-Flag (wg. Loeschen v.Frames)
    friend class Sw3IoImp;    // Zugriff u.a. auf den Drawing Layer
    friend class SwCompareData; // fuers Undo vom CompareDoc

    friend void _InitCore();
    friend void _FinitCore();

    //Fuer das Loeschen der Header-/Footer-Formate
    //Implementierung und Benutzung in frmatr.cxx
    friend void DelHFFormat( SwClient *, SwFrmFmt * );

#if !( defined(PRODUCT) || defined(MAC) || defined(PM2) )
    friend class SwUndoWriter;  // fuers Schreiben der Undo/Redo-History
#endif

    //---------------- private Member --------------------------------

    // -------------------------------------------------------------------
    // die Objecte
    SwNodes     aNodes;                 // Inhalt des Dokumentes
    SwNodes     aUndoNodes;             // Inhalt fuer das Undo
    SwAttrPool  aAttrPool;              // der Attribut Pool
    SwPageDescs aPageDescs;             // PageDescriptoren
    Link        aOle2Link;              // OLE 2.0-Benachrichtigung
    AutoTimer   aIdleTimer;             // der eigene IdleTimer
    Timer       aChartTimer;            // der Timer fuers Update aller Charts
    Timer       aOLEModifiedTimer;      // Timer for update modified OLE-Objecs
    String      aDBName;                // logischer Datenbankname
    String      sSectionPasswd;         // Passwort fuer geschuetzte Bereiche
    String      sTOIAutoMarkURL;        // ::com::sun::star::util::URL of table of index AutoMark file
    SvStringsDtor aPatternNms;          // Array fuer die Namen der Dokument-Vorlagen


    // -------------------------------------------------------------------
    // die Pointer
                                //Defaultformate
    SwFrmFmt        *pDfltFrmFmt;
    SwFrmFmt        *pEmptyPageFmt;     // Format fuer die Default-Leerseite.
    SwFrmFmt        *pColumnContFmt;    // Format fuer Spaltencontainer
    SwCharFmt       *pDfltCharFmt;
    SwTxtFmtColl    *pDfltTxtFmtColl;   // Defaultformatcollections
    SwGrfFmtColl    *pDfltGrfFmtColl;

    SwFrmFmts       *pFrmFmtTbl;        // Formattabellen
    SwCharFmts      *pCharFmtTbl;
    SwSpzFrmFmts    *pSpzFrmFmtTbl;
    SwSectionFmts   *pSectionFmtTbl;
    SwFrmFmts       *pTblFrmFmtTbl;     // spz. fuer Tabellen
    SwTxtFmtColls   *pTxtFmtCollTbl;    // FormatCollections
    SwGrfFmtColls   *pGrfFmtCollTbl;

    SwBookmarks     *pBookmarkTbl;      //Bookmarks

    SwTOXTypes      *pTOXTypes;         // Verzeichnisse
    SwDefTOXBase_Impl * pDefTOXBases;   // defaults of SwTOXBase's

    SwRootFrm       *pLayout;           // Rootframe des spezifischen Layouts.
    SdrModel        *pDrawModel;        // StarView Drawing

    SwUndos         *pUndos;            // Undo/Redo History

    SwDocUpdtFld    *pUpdtFlds;         // Struktur zum Field-Update
    SwFldTypes      *pFldTypes;         // Feldtypen
    SwNewDBMgr      *pNewDBMgr;         // Pointer auf den neuen DBMgr fuer
                                        // Evaluierung der DB-Fields
    SfxPrinter      *pPrt;

    SwDoc           *pGlossaryDoc;      // Pointer auf das Glossary-Dokument. Dieses

    SwNumRule       *pOutlineRule;
    SwFtnInfo       *pFtnInfo;
    SwEndNoteInfo   *pEndNoteInfo;
    SwLineNumberInfo *pLineNumberInfo;
    SwFtnIdxs       *pFtnIdxs;
    SwDocStat       *pDocStat;          // Statistic Informationen
    SfxDocumentInfo *pSwgInfo;          // Dokumentinformationen
    SvxMacroTableDtor *pMacroTable;     // Tabelle der dokumentglobalen Macros

    SwDocShell      *pDocShell;         // Ptr auf die SfxDocShell vom Doc
    SvEmbeddedObjectRef* pDocShRef;     // fuers Kopieren von OLE-Nodes (wenn keine
                                        // DocShell gesetzt ist, muss dieser
                                        // Ref-Pointer gesetzt sein!!!!)
    SvxLinkManager  *pLinkMgr;          // Liste von Verknuepften (Grafiken/DDE/OLE)

    SwAutoCorrExceptWord *pACEWord;     // fuer die automatische Uebernahme von
                                        // autokorrigierten Woertern, die "zurueck"
                                        // verbessert werden
    SwURLStateChanged *pURLStateChgd;   // SfxClient fuer Aenderungen in der
                                        // INetHistory
    SvNumberFormatter *pNumberFormatter;    // NumFormatter fuer die Tabellen/Felder
    SwNumRuleTbl    *pNumRuleTbl;           // Liste aller benannten NumRules
    SwRedlineTbl    *pRedlineTbl;           // Liste aller Redlines
    String          *pAutoFmtRedlnComment;  // Kommentar fuer Redlines, die
                                        // uebers Autoformat eingefuegt werden
    SwUnoCrsrTbl    *pUnoCrsrTbl;

    SwPagePreViewPrtData *pPgPViewPrtData;      // Einzuege/Abstaende fuers
                                        // Drucken der Seitenansicht
    SwPaM           *pExtInputRing;

    SwLayouter      *pLayouter;     // ::com::sun::star::frame::Controller for complex layout formatting
                                    // like footnote/endnote in sections

    SwUnoCallBack   *pUnoCallBack;
    // -------------------------------------------------------------------
    // sonstige
    sal_uInt16  nUndoPos;               // akt. Undo-InsertPosition (fuers Redo!)
    sal_uInt16  nUndoSavePos;           // Position im Undo-Array, ab der das Doc
                                    // nicht als modifiziert gilt
    sal_uInt16  nUndoCnt;               // Anzahl von Undo Aktionen
    sal_uInt16  nUndoSttEnd;            // != 0 -> innerhalb einer Klammerung

    sal_uInt16 nAutoFmtRedlnCommentNo;  // SeqNo fuers UI-seitige zusammenfassen
                                    // von AutoFmt-Redlines. Wird vom SwAutoFmt
                                    // verwaltet!
    sal_uInt16  nLinkUpdMode;           // UpdateMode fuer Links
    sal_uInt16  nFldUpdMode;            // Mode fuer Felder/Charts automatisch aktualisieren
    sal_uInt16  nLatin_CJK;         // distance between Latin- and CJK-script
    sal_uInt16  nLatin_CTL;         // distance between Latin- and CTL-script
    sal_uInt16  nCJK_CTL;           // distance between CJK- and CTL-script

    SwRedlineMode eRedlineMode;     // aktueller Redline Modus

    sal_Int8    nLinkCt;            // wieviele kennen das Dokument
    sal_Int8    nLockExpFld;        // Wenn != 0 hat UpdateExpFlds() keine Wirkung

    sal_Int8    nHeaven;            // LayerIds, Heaven == ueber dem Dokument
    sal_Int8    nHell;              //           Hell   == unter dem Dokument
    sal_Int8    nControls;          //           Controls == ganz oben

    sal_Bool    bGlossDoc       : 1;    //TRUE: ist ein Textbaustein Dokument
    sal_Bool    bModified       : 1;    //TRUE: Dokument ist veraendert
    sal_Bool    bDtor           : 1;    //TRUE: ist im SwDoc DTOR
                                        // leider auch temporaer von
                                        // SwSwgReader::InLayout(), wenn fehlerhafte
                                        //Frames geloescht werden muessen
    sal_Bool    bUndo           : 1;    // TRUE: Undo eingeschaltet
    sal_Bool    bGroupUndo      : 1;    // TRUE: Undos werden gruppiert
    sal_Bool    bPageNums       : 1;    // TRUE: es gibt virtuelle Seitennummern
    sal_Bool    bLoaded         : 1;    // TRUE: ein geladenes Doc
    sal_Bool    bUpdateExpFld   : 1;    // TRUE: Expression-Felder updaten
    sal_Bool    bNewDoc         : 1;    // TRUE: neues Doc
    sal_Bool    bNewFldLst      : 1;    // TRUE: Felder-Liste neu aufbauen
    sal_Bool    bCopyIsMove     : 1;    // TRUE: Copy ist ein verstecktes Move
    sal_Bool    bNoDrawUndoObj  : 1;    // TRUE: keine DrawUndoObjecte speichern
    sal_Bool    bVisibleLinks   : 1;    // TRUE: Links werden sichtbar eingefuegt
    sal_Bool    bBrowseMode     : 1;    // TRUE: Dokument im BrowseModus anzeigen
    sal_Bool    bInReading      : 1;    // TRUE: Dokument wird gerade gelesen
    sal_Bool    bUpdateTOX      : 1;    // TRUE: nach Dokument laden die TOX Updaten
    sal_Bool    bInLoadAsynchron: 1;    // TRUE: Dokument wird gerade asynchron geladen
    sal_Bool    bHTMLMode       : 1;    // TRUE: Dokument ist im HTMLMode
    sal_Bool    bHeadInBrowse   : 1;    // TRUE: Header sind im BrowseMode aktiviert
    sal_Bool    bFootInBrowse   : 1;    // TRUE: Footer sind im BrowseMode aktiviert
    sal_Bool    bFrmBeepEnabled : 1;    // TRUE: Frames beepen wenn der Inhalt zu gross wird
    sal_Bool    bInCallModified : 1;    // TRUE: im Set/Reset-Modified Link
    sal_Bool    bIsGlobalDoc    : 1;    // TRUE: es ist ein GlobalDokument
    sal_Bool    bGlblDocSaveLinks : 1;  // TRUE: im GlobalDoc. gelinkte Sect. mit speichern
    sal_Bool    bIsLabelDoc     : 1;    // TRUE: es ist ein Etiketten-Dokument
    sal_Bool    bIsAutoFmtRedline : 1;  // TRUE: die Redlines werden vom Autoformat aufgezeichnet
    sal_Bool    bOLEPrtNotifyPending:1; // TRUE: Printer  ist geaendert und beim
                                    //       Erzeugen der ::com::sun::star::sdbcx::View ist eine Benachrichtigung
                                    //       der OLE-Objekte PrtOLENotify() notwendig.
    sal_Bool    bAllOLENotify       :1; // True: Benachrichtigung aller Objekte ist notwendig
    sal_Bool    bIsRedlineMove : 1;     // True: die Redlines werden in/aus der Section verschoben
    sal_Bool    bInsOnlyTxtGlssry : 1;  // True: insert 'only text' glossary into doc
    sal_Bool    bContains_MSVBasic : 1; // True: MS-VBasic exist is in our storage
    sal_Bool    bPurgeOLE : 1;          // TRUE: Purge OLE-Objects

    // -------------------------------------------------------------------
    // static - Members
    static SvStringsDtor    *pTextNmArray,          // Array fuer alle
                            *pListsNmArray,         // Pool-Vorlagen-Namen
                            *pExtraNmArray,
                            *pRegisterNmArray,
                            *pDocNmArray,
                            *pHTMLNmArray;
    static SvStringsDtor    *pFrmFmtNmArray,
                            *pChrFmtNmArray,
                            *pHTMLChrFmtNmArray;
    static SvStringsDtor    *pPageDescNmArray;
    static SvStringsDtor    *pNumRuleNmArray;

    static SwAutoCompleteWord *pACmpltWords;    // Liste aller Worte fuers AutoComplete
    static sal_uInt16 nUndoActions;     // anzahl von Undo ::com::sun::star::chaos::Action

    // -------------------------------------------------------------------
    // Dummies Members fuers FileFormat
    String sDummy1;             // Dummy-Member.
    String sDummy2;
    sal_uInt32  n32Dummy1;
    sal_uInt32  n32Dummy2;
    sal_Int8   n8Dummy1;
    sal_Int8   n8Dummy2;


    //---------------- private Methoden ------------------------------

    void AppendUndo(SwUndo*);   // interne Verkuerzung fuer Insert am Ende
    void ClearRedo();           // loescht alle UndoObjecte von nUndoPos
                                // bis zum Ende des Undo-Arrays
    sal_Bool DelUndoObj( sal_uInt16 nEnde  );   // loescht alle UndoObjecte vom Anfang
                                        // bis zum angegebenen Ende
    DECL_LINK( AddDrawUndo, SdrUndoAction * );
                                        // DrawModel
    void DrawNotifyUndoHdl();   // wegen CLOOKs
    void DrawSetRefDevice();    // wegen CLOOKs

        // nur fuer den internen Gebrauch deshalb privat.
        // Kopieren eines Bereiches im oder in ein anderes Dokument !
        // Die Position darf nicht im Bereich liegen !!
    sal_Bool _Copy( SwPaM&, SwPosition&,
                sal_Bool MakeNewFrms = sal_True, SwPaM* pCpyRng = 0 ) const;    // in ndcopy.cxx

    SwFlyFrmFmt* _MakeFlySection( const SwPosition& rAnchPos,
                                const SwCntntNode& rNode, RndStdIds eRequestId,
                                const SfxItemSet* pFlyAttrSet,
                                SwFrmFmt* = 0 );

    SwFlyFrmFmt* _InsNoTxtNode( const SwPosition&rPos, SwNoTxtNode*,
                                const SfxItemSet* pFlyAttrSet,
                                const SfxItemSet* pGrfAttrSet,
                                SwFrmFmt* = 0 );

    void _CopyFlyInFly( const SwNodeRange& rRg, const SwNodeIndex& rSttIdx,
                        sal_Bool bCopyFlyAtFly = sal_False ) const; // steht im ndcopy.cxx
    sal_Bool SetFlyFrmAnchor( SwFrmFmt& rFlyFmt, SfxItemSet& rSet, sal_Bool bNewFrms );

    typedef SwFmt* (SwDoc:: *FNCopyFmt)( const String&, SwFmt* );
    SwFmt* CopyFmt( const SwFmt& rFmt, const SvPtrarr& rFmtArr,
                        FNCopyFmt fnCopyFmt, const SwFmt& rDfltFmt );
    void CopyFmtArr( const SvPtrarr& rSourceArr, SvPtrarr& rDestArr,
                        FNCopyFmt fnCopyFmt, SwFmt& rDfltFmt );
    void _CopyPageDescHeaderFooter( sal_Bool bCpyHeader,
                                const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt );
    SwFmt* FindFmtByName( const SvPtrarr& rFmtArr,
                                    const String& rName ) const;
    SfxPrinter* _GetPrt() const;
    void        PrtDataChanged();   //Printer oder JobSetup geandert, es muss
                                    //fuer entsprechende Invalidierungen und
                                    //Benachrichtigungen gesorgt werden.

    static SvStringsDtor* NewNmArray( SvStringsDtor*&, sal_uInt16 nStt, sal_uInt16 nEnd );
    SvStringsDtor* NewTextNmArray() const;
    SvStringsDtor* NewListsNmArray() const;
    SvStringsDtor* NewExtraNmArray() const;
    SvStringsDtor* NewRegisterNmArray() const;
    SvStringsDtor* NewDocNmArray() const;
    SvStringsDtor* NewHTMLNmArray() const;
    SvStringsDtor* NewFrmFmtNmArray() const;
    SvStringsDtor* NewChrFmtNmArray() const;
    SvStringsDtor* NewHTMLChrFmtNmArray() const;
    SvStringsDtor* NewPageDescNmArray() const;
    SvStringsDtor* NewNumRuleNmArray() const;


    // gcc: aFtnInfo::CopyCtor ist private, also muessen wir uns auch schuetzen
    SwDoc( const SwDoc &);

    //wird nur von SwSwgReader::InlayoutFrames() verwendet!
    inline void SetInDtor( sal_Bool b ) { bDtor = b; }

    // fuer Felder:
    void _InitFieldTypes();     // wird vom CTOR gerufen!!
    void _MakeFldList( int eMode );
    void RenameUserFld( const String& rOldName, const String& rNewName,
                        String& rFormel );
    // Datenbankfelder:
    void UpdateDBNumFlds( SwDBNameInfField& rDBFld, SwCalc& rCalc );
    void AddUsedDBToList( SvStringsDtor& rDBNameList,
                          const SvStringsDtor& rUsedDBNames );
    void AddUsedDBToList( SvStringsDtor& rDBNameList, const String& rDBName );
    sal_Bool IsNameInArray( const SvStringsDtor& rOldNames, const String& rName );
    void GetAllDBNames( SvStringsDtor& rAllDBNames );
    void ReplaceUsedDBs( const SvStringsDtor& rUsedDBNames,
                        const String& rNewName, String& rFormel );
    SvStringsDtor& FindUsedDBs( const SvStringsDtor& rAllDBNames,
                                const String& rFormel,
                                SvStringsDtor& rUsedDBNames );

    void InitDrawModel();
    void ReleaseDrawModel();

    void _CreateNumberFormatter();

    sal_Bool _UnProtectTblCells( SwTable& rTbl );

    // erzeuge Anhand der vorgebenen Collection Teildokumente
    // falls keine angegeben ist, nehme die Kapitelvorlage der 1. Ebene
    sal_Bool SplitDoc( sal_uInt16 eDocType, const String& rPath,
                        const SwTxtFmtColl* pSplitColl );

    // Charts der angegebenen Tabelle updaten
    void _UpdateCharts( const SwTable& rTbl, ViewShell& rVSh ) const;

    // unser eigener 'IdlTimer' ruft folgende Methode
    DECL_LINK( DoIdleJobs, Timer * );
    // der CharTimer ruft diese Methode
    DECL_LINK( DoUpdateAllCharts, Timer * );
    DECL_LINK( DoUpdateModifiedOLE, Timer * );

public:
    SwDoc();
    ~SwDoc();

          SwNodes& GetNodes()                   { return aNodes; }
    const SwNodes& GetNodes() const             { return aNodes; }

    DECL_STATIC_LINK( SwDoc, BackgroundDone, SvxBrushItem *);
    DECL_LINK(CalcFieldValueHdl, EditFieldInfo*);

    // Links un-/sichtbar in LinkManager einfuegen (gelinkte Bereiche)
    sal_Bool IsVisibleLinks() const                  { return bVisibleLinks; }
    void SetVisibleLinks( sal_Bool bFlag )           { bVisibleLinks = bFlag; }

          SvxLinkManager& GetLinkManager()       { return *pLinkMgr; }
    const SvxLinkManager& GetLinkManager() const { return *pLinkMgr; }

        // erfrage das spezifische Layout
    const SwRootFrm* GetRootFrm() const { return pLayout; }
          SwRootFrm* GetRootFrm()       { return pLayout; }
        // der alte muss !! von der Shell geloescht werden !!
    void SetRootFrm( SwRootFrm* pNew )  { pLayout = pNew; }

    void StartIdleTimer()               { aIdleTimer.Start(); }
    void StopIdleTimer()                { aIdleTimer.Stop();  }
    sal_Bool IsIdleTimerActive() const  { return aIdleTimer.IsActive(); }

    sal_Bool IsOLEPrtNotifyPending() const  { return bOLEPrtNotifyPending; }
    void SetOLEPrtNotifyPending()       { bOLEPrtNotifyPending = sal_True; }
    void PrtOLENotify( sal_Bool bAll ); //Alle oder nur Markierte

    sal_Bool IsPurgeOLE() const             { return bPurgeOLE; }
    void SetPurgeOLE( sal_Bool bFlag )      { bPurgeOLE = bFlag; }

        // das Dokument im Browse-Modus anzeigen
    void SetBrowseMode( sal_Bool bFlag = sal_True )     { bBrowseMode = bFlag; }
    sal_Bool IsBrowseMode() const                       { return bBrowseMode; }
    void SetHTMLMode( sal_Bool bFlag = sal_True )       { bHTMLMode = bFlag; }
    sal_Bool IsHTMLMode() const                         { return bHTMLMode; }
    void SetHeadInBrowse( sal_Bool bFlag = sal_True )   { bHeadInBrowse = bFlag; }
    sal_Bool IsHeadInBrowse() const                     { return bHeadInBrowse; }
    void SetFootInBrowse( sal_Bool bFlag = sal_True )   { bFootInBrowse = bFlag; }
    sal_Bool IsFootInBrowse() const                     { return bFootInBrowse; }
    void SetFrmBeepEnabled( sal_Bool bFlag = sal_True ) { bFrmBeepEnabled = bFlag; }
    sal_Bool IsFrmBeepEnabled() const                   { return bFrmBeepEnabled; }

    void SetGlobalDoc( sal_Bool bFlag = sal_True )      { bIsGlobalDoc = bFlag; }
    sal_Bool IsGlobalDoc() const                        { return bIsGlobalDoc; }
    void SetGlblDocSaveLinks( sal_Bool bFlag=sal_True ) { bGlblDocSaveLinks = bFlag; }
    sal_Bool IsGlblDocSaveLinks() const                 { return bGlblDocSaveLinks; }

    void SetLabelDoc( sal_Bool bFlag = sal_True )       { bIsLabelDoc = bFlag; }
    sal_Bool IsLabelDoc() const                         { return bIsLabelDoc; }

    sal_uInt16 GetLinkUpdMode() const;
    void SetLinkUpdMode( sal_uInt16 nMode )             { nLinkUpdMode = nMode; }
    // nur fuer den Writer, weil dieser das richtige Enum schreiben muss!
    sal_uInt16 _GetLinkUpdMode() const                  { return nLinkUpdMode; }

    sal_uInt16  GetLatin_CJK() const { return nLatin_CJK; }
    sal_uInt16  GetLatin_CTL() const { return nLatin_CTL; }
    sal_uInt16  GetCJK_CTL() const { return nCJK_CTL; }

    inline sal_Bool IsInDtor() const { return bDtor; }

    // SS fuer das Drawing, Model und LayerId's
    const SdrModel* GetDrawModel() const    { return pDrawModel; }
          SdrModel* GetDrawModel()          { return pDrawModel; }
    sal_Int8 GetHeavenId() const            { return nHeaven; }
    sal_Int8 GetHellId() const              { return nHell;   }
    sal_Int8 GetControlsId() const          { return nControls;   }

    // liefert zu allen fliegenden Rahmen die Position im Dokument.
    // Wird ein Pam-Pointer uebergeben, muessen die absatzgebundenen
    // FlyFrames von der ::com::sun::star::awt::Selection vollstaendig umschlossen sein
    // ( Start < Pos < End ) !!!
    // (wird fuer die Writer benoetigt)
    void GetAllFlyFmts( SwPosFlyFrms& rPosFlyFmts, const SwPaM* = 0,
                        sal_Bool bDrawAlso = sal_False ) const;

        // dokumentglobale Macros
    sal_Bool HasGlobalMacro(sal_uInt16 nEvent) const;
    const SvxMacro& GetGlobalMacro(sal_uInt16 nEvent) const;
    void SetGlobalMacro(sal_uInt16 nEvent, const SvxMacro&);
    sal_Bool DelGlobalMacro(sal_uInt16 nEvent);
    const SvxMacroTableDtor& GetMacroTable() const { return *pMacroTable; }

    // Fussnoten Informationen
    const SwFtnInfo& GetFtnInfo() const         { return *pFtnInfo; }
    void SetFtnInfo(const SwFtnInfo& rInfo);
    const SwEndNoteInfo& GetEndNoteInfo() const { return *pEndNoteInfo; }
    void SetEndNoteInfo(const SwEndNoteInfo& rInfo);

    // Zeilennummerierung
    const SwLineNumberInfo& GetLineNumberInfo() const { return *pLineNumberInfo; }
    void SetLineNumberInfo(const SwLineNumberInfo& rInfo);

          SwFtnIdxs& GetFtnIdxs()       { return *pFtnIdxs; }
    const SwFtnIdxs& GetFtnIdxs() const { return *pFtnIdxs; }
    // Fussnoten im Bereich aendern
    sal_Bool SetCurFtn( const SwPaM& rPam, const String& rNumStr,
                        sal_uInt16 nNumber, sal_Bool bIsEndNote );

    // wegen swrtf.cxx und define private public, jetzt hier
    SwFlyFrmFmt  *MakeFlyFrmFmt (const String &rFmtName, SwFrmFmt *pDerivedFrom);
    SwDrawFrmFmt *MakeDrawFrmFmt(const String &rFmtName, SwFrmFmt *pDerivedFrom);

          SwLayouter* GetLayouter()       { return pLayouter; }
    const SwLayouter* GetLayouter() const { return pLayouter; }
    void SetLayouter( SwLayouter* pNew )  { pLayouter = pNew; }

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            Spell(SwPaM&, ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic::XSpellChecker1 > &,
                    sal_uInt16* pPageCnt, sal_uInt16* pPageSt ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic::XHyphenatedWord >
        Hyphenate( SwPaM *pPam, const Point &rCrsrPos,
                       sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    const SwBookmarks& GetBookmarks() const { return *pBookmarkTbl; }
    SwBookmark* MakeBookmark( const SwPaM& rPaM, const KeyCode&,
            const String& rName, const String& rShortName, BOOKMARK_TYPE eMark = BOOKMARK);
    void DelBookmark( sal_uInt16 nPos );
    void DelBookmark( const String& rName );
    sal_uInt16 FindBookmark( const String& rName );
        // erzeugt einen eindeutigen Namen. Der Name selbst muss vorgegeben
        // werden, es wird dann bei gleichen Namen nur durchnumeriert.
    void MakeUniqueBookmarkName( String& rNm );
    // Anzahl der ::com::sun::star::text::Bookmarks, gfs. nur "echte"
    sal_uInt16 GetBookmarkCnt(sal_Bool bBkmrk = sal_False) const;
    SwBookmark& GetBookmark( sal_uInt16, sal_Bool bBkmrk = sal_False);

    // Textbaustein Dokument?
    void SetGlossDoc( sal_Bool bGlssDc = sal_True ) { bGlossDoc = bGlssDc; }
    sal_Bool IsGlossDoc() const                 { return bGlossDoc; }
    sal_Bool IsInsOnlyTextGlossary() const      { return bInsOnlyTxtGlssry; }

    //Bei Formataenderungen muss das zu Fuss gerufen werden!
    void SetModified();
    void ResetModified();
    sal_Bool IsModified() const { return bModified; }   //Dokumentaenderungen?

    sal_Bool IsInCallModified() const       { return bInCallModified; }
    void SetUndoNoResetModified()       { nUndoSavePos = USHRT_MAX; }
    sal_Bool IsUndoNoResetModified() const  { return USHRT_MAX == nUndoSavePos; }

        //Die neuen (hoffentlich bestaendigen) Schnittstellen fuer alles,
        //was mit dem Layout zu tun hat.
    SwFrmFmt *MakeLayoutFmt( RndStdIds eRequest,
                             SwFrmFmt* = 0,
                             const SfxItemSet* pSet = 0 );
        // JP 08.05.98: fuer Flys muss jetzt diese Schnittstelle benutzt
        //              werden. pAnchorPos muss gesetzt sein, wenn keine
        //              Seitenbindung vorliegt UND der ::com::sun::star::chaos::Anchor nicht schon
        //              im FlySet/FrmFmt mit gueltiger CntntPos gesetzt ist
    SwFlyFrmFmt* MakeFlySection( RndStdIds eAnchorType,
                                const SwPosition* pAnchorPos,
                                const SfxItemSet* pSet = 0,
                                SwFrmFmt *pParent = 0 );
    SwFlyFrmFmt* MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                const SwSelBoxes* pSelBoxes = 0,
                                SwFrmFmt *pParent = 0 );

    void      DelLayoutFmt( SwFrmFmt *pFmt );
    SwFrmFmt *CopyLayoutFmt( const SwFrmFmt& rSrc,
                            const SwFmtAnchor& rNewAnchor,
                            sal_Bool bSetTxtFlyAtt = sal_True, sal_Bool bMakeFrms = sal_True );
    void CopyWithFlyInFly( const SwNodeRange& rRg,
                            const SwNodeIndex& rInsPos,
                            sal_Bool bMakeNewFrms = sal_True,
                            sal_Bool bDelRedlines = sal_True,
                            sal_Bool bCopyFlyAtFly = sal_False ) const;
    sal_Bool IsInHeaderFooter( const SwNodeIndex& rIdx ) const;

    sal_Bool SetFlyFrmAttr( SwFrmFmt& rFlyFmt, SfxItemSet& rSet );
    sal_Bool SetFrmFmtToFly( SwFrmFmt& rFlyFmt, SwFrmFmt& rNewFmt,
                        SfxItemSet* pSet = 0, sal_Bool bKeepOrient = sal_False );

    // Abstakt fuellen
    void Summary( SwDoc* pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, sal_Bool bImpress );

    /* Verwaltet SwDoc::nLinkCt, sollte (paarig!) von allen benutzt werden,
     * die einen Pointer auf dieses Dokument halten, denn wenn Remove() 0
     * liefert, wird das Dokument zerstoert! Oder anders: Jeder bei dem
     * Remove() 0 liefert, muss das Dokument zerstoeren.
     */
    sal_Int8 AddLink()                  { return nLinkCt++; }
    sal_Int8 RemoveLink()               { return nLinkCt ? --nLinkCt : nLinkCt; }
    sal_Int8 GetLinkCnt() const         { return nLinkCt; }

        // UndoHistory am Dokument pflegen
        // bei Save, SaveAs, Create wird UndoHistory zurueckgesetzt ???
    void DoUndo(sal_Bool bUn = sal_True) { bUndo = bUn; }
    sal_Bool DoesUndo() const { return bUndo; }
        // Zusammenfassen von Kontinuierlichen Insert/Delete/Overwrite von
        // Charaktern. Default ist ::com::sun::star::sdbcx::Group-Undo.
    void DoGroupUndo(sal_Bool bUn = sal_True) { bGroupUndo = bUn; }
    sal_Bool DoesGroupUndo() const { return bGroupUndo; }

        // macht rueckgaengig:
        // 0 letzte Aktion, sonst Aktionen bis zum Start der Klammerung nUndoId
        // In rUndoRange wird der restaurierte Bereich gesetzt.
    sal_Bool Undo( SwUndoIter& );
        // setzt Undoklammerung auf, nUndoId kommt von der UI-Seite
    sal_uInt16 StartUndo( sal_uInt16 nUndoId = 0 );
        // schliesst Klammerung der nUndoId
    sal_uInt16 EndUndo( sal_uInt16 nUndoId = 0 );
        // loescht die gesamten UndoObjecte ( fuer Methoden die am Nodes
        // Array drehen ohne entsprechendes Undo !!)
    void DelAllUndoObj();
        // liefert die Id der letzten undofaehigen Aktion zurueck
        // oder USHRT_MAX
        // fuellt ggf. VARARR mit ::com::sun::star::sdbcx::User-UndoIds
    sal_uInt16 GetUndoIds( String* pStr = 0, SwUndoIds *pUndoIds = 0) const;
        // gibt es Klammerung mit der Id?
    sal_Bool HasUndoId(sal_uInt16 nId) const;
        // die drei folgenden Methoden werden beim Undo und nur dort
        // benoetigt. Sollten sonst nicht aufgerufen werden.
    const SwNodes* GetUndoNds() const { return &aUndoNodes; }

        // bei einstufigem Undo muss die History und das Undo-Array geloescht
        // werden. Dabei ist aber zu beachten, das nur vollstaendige Gruppen
        // geloescht werden koennen.
        // Sind beide FLAGS auf sal_False, kann erfragt werden, ob sich in der
        // History eine offene Gruppe befindet. ( return = sal_True )
        // bDelHisory = sal_True:   loesche die Undo-History
        // bDelUndoNds = sal_True: loesche das Undo-Nodes-Arary
    sal_Bool DelUndoGroups( sal_Bool bDelUndoNds=sal_True, sal_Bool bDelHistory=sal_True );
    SwUndo* RemoveLastUndo( sal_uInt16 nUndoId );

        // abfragen/setzen der Anzahl von wiederherstellbaren Undo-Actions
    static sal_uInt16 GetUndoActionCount()              { return nUndoActions; }
    static void SetUndoActionCount( sal_uInt16 nNew )   { nUndoActions = nNew; }

        // Redo
        // wiederholt
    sal_Bool Redo( SwUndoIter& );
        // liefert die Id der letzten Redofaehigen Aktion zurueck
        // fuellt ggf. VARARR mit RedoIds
    sal_uInt16 GetRedoIds( String* pStr = 0, SwUndoIds *pRedoIds = 0) const;

        // Repeat
        // wiederholt
    sal_Bool Repeat( SwUndoIter&, sal_uInt16 nRepeatCnt = 1 );
        // liefert die Id der letzten Repeatfaehigen Aktion zurueck
        // fuellt ggf. VARARR mit RedoIds
    sal_uInt16 GetRepeatIds( String* pStr = 0, SwUndoIds *pRedoIds = 0) const;

    /* Felder */
    const SwFldTypes *GetFldTypes() const { return pFldTypes; }
    SwFieldType *InsertFldType(const SwFieldType &);
    SwFieldType *GetSysFldType( const sal_uInt16 eWhich ) const;
    SwFieldType* GetFldType(sal_uInt16 nResId, const String& rName) const;
    void RemoveFldType(sal_uInt16 nFld);
    void UpdateFlds( SfxPoolItem* pNewHt = 0, sal_Bool bCloseDB = sal_False );
    void InsDeletedFldType( SwFieldType & );
    sal_Bool RenameUserFields(const String& rOldName, const String& rNewName);

        // rufe das Update der Expression Felder auf; alle Ausdruecke werden
        // neu berechnet.
    void UpdateRefFlds( SfxPoolItem* pHt = 0);
    void UpdateTblFlds( SfxPoolItem* pHt = 0);
    void UpdateExpFlds( SwTxtFld* pFld = 0, sal_Bool bUpdateRefFlds = sal_True );
    void UpdateUsrFlds();
    void UpdatePageFlds( SfxPoolItem* );
    void LockExpFlds()                  { ++nLockExpFld; }
    void UnlockExpFlds()                { if( nLockExpFld ) --nLockExpFld; }
    sal_Bool IsExpFldsLocked() const        { return 0 != nLockExpFld; }
    SwDocUpdtFld& GetUpdtFlds() const   { return *pUpdtFlds; }
    sal_Bool SetFieldsDirty( sal_Bool b, const SwNode* pChk = 0, sal_uInt32 nLen = 0 );

    void SetFixFields( sal_Bool bOnlyTimeDate = sal_False,
                        const DateTime* pNewDateTime = 0 );

    sal_uInt16 GetFldUpdateFlags() const;
    void SetFldUpdateFlags( sal_uInt16 eMode )          { nFldUpdMode = eMode; }
    // nur fuer den Writer, weil dieser das richtige Enum schreiben muss!
    sal_uInt16 _GetFldUpdateFlags() const               { return nFldUpdMode; }

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    // Zustaende ueber Ladezustand
    // frueher DocInfo
    //
    sal_Bool IsPageNums() const                         { return bPageNums;     }
    sal_Bool IsLoaded() const                           { return bLoaded;       }
    sal_Bool IsUpdateExpFld() const                     { return bUpdateExpFld; }
    sal_Bool IsNewDoc() const                           { return bNewDoc;       }

    void SetPageNums(sal_Bool b = sal_True)                 { bPageNums = b;        }
    void SetNewDoc(sal_Bool b = sal_True)                   { bNewDoc = b;          }
    void SetUpdateExpFldStat(sal_Bool b = sal_True)         { bUpdateExpFld = b;    }
    void SetLoaded(sal_Bool b = sal_True)                   { bLoaded = b;          }

        // Setze im Calculator alle SetExpresion Felder, die bis zur
        // angegebenen Position (Node [ + ::com::sun::star::ucb::Content]) gueltig sind. Es kann
        // eine erzeugte Liste aller Felder mit uebergegeben werden.
        // (ist die Adresse != 0, und der Pointer == 0 wird eine neue
        // Liste returnt.)
    void FldsToCalc( SwCalc& rCalc, sal_uInt32 nLastNd = ULONG_MAX,
                    sal_uInt16 nLastCntnt = USHRT_MAX );
    void FldsToCalc( SwCalc& rCalc, const _SetGetExpFld& rToThisFld );
    void FldsToExpand( SwHash**& ppTbl, sal_uInt16& rTblSize,
                        const _SetGetExpFld& rToThisFld );
    sal_Bool IsNewFldLst() const { return bNewFldLst; }
    void SetNewFldLst( sal_Bool bFlag = sal_True ) { bNewFldLst = bFlag; }
    void InsDelFldInFldLst( sal_Bool bIns, const SwTxtFld& rFld );

    /*  Datenbank &&  DB-Manager */
    void SetNewDBMgr( SwNewDBMgr* pNewMgr )     { pNewDBMgr = pNewMgr; }
    SwNewDBMgr* GetNewDBMgr() const             { return pNewDBMgr; }
    void ChangeDBFields( const SvStringsDtor& rOldNames,
                        const String& rNewName );
    void SetInitDBFields(sal_Bool b);
    // Von Feldern verwendete Datenbanken herausfinden
    void GetAllUsedDB( SvStringsDtor& rDBNameList,
                       const SvStringsDtor* pAllDBNames = 0 );

    void ChgDBName( const String& rNewName );
    String GetDBName();
    const String& GetDBDesc();
    const String& _GetDBDesc() const { return aDBName; }


        // Kopieren eines Bereiches im oder in ein anderes Dokument !
        // Die Position kann auch im Bereich liegen !!
    sal_Bool Copy( SwPaM&, SwPosition& ) const; // in ndcopy.cxx

        // Loesche die Section, in der der Node steht.
    void DeleteSection( SwNode *pNode );

    sal_Bool Delete( SwPaM& );              //loeschen eines Bereiches
    sal_Bool DelFullPara( SwPaM& );         //loeschen gesamter Absaetze
    sal_Bool DeleteAndJoin( SwPaM& );       //komplett loeschen eines Bereiches

    sal_Bool Move( SwPaM&, SwPosition&, SwMoveFlags = DOC_MOVEDEFAULT ); //verschieben eines Bereiches
    sal_Bool Move( SwNodeRange&, SwNodeIndex&,
                SwMoveFlags = DOC_MOVEDEFAULT );  // verschieben ganzer Nodes
    sal_Bool MoveAndJoin( SwPaM&, SwPosition&, SwMoveFlags = DOC_MOVEDEFAULT ); //verschieben eines Bereiches
        //Ueberschreiben eines einzelnen Zeichens. rRg.Start() bezeichnet
        //den Node und die Position in dem Node, an der eingefuegt wird,
    sal_Bool Overwrite( const SwPaM &rRg, sal_Unicode c );
        //Ueberschreiben eines Strings in einem bestehenden Textnode.
    sal_Bool Overwrite(const SwPaM &rRg, const String& rStr );
        //Einfuegen eines einzelnen Zeichens. rRg.Start() bezeichnet
        //den Node und die Position in dem Node, an der eingefuegt wird.
    sal_Bool Insert(const SwPaM &rRg, sal_Unicode c );
        //Einfuegen eines Strings in einen bestehenden
        //Textnode. Der Text wird kopiert.
    sal_Bool Insert( const SwPaM &rRg, const String &,
                     sal_Bool bHintExpand = sal_True );

        //Einfuegen einer Grafik, Formel. Die XXXX werden kopiert.
    SwFlyFrmFmt* Insert(const SwPaM &rRg,
                        const String& rGrfName,
                        const String& rFltName,
                        const Graphic* pGraphic = 0,
                        const SfxItemSet* pFlyAttrSet = 0,
                        const SfxItemSet* pGrfAttrSet = 0,
                        SwFrmFmt* = 0 );
        // austauschen einer Grafik (mit Undo)
    void ReRead( SwPaM&, const String& rGrfName, const String& rFltName,
                const Graphic* pGraphic );

        //Einfuegen eines DrawObjectes. Das Object muss bereits im DrawModel
        // angemeldet sein.
    SwDrawFrmFmt* Insert( const SwPaM &rRg,
                          SdrObject& rDrawObj,
                          const SfxItemSet* pFlyAttrSet = 0,
                          SwFrmFmt* = 0 );
    String GetUniqueGrfName() const;

        //Einfuegen von OLE-Objecten.
    SwFlyFrmFmt* Insert( const SwPaM &rRg, SvInPlaceObject *,
                        const SfxItemSet* pFlyAttrSet = 0,
                        const SfxItemSet* pGrfAttrSet = 0,
                        SwFrmFmt* = 0 );
    String GetUniqueOLEName() const;

    // ein bischen wa fuer die benannten FlyFrames
    String GetUniqueFrameName() const;
    void SetFlyName( SwFlyFrmFmt& rFmt, const String& rName );
    const SwFlyFrmFmt* FindFlyByName( const String& rName, sal_Int8 nNdTyp = 0 ) const;

    void GetGrfNms( const SwFlyFrmFmt& rFmt, String* pGrfName,
                    String* pFltName ) const;

    // setze bei allen Flys ohne Namen einen gueltigen (Wird von den Readern
    // nach dem Einlesen gerufen )
    void SetAllUniqueFlyNames();

        //Aufspalten eines Nodes an rPos (nur fuer den TxtNode implementiert)
    sal_Bool SplitNode(const SwPosition &rPos, sal_Bool bChkTableStart = sal_False );
    sal_Bool AppendTxtNode( SwPosition& rPos ); // nicht const!

    // Ersetz einen selektierten Bereich in einem TextNode mit dem
    // String. Ist fuers Suchen&Ersetzen gedacht.
    // bRegExpRplc - ersetze Tabs (\\t) und setze den gefundenen String
    //               ein ( nicht \& )
    //              z.B.: Fnd: "zzz", Repl: "xx\t\\t..&..\&"
    //                      --> "xx\t<Tab>..zzz..&"
    sal_Bool Replace( SwPaM& rPam, const String& rNewStr,
                    sal_Bool bRegExpRplc = sal_False );

    /*
     * Einfuegen eines Attributs. Erstreckt sich rRg ueber
     * mehrere Nodes, wird das Attribut aufgespaltet, sofern
     * dieses Sinn macht. Nodes, in denen dieses Attribut keinen
     * Sinn macht, werden ignoriert.  In vollstaendig in der
     * Selektion eingeschlossenen Nodes wird das Attribut zu
     * harter Formatierung, in den anderen (Text-)Nodes wird das
     * Attribut in das Attributearray eingefuegt. Bei einem
     * Zeichenattribut wird ein "leerer" Hint eingefuegt,
     * wenn keine Selektion
     * vorliegt; andernfalls wird das Attribut als harte
     * Formatierung dem durch rRg.Start() bezeichneten Node
     * hinzugefuegt.  Wenn das Attribut nicht eingefuegt werden
     * konnte, liefert die Methode sal_False.
     */
    sal_Bool Insert( const SwPaM &rRg, const SfxPoolItem&, sal_uInt16 nFlags = 0  );
    sal_Bool Insert( const SwPaM &rRg, const SfxItemSet&, sal_uInt16 nFlags = 0 );

        //Zuruecksetzen der Attribute; es werden alle TxtHints und bei
        //vollstaendiger Selektion harte Formatierung (AUTO-Formate) entfernt
    void ResetAttr(const SwPaM &rRg, sal_Bool bTxtAttr = sal_True,
                        const SvUShortsSort* = 0 );
    void RstTxtAttr(const SwPaM &rRg, BOOL bInclRefToxMark = FALSE );

        // Setze das Attribut im angegebenen Format. Ist Undo aktiv, wird
        // das alte in die Undo-History aufgenommen
    void SetAttr( const SfxPoolItem&, SwFmt& );
    void SetAttr( const SfxItemSet&, SwFmt& );

        // Setze das Attribut als neues default Attribut in diesem Dokument.
        // Ist Undo aktiv, wird das alte in die Undo-History aufgenommen
    void SetDefault( const SfxPoolItem& );
    void SetDefault( const SfxItemSet& );

    // Erfrage das Default Attribut in diesem Dokument.
    const SfxPoolItem& GetDefault( sal_uInt16 nFmtHint ) const;
    // TextAttribute nicht mehr aufspannen lassen
    sal_Bool DontExpandFmt( const SwPosition& rPos, sal_Bool bFlag = sal_True );

    /* Formate */
    const SwFrmFmts* GetFrmFmts() const     { return pFrmFmtTbl; }
          SwFrmFmts* GetFrmFmts()           { return pFrmFmtTbl; }
    const SwCharFmts* GetCharFmts() const   { return pCharFmtTbl;}

    /* LayoutFormate (Rahmen, DrawObjecte), mal const mal nicht */
    const SwSpzFrmFmts* GetSpzFrmFmts() const   { return pSpzFrmFmtTbl; }
          SwSpzFrmFmts* GetSpzFrmFmts()         { return pSpzFrmFmtTbl; }

    const SwFrmFmt *GetDfltFrmFmt() const   { return pDfltFrmFmt; }
          SwFrmFmt *GetDfltFrmFmt()         { return pDfltFrmFmt; }
    const SwFrmFmt *GetEmptyPageFmt() const { return pEmptyPageFmt; }
          SwFrmFmt *GetEmptyPageFmt()       { return pEmptyPageFmt; }
    const SwFrmFmt *GetColumnContFmt() const{ return pColumnContFmt; }
          SwFrmFmt *GetColumnContFmt()      { return pColumnContFmt; }
    const SwCharFmt *GetDfltCharFmt() const { return pDfltCharFmt;}
          SwCharFmt *GetDfltCharFmt()       { return pDfltCharFmt;}

    SwFrmFmt  *MakeFrmFmt(const String &rFmtName, SwFrmFmt *pDerivedFrom);
    void       DelFrmFmt( SwFrmFmt *pFmt );
    SwFrmFmt* FindFrmFmtByName( const String& rName ) const
        {   return (SwFrmFmt*)FindFmtByName( (SvPtrarr&)*pFrmFmtTbl, rName ); }
    SwFrmFmt* FindSpzFrmFmtByName( const String& rName ) const
        {   return (SwFrmFmt*)FindFmtByName( (SvPtrarr&)*pSpzFrmFmtTbl, rName ); }

    SwCharFmt *MakeCharFmt(const String &rFmtName, SwCharFmt *pDerivedFrom);
    void       DelCharFmt(sal_uInt16 nFmt);
    void       DelCharFmt(SwCharFmt* pFmt);
    SwCharFmt* FindCharFmtByName( const String& rName ) const
        {   return (SwCharFmt*)FindFmtByName( (SvPtrarr&)*pCharFmtTbl, rName ); }

    /* Formatcollections (Vorlagen) */
    // TXT
    const SwTxtFmtColl* GetDfltTxtFmtColl() const { return pDfltTxtFmtColl; }
    const SwTxtFmtColls *GetTxtFmtColls() const { return pTxtFmtCollTbl; }
    SwTxtFmtColl *MakeTxtFmtColl( const String &rFmtName,
                                  SwTxtFmtColl *pDerivedFrom );
    SwConditionTxtFmtColl* MakeCondTxtFmtColl( const String &rFmtName,
                                               SwTxtFmtColl *pDerivedFrom );
    void DelTxtFmtColl(sal_uInt16 nFmt);
    void DelTxtFmtColl( SwTxtFmtColl* pColl );
    sal_Bool SetTxtFmtColl( const SwPaM &rRg, SwTxtFmtColl *pFmt,
                        sal_Bool bReset = sal_True);
    SwTxtFmtColl* FindTxtFmtCollByName( const String& rName ) const
        {   return (SwTxtFmtColl*)FindFmtByName( (SvPtrarr&)*pTxtFmtCollTbl, rName ); }

        // GRF
    const SwGrfFmtColl* GetDfltGrfFmtColl() const   { return pDfltGrfFmtColl; }
    const SwGrfFmtColls *GetGrfFmtColls() const     { return pGrfFmtCollTbl; }
    SwGrfFmtColl *MakeGrfFmtColl(const String &rFmtName,
                                    SwGrfFmtColl *pDerivedFrom);
    void DelGrfFmtColl(sal_uInt16 nFmt);
    void DelGrfFmtColl( SwGrfFmtColl* pColl );
    SwGrfFmtColl* FindGrfFmtCollByName( const String& rName ) const
        {   return (SwGrfFmtColl*)FindFmtByName( (SvPtrarr&)*pGrfFmtCollTbl, rName ); }

        // Tabellen-Formate
    const SwFrmFmts* GetTblFrmFmts() const  { return pTblFrmFmtTbl; }
          SwFrmFmts* GetTblFrmFmts()        { return pTblFrmFmtTbl; }
    sal_uInt16 GetTblFrmFmtCount( sal_Bool bUsed ) const;
    SwFrmFmt& GetTblFrmFmt(sal_uInt16 nFmt, sal_Bool bUsed ) const;
    SwTableFmt* MakeTblFrmFmt(const String &rFmtName, SwFrmFmt *pDerivedFrom);
    void        DelTblFrmFmt( SwTableFmt* pFmt );
    SwTableFmt* FindTblFmtByName( const String& rName, sal_Bool bAll = sal_False ) const;

    //Rahmenzugriff
    //iterieren ueber Flys - fuer Basic-Collections
    sal_uInt16 GetFlyCount(FlyCntType eType = FLYCNTTYPE_ALL) const;
    SwFrmFmt* GetFlyNum(sal_uInt16 nIdx, FlyCntType eType = FLYCNTTYPE_ALL);


    // kopiere die Formate in die eigenen Arrays und returne diese
    SwFrmFmt  *CopyFrmFmt ( const SwFrmFmt& );
    SwCharFmt *CopyCharFmt( const SwCharFmt& );
    SwTxtFmtColl* CopyTxtColl( const SwTxtFmtColl& rColl );
    SwGrfFmtColl* CopyGrfColl( const SwGrfFmtColl& rColl );

        // ersetze alle Formate mit denen aus rSource
    void ReplaceStyles( SwDoc& rSource );

        // Gebe die "Auto-Collection" mit der Id zurueck. Existiert
        // sie noch nicht, dann erzuege sie
        // Ist der String-Pointer definiert, dann erfrage nur die
        // Beschreibung der Attribute, !! es legt keine Vorlage an !!
    SwTxtFmtColl* GetTxtCollFromPool( sal_uInt16 nId, String* pDescription = 0,
        SfxItemPresentation ePres = SFX_ITEM_PRESENTATION_COMPLETE,
        SfxMapUnit eCoreMetric = SFX_MAPUNIT_TWIP,
        SfxMapUnit ePresMetric = SFX_MAPUNIT_TWIP );
        // return das geforderte automatische  Format - Basis-Klasse !
    SwFmt* GetFmtFromPool( sal_uInt16 nId, String* pDescription = 0,
        SfxItemPresentation ePres = SFX_ITEM_PRESENTATION_COMPLETE,
        SfxMapUnit eCoreMetric = SFX_MAPUNIT_TWIP,
        SfxMapUnit ePresMetric = SFX_MAPUNIT_TWIP );
        // returne das geforderte automatische Format
    SwFrmFmt* GetFrmFmtFromPool( sal_uInt16 nId, String* pDescription = 0 )
        { return (SwFrmFmt*)GetFmtFromPool( nId, pDescription ); }
    SwCharFmt* GetCharFmtFromPool( sal_uInt16 nId, String* pDescription = 0 )
        { return (SwCharFmt*)GetFmtFromPool( nId, pDescription ); }
        // returne die geforderte automatische Seiten-Vorlage
    SwPageDesc* GetPageDescFromPool( sal_uInt16 nId, String* pDescription = 0,
        SfxItemPresentation ePres = SFX_ITEM_PRESENTATION_COMPLETE,
        SfxMapUnit eCoreMetric = SFX_MAPUNIT_TWIP,
        SfxMapUnit ePresMetric = SFX_MAPUNIT_TWIP );
    SwNumRule* GetNumRuleFromPool( sal_uInt16 nId, String* pDescription = 0,
        SfxItemPresentation ePres = SFX_ITEM_PRESENTATION_COMPLETE,
        SfxMapUnit eCoreMetric = SFX_MAPUNIT_TWIP,
        SfxMapUnit ePresMetric = SFX_MAPUNIT_TWIP );

        // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
        // benutzt wird
    sal_Bool IsPoolTxtCollUsed( sal_uInt16 nId ) const;
    sal_Bool IsPoolFmtUsed( sal_uInt16 nId ) const;
    sal_Bool IsPoolPageDescUsed( sal_uInt16 nId ) const;
    sal_Bool IsPoolNumRuleUsed( sal_uInt16 nId ) const;

    // erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird
    sal_Bool IsUsed( const SwModify& ) const;
    sal_Bool IsUsed( const SwNumRule& ) const;

        // ist der Name ein Pool-Vorlagen-Name, returne seine ID,
        // sonst USHRT_MAX
    sal_uInt16 GetPoolId( const String& rName, SwGetPoolIdFromName ) const;

    const SvStringsDtor& GetTextNmArray() const
        { return pTextNmArray ? *pTextNmArray : *NewTextNmArray(); }
    const SvStringsDtor& GetListsNmArray() const
        { return pListsNmArray ? *pListsNmArray : *NewListsNmArray(); }
    const SvStringsDtor& GetExtraNmArray() const
        { return pExtraNmArray ? *pExtraNmArray : *NewExtraNmArray(); }
    const SvStringsDtor& GetRegisterNmArray() const
        { return pRegisterNmArray ? *pRegisterNmArray : *NewRegisterNmArray(); }
    const SvStringsDtor& GetDocNmArray() const
        { return pDocNmArray ? *pDocNmArray : *NewDocNmArray(); }
    const SvStringsDtor& GetHTMLNmArray() const
        { return pHTMLNmArray ? *pHTMLNmArray : *NewHTMLNmArray(); }
    const SvStringsDtor& GetFrmFmtNmArray() const
        { return pFrmFmtNmArray ? *pFrmFmtNmArray : *NewFrmFmtNmArray(); }
    const SvStringsDtor& GetChrFmtNmArray() const
        { return pChrFmtNmArray ? *pChrFmtNmArray : *NewChrFmtNmArray(); }
    const SvStringsDtor& GetHTMLChrFmtNmArray() const
        { return pHTMLChrFmtNmArray ? *pHTMLChrFmtNmArray : *NewHTMLChrFmtNmArray(); }
    const SvStringsDtor& GetPageDescNmArray() const
        { return pPageDescNmArray ? *pPageDescNmArray : *NewPageDescNmArray(); }
    const SvStringsDtor& GetNumRuleNmArray() const
        { return pNumRuleNmArray ? *pNumRuleNmArray : *NewNumRuleNmArray(); }

        // erfrage zu einer PoolId den Namen (steht im poolfmt.cxx)
    static String& GetPoolNm( sal_uInt16 nId, String& rFillNm );

        // setze den Namen der neu geladenen Dokument-Vorlage
    sal_uInt16 SetDocPattern( const String& rPatternName );
        // gebe die Anzahl von geladenen Dok-VorlagenNamen zurueck.
        // !! Die nicht mehr benutzten sind 0-Pointer !!
    sal_uInt16 GetDocPatternCnt() const { return aPatternNms.Count(); }
        // gebe den Dok-VorlagenNamen zurueck. !!! Kann auch 0 sein !!!
    String* GetDocPattern( sal_uInt16 nPos ) const { return aPatternNms[nPos]; }
    // loeche die nicht mehr benutzten Pattern-Namen aus dem Array.

    // alle nicht mehr referenzierten Namen werden durch 0-Pointer
    // ersetzt. Diese Positionen koennen wieder vergeben werden.
    void ReOrgPatternHelpIds();


        // Loesche alle nicht referenzierten FeldTypen
    void GCFieldTypes();                // impl. in docfld.cxx

        // akt. Dokument mit Textbausteindokument verbinden/erfragen
    void SetGlossaryDoc( SwDoc* pDoc ) { pGlossaryDoc = pDoc; }
    SwDoc* GetGlossaryDoc() const { return pGlossaryDoc; }
        // jetzt mit einem verkappten Reader/Writer/Dokument

    // travel over PaM Ring
    sal_Bool InsertGlossary( SwTextBlocks& rBlock, const String& rEntry,
                        SwPaM& rPaM, SwCrsrShell* pShell = 0);

    String GetCurWord(SwPaM&);

    //  JobSetup und Freunde
    SfxPrinter* GetPrt( sal_Bool bCreate ) const
        { if( !bCreate || pPrt ) return pPrt; else return _GetPrt(); }
    SfxPrinter* GetPrt() const { return pPrt; }
    inline void _SetPrt( SfxPrinter *pP )   { pPrt = pP; }
    void        SetPrt( SfxPrinter *pP );
    const JobSetup* GetJobsetup() const;
    void        SetJobsetup( const JobSetup& rJobSetup );

        // Dokument - Info
            // legt sie auf jedenfall an!
    const SfxDocumentInfo* GetInfo();
            // kann auch 0 sein !!!
    const SfxDocumentInfo* GetpInfo() const { return pSwgInfo; }

        // setze ueber die DocShell in den entsp. Storage-Stream. Hier wird
        // jetzt die DocInfo verwaltet. Fuer die Felder ist am Doc eine Kopie
        // der Info, um einen schnellen Zugriff zu ermoeglichen.
        // (impl. in docsh2.cxx)
    void SetInfo( const SfxDocumentInfo& rInfo );
        // die DocInfo hat siche geaendert (Notify ueber die DocShell)
        // stosse die entsp. Felder zum Updaten an.
    void DocInfoChgd( const SfxDocumentInfo& rInfo );

        //  Dokument - Statistics
    inline const SwDocStat  &GetDocStat() const { return *pDocStat; }
    void SetDocStat( const SwDocStat& rStat );
    void UpdateDocStat( SwDocStat& rStat, sal_uInt16 nNumPages );

        //PageDescriptor-Schnittstelle
    sal_uInt16 GetPageDescCnt() const { return aPageDescs.Count(); }
    const SwPageDesc& GetPageDesc( sal_uInt16 i ) const { return *aPageDescs[i]; }
    SwPageDesc* FindPageDescByName( const String& rName,
                                    sal_uInt16* pPos = 0 ) const;

        // kopiere den gesamten PageDesc - ueber Dokumentgrenzen und "tief"!
        // optional kann das kopieren der PoolFmtId, -HlpId verhindert werden
    void CopyPageDesc( const SwPageDesc& rSrcDesc, SwPageDesc& rDstDesc,
                        sal_Bool bCopyPoolIds = sal_True );

        // kopiere die Kopzeile (mit dem Inhalt!) aus dem SrcFmt
        // ins DestFmt ( auch ueber Doc grenzen hinaus!)
    void CopyHeader( const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt )
        { _CopyPageDescHeaderFooter( sal_True, rSrcFmt, rDestFmt ); }
        // kopiere die Fusszeile (mit dem Inhalt!) aus dem SrcFmt
        // ins DestFmt ( auch ueber Doc grenzen hinaus!)
    void CopyFooter( const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt )
        { _CopyPageDescHeaderFooter( sal_False, rSrcFmt, rDestFmt ); }

        //fuer Reader
    SwPageDesc& _GetPageDesc( sal_uInt16 i ) const { return *aPageDescs[i]; }
    void ChgPageDesc( sal_uInt16 i, const SwPageDesc& );
    void DelPageDesc( sal_uInt16 i );
    sal_uInt16 MakePageDesc( const String &rName, const SwPageDesc* pCpy = 0 );

        // Methoden fuer die Verzeichnisse:
        // - Verzeichnismarke einfuegen loeschen travel
    sal_uInt16 GetCurTOXMark( const SwPosition& rPos, SwTOXMarks& ) const;
    void Delete( SwTOXMark* pTOXMark );
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rCurTOXMark,
                                SwTOXSearch eDir, sal_Bool bInReadOnly );

        // - Verzeichnis einfuegen, und bei Bedarf erneuern
    const SwTOXBaseSection* InsertTableOf( const SwPosition& rPos,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = 0,
                                            sal_Bool bExpand = sal_False );
    const SwTOXBaseSection* InsertTableOf( sal_uInt32 nSttNd, sal_uInt32 nEndNd,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = 0                                          );
    const SwTOXBase* GetCurTOX( const SwPosition& rPos ) const;
    const SwAttrSet& GetTOXBaseAttrSet(const SwTOXBase& rTOX) const;

    sal_Bool DeleteTOX( const SwTOXBase& rTOXBase, sal_Bool bDelNodes = sal_False );
    String GetUniqueTOXBaseName( const SwTOXType& rType,
                                const String* pChkStr = 0 ) const;

    sal_Bool SetTOXBaseName(const SwTOXBase& rTOXBase, const String& rName);
    void SetTOXBaseProtection(const SwTOXBase& rTOXBase, sal_Bool bProtect);

    // nach einlesen einer Datei alle Verzeichnisse updaten
    void SetUpdateTOX( sal_Bool bFlag = sal_True )      { bUpdateTOX = bFlag; }
    sal_Bool IsUpdateTOX() const                    { return bUpdateTOX; }

    const String&   GetTOIAutoMarkURL() const {return sTOIAutoMarkURL;}
    void            SetTOIAutoMarkURL(const String& rSet)  {sTOIAutoMarkURL = rSet;}
    void            ApplyAutoMark();

    sal_Bool IsInReading() const                    { return bInReading; }

    // - Verzeichnis-Typen verwalten
    sal_uInt16 GetTOXTypeCount( TOXTypes eTyp ) const;
    const SwTOXType* GetTOXType( TOXTypes eTyp, sal_uInt16 nId ) const;
    sal_Bool DeleteTOXType( TOXTypes eTyp, sal_uInt16 nId );
    const SwTOXType* InsertTOXType( const SwTOXType& rTyp );
    const SwTOXTypes& GetTOXTypes() const { return *pTOXTypes; }

    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, sal_Bool bCreate = sal_False );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    // - Schluessel fuer die Indexverwaltung
    sal_uInt16 GetTOIKeys( SwTOIKeyType eTyp, SvStringsSort& rArr ) const;

    // Sortieren Tabellen Text
    sal_Bool SortTbl(const SwSelBoxes& rBoxes, const SwSortOptions&);
    sal_Bool SortText(const SwPaM&, const SwSortOptions&);

        // korrigiere die im Dokument angemeldeten SwPosition-Objecte,
        // wie z.B. die ::com::sun::star::text::Bookmarks oder die Verzeichnisse.
        // JP 22.06.95: ist bMoveCrsr gesetzt, verschiebe auch die Crsr

        // Setzt alles in rOldNode auf rNewPos + Offset
    void CorrAbs( const SwNodeIndex& rOldNode, const SwPosition& rNewPos,
                    const xub_StrLen nOffset = 0, sal_Bool bMoveCrsr = sal_False );
        // Setzt alles im Bereich von [rStartNode, rEndNode] nach rNewPos
    void CorrAbs( const SwNodeIndex& rStartNode, const SwNodeIndex& rEndNode,
                    const SwPosition& rNewPos, sal_Bool bMoveCrsr = sal_False );
        // Setzt alles im Bereich von rRange nach rNewPos
    void CorrAbs( const SwPaM& rRange, const SwPosition& rNewPos,
                    sal_Bool bMoveCrsr = sal_False );
        // Setzt alles in rOldNode auf relative Pos
    void CorrRel( const SwNodeIndex& rOldNode, const SwPosition& rNewPos,
                    const xub_StrLen nOffset = 0, sal_Bool bMoveCrsr = sal_False );

        // GliederungsRegeln erfragen / setzen
    const SwNumRule* GetOutlineNumRule() const { return pOutlineRule; }
    void SetOutlineNumRule( const SwNumRule& rRule );
    // Gliederung - hoch-/runterstufen
    sal_Bool OutlineUpDown( const SwPaM& rPam, short nOffset = 1 );
    // Gliederung - hoch-/runtermoven
    sal_Bool MoveOutlinePara( const SwPaM& rPam, short nOffset = 1);
        // zu diesem Gliederungspunkt
    sal_Bool GotoOutline( SwPosition& rPos, const String& rName ) const;
    // die Aenderungen an den Gliederungsvorlagen in die OutlineRule uebernehmen
    void SetOutlineLSpace( sal_uInt8 nLevel, short nFirstLnOfst, sal_uInt16 nLSpace );

        // setzt, wenn noch keine Numerierung, sonst wird geaendert
        // arbeitet mit alten und neuen Regeln, nur Differenzen aktualisieren
    void SetNumRule( const SwPaM&, const SwNumRule&,
                                        sal_Bool bSetAbsLSpace = sal_True );
        // ab hier neu starten lassen oder den Start wieder aufheben
    void SetNumRuleStart( const SwPosition& rPos, sal_Bool bFlag = sal_True );
    void SetNodeNumStart( const SwPosition& rPos, sal_uInt16 nStt = USHRT_MAX );

    SwNumRule* GetCurrNumRule( const SwPosition& rPos ) const;
    SwNumRuleTbl& GetNumRuleTbl() { return *pNumRuleTbl; }
    const SwNumRuleTbl& GetNumRuleTbl() const { return *pNumRuleTbl; }
    sal_uInt16 MakeNumRule( const String &rName, const SwNumRule* pCpy = 0 );
    sal_uInt16 FindNumRule( const String& rName ) const;
    SwNumRule* FindNumRulePtr( const String& rName ) const;
    // loeschen geht nur, wenn die ::com::sun::star::chaos::Rule niemand benutzt!
    sal_Bool DelNumRule( const String& rName );
    String GetUniqueNumRuleName( const String* pChkStr = 0, sal_Bool bAutoNum = sal_True ) const;
    void UpdateNumRule( const String& rName, sal_uInt32 nUpdPos );
    void UpdateNumRule();   // alle invaliden Updaten
    void ChgNumRuleFmts( const SwNumRule& rRule );
    sal_Bool ReplaceNumRule( const SwPosition& rPos, const String& rOldRule,
                        const String& rNewRule );

        // zum naechsten/vorhergehenden Punkt auf gleicher Ebene
    sal_Bool GotoNextNum( SwPosition&, sal_Bool bOverUpper = sal_True,
                        sal_uInt8* pUpper = 0, sal_uInt8* pLower = 0 );
    sal_Bool GotoPrevNum( SwPosition&, sal_Bool bOverUpper = sal_True,
                        sal_uInt8* pUpper = 0, sal_uInt8* pLower = 0 );

        // Absaetze ohne Numerierung, aber mit Einzuegen
    sal_Bool NoNum( const SwPaM& );
        // Loeschen, Splitten der Aufzaehlungsliste
    sal_Bool DelNumRules( const SwPaM& );
        // Hoch-/Runterstufen
    sal_Bool NumUpDown( const SwPaM&, sal_Bool bDown = sal_True );
        // Bewegt selektierte Absaetze (nicht nur Numerierungen)
        // entsprechend des Offsets. (negativ: zum Doc-Anf.)
    sal_Bool MoveParagraph( const SwPaM&, long nOffset = 1, sal_Bool bIsOutlMv = sal_False );
        // No-/Numerierung ueber Delete/Backspace ein/abschalten
    sal_Bool NumOrNoNum( const SwNodeIndex& rIdx, sal_Bool bDel = sal_False,
                    sal_Bool bOutline = sal_False );
        // Animation der Grafiken stoppen
    void StopNumRuleAnimations( OutputDevice* );

        // fuege eine neue Tabelle auf der Position rPos ein. (es
        // wird vor dem Node eingefuegt !!)
        //JP 28.10.96:
        //  fuer AutoFormat bei der Eingabe: dann muessen die Spalten
        //  auf die vordefinierten Breite gesetzt werden. Im Array stehen die
        //  Positionen der Spalten!! (nicht deren Breite!)
    const SwTable* InsertTable( const SwPosition& rPos, sal_uInt16 nRows,
                                sal_uInt16 nCols, SwHoriOrient eAdjust,
                                sal_uInt16 nInsert = HEADLINE_NO_BORDER,
                                const SwTableAutoFmt* pTAFmt = 0,
                                const SvUShorts* pColArr = 0 );

        // steht der Index in einer Tabelle, dann returne den TableNode sonst 0
                 SwTableNode* IsIdxInTbl( const SwNodeIndex& rIdx );
    inline const SwTableNode* IsIdxInTbl( const SwNodeIndex& rIdx ) const;

        // erzeuge aus dem makierten Bereich eine ausgeglichene Tabelle
    const SwTable* TextToTable( const SwPaM& rRange, sal_Unicode cCh,
                                SwHoriOrient eAdjust,
                                sal_uInt16 nInsert = HEADLINE_NO_BORDER,
                                const SwTableAutoFmt* = 0 );
        // erzeuge aus der Tabelle wieder normalen Text
    sal_Bool TableToText( const SwTableNode* pTblNd, sal_Unicode cCh );
        // einfuegen von Spalten/Zeilen in der Tabelle
    sal_Bool InsertCol( const SwCursor& rCursor,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
    sal_Bool InsertCol( const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
    sal_Bool InsertRow( const SwCursor& rCursor,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
    sal_Bool InsertRow( const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
        // loeschen von Spalten/Zeilen in der Tabelle
    sal_Bool DeleteRowCol( const SwSelBoxes& rBoxes );
    sal_Bool DeleteRow( const SwCursor& rCursor );
    sal_Bool DeleteCol( const SwCursor& rCursor );
        // teilen / zusammenfassen von Boxen in der Tabelle
    sal_Bool SplitTbl( const SwSelBoxes& rBoxes, sal_Bool bVert = sal_True, sal_uInt16 nCnt=1 );
        // returnt den enum TableMergeErr
    sal_uInt16 MergeTbl( SwPaM& rPam );
    String GetUniqueTblName() const;
    sal_Bool IsInsTblFormatNum() const;
    sal_Bool IsInsTblChangeNumFormat() const;
    sal_Bool IsInsTblAlignNum() const;

        // aus der FEShell wg.. Undo und bModified
    void GetTabCols( SwTabCols &rFill, const SwCursor* pCrsr,
                    const SwCellFrm* pBoxFrm = 0 ) const;
    void SetTabCols( const SwTabCols &rNew, sal_Bool bCurRowOnly,
                    const SwCursor* pCrsr, const SwCellFrm* pBoxFrm = 0 );

    // Direktzugriff fuer Uno
    void SetTabCols(SwTable& rTab, const SwTabCols &rNew, SwTabCols &rOld,
                                    const SwTableBox *pStart, sal_Bool bCurRowOnly);

    void SetHeadlineRepeat( SwTable &rTable, sal_Bool bSet );
        // AutoFormat fuer die Tabelle/TabellenSelection
    sal_Bool SetTableAutoFmt( const SwSelBoxes& rBoxes, const SwTableAutoFmt& rNew );
        // Erfrage wie attributiert ist
    sal_Bool GetTableAutoFmt( const SwSelBoxes& rBoxes, SwTableAutoFmt& rGet );
        // setze das TabelleAttribut Undo auf:
    void AppendUndoForAttrTable( const SwTable& rTbl );
        // setze die Spalten/Zeilen/ZTellen Breite/Hoehe
    sal_Bool SetColRowWidthHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                                SwTwips nAbsDiff, SwTwips nRelDiff );
    SwTableBoxFmt* MakeTableBoxFmt();
    SwTableLineFmt* MakeTableLineFmt();
    // teste ob die Box ein numerischen Wert darstellt und aender dann ggfs.
    // das Format der Box
    void ChkBoxNumFmt( SwTableBox& rAktBox, sal_Bool bCallUpdate );
    void SetTblBoxFormulaAttrs( SwTableBox& rBox, const SfxItemSet& rSet );
    void ClearBoxNumAttrs( const SwNodeIndex& rNode );

    sal_Bool CopyTblInTbl( const SwTable& rSrcTable, SwTable& rDestTbl,
                        const SwNodeIndex& rBoxIdx );
    sal_Bool InsCopyOfTbl( SwPosition& rInsPos, const SwSelBoxes& rBoxes,
                        const SwTable* pCpyTbl = 0, sal_Bool bCpyName = sal_False,
                        sal_Bool bCorrPos = sal_False );

    sal_Bool UnProtectCells( const String& rTblName );
    sal_Bool UnProtectCells( const SwSelBoxes& rBoxes );
    sal_Bool UnProtectTbls( const SwPaM& rPam );
    sal_Bool HasTblAnyProtection( const SwPosition* pPos,
                              const String* pTblName = 0,
                              sal_Bool* pFullTblProtection = 0 );

    // Tabelle an der Position in der GrundLine aufsplitten, sprich eine
    // neue Tabelle erzeugen.
    sal_Bool SplitTable( const SwPosition& rPos, sal_uInt16 eMode = 0,
                        sal_Bool bCalcNewSize = sal_False );
    // und die Umkehrung davon. rPos muss in der Tabelle stehen, die bestehen
    // bleibt. Das Flag besagt ob die aktuelle mit der davor oder dahinter
    // stehenden vereint wird.
    sal_Bool MergeTable( const SwPosition& rPos, sal_Bool bWithPrev = sal_True,
                        sal_uInt16 nMode = 0 );

    // Raeume die Umrandung innerhalb der Tabelle ein wenig auf (doppelte
    // an einer Kante erkennen und beseitigen)
    sal_Bool GCTableBorder( const SwPosition& rPos );
    // Charts der angegebenen Tabelle zum Update bewegen
    void UpdateCharts( const String &rName ) const;
    // update all charts, for that exists any table
    void UpdateAllCharts()          { DoUpdateAllCharts( 0 ); }
    // Tabelle wird umbenannt und aktualisiert die Charts
    void SetTableName( SwFrmFmt& rTblFmt, const String &rNewName );

    // returne zum Namen die im Doc gesetzte Referenz
    const SwFmtRefMark* GetRefMark( const String& rName ) const;
    // returne die RefMark per Index - fuer Uno
    const SwFmtRefMark* GetRefMark( sal_uInt16 nIndex ) const;
    // returne die Namen aller im Doc gesetzten Referenzen.
    //  Ist der ArrayPointer 0 dann returne nur, ob im Doc. eine RefMark
    //  gesetzt ist
    sal_uInt16 GetRefMarks( SvStringsDtor* = 0 ) const;

    //Einfuegen einer Beschriftung - falls ein FlyFormat erzeugt wird, so
    // returne dieses.
    SwFlyFrmFmt* InsertLabel( const SwLabelType eType, const String &rTxt,
                    const sal_Bool bBefore, const sal_uInt16 nId, const sal_uInt32 nIdx,
                    const sal_Bool bCpyBrd = sal_True );
    SwFlyFrmFmt* InsertDrawLabel( const String &rTxt, const sal_uInt16 nId,
                                  SdrObject& rObj );

    // erfrage den Attribut Pool
    const SwAttrPool& GetAttrPool() const   { return aAttrPool; }
          SwAttrPool& GetAttrPool()         { return aAttrPool; }

    // suche ueber das Layout eine EditShell und ggfs. eine ViewShell
    SwEditShell* GetEditShell( ViewShell** ppSh = 0 ) const;

    // OLE 2.0-Benachrichtung
    inline       void  SetOle2Link(const Link& rLink) {aOle2Link = rLink;}
    inline const Link& GetOle2Link() const {return aOle2Link;}

    // SS fuer Bereiche
    SwSection* Insert( const SwPaM& rRange, const SwSection& rNew,
                        const SfxItemSet* pAttr = 0, sal_Bool bUpdate = sal_True );
    sal_uInt16 IsInsRegionAvailable( const SwPaM& rRange,
                                const SwNode** ppSttNd = 0 ) const;
    SwSection* GetCurrSection( const SwPosition& rPos ) const;
    SwSectionFmts& GetSections() { return *pSectionFmtTbl; }
    const SwSectionFmts& GetSections() const { return *pSectionFmtTbl; }
    SwSectionFmt *MakeSectionFmt( SwSectionFmt *pDerivedFrom );
    void DelSectionFmt( SwSectionFmt *pFmt, sal_Bool bDelNodes = sal_False );
    void ChgSection( sal_uInt16 nSect, const SwSection&, const SfxItemSet* = 0 );
    String GetUniqueSectionName( const String* pChkStr = 0 ) const;

        // Passwort fuer geschuetzte Bereiche erfragen/setzen
    void ChgSectionPasswd( const String& sNew );
    const String& GetSectionPasswd() const { return sSectionPasswd; }

    // Pointer auf die SfxDocShell vom Doc, kann 0 sein !!!
          SwDocShell* GetDocShell()         { return pDocShell; }
    const SwDocShell* GetDocShell() const   { return pDocShell; }
    void SetDocShell( SwDocShell* pDSh );

    // falls beim Kopieren von OLE-Nodes eine DocShell angelegt werden muss,
    // dann MUSS der Ref-Pointer besetzt sein!!!!
    SvEmbeddedObjectRef* GetRefForDocShell()            { return pDocShRef; }
    void SetRefForDocShell( SvEmbeddedObjectRef* p )    { pDocShRef = p; }

        // fuer die TextBausteine - diese habe nur ein SvPersist zur
        // Verfuegung
         SvPersist* GetPersist() const;
    void SetPersist( SvPersist* );

    // Pointer auf den Storage des SfxDocShells, kann 0 sein !!!
    SvStorage* GetDocStorage();

        // abfrage/setze Flag, ob das Dokument im asynchronen Laden ist
    sal_Bool IsInLoadAsynchron() const              { return bInLoadAsynchron; }
    void SetInLoadAsynchron( sal_Bool bFlag )       { bInLoadAsynchron = bFlag; }

    // SS fuers Linken von Dokumentteilen
    sal_Bool GetData( const String& rItem, SvData& rData ) const;
    sal_Bool ChangeData( const String& rItem, const SvData& rData );
    SvPseudoObject* CreateHotLink( const String& rItem );
    // erzeuge um das zu Servende Object eine Selektion
    sal_Bool SelectServerObj( const String& rStr, SwPaM*& rpPam,
                            SwNodeRange*& rpRange ) const;
    // erfage alle zu servendenen Objecte
    sal_uInt16 GetServerObjects( SvStrings& rStrArr ) const;

    // fuer Drag&Move: ( z.B. RefMarks "verschieben" erlauben )
    sal_Bool IsCopyIsMove() const               { return bCopyIsMove; }
    void SetCopyIsMove( sal_Bool bFlag )        { bCopyIsMove = bFlag; }

    // fuers Draw-Undo: Aktionen auf Flys wollen wir selbst behandeln
    sal_Bool IsNoDrawUndoObj() const            { return bNoDrawUndoObj; }
    void SetNoDrawUndoObj( sal_Bool bFlag )     { bNoDrawUndoObj = bFlag; }
    SwDrawContact* GroupSelection( SdrView& );
    void UnGroupSelection( SdrView& );
    sal_Bool DeleteSelection( SwDrawView& );

    // Invalidiert OnlineSpell-WrongListen
    void SpellItAgainSam( sal_Bool bInvalid, sal_Bool bOnlyWrong );
    void InvalidateAutoCompleteFlag();

    SdrModel* _MakeDrawModel();
    inline SdrModel* MakeDrawModel()
    { return GetDrawModel() ? GetDrawModel() : _MakeDrawModel(); }
    void SetCalcFieldValueHdl(Outliner* pOutliner);

    // erfrage ob die ::com::sun::star::util::URL besucht war. Uebers Doc, falls nur ein ::com::sun::star::text::Bookmark
    // angegeben ist. Dann muss der Doc. Name davor gesetzt werden!
    sal_Bool IsVisitedURL( const String& rURL ) const;

    // speicher die akt. Werte fuer die automatische Aufnahme von Ausnahmen
    // in die Autokorrektur
    void SetAutoCorrExceptWord( SwAutoCorrExceptWord* pNew );
    SwAutoCorrExceptWord* GetAutoCorrExceptWord()       { return pACEWord; }

    const SwFmtINetFmt* FindINetAttr( const String& rName ) const;

    // rufe ins dunkle Basic, mit evt. Return String
    sal_Bool ExecMacro( const SvxMacro& rMacro, String* pRet = 0, SbxArray* pArgs = 0 );
    // rufe ins dunkle Basic/JavaScript
    sal_uInt16 CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                        sal_Bool bChkPtr = sal_False, SbxArray* pArgs = 0,
                        const Link* pCallBack = 0 );

    // linken Rand ueber Objectleiste einstellen (aenhlich dem Stufen von
    // Numerierungen), optional kann man "um" den Offset stufen oder "auf"
    // die Position gestuft werden (bModulus = sal_True)
    void MoveLeftMargin( const SwPaM& rPam, sal_Bool bRight = sal_True,
                        sal_Bool bModulus = sal_True );

    // Numberformatter erfragen
    inline       SvNumberFormatter* GetNumberFormatter( sal_Bool bCreate = sal_True );
    inline const SvNumberFormatter* GetNumberFormatter( sal_Bool bCreate = sal_True ) const;

    // loesche den nicht sichtbaren ::com::sun::star::ucb::Content aus dem Document, wie z.B.:
    // versteckte Bereiche, versteckte Absaetze
    sal_Bool RemoveInvisibleContent();
    // embedded alle lokalen Links (Bereiche/Grafiken)
    sal_Bool EmbedAllLinks();
    // erzeuge Anhand der vorgebenen Collection Teildokumente
    // falls keine angegeben ist, nehme die Kapitelvorlage der 1. Ebene
    sal_Bool GenerateGlobalDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );
    sal_Bool GenerateHTMLDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );


    // alles fuers Redlining
    SwRedlineMode GetRedlineMode() const { return eRedlineMode; }
    sal_Bool IsRedlineOn() const { return ::IsRedlineOn( eRedlineMode ); }
    sal_Bool IsIgnoreRedline() const { return REDLINE_IGNORE & eRedlineMode; }
    void SetRedlineMode_intern( sal_uInt16 eMode ) { eRedlineMode = (SwRedlineMode)eMode; }
    void SetRedlineMode( sal_uInt16 eMode );

    const SwRedlineTbl& GetRedlineTbl() const { return *pRedlineTbl; }
    sal_Bool AppendRedline( SwRedline* pPtr, sal_Bool bCallDelete = sal_True );
    sal_Bool SplitRedline( const SwPaM& rPam );
    sal_Bool DeleteRedline( const SwPaM& rPam, sal_Bool bSaveInUndo = sal_True,
                        sal_uInt16 nDelType = USHRT_MAX );
    sal_Bool DeleteRedline( const SwStartNode& rSection, sal_Bool bSaveInUndo = sal_True,
                        sal_uInt16 nDelType = USHRT_MAX );
    void DeleteRedline( sal_uInt16 nPos );
    sal_uInt16 GetRedlinePos( const SwNode& rNd, sal_uInt16 nType = USHRT_MAX ) const;
    void CompressRedlines();
    const SwRedline* GetRedline( const SwPosition& rPos,
                                sal_uInt16* pFndPos = 0 ) const;

    sal_Bool IsRedlineMove() const              { return bIsRedlineMove; }
    void SetRedlineMove( sal_Bool bFlag )       { bIsRedlineMove = bFlag; }

    sal_Bool AcceptRedline( sal_uInt16 nPos, sal_Bool bCallDelete = sal_True );
    sal_Bool AcceptRedline( const SwPaM& rPam, sal_Bool bCallDelete = sal_True );
    sal_Bool RejectRedline( sal_uInt16 nPos, sal_Bool bCallDelete = sal_True );
    sal_Bool RejectRedline( const SwPaM& rPam, sal_Bool bCallDelete = sal_True );
    const SwRedline* SelNextRedline( SwPaM& rPam ) const;
    const SwRedline* SelPrevRedline( SwPaM& rPam ) const;

    // alle Redline invalidieren, die Darstellung hat sich geaendert
    void UpdateRedlineAttr();
    // legt gegebenenfalls einen neuen Author an
    sal_uInt16 GetRedlineAuthor();
    // fuer die Reader usw. - neuen Author in die Tabelle eintragen
    sal_uInt16 InsertRedlineAuthor( const String& );
    // Kommentar am Redline an der Position setzen
    sal_Bool SetRedlineComment( const SwPaM& rPam, const String& rS );

    //  vergleiche zwei Dokument miteinander
    long CompareDoc( const SwDoc& rDoc );
    // merge zweier Dokumente
    long MergeDoc( const SwDoc& rDoc );
    // setze Kommentar-Text fuers Redline, das dann per AppendRedline
    // hereinkommt. Wird vom Autoformat benutzt. 0-Pointer setzt den Modus
    // wieder zurueck. Die SequenceNummer ist fuers UI-seitige zusammen-
    // fassen von Redlines.
    void SetAutoFmtRedlineComment( const String* pTxt, sal_uInt16 nSeqNo = 0 );

    sal_Bool IsAutoFmtRedline() const           { return bIsAutoFmtRedline; }
    void SetAutoFmtRedline( sal_Bool bFlag )    { bIsAutoFmtRedline = bFlag; }

    // fuer AutoFormat: mit Undo/Redlining - Behandlung
    void SetTxtFmtCollByAutoFmt( const SwPosition& rPos, sal_uInt16 nPoolId,
                                const SfxItemSet* pSet = 0 );
    void SetFmtItemByAutoFmt( const SwPaM& rPam, const SfxPoolItem& );

    // !!!NUR fuer die SW-Textblocks!! beachtet kein LAYOUT!!!
    void ClearDoc();        // loescht den gesamten Inhalt.

    // erfrage / setze die Daten fuer die PagePreView
    const SwPagePreViewPrtData* GetPreViewPrtData() const { return pPgPViewPrtData; }
    // wenn der Pointer == 0 ist, dann wird im Doc der Pointer zerstoert,
    // ansonsten wird das Object kopiert.
    // Der Pointer geht NICHT in den Besitz des Doc's!!
    void SetPreViewPrtData( const SwPagePreViewPrtData* pData );

    // update all modified OLE-Objects. The modification is called over the
    // StarOne - Interface              --> Bug 67026
    void SetOLEObjModified()
    {   if( GetRootFrm() ) aOLEModifiedTimer.Start(); }

    // -------------------- Uno - Schnittstellen ---------------------------
    const SwUnoCrsrTbl& GetUnoCrsrTbl() const       { return *pUnoCrsrTbl; }
    SwUnoCrsr* CreateUnoCrsr( const SwPosition& rPos, sal_Bool bTblCrsr = sal_False );
    // -------------------- Uno - Schnittstellen ---------------------------

    // -------------------- FeShell - Schnittstellen -----------------------
    // !!!!! diese gehen immer davon aus, das ein Layout existiert  !!!!
    sal_Bool ChgAnchor( const SdrMarkList &rMrkList, int eAnchorId,
                        sal_Bool bSameOnly, sal_Bool bPosCorr );

    void SetRowHeight( const SwCursor& rCursor, const SwFmtFrmSize &rNew );
    void GetRowHeight( const SwCursor& rCursor, SwFmtFrmSize *& rpSz ) const;
    sal_Bool BalanceRowHeight( const SwCursor& rCursor, sal_Bool bTstOnly = sal_True );
    void SetRowBackground( const SwCursor& rCursor, const SvxBrushItem &rNew );
    sal_Bool GetRowBackground( const SwCursor& rCursor, SvxBrushItem &rToFill ) const;
    void SetTabBorders( const SwCursor& rCursor, const SfxItemSet& rSet );
    void SetTabLineStyle( const SwCursor& rCursor,
                          const Color* pColor, sal_Bool bSetLine,
                          const SvxBorderLine* pBorderLine );
    void GetTabBorders( const SwCursor& rCursor, SfxItemSet& rSet ) const;
    void SetBoxAttr( const SwCursor& rCursor, const SfxPoolItem &rNew );
    sal_Bool GetBoxBackground( const SwCursor& rCursor, SvxBrushItem &rToFill ) const;
    void SetBoxAlign( const SwCursor& rCursor, sal_uInt16 nAlign );
    sal_uInt16 GetBoxAlign( const SwCursor& rCursor ) const;
    void AdjustCellWidth( const SwCursor& rCursor, sal_Bool bBalance = sal_False );

    int Chainable( const SwFrmFmt &rSource, const SwFrmFmt &rDest );
    int Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest );
    void Unchain( SwFrmFmt &rFmt );

    // fuers Copy/Move aus der FrmShell
    SdrObject* CloneSdrObj( const SdrObject&, sal_Bool bMoveWithinDoc = sal_False,
                            sal_Bool bInsInPage = sal_True );

    //
    // -------------------- FeShell - Schnittstellen Ende ------------------


    // Schnittstelle fuer die TextInputDaten - ( fuer die Texteingabe
    // von japanischen/chinesischen Zeichen)
    SwExtTextInput* CreateExtTextInput( const SwPaM& rPam );
    void DeleteExtTextInput( SwExtTextInput* pDel );
    SwExtTextInput* GetExtTextInput( const SwNode& rNd,
                                xub_StrLen nCntntPos = STRING_NOTFOUND) const;

    // Schnistelle fuer den Zugriff auf die AutoComplete-Liste
    static SwAutoCompleteWord& GetAutoCompleteWords() { return *pACmpltWords; }

    sal_Bool ContainsMSVBasic() const           { return bContains_MSVBasic; }
    void SetContainsMSVBasic( sal_Bool bFlag )  { bContains_MSVBasic = bFlag; }

    // ------------------- Zugriff auf Dummy-Member --------------------

    sal_Bool IsParaSpaceMax() const { return n8Dummy1 & DUMMY_PARASPACEMAX; }
    sal_Bool IsParaSpaceMaxAtPages() const { return n8Dummy1 & DUMMY_PARASPACEMAX_AT_PAGES; }
    void SetParaSpaceMax( sal_Bool bNew, sal_Bool bAtPages )
    {
        if( bNew ) n8Dummy1 |= DUMMY_PARASPACEMAX; else n8Dummy1 &= ~DUMMY_PARASPACEMAX;
        if( bAtPages ) n8Dummy1 |= DUMMY_PARASPACEMAX_AT_PAGES; else n8Dummy1 &= ~DUMMY_PARASPACEMAX_AT_PAGES;
    }

    void SetULongDummy1( sal_uInt32 n ) { n32Dummy1 = n; }
    void SetULongDummy2( sal_uInt32 n ) { n32Dummy2 = n; }
    void SetByteDummy1( sal_Int8 n ) { n8Dummy1 = n; }
    void SetByteDummy2( sal_Int8 n ) { n8Dummy2 = n; }
    void SetStringDummy1( const String& r ) { sDummy1 = r; }
    void SetStringDummy2( const String& r ) { sDummy2 = r; }
    sal_uInt32 GetULongDummy1() const { return n32Dummy1; }
    sal_uInt32 GetULongDummy2() const { return n32Dummy2; }
    sal_Int8 GetByteDummy1() const { return n8Dummy1; }
    sal_Int8 GetByteDummy2() const { return n8Dummy2; }
    const String& GetStringDummy1() const { return sDummy1; }
    const String& GetStringDummy2() const { return sDummy2; }

    // call back for API wrapper
    SwModify*   GetUnoCallBack() const;
};


// Diese Methode wird im Dtor vom SwDoc gerufen und loescht den Cache
// der Konturobjekte
void ClrContourCache();


//------------------ inline impl. ---------------------------------

inline const SwTableNode* SwDoc::IsIdxInTbl( const SwNodeIndex& rIdx ) const
{
    return ((SwDoc*)this)->IsIdxInTbl( rIdx );
}

inline SvNumberFormatter* SwDoc::GetNumberFormatter( sal_Bool bCreate )
{
    if( bCreate && !pNumberFormatter )
        _CreateNumberFormatter();
    return pNumberFormatter;
}

inline const SvNumberFormatter* SwDoc::GetNumberFormatter( sal_Bool bCreate ) const
{
    return ((SwDoc*)this)->GetNumberFormatter( bCreate );
}


// ist das NodesArray das normale vom Doc? (nicht das UndoNds, .. )
// Implementierung steht hier, weil man dazu Doc kennen muss!
inline sal_Bool SwNodes::IsDocNodes() const
{
    return this == &pMyDoc->GetNodes();
}



#endif  //_DOC_HXX
