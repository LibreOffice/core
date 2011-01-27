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
#ifndef _WRTSH_HXX
#define _WRTSH_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include "swdllapi.h"
#include <fesh.hxx>
#include <sortopt.hxx>
#include <swurl.hxx>
#include <IMark.hxx>

class Window;
class OutputDevice;
class SbxArray;
class SwDoc;
class SpellCheck;
class SwViewOption;
class SwFlyFrmAttrMgr;
class SwField;
class SwTOXBase;
class SdrView;
class SwView;
class SvGlobalName;
class SwInputFieldList;
class SwSectionData;
class Timer;
class SvxMacro;
class SwFmtINetFmt;
class SvxINetBookmark;
class SvxAutoCorrect;
class NaviContentBookmark;
struct SwCallMouseEvent;
class DropEvent;
struct ChartSelectionInfo;
class SfxStringListItem;
class SfxRequest;

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }


typedef sal_Int32 SelectionType;
namespace nsSelectionType
{
    const SelectionType SEL_TXT             = CNT_TXT;  // Text, niemals auch Rahmen    0x0001
    const SelectionType SEL_GRF             = CNT_GRF;  // Grafik                       0x0002
    const SelectionType SEL_OLE             = CNT_OLE;  // OLE                          0x0010
    const SelectionType SEL_FRM             = 0x000020; // Rahmen, keine Inhaltsform
    const SelectionType SEL_NUM             = 0x000040; // NumListe
    const SelectionType SEL_TBL             = 0x000080; // Cursor steht in Tabelle
    const SelectionType SEL_TBL_CELLS       = 0x000100; // Tabellenzellen sind selektiert
    const SelectionType SEL_DRW             = 0x000200; // Zeichenobjekte (Rechteck, Kreis...)
    const SelectionType SEL_DRW_TXT         = 0x000400; // Draw-Textobjekte im Editmode
    const SelectionType SEL_BEZ             = 0x000800; // Bezierobjekte editieren
    const SelectionType SEL_DRW_FORM        = 0x001000; // Zeichenobjekte: DB-Forms
    const SelectionType SEL_FOC_FRM_CTRL    = 0x002000; // a form control is focused. Neither set nor evaluated by the SwWrtShell itself, only by it's clients.
    const SelectionType SEL_MEDIA           = 0x004000; // Media object
    const SelectionType SEL_EXTRUDED_CUSTOMSHAPE = 0x008000;    // extruded custom shape
    const SelectionType SEL_FONTWORK        = 0x010000; // fontwork
    const SelectionType SEL_POSTIT          = 0x020000; //annotation
}


class SW_DLLPUBLIC SwWrtShell: public SwFEShell
{
private:
    using SwCrsrShell::Left;
    using SwCrsrShell::Right;
    using SwCrsrShell::Up;
    using SwCrsrShell::Down;
    using SwCrsrShell::LeftMargin;
    using SwCrsrShell::RightMargin;
    using SwCrsrShell::SelectTxtAttr;
    using SwCrsrShell::GotoPage;
    using SwFEShell::InsertObject;
    using SwEditShell::AutoCorrect;
    using SwCrsrShell::GotoMark;

public:

    using SwEditShell::Insert;

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
    SelectionType   GetSelectionType() const;

    BOOL    IsModePushed() const { return 0 != pModeStack; }
    void    PushMode();
    void    PopMode();

    void    SttSelect();
    void    EndSelect();
    BOOL    IsInSelect() const { return bInSelect; }
    void    SetInSelect(BOOL bSel = TRUE) { bInSelect = bSel; }
        // Liegt eine Text- oder Rahmenselektion vor?
    BOOL    HasSelection() const { return SwCrsrShell::HasSelection() ||
                                        IsMultiSelection() || IsSelFrmMode() || IsObjSelected(); }
    BOOL Pop( BOOL bOldCrsr = TRUE );

    void    EnterStdMode();
    BOOL    IsStdMode() const { return !bExtMode && !bAddMode && !bBlockMode; }

    void    EnterExtMode();
    void    LeaveExtMode();
    long    ToggleExtMode();
    BOOL    IsExtMode() const { return bExtMode; }

    void    EnterAddMode();
    void    LeaveAddMode();
    long    ToggleAddMode();
    BOOL    IsAddMode() const { return bAddMode; }

    void    EnterBlockMode();
    void    LeaveBlockMode();
    long    ToggleBlockMode();
    BOOL    IsBlockMode() const { return bBlockMode; }

