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
#ifndef _WRTSH_HXX
#define _WRTSH_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include "swdllapi.h"
#include <fesh.hxx>
#include <sortopt.hxx>
#include <swurl.hxx>
#include <IMark.hxx>
#include "navmgr.hxx"

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

    typedef long (SwWrtShell::*SELECTFUNC)(const Point *, sal_Bool bProp );

    SELECTFUNC  fnDrag;
    SELECTFUNC  fnSetCrsr;
    SELECTFUNC  fnEndDrag;
    SELECTFUNC  fnKillSel;

    // Alle Selektionen aufheben
    long ResetSelect( const Point *, sal_Bool );

    //setzt den Cursorstack nach dem Bewegen mit PageUp/-Down
    //zurueck, falls ein Stack aufgebaut ist
    inline void ResetCursorStack();
    SelectionType   GetSelectionType() const;

    sal_Bool    IsModePushed() const { return 0 != pModeStack; }
    void    PushMode();
    void    PopMode();

    void    SttSelect();
    void    EndSelect();
    sal_Bool    IsInSelect() const { return bInSelect; }
    void    SetInSelect(sal_Bool bSel = sal_True) { bInSelect = bSel; }
        // Liegt eine Text- oder Rahmenselektion vor?
    sal_Bool    HasSelection() const { return SwCrsrShell::HasSelection() ||
                                        IsMultiSelection() || IsSelFrmMode() || IsObjSelected(); }
    sal_Bool Pop( sal_Bool bOldCrsr = sal_True );

    void    EnterStdMode();
    sal_Bool    IsStdMode() const { return !bExtMode && !bAddMode && !bBlockMode; }

    void    EnterExtMode();
    void    LeaveExtMode();
    long    ToggleExtMode();
    sal_Bool    IsExtMode() const { return bExtMode; }

    void    EnterAddMode();
    void    LeaveAddMode();
    long    ToggleAddMode();
    sal_Bool    IsAddMode() const { return bAddMode; }

    void    EnterBlockMode();
    void    LeaveBlockMode();
    long    ToggleBlockMode();
    sal_Bool    IsBlockMode() const { return bBlockMode; }

    void    SetInsMode( sal_Bool bOn = sal_True );
    void    ToggleInsMode() { SetInsMode( !bIns ); }
    sal_Bool    IsInsMode() const { return bIns; }
    void    SetRedlineModeAndCheckInsMode( sal_uInt16 eMode );

    void    EnterSelFrmMode(const Point *pStartDrag = 0);
    void    LeaveSelFrmMode();
    sal_Bool    IsSelFrmMode() const { return bLayoutMode; }
        // Selektion von Rahmen aufheben
    void    UnSelectFrm();

    void    Invalidate();

    // Tabellenzellen selektieren fuer Bearbeiten von Formeln in der Ribbonbar
    inline void SelTblCells( const Link &rLink, sal_Bool bMark = sal_True );
    inline void EndSelTblCells();

    //Wortweisen oder zeilenweisen Selektionsmodus verlassen. Wird
    //in der Regel in MB-Up gerufen.
    sal_Bool    IsExtSel() const { return bSelWrd || bSelLn; }

    // erfrage, ob der akt. fnDrag - Pointer auf BeginDrag gesetzt ist
    // Wird fuer MouseMove gebraucht, um die Bugs 55592/55931 zu umgehen.
    inline sal_Bool Is_FnDragEQBeginDrag() const;

    //Basisabfragen
    sal_Bool    IsInWrd()           { return IsInWord(); }
    sal_Bool    IsSttWrd()          { return IsStartWord(); }
    sal_Bool    IsEndWrd();
    sal_Bool    IsSttOfPara() const { return IsSttPara(); }
    sal_Bool    IsEndOfPara() const { return IsEndPara(); }

    //Word bzw. Satz selektieren.
    sal_Bool    SelNearestWrd();
    sal_Bool    SelWrd      (const Point * = 0, sal_Bool bProp=sal_False );
    // --> FME 2004-07-30 #i32329# Enhanced selection
    void    SelSentence (const Point * = 0, sal_Bool bProp=sal_False );
    void    SelPara     (const Point * = 0, sal_Bool bProp=sal_False );
    // <--
    long    SelAll();

    //Basiscursortravelling
