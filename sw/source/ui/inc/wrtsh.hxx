/*************************************************************************
 *
 *  $RCSfile: wrtsh.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:21:32 $
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
#ifndef _WRTSH_HXX
#define _WRTSH_HXX

#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _SORTOPT_HXX
#include <sortopt.hxx>
#endif
#ifndef _SWURL_HXX
#include <swurl.hxx>
#endif

class Window;
class SbxArray;
class SwDoc;
class SvInPlaceObjectRef;
class SvEmbeddedObjectRef;
class SpellCheck;
class SwViewOption;
class SwFlyFrmAttrMgr;
class SwField;
class SwTOXBase;
class SdrView;
class SwView;
class SvGlobalName;
class SwInputFieldList;
class SwSection;
class Timer;
class SvxMacro;
class SwFmtINetFmt;
class SvxINetBookmark;
class SvxAutoCorrect;
class NaviContentBookmark;
struct SwCallMouseEvent;
class DropEvent;
struct ChartSelectionInfo;


class SwWrtShell: public SwFEShell
{
public:
    typedef long (SwWrtShell::*SELECTFUNC)(const Point *, BOOL bProp );

    SELECTFUNC  fnDrag;
    SELECTFUNC  fnSetCrsr;
    SELECTFUNC  fnEndDrag;
    SELECTFUNC  fnKillSel;

    // Alle Selektionen aufheben
    long ResetSelect( const Point *, BOOL );

    //setzt den Cursorstack nach dem Bewegen mit PageUp/-Down
    //zurueck, falls ein Stack aufgebaut ist
    inline void ResetCursorStack();

    enum SelectionType
    {
        SEL_TXT       = CNT_TXT, // Text, niemals auch Rahmen   0x0001
        SEL_GRF       = CNT_GRF, // Grafik                      0x0002
        SEL_OLE       = CNT_OLE, // OLE                         0x0010
        SEL_FRM       = 0x0020, // Rahmen, keine Inhaltsform
        SEL_NUM       = 0x0040, // NumListe
        SEL_TBL       = 0x0080, // Cursor steht in Tabelle
        SEL_TBL_CELLS = 0x0100, // Tabellenzellen sind selektiert
        SEL_DRW       = 0x0200, // Zeichenobjekte (Rechteck, Kreis...)
        SEL_DRW_TXT   = 0x0400, // Draw-Textobjekte im Editmode
        SEL_BEZ       = 0x0800, // Bezierobjekte editieren
        SEL_DRW_FORM  = 0x1000  // Zeichenobjekte: DB-Forms
    };
    int     GetSelectionType() const;

    BOOL    IsModePushed() const { return 0 != pModeStack; }
    void    PushMode();
    void    PopMode();

    void    SttSelect();
    void    EndSelect();
    BOOL    IsInSelect() const { return bInSelect; }
    void    SetInSelect(BOOL bSel = TRUE) { bInSelect = bSel; }
        // Liegt eine Text- oder Rahmenselektion vor?
    BOOL    HasSelection() const { return IsSelection() || IsSelFrmMode() || IsObjSelected(); }
    FASTBOOL Pop( BOOL bOldCrsr = TRUE );

    void    EnterStdMode();
    BOOL    IsStdMode() const { return !bExtMode && !bAddMode; }

    void    EnterExtMode();
    void    LeaveExtMode();
    long    ToggleExtMode();
    BOOL    IsExtMode() const { return bExtMode; }

    void    EnterAddMode();
    void    LeaveAddMode();
    long    ToggleAddMode();
    BOOL    IsAddMode() const { return bAddMode; }

    void    SetInsMode( BOOL bOn = TRUE );
    void    ToggleInsMode() { SetInsMode( !bIns ); }
    BOOL    IsInsMode() const { return bIns; }

    void    EnterSelFrmMode(const Point *pStartDrag = 0);
    void    LeaveSelFrmMode();
    BOOL    IsSelFrmMode() const { return bLayoutMode; }
        // Selektion von Rahmen aufheben
    void    UnSelectFrm();

    // Tabellenzellen selektieren fuer Bearbeiten von Formeln in der Ribbonbar
    inline void SelTblCells( const Link &rLink, BOOL bMark = TRUE );
    inline void EndSelTblCells();

    //Wortweisen oder zeilenweisen Selektionsmodus verlassen. Wird
    //in der Regel in MB-Up gerufen.
    BOOL    IsExtSel() const { return bSelWrd || bSelLn; }

    // erfrage, ob der akt. fnDrag - Pointer auf BeginDrag gesetzt ist
    // Wird fuer MouseMove gebraucht, um die Bugs 55592/55931 zu umgehen.
    inline BOOL Is_FnDragEQBeginDrag() const;

    //Basisabfragen
    BOOL    IsInWrd()           { return IsInWord(); }
    BOOL    IsSttWrd()          { return IsStartWord(); }
    BOOL    IsEndWrd();
    BOOL    IsSttOfPara() const { return IsSttPara(); }
    BOOL    IsEndOfPara() const { return IsEndPara(); }

    //Word bzw. Satz selektieren.
    BOOL    SelNearestWrd();
    BOOL    SelWrd      (const Point * = 0, BOOL bProp=FALSE );
    BOOL    SelSentence (const Point * = 0, BOOL bProp=FALSE );
    void    SelLine     (const Point * = 0, BOOL bProp=FALSE );
    long    SelAll();

    //Basiscursortravelling
typedef FASTBOOL (SwWrtShell:: *FNSimpleMove)();
    FASTBOOL SimpleMove( FNSimpleMove, FASTBOOL bSelect );

    FASTBOOL Left       ( FASTBOOL bSelect = FALSE, USHORT nCount = 1,
                            BOOL bBasicCall = FALSE );
    FASTBOOL Right      ( FASTBOOL bSelect = FALSE, USHORT nCount = 1,
                            BOOL bBasicCall = FALSE );
    FASTBOOL Up         ( FASTBOOL bSelect = FALSE, USHORT nCount = 1,
                            BOOL bBasicCall = FALSE );
    FASTBOOL Down       ( FASTBOOL bSelect = FALSE, USHORT nCount = 1,
                            BOOL bBasicCall = FALSE );
    FASTBOOL NxtWrd     ( FASTBOOL bSelect = FALSE ) { return SimpleMove( &SwWrtShell::_NxtWrd, bSelect ); }
    FASTBOOL PrvWrd     ( FASTBOOL bSelect = FALSE ) { return SimpleMove( &SwWrtShell::_PrvWrd, bSelect ); }
    FASTBOOL LeftMargin ( FASTBOOL bSelect = FALSE, FASTBOOL bBasicCall = FALSE );
    FASTBOOL RightMargin( FASTBOOL bSelect = FALSE, FASTBOOL bBasicCall = FALSE );
    FASTBOOL SttDoc     ( FASTBOOL bSelect = FALSE );
    FASTBOOL EndDoc     ( FASTBOOL bSelect = FALSE );

    FASTBOOL SttNxtPg   ( FASTBOOL bSelect = FALSE );
    FASTBOOL SttPrvPg   ( FASTBOOL bSelect = FALSE );
    FASTBOOL EndNxtPg   ( FASTBOOL bSelect = FALSE );
    FASTBOOL EndPrvPg   ( FASTBOOL bSelect = FALSE );
    FASTBOOL SttPg      ( FASTBOOL bSelect = FALSE );
    FASTBOOL EndPg      ( FASTBOOL bSelect = FALSE );
    FASTBOOL SttPara    ( FASTBOOL bSelect = FALSE );
    FASTBOOL EndPara    ( FASTBOOL bSelect = FALSE );
    FASTBOOL FwdPara    ( FASTBOOL bSelect = FALSE )
                { return SimpleMove( &SwWrtShell::_FwdPara, bSelect ); }
    FASTBOOL BwdPara    ( FASTBOOL bSelect = FALSE )
                { return SimpleMove( &SwWrtShell::_BwdPara, bSelect ); }
    FASTBOOL FwdSentence( FASTBOOL bSelect = FALSE )
                { return SimpleMove( &SwWrtShell::_FwdSentence, bSelect ); }
    FASTBOOL BwdSentence( FASTBOOL bSelect = FALSE )
                { return SimpleMove( &SwWrtShell::_BwdSentence, bSelect ); }

    FASTBOOL SelectTableRow();
    FASTBOOL SelectTableCol();
    FASTBOOL SelectTxtAttr( USHORT nWhich, const SwTxtAttr* pAttr = 0 );

    // Spaltenweise Spruenge
    FASTBOOL StartOfColumn      ( FASTBOOL bSelect = FALSE );
    FASTBOOL EndOfColumn        ( FASTBOOL bSelect = FALSE );
    FASTBOOL StartOfNextColumn  ( FASTBOOL bSelect = FALSE );
    FASTBOOL EndOfNextColumn    ( FASTBOOL bSelect = FALSE );
    FASTBOOL StartOfPrevColumn  ( FASTBOOL bSelect = FALSE );
    FASTBOOL EndOfPrevColumn    ( FASTBOOL bSelect = FALSE );

    // setze den Cursor auf die Seite "nPage" an den Anfang
    // zusaetzlich zu der gleichnamigen Implementierung in crsrsh.hxx
    // werden hier alle bestehenden Selektionen vor dem Setzen des
    // Cursors aufgehoben
    BOOL    GotoPage(USHORT nPage, BOOL bRecord = TRUE);

    //setzen des Cursors; merken der alten Position fuer Zurueckblaettern.
    DECL_LINK( ExecFlyMac, void * );

    BOOL    PageCrsr(SwTwips lOffset, BOOL bSelect);

    // Felder Update
    void    UpdateInputFlds( SwInputFieldList* pLst = 0, BOOL bOnlyInSel = FALSE );

    void    NoEdit(BOOL bHideCrsr = TRUE);
    void    Edit();
    BOOL    IsNoEdit() const { return bNoEdit; }

    // Loeschen
    long    DelToEndOfLine();
    long    DelToStartOfLine();
    long    DelLine();
    long    DelLeft();

    // loescht auch Rahmen bzw. setzt den Cursor in den Rahmen,
    // wenn bDelFrm == FALSE ist
    long    DelRight( BOOL bDelFrm = FALSE );
    long    DelToEndOfPara();
    long    DelToStartOfPara();
    long    DelToEndOfSentence();
    long    DelToStartOfSentence();
    long    DelNxtWord();
    long    DelPrvWord();

    // Prueft, ob eine Wortselektion vorliegt.
    // Gemaess den Regeln fuer intelligentes Cut / Paste
    // werden umgebende Spaces rausgeschnitten.
    // Liefert Art der Wortselektion zurueck (siehe enum)
    enum {
            NO_WORD = 0,
            WORD_SPACE_BEFORE = 1,
            WORD_SPACE_AFTER = 2,
            WORD_NO_SPACE = 3
        };
    int     IntelligentCut(int nSelectionType, BOOL bCut = TRUE);

    // Editieren
    void    Insert(SwField &);
    void    Insert(const String &);
    void    InsertByWord( const String & );
    void    InsertPageBreak(const String *pPageDesc = 0, USHORT nPgNum = 0 );
    void    InsertLineBreak();
    void    InsertColumnBreak();
    void    InsertFootnote(const String &, BOOL bEndNote = FALSE, BOOL bEdit = TRUE );
    void    SplitNode( BOOL bAutoFormat = FALSE, BOOL bCheckTableStart = TRUE );
    BOOL    CanInsert();

    // Verzeichnisse
    void    InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = 0);
    BOOL    UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = 0);

    // Numerierung und Bullets
    void    NumOn();
    void    BulletOn();

    // Graphic
    void    Insert( const String &rPath, const String &rFilter,
                    const Graphic &, SwFlyFrmAttrMgr * = 0,
                    BOOL bRule = FALSE );
    //OLE
    void    Insert      ( SvInPlaceObjectRef *pObj,     // != 0 fuer Clipboard
                          SvGlobalName *pName = 0,      // != 0 entspr. Object erzeugen.
                          BOOL bActivate = TRUE,
                          USHORT nSlotId = 0);          // SlotId fuer Dialog
    BOOL    InsertOle   ( SvInPlaceObjectRef aRef  );   // In die Core einfuegen.
    void    LaunchOLEObj( long nVerb = 0 );             // Server starten
    BOOL    FinishOLEObj();                             // Server wird beendet
    BOOL    IsOLEObj() const { return GetCntType() == CNT_OLE;}
    virtual void CalcAndSetScale( SvEmbeddedObjectRef xObj,
                                  const SwRect *pFlyPrtRect = 0,
                                  const SwRect *pFlyFrmRect = 0 );
    virtual void ConnectObj( SvInPlaceObjectRef xIPObj, const SwRect &rPrt,
                             const SwRect &rFrm );
    DECL_LINK( ChartSelectionHdl, ChartSelectionInfo * );

    // Vorlagen und Formate

    // enum gibt an, was geschehen soll, wenn das Style nicht gefunden wurde
    enum GetStyle { GETSTYLE_NOCREATE,          // keins anlegen
                    GETSTYLE_CREATESOME,        // falls auf PoolId mapt anlegen
                    GETSTYLE_CREATEANY };       // ggfs Standard returnen

    SwTxtFmtColl*   GetParaStyle(const String &rCollName,
                                    GetStyle eCreate = GETSTYLE_NOCREATE);
    SwCharFmt*      GetCharStyle(const String &rFmtName,
                                    GetStyle eCreate = GETSTYLE_NOCREATE);
    SwFrmFmt*       GetTblStyle(const String &rFmtName);

    void    SetPageStyle(const String &rCollName);

    String  GetCurPageStyle( const BOOL bCalcFrm = TRUE ) const;

    // Aktuelle Vorlage anhand der geltenden Attribute aendern
    void    QuickUpdateStyle();

    enum DoType { UNDO, REDO, REPEAT };

    void    Do( DoType eDoType );
    String  GetDoString( DoType eDoType ) const;
    String  GetRepeatString() const;

    //Suchen oder Ersetzen
    ULONG SearchPattern(const utl::SearchParam& rParam,
                         SwDocPositions eStart, SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         int bReplace = FALSE );

    ULONG SearchTempl  (const String &rTempl,
                         SwDocPositions eStart, SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         const String* pReplTempl = 0 );

    ULONG SearchAttr   (const SfxItemSet& rFindSet,
                         BOOL bNoColls,
                         SwDocPositions eStart,SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         const utl::SearchParam* pParam = 0,
                         const SfxItemSet* pReplaceSet = 0);

    void AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar = ' ' );

    // Aktion vor Cursorbewegung
    // Hebt gfs. Selektionen auf, triggert Timer und GCAttr()
    void    MoveCrsr( FASTBOOL bWithSelect = FALSE );

    // Eingabefelder updaten
    BOOL    StartInputFldDlg(SwField*, BOOL bNextButton);

    //"Handler" fuer Anederungen an der DrawView - fuer Controls.
    virtual void DrawSelChanged( SdrView * );

    // springe zum Bookmark und setze die "Selections-Flags" wieder richtig
    FASTBOOL GotoBookmark( USHORT nPos );
    FASTBOOL GotoBookmark( USHORT nPos, BOOL bSelect, BOOL bStart );
    FASTBOOL GotoBookmark( const String& rName );
    FASTBOOL GoNextBookmark(); // TRUE, wenn's noch eine gab
    FASTBOOL GoPrevBookmark();

    // Zugehoerige SwView ermitteln
    const SwView&       GetView() const { return rView; }
    SwView&             GetView() { return rView; }

    //Weil es sonst keiner macht, gibt es hier eine ExecMacro()
    void ExecMacro( const SvxMacro& rMacro, String* pRet = 0, SbxArray* pArgs = 0 );
    // rufe ins dunkle Basic/JavaScript
    USHORT CallEvent( USHORT nEvent, const SwCallMouseEvent& rCallEvent,
                        BOOL bCheckPtr = FALSE, SbxArray* pArgs = 0,
                        const Link* pCallBack = 0 );

    void    StartBasicAction();
    void    EndBasicAction();
    void    SetBasicActionCount(USHORT nSet);
    USHORT  EndAllBasicActions();

    // ein Klick aus das angegebene Feld. Der Cursor steht auf diesem.
    // Fuehre die vor definierten Aktionen aus.
    void ClickToField( const SwField& rFld, USHORT nFilter = URLLOAD_NOFILTER );
    void ClickToINetAttr( const SwFmtINetFmt& rItem, USHORT nFilter = URLLOAD_NOFILTER );
    BOOL ClickToINetGrf( const Point& rDocPt, USHORT nFilter = URLLOAD_NOFILTER );
    inline BOOL IsInClickToEdit() const ;

    // fall ein URL-Button selektiert ist, dessen URL returnen, ansonsten
    // einen LeerString
    BOOL GetURLFromButton( String& rURL, String& rDescr ) const;
    BOOL SetURLToButton( const String& rURL, const String& rDescr );

    void NavigatorPaste( const NaviContentBookmark& rBkmk, const DropEvent* pEvt );

    // die Core erzeugt eine Selektion, das SttSelect muss gerufen werden
    virtual void NewCoreSelection();

    // autom. Update von Vorlagen
    void AutoUpdateFrame(SwFrmFmt* pFmt, const SfxItemSet& rStyleSet);
    void AutoUpdatePara(SwTxtFmtColl* pColl, const SfxItemSet& rStyleSet);

    // Link fuers einfuegen von Bereichen uebers Drag&Drop/Clipboard
    DECL_STATIC_LINK( SwWrtShell, InsertRegionDialog, SwSection* );


    //ctoren, der erstere ist eine Art kontrollierter copy ctor fuer weitere
    //Sichten auf ein Dokument
    SwWrtShell( SwWrtShell&, Window *pWin, SwView &rShell);
    SwWrtShell( SwDoc& rDoc, Window *pWin, SwView &rShell,
                SwRootFrm* pMaster = 0, const SwViewOption *pViewOpt = 0);
    virtual ~SwWrtShell();

private:

    void    OpenMark();
    void    CloseMark( BOOL bOkFlag );

    String  GetWrdDelim();
    String  GetSDelim();
    String  GetBothDelim();

    struct ModeStack
    {
        ModeStack   *pNext;
        BOOL        bAdd,
                    bExt,
                    bIns;
        ModeStack(ModeStack *pNextMode, BOOL _bIns, BOOL _bExt, BOOL _bAdd):
            pNext(pNextMode),
            bIns(_bIns),
            bExt(_bExt),
            bAdd(_bAdd) {}
    } *pModeStack;

    // Cursor bei PageUp / -Down mitnehmen
    enum PageMove
    {
        MV_NO,
        MV_PAGE_UP,
        MV_PAGE_DOWN
    }  ePageMove;

    struct CrsrStack
    {
        Point aDocPos;
        CrsrStack *pNext;
        BOOL bValidCurPos : 1;
        BOOL bIsFrmSel : 1;
        SwTwips lOffset;

        CrsrStack( BOOL bValid, BOOL bFrmSel, const Point &rDocPos,
                    SwTwips lOff, CrsrStack *pN )
            : aDocPos(rDocPos), lOffset(lOff), pNext(pN)
        {
            bValidCurPos = bValid;
            bIsFrmSel = bFrmSel;
        }

    } *pCrsrStack;

    SwView  &rView;

    Point   aDest;
    BOOL    bDestOnStack;
    BOOL    HasCrsrStack() const { return 0 != pCrsrStack; }
    BOOL    PushCrsr(SwTwips lOffset, BOOL bSelect);
    BOOL    PopCrsr(BOOL bUpdate, BOOL bSelect = FALSE);

    // ENDE Cursor bei PageUp / -Down mitnehmen
    FASTBOOL _SttWrd();
    FASTBOOL _EndWrd();
    FASTBOOL _NxtWrd();
    FASTBOOL _PrvWrd();
    FASTBOOL _FwdSentence();
    FASTBOOL _BwdSentence();
    FASTBOOL _FwdPara();
    FASTBOOL _BwdPara();

        //  Selektionen
    BOOL    bIns            :1;
    BOOL    bInSelect       :1;
    BOOL    bExtMode        :1;
    BOOL    bAddMode        :1;
    BOOL    bLayoutMode     :1;
    BOOL    bNoEdit         :1;
    BOOL    bCopy           :1;
    BOOL    bSelWrd         :1;
    BOOL    bSelLn          :1;
    BOOL    bIsInClickToEdit:1;
    BOOL    bClearMark      :1;     // Selektion fuer ChartAutoPilot nicht loeschen

    Point   aStart;
    Link    aSelTblLink;

    SELECTFUNC  fnLeaveSelect;

    //setzt den Cursorstack nach dem Bewegen mit PageUp/-Down zurueck.
    void    _ResetCursorStack();

    void    SttDragDrop(Timer *);
    long    SetCrsr(const Point *, BOOL bProp=FALSE );
    long    SetCrsrKillSel(const Point *, BOOL bProp=FALSE );

    long    StdSelect(const Point *, BOOL bProp=FALSE );
    long    BeginDrag(const Point *, BOOL bProp=FALSE );
    long    Drag(const Point *, BOOL bProp=FALSE );
    long    EndDrag(const Point *, BOOL bProp=FALSE );

    long    ExtSelWrd(const Point *, BOOL bProp=FALSE );
    long    ExtSelLn(const Point *, BOOL bProp=FALSE );

    //Verschieben von Text aus Drag and Drop; Point ist
    //Destination fuer alle Selektionen.
    long    MoveText(const Point *, BOOL bProp=FALSE );

    long    BeginFrmDrag(const Point *, BOOL bProp=FALSE );

    //nach SSize/Move eines Frames Update; Point ist Destination.
    long    UpdateLayoutFrm(const Point *, BOOL bProp=FALSE );

    long    SttLeaveSelect(const Point *, BOOL bProp=FALSE );
    long    AddLeaveSelect(const Point *, BOOL bProp=FALSE );
    long    Ignore(const Point *, BOOL bProp=FALSE );

    void    LeaveExtSel() { bSelWrd = bSelLn = FALSE;}
    BOOL    _CanInsert();

    BOOL    GoStart(BOOL KeepArea = FALSE, BOOL * = 0,
            BOOL bSelect = FALSE, BOOL bDontMoveRegion = FALSE);
    BOOL    GoEnd(BOOL KeepArea = FALSE, BOOL * = 0);

    enum BookMarkMove
    {
        BOOKMARK_INDEX,
        BOOKMARK_NEXT,
        BOOKMARK_PREV,
        BOOKMARK_LAST_LAST_ENTRY
    };

    FASTBOOL MoveBookMark(  BookMarkMove eFuncId,
                            USHORT nPos = 0,
                            BOOL bStart = TRUE );
};

inline void SwWrtShell::ResetCursorStack()
{
    if ( HasCrsrStack() )
        _ResetCursorStack();
}

inline void SwWrtShell::SelTblCells(const Link &rLink, BOOL bMark )
{
    SetSelTblCells( TRUE );
    bClearMark = bMark;
    aSelTblLink = rLink;
}
inline void SwWrtShell::EndSelTblCells()
{
    SetSelTblCells( FALSE );
    bClearMark = TRUE;
}

inline BOOL SwWrtShell::IsInClickToEdit() const { return bIsInClickToEdit; }

inline BOOL SwWrtShell::Is_FnDragEQBeginDrag() const
{
#ifdef GCC
    SELECTFUNC  fnTmp = &SwWrtShell::BeginDrag;
    return fnDrag == fnTmp;
#else
    return fnDrag == &SwWrtShell::BeginDrag;
#endif
}

#endif