    void    SetInsMode( BOOL bOn = TRUE );
    void    ToggleInsMode() { SetInsMode( !bIns ); }
    BOOL    IsInsMode() const { return bIns; }
    void    SetRedlineModeAndCheckInsMode( USHORT eMode );

    void    EnterSelFrmMode(const Point *pStartDrag = 0);
    void    LeaveSelFrmMode();
    BOOL    IsSelFrmMode() const { return bLayoutMode; }
        // Selektion von Rahmen aufheben
    void    UnSelectFrm();

    void    Invalidate();

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
    // --> FME 2004-07-30 #i32329# Enhanced selection
    void    SelSentence (const Point * = 0, BOOL bProp=FALSE );
    void    SelPara     (const Point * = 0, BOOL bProp=FALSE );
    // <--
    long    SelAll();

    //Basiscursortravelling
typedef BOOL (SwWrtShell:: *FNSimpleMove)();
    BOOL SimpleMove( FNSimpleMove, BOOL bSelect );

    BOOL Left       ( USHORT nMode, BOOL bSelect,
                            USHORT nCount, BOOL bBasicCall, BOOL bVisual = FALSE );
    BOOL Right      ( USHORT nMode, BOOL bSelect,
                            USHORT nCount, BOOL bBasicCall, BOOL bVisual = FALSE );
    BOOL Up         ( BOOL bSelect = FALSE, USHORT nCount = 1,
                            BOOL bBasicCall = FALSE );
    BOOL Down       ( BOOL bSelect = FALSE, USHORT nCount = 1,
                            BOOL bBasicCall = FALSE );
    BOOL NxtWrd     ( BOOL bSelect = FALSE ) { return SimpleMove( &SwWrtShell::_NxtWrd, bSelect ); }
    BOOL PrvWrd     ( BOOL bSelect = FALSE ) { return SimpleMove( &SwWrtShell::_PrvWrd, bSelect ); }

    BOOL LeftMargin ( BOOL bSelect, BOOL bBasicCall );
    BOOL RightMargin( BOOL bSelect, BOOL bBasicCall );

    BOOL SttDoc     ( BOOL bSelect = FALSE );
    BOOL EndDoc     ( BOOL bSelect = FALSE );

    BOOL SttNxtPg   ( BOOL bSelect = FALSE );
    BOOL SttPrvPg   ( BOOL bSelect = FALSE );
    BOOL EndNxtPg   ( BOOL bSelect = FALSE );
    BOOL EndPrvPg   ( BOOL bSelect = FALSE );
    BOOL SttPg      ( BOOL bSelect = FALSE );
    BOOL EndPg      ( BOOL bSelect = FALSE );
    BOOL SttPara    ( BOOL bSelect = FALSE );
    BOOL EndPara    ( BOOL bSelect = FALSE );
    BOOL FwdPara    ( BOOL bSelect = FALSE )
                { return SimpleMove( &SwWrtShell::_FwdPara, bSelect ); }
    BOOL BwdPara    ( BOOL bSelect = FALSE )
                { return SimpleMove( &SwWrtShell::_BwdPara, bSelect ); }
    BOOL FwdSentence( BOOL bSelect = FALSE )
                { return SimpleMove( &SwWrtShell::_FwdSentence, bSelect ); }
    BOOL BwdSentence( BOOL bSelect = FALSE )
                { return SimpleMove( &SwWrtShell::_BwdSentence, bSelect ); }

    // --> FME 2004-07-30 #i20126# Enhanced table selection
    BOOL SelectTableRowCol( const Point& rPt, const Point* pEnd = 0, bool bRowDrag = false );
    // <--
    BOOL SelectTableRow();
    BOOL SelectTableCol();
    BOOL SelectTableCell();

    BOOL SelectTxtAttr( USHORT nWhich, const SwTxtAttr* pAttr = 0 );

    // Spaltenweise Spruenge
    BOOL StartOfColumn      ( BOOL bSelect = FALSE );
    BOOL EndOfColumn        ( BOOL bSelect = FALSE );
    BOOL StartOfNextColumn  ( BOOL bSelect = FALSE );
    BOOL EndOfNextColumn    ( BOOL bSelect = FALSE );
    BOOL StartOfPrevColumn  ( BOOL bSelect = FALSE );
    BOOL EndOfPrevColumn    ( BOOL bSelect = FALSE );