typedef sal_Bool (SwWrtShell:: *FNSimpleMove)();
    sal_Bool SimpleMove( FNSimpleMove, sal_Bool bSelect );

    sal_Bool Left       ( sal_uInt16 nMode, sal_Bool bSelect,
                            sal_uInt16 nCount, sal_Bool bBasicCall, sal_Bool bVisual = sal_False );
    sal_Bool Right      ( sal_uInt16 nMode, sal_Bool bSelect,
                            sal_uInt16 nCount, sal_Bool bBasicCall, sal_Bool bVisual = sal_False );
    sal_Bool Up         ( sal_Bool bSelect = sal_False, sal_uInt16 nCount = 1,
                            sal_Bool bBasicCall = sal_False );
    sal_Bool Down       ( sal_Bool bSelect = sal_False, sal_uInt16 nCount = 1,
                            sal_Bool bBasicCall = sal_False );
    sal_Bool NxtWrd     ( sal_Bool bSelect = sal_False ) { return SimpleMove( &SwWrtShell::_NxtWrd, bSelect ); }
    sal_Bool PrvWrd     ( sal_Bool bSelect = sal_False ) { return SimpleMove( &SwWrtShell::_PrvWrd, bSelect ); }

    sal_Bool LeftMargin ( sal_Bool bSelect, sal_Bool bBasicCall );
    sal_Bool RightMargin( sal_Bool bSelect, sal_Bool bBasicCall );

    sal_Bool SttDoc     ( sal_Bool bSelect = sal_False );
    sal_Bool EndDoc     ( sal_Bool bSelect = sal_False );

    sal_Bool SttNxtPg   ( sal_Bool bSelect = sal_False );
    sal_Bool SttPrvPg   ( sal_Bool bSelect = sal_False );
    sal_Bool EndNxtPg   ( sal_Bool bSelect = sal_False );
    sal_Bool EndPrvPg   ( sal_Bool bSelect = sal_False );
    sal_Bool SttPg      ( sal_Bool bSelect = sal_False );
    sal_Bool EndPg      ( sal_Bool bSelect = sal_False );
    sal_Bool SttPara    ( sal_Bool bSelect = sal_False );
    sal_Bool EndPara    ( sal_Bool bSelect = sal_False );
    sal_Bool FwdPara    ( sal_Bool bSelect = sal_False )
                { return SimpleMove( &SwWrtShell::_FwdPara, bSelect ); }
    sal_Bool BwdPara    ( sal_Bool bSelect = sal_False )
                { return SimpleMove( &SwWrtShell::_BwdPara, bSelect ); }
    sal_Bool FwdSentence( sal_Bool bSelect = sal_False )
                { return SimpleMove( &SwWrtShell::_FwdSentence, bSelect ); }
    sal_Bool BwdSentence( sal_Bool bSelect = sal_False )
                { return SimpleMove( &SwWrtShell::_BwdSentence, bSelect ); }

    // --> FME 2004-07-30 #i20126# Enhanced table selection
    sal_Bool SelectTableRowCol( const Point& rPt, const Point* pEnd = 0, bool bRowDrag = false );
    // <--
    sal_Bool SelectTableRow();
    sal_Bool SelectTableCol();
    sal_Bool SelectTableCell();

    sal_Bool SelectTxtAttr( sal_uInt16 nWhich, const SwTxtAttr* pAttr = 0 );

    // Spaltenweise Spruenge
    sal_Bool StartOfColumn      ( sal_Bool bSelect = sal_False );
    sal_Bool EndOfColumn        ( sal_Bool bSelect = sal_False );
    sal_Bool StartOfNextColumn  ( sal_Bool bSelect = sal_False );
    sal_Bool EndOfNextColumn    ( sal_Bool bSelect = sal_False );
    sal_Bool StartOfPrevColumn  ( sal_Bool bSelect = sal_False );
    sal_Bool EndOfPrevColumn    ( sal_Bool bSelect = sal_False );

    // setze den Cursor auf die Seite "nPage" an den Anfang
    // zusaetzlich zu der gleichnamigen Implementierung in crsrsh.hxx
    // werden hier alle bestehenden Selektionen vor dem Setzen des
    // Cursors aufgehoben
    sal_Bool    GotoPage( sal_uInt16 nPage, sal_Bool bRecord );

    //setzen des Cursors; merken der alten Position fuer Zurueckblaettern.
    DECL_LINK( ExecFlyMac, void * );

    sal_Bool    PageCrsr(SwTwips lOffset, sal_Bool bSelect);

    // Felder Update
    void    UpdateInputFlds( SwInputFieldList* pLst = 0, sal_Bool bOnlyInSel = sal_False );

    void    NoEdit(sal_Bool bHideCrsr = sal_True);
    void    Edit();
    sal_Bool    IsNoEdit() const { return bNoEdit; }

    sal_Bool IsRetainSelection() const { return mbRetainSelection; }
    void SetRetainSelection( sal_Bool bRet ) { mbRetainSelection = bRet; }

    // change current data base and notify
    void ChgDBData(const SwDBData& SwDBData);

    // Loeschen
    long    DelToEndOfLine();
    long    DelToStartOfLine();
    long    DelLine();
    long    DelLeft();

    // loescht auch Rahmen bzw. setzt den Cursor in den Rahmen,
    // wenn bDelFrm == sal_False ist
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
    int     IntelligentCut(int nSelectionType, sal_Bool bCut = sal_True);

    // Editieren
    void    Insert(SwField &);
    void    Insert(const String &);
    // Graphic
    void    Insert( const String &rPath, const String &rFilter,
                    const Graphic &, SwFlyFrmAttrMgr * = 0,
                    sal_Bool bRule = sal_False );

    void    InsertByWord( const String & );
    void    InsertPageBreak(const String *pPageDesc = 0, sal_uInt16 nPgNum = 0 );
    void    InsertLineBreak();
    void    InsertColumnBreak();
    void    InsertFootnote(const String &, sal_Bool bEndNote = sal_False, sal_Bool bEdit = sal_True );
    void    SplitNode( sal_Bool bAutoFormat = sal_False, sal_Bool bCheckTableStart = sal_True );
    sal_Bool    CanInsert();

    // Verzeichnisse
    void    InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = 0);
    sal_Bool    UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = 0);

    // Numerierung und Bullets
    /**
       Turns on numbering or bullets.

       @param bNum    sal_True: turn on numbering
                      sal_False: turn on bullets
    */
    void    NumOrBulletOn(sal_Bool bNum); // #i29560#
    void    NumOrBulletOff(); // #i29560#
    void    NumOn();
    void    BulletOn();

    //OLE
    void    InsertObject(     /*SvInPlaceObjectRef *pObj, */       // != 0 fuer Clipboard
                          const svt::EmbeddedObjectRef&,
                          SvGlobalName *pName = 0,      // != 0 entspr. Object erzeugen.
                          sal_Bool bActivate = sal_True,
                          sal_uInt16 nSlotId = 0);       // SlotId fuer Dialog

    sal_Bool    InsertOleObject( const svt::EmbeddedObjectRef& xObj, SwFlyFrmFmt **pFlyFrmFmt = 0 );
    void    LaunchOLEObj( long nVerb = 0 );             // Server starten
    sal_Bool    IsOLEObj() const { return GetCntType() == CNT_OLE;}
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

    String  GetCurPageStyle( const sal_Bool bCalcFrm = sal_True ) const;

    // Aktuelle Vorlage anhand der geltenden Attribute aendern
    void    QuickUpdateStyle();

    enum DoType { UNDO, REDO, REPEAT };

    void    Do( DoType eDoType, sal_uInt16 nCnt = 1 );
    String  GetDoString( DoType eDoType ) const;
    String  GetRepeatString() const;
    sal_uInt16  GetDoStrings( DoType eDoType, SfxStringListItem& rStrLstItem ) const;

    //Suchen oder Ersetzen
    sal_uLong SearchPattern(const com::sun::star::util::SearchOptions& rSearchOpt,
                         sal_Bool bSearchInNotes,
                         SwDocPositions eStart, SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         int bReplace = sal_False );

    sal_uLong SearchTempl  (const String &rTempl,
                         SwDocPositions eStart, SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         const String* pReplTempl = 0 );

    sal_uLong SearchAttr   (const SfxItemSet& rFindSet,
                         sal_Bool bNoColls,
                         SwDocPositions eStart,SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                         const SfxItemSet* pReplaceSet = 0);

    void AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar = ' ' );

    // Aktion vor Cursorbewegung
    // Hebt gfs. Selektionen auf, triggert Timer und GCAttr()
    void    MoveCrsr( sal_Bool bWithSelect = sal_False );

    // Eingabefelder updaten
    sal_Bool    StartInputFldDlg(SwField*, sal_Bool bNextButton, Window* pParentWin = 0, ByteString* pWindowState = 0);
    // update DropDown fields
    sal_Bool    StartDropDownFldDlg(SwField*, sal_Bool bNextButton, ByteString* pWindowState = 0);

    //"Handler" fuer Anederungen an der DrawView - fuer Controls.
    virtual void DrawSelChanged( );

    // springe zum Bookmark und setze die "Selections-Flags" wieder richtig
    sal_Bool GotoMark( const ::sw::mark::IMark* const pMark );
    sal_Bool GotoMark( const ::sw::mark::IMark* const pMark, sal_Bool bSelect, sal_Bool bStart );
    sal_Bool GotoMark( const ::rtl::OUString& rName );
    sal_Bool GoNextBookmark(); // sal_True, wenn's noch eine gab
    sal_Bool GoPrevBookmark();

    bool GotoFieldmark(::sw::mark::IFieldmark const * const pMark);

    sal_Bool GotoField( const SwFmtFld& rFld );

    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    sal_Bool SelectNextPrevHyperlink( sal_Bool bNext = sal_True );

    // Zugehoerige SwView ermitteln
    const SwView&       GetView() const { return rView; }
    SwView&             GetView() { return rView; }

    //Weil es sonst keiner macht, gibt es hier eine ExecMacro()
    void ExecMacro( const SvxMacro& rMacro, String* pRet = 0, SbxArray* pArgs = 0 );
    // rufe ins dunkle Basic/JavaScript
    sal_uInt16 CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                        sal_Bool bCheckPtr = sal_False, SbxArray* pArgs = 0,
                        const Link* pCallBack = 0 );

    // ein Klick aus das angegebene Feld. Der Cursor steht auf diesem.
    // Fuehre die vor definierten Aktionen aus.
    void ClickToField( const SwField& rFld );
    void ClickToINetAttr( const SwFmtINetFmt& rItem, sal_uInt16 nFilter = URLLOAD_NOFILTER );
    sal_Bool ClickToINetGrf( const Point& rDocPt, sal_uInt16 nFilter = URLLOAD_NOFILTER );
    inline sal_Bool IsInClickToEdit() const ;

    // fall ein URL-Button selektiert ist, dessen URL returnen, ansonsten
    // einen LeerString
    sal_Bool GetURLFromButton( String& rURL, String& rDescr ) const;

    void NavigatorPaste( const NaviContentBookmark& rBkmk,
                         const sal_uInt16 nAction );

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

    sal_Bool TryRemoveIndent(); // #i23725#

    String GetSelDescr() const;

    SwNavigationMgr& GetNavigationMgr();
    void addCurrentPosition();
    sal_Bool GotoFly( const String& rName, FlyCntType eType = FLYCNTTYPE_ALL,
         sal_Bool bSelFrame = sal_True );
    sal_Bool GotoINetAttr( const SwTxtINetFmt& rAttr );
    void GotoOutline( sal_uInt16 nIdx );
    sal_Bool GotoOutline( const String& rName );
    sal_Bool GotoRegion( const String& rName );
    sal_Bool GotoRefMark( const String& rRefMark, sal_uInt16 nSubType = 0,
        sal_uInt16 nSeqNo = 0 );
    sal_Bool GotoNextTOXBase( const String* pName = 0);
    sal_Bool GotoTable( const String& rName );
    sal_Bool GotoFld( const SwFmtFld& rFld );
    const SwRedline* GotoRedline( sal_uInt16 nArrPos, sal_Bool bSelect = sal_False);