    // setze den Cursor auf die Seite "nPage" an den Anfang
    // zusaetzlich zu der gleichnamigen Implementierung in crsrsh.hxx
    // werden hier alle bestehenden Selektionen vor dem Setzen des
    // Cursors aufgehoben
    BOOL    GotoPage( USHORT nPage, BOOL bRecord );

    //setzen des Cursors; merken der alten Position fuer Zurueckblaettern.
    DECL_LINK( ExecFlyMac, void * );

    BOOL    PageCrsr(SwTwips lOffset, BOOL bSelect);

    // Felder Update
    void    UpdateInputFlds( SwInputFieldList* pLst = 0, BOOL bOnlyInSel = FALSE );

    void    NoEdit(BOOL bHideCrsr = TRUE);
    void    Edit();
    BOOL    IsNoEdit() const { return bNoEdit; }

    BOOL IsRetainSelection() const { return mbRetainSelection; }
    void SetRetainSelection( BOOL bRet ) { mbRetainSelection = bRet; }

    // change current data base and notify
    void ChgDBData(const SwDBData& SwDBData);

    // Loeschen
    long    DelToEndOfLine();
    long    DelToStartOfLine();
    long    DelLine();
    long    DelLeft();

    // loescht auch Rahmen bzw. setzt den Cursor in den Rahmen,
    // wenn bDelFrm == FALSE ist
    long    DelRight();
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
    enum word {
            NO_WORD = 0,
            WORD_SPACE_BEFORE = 1,
            WORD_SPACE_AFTER = 2,
            WORD_NO_SPACE = 3
        };
    int     IntelligentCut(int nSelectionType, BOOL bCut = TRUE);

    // Editieren
    void    Insert(SwField &);
    void    Insert(const String &);
    // Graphic
    void    Insert( const String &rPath, const String &rFilter,
                    const Graphic &, SwFlyFrmAttrMgr * = 0,
                    BOOL bRule = FALSE );

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
    /**
       Turns on numbering or bullets.

       @param bNum    TRUE: turn on numbering
                      FALSE: turn on bullets
    */
    void    NumOrBulletOn(BOOL bNum); // #i29560#
    void    NumOrBulletOff(); // #i29560#
    void    NumOn();
    void    BulletOn();

    //OLE
    void    InsertObject(     /*SvInPlaceObjectRef *pObj, */       // != 0 fuer Clipboard
                          const svt::EmbeddedObjectRef&,
                          SvGlobalName *pName = 0,      // != 0 entspr. Object erzeugen.
                          BOOL bActivate = TRUE,
                          USHORT nSlotId = 0);       // SlotId fuer Dialog