private:

    SW_DLLPRIVATE void  OpenMark();
    SW_DLLPRIVATE void  CloseMark( sal_Bool bOkFlag );

    SW_DLLPRIVATE String    GetWrdDelim();
    SW_DLLPRIVATE String    GetSDelim();
    SW_DLLPRIVATE String    GetBothDelim();

    struct ModeStack
    {
        ModeStack   *pNext;
        sal_Bool        bAdd,
                    bBlock,
                    bExt,
                    bIns;
        ModeStack(ModeStack *pNextMode, sal_Bool _bIns, sal_Bool _bExt, sal_Bool _bAdd, sal_Bool _bBlock):
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
        sal_Bool bValidCurPos : 1;
        sal_Bool bIsFrmSel : 1;
        SwTwips lOffset;

        CrsrStack( sal_Bool bValid, sal_Bool bFrmSel, const Point &rDocPos,
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
    SwNavigationMgr aNavigationMgr;

    Point   aDest;
    sal_Bool    bDestOnStack;
    sal_Bool    HasCrsrStack() const { return 0 != pCrsrStack; }
    SW_DLLPRIVATE sal_Bool  PushCrsr(SwTwips lOffset, sal_Bool bSelect);
    SW_DLLPRIVATE sal_Bool  PopCrsr(sal_Bool bUpdate, sal_Bool bSelect = sal_False);

    // ENDE Cursor bei PageUp / -Down mitnehmen
    SW_DLLPRIVATE sal_Bool _SttWrd();
    SW_DLLPRIVATE sal_Bool _EndWrd();
    SW_DLLPRIVATE sal_Bool _NxtWrd();
    SW_DLLPRIVATE sal_Bool _PrvWrd();
    // --> OD 2008-08-06 #i92468#
    SW_DLLPRIVATE sal_Bool _NxtWrdForDelete();
    SW_DLLPRIVATE sal_Bool _PrvWrdForDelete();
    // <--
    SW_DLLPRIVATE sal_Bool _FwdSentence();
    SW_DLLPRIVATE sal_Bool _BwdSentence();
    sal_Bool _FwdPara();
    SW_DLLPRIVATE sal_Bool _BwdPara();

        //  Selektionen
    sal_Bool    bIns            :1;
    sal_Bool    bInSelect       :1;
    sal_Bool    bExtMode        :1;
    sal_Bool    bAddMode        :1;
    sal_Bool    bBlockMode      :1;
    sal_Bool    bLayoutMode     :1;
    sal_Bool    bNoEdit         :1;
    sal_Bool    bCopy           :1;
    sal_Bool    bSelWrd         :1;
    sal_Bool    bSelLn          :1;
    sal_Bool    bIsInClickToEdit:1;
    sal_Bool    bClearMark      :1;     // Selektion fuer ChartAutoPilot nicht loeschen
    sal_Bool    mbRetainSelection :1; // Do not remove selections

    Point   aStart;
    Link    aSelTblLink;

    SELECTFUNC  fnLeaveSelect;

    //setzt den Cursorstack nach dem Bewegen mit PageUp/-Down zurueck.
    SW_DLLPRIVATE void  _ResetCursorStack();

    SW_DLLPRIVATE void  SttDragDrop(Timer *);

    using SwCrsrShell::SetCrsr;
    SW_DLLPRIVATE long  SetCrsr(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  SetCrsrKillSel(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  StdSelect(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  BeginDrag(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  Drag(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  EndDrag(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  ExtSelWrd(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  ExtSelLn(const Point *, sal_Bool bProp=sal_False );

    //Verschieben von Text aus Drag and Drop; Point ist
    //Destination fuer alle Selektionen.
    SW_DLLPRIVATE long  MoveText(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  BeginFrmDrag(const Point *, sal_Bool bProp=sal_False );

    //nach SSize/Move eines Frames Update; Point ist Destination.
    SW_DLLPRIVATE long  UpdateLayoutFrm(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  SttLeaveSelect(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  AddLeaveSelect(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  Ignore(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE void  LeaveExtSel() { bSelWrd = bSelLn = sal_False;}
    SW_DLLPRIVATE sal_Bool  _CanInsert();

    SW_DLLPRIVATE sal_Bool  GoStart(sal_Bool KeepArea = sal_False, sal_Bool * = 0,
            sal_Bool bSelect = sal_False, sal_Bool bDontMoveRegion = sal_False);
    SW_DLLPRIVATE sal_Bool  GoEnd(sal_Bool KeepArea = sal_False, sal_Bool * = 0);

    enum BookMarkMove
    {
        BOOKMARK_INDEX,
        BOOKMARK_NEXT,
        BOOKMARK_PREV,
        BOOKMARK_LAST_LAST_ENTRY
    };

    SW_DLLPRIVATE sal_Bool MoveBookMark(BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark=NULL);
};

inline void SwWrtShell::ResetCursorStack()
{
    if ( HasCrsrStack() )
        _ResetCursorStack();
}

inline void SwWrtShell::SelTblCells(const Link &rLink, sal_Bool bMark )
{
    SetSelTblCells( sal_True );
    bClearMark = bMark;
    aSelTblLink = rLink;
}
inline void SwWrtShell::EndSelTblCells()
{
    SetSelTblCells( sal_False );
    bClearMark = sal_True;
}

inline sal_Bool SwWrtShell::IsInClickToEdit() const { return bIsInClickToEdit; }

inline sal_Bool SwWrtShell::Is_FnDragEQBeginDrag() const
{
#ifdef GCC
    SELECTFUNC  fnTmp = &SwWrtShell::BeginDrag;
    return fnDrag == fnTmp;
#else
    return sal::static_int_cast< sal_Bool >(fnDrag == &SwWrtShell::BeginDrag);
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