    BOOL    InsertOleObject( const svt::EmbeddedObjectRef& xObj, SwFlyFrmFmt **pFlyFrmFmt = 0 );
    void    LaunchOLEObj( long nVerb = 0 );             // Server starten
    BOOL    IsOLEObj() const { return GetCntType() == CNT_OLE;}
    virtual void MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset );
    virtual void CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                                  const SwRect *pFlyPrtRect = 0,
                                  const SwRect *pFlyFrmRect = 0 );
    virtual void ConnectObj( svt::EmbeddedObjectRef&  xIPObj, const SwRect &rPrt,
                             const SwRect &rFrm );

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

    void    Do( DoType eDoType, USHORT nCnt = 1 );
    String  GetDoString( DoType eDoType ) const;
    String  GetRepeatString() const;
    USHORT  GetDoStrings( DoType eDoType, SfxStringListItem& rStrLstItem ) const;

    //Suchen oder Ersetzen
    ULONG SearchPattern(const com::sun::star::util::SearchOptions& rSearchOpt,
                         BOOL bSearchInNotes,
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
                         const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                         const SfxItemSet* pReplaceSet = 0);

    void AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar = ' ' );

    // Aktion vor Cursorbewegung
    // Hebt gfs. Selektionen auf, triggert Timer und GCAttr()
    void    MoveCrsr( BOOL bWithSelect = FALSE );

    // Eingabefelder updaten
    BOOL    StartInputFldDlg(SwField*, BOOL bNextButton, Window* pParentWin = 0, ByteString* pWindowState = 0);
    // update DropDown fields
    BOOL    StartDropDownFldDlg(SwField*, BOOL bNextButton, ByteString* pWindowState = 0);

    //"Handler" fuer Anederungen an der DrawView - fuer Controls.
    virtual void DrawSelChanged( );

    // springe zum Bookmark und setze die "Selections-Flags" wieder richtig
    BOOL GotoMark( const ::sw::mark::IMark* const pMark );
    BOOL GotoMark( const ::sw::mark::IMark* const pMark, BOOL bSelect, BOOL bStart );
    BOOL GotoMark( const ::rtl::OUString& rName );
    BOOL GoNextBookmark(); // TRUE, wenn's noch eine gab
    BOOL GoPrevBookmark();

    bool GotoFieldmark(::sw::mark::IFieldmark const * const pMark);

    BOOL GotoField( const SwFmtFld& rFld );

    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    BOOL SelectNextPrevHyperlink( BOOL bNext = TRUE );

    // Zugehoerige SwView ermitteln
    const SwView&       GetView() const { return rView; }
    SwView&             GetView() { return rView; }

    //Weil es sonst keiner macht, gibt es hier eine ExecMacro()
    void ExecMacro( const SvxMacro& rMacro, String* pRet = 0, SbxArray* pArgs = 0 );
    // rufe ins dunkle Basic/JavaScript
    USHORT CallEvent( USHORT nEvent, const SwCallMouseEvent& rCallEvent,
                        BOOL bCheckPtr = FALSE, SbxArray* pArgs = 0,
                        const Link* pCallBack = 0 );

    // ein Klick aus das angegebene Feld. Der Cursor steht auf diesem.
    // Fuehre die vor definierten Aktionen aus.
    void ClickToField( const SwField& rFld );
    void ClickToINetAttr( const SwFmtINetFmt& rItem, USHORT nFilter = URLLOAD_NOFILTER );
    BOOL ClickToINetGrf( const Point& rDocPt, USHORT nFilter = URLLOAD_NOFILTER );
    inline BOOL IsInClickToEdit() const ;

    // fall ein URL-Button selektiert ist, dessen URL returnen, ansonsten
    // einen LeerString
    BOOL GetURLFromButton( String& rURL, String& rDescr ) const;

    void NavigatorPaste( const NaviContentBookmark& rBkmk,
                         const USHORT nAction );

    // die Core erzeugt eine Selektion, das SttSelect muss gerufen werden
    virtual void NewCoreSelection();

    // autom. Update von Vorlagen
    void AutoUpdateFrame(SwFrmFmt* pFmt, const SfxItemSet& rStyleSet);
    void AutoUpdatePara(SwTxtFmtColl* pColl, const SfxItemSet& rStyleSet);

    // Link fuers einfuegen von Bereichen uebers Drag&Drop/Clipboard
    DECL_STATIC_LINK( SwWrtShell, InsertRegionDialog, SwSectionData* );


    //ctoren, der erstere ist eine Art kontrollierter copy ctor fuer weitere
    //Sichten auf ein Dokument
    SwWrtShell( SwWrtShell&, Window *pWin, SwView &rShell);
    SwWrtShell( SwDoc& rDoc, Window *pWin, SwView &rShell,
                const SwViewOption *pViewOpt = 0);
    virtual ~SwWrtShell();

    BOOL TryRemoveIndent(); // #i23725#

    String GetSelDescr() const;

private:

    SW_DLLPRIVATE void  OpenMark();
    SW_DLLPRIVATE void  CloseMark( BOOL bOkFlag );

    SW_DLLPRIVATE String    GetWrdDelim();
    SW_DLLPRIVATE String    GetSDelim();
    SW_DLLPRIVATE String    GetBothDelim();

    struct ModeStack
    {
        ModeStack   *pNext;
        BOOL        bAdd,
                    bBlock,
                    bExt,
                    bIns;
        ModeStack(ModeStack *pNextMode, BOOL _bIns, BOOL _bExt, BOOL _bAdd, BOOL _bBlock):
            pNext(pNextMode),
            bAdd(_bAdd),
            bBlock(_bBlock),
            bExt(_bExt),
            bIns(_bIns)
             {}
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
            : aDocPos(rDocPos),
            pNext(pN),
            bValidCurPos( bValid ),
            bIsFrmSel( bFrmSel ),
            lOffset(lOff)
        {


        }

    } *pCrsrStack;

    SwView  &rView;

    Point   aDest;
    BOOL    bDestOnStack;
    BOOL    HasCrsrStack() const { return 0 != pCrsrStack; }
    SW_DLLPRIVATE BOOL  PushCrsr(SwTwips lOffset, BOOL bSelect);
    SW_DLLPRIVATE BOOL  PopCrsr(BOOL bUpdate, BOOL bSelect = FALSE);

    // ENDE Cursor bei PageUp / -Down mitnehmen
    SW_DLLPRIVATE BOOL _SttWrd();
    SW_DLLPRIVATE BOOL _EndWrd();
    SW_DLLPRIVATE BOOL _NxtWrd();
    SW_DLLPRIVATE BOOL _PrvWrd();
    // --> OD 2008-08-06 #i92468#
    SW_DLLPRIVATE BOOL _NxtWrdForDelete();
    SW_DLLPRIVATE BOOL _PrvWrdForDelete();
    // <--
    SW_DLLPRIVATE BOOL _FwdSentence();
    SW_DLLPRIVATE BOOL _BwdSentence();
    BOOL _FwdPara();
    SW_DLLPRIVATE BOOL _BwdPara();

        //  Selektionen
    BOOL    bIns            :1;
    BOOL    bInSelect       :1;
    BOOL    bExtMode        :1;
    BOOL    bAddMode        :1;
    BOOL    bBlockMode      :1;
    BOOL    bLayoutMode     :1;
    BOOL    bNoEdit         :1;
    BOOL    bCopy           :1;
    BOOL    bSelWrd         :1;
    BOOL    bSelLn          :1;
    BOOL    bIsInClickToEdit:1;
    BOOL    bClearMark      :1;     // Selektion fuer ChartAutoPilot nicht loeschen
    BOOL    mbRetainSelection :1; // Do not remove selections

    Point   aStart;
    Link    aSelTblLink;

    SELECTFUNC  fnLeaveSelect;

    //setzt den Cursorstack nach dem Bewegen mit PageUp/-Down zurueck.
    SW_DLLPRIVATE void  _ResetCursorStack();

    SW_DLLPRIVATE void  SttDragDrop(Timer *);

    using SwCrsrShell::SetCrsr;
    SW_DLLPRIVATE long  SetCrsr(const Point *, BOOL bProp=FALSE );

    SW_DLLPRIVATE long  SetCrsrKillSel(const Point *, BOOL bProp=FALSE );

    SW_DLLPRIVATE long  StdSelect(const Point *, BOOL bProp=FALSE );
    SW_DLLPRIVATE long  BeginDrag(const Point *, BOOL bProp=FALSE );
    SW_DLLPRIVATE long  Drag(const Point *, BOOL bProp=FALSE );
    SW_DLLPRIVATE long  EndDrag(const Point *, BOOL bProp=FALSE );

    SW_DLLPRIVATE long  ExtSelWrd(const Point *, BOOL bProp=FALSE );
    SW_DLLPRIVATE long  ExtSelLn(const Point *, BOOL bProp=FALSE );

    //Verschieben von Text aus Drag and Drop; Point ist
    //Destination fuer alle Selektionen.
    SW_DLLPRIVATE long  MoveText(const Point *, BOOL bProp=FALSE );

    SW_DLLPRIVATE long  BeginFrmDrag(const Point *, BOOL bProp=FALSE );

    //nach SSize/Move eines Frames Update; Point ist Destination.
    SW_DLLPRIVATE long  UpdateLayoutFrm(const Point *, BOOL bProp=FALSE );

    SW_DLLPRIVATE long  SttLeaveSelect(const Point *, BOOL bProp=FALSE );
    SW_DLLPRIVATE long  AddLeaveSelect(const Point *, BOOL bProp=FALSE );
    SW_DLLPRIVATE long  Ignore(const Point *, BOOL bProp=FALSE );

    SW_DLLPRIVATE void  LeaveExtSel() { bSelWrd = bSelLn = FALSE;}
    SW_DLLPRIVATE BOOL  _CanInsert();

    SW_DLLPRIVATE BOOL  GoStart(BOOL KeepArea = FALSE, BOOL * = 0,
            BOOL bSelect = FALSE, BOOL bDontMoveRegion = FALSE);
    SW_DLLPRIVATE BOOL  GoEnd(BOOL KeepArea = FALSE, BOOL * = 0);

    enum BookMarkMove
    {
        BOOKMARK_INDEX,
        BOOKMARK_NEXT,
        BOOKMARK_PREV,
        BOOKMARK_LAST_LAST_ENTRY
    };

    SW_DLLPRIVATE BOOL MoveBookMark(BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark=NULL);
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
    return sal::static_int_cast< BOOL >(fnDrag == &SwWrtShell::BeginDrag);
#endif
}

#endif
