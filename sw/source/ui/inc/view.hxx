/*************************************************************************
 *
 *  $RCSfile: view.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-19 13:31:07 $
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
#ifndef _SWVIEW_HXX
#define _SWVIEW_HXX

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _VIEWFAC_HXX //autogen
#include <sfx2/viewfac.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SHELLID_HXX
#include <shellid.hxx>
#endif

class SwBaseShell;
class Button;
class ImageButton;
class SwTxtFmtColl;
class SwPageDesc;
class SwFrmFmt;
class SwCharFmt;
class SwNumRule;
class SwViewOption;
class SwGlossaryHdl;
class SwDrawBase;
class SvxRuler;
class SvxLRSpaceItem;
class SwDocShell;
class SwScrollbar;
class PrintDialog;
class SvxVCBrowser;
class SvBorder;
class Ruler;
class OutlinerView;
class SvxSearchItem;
class SearchAttrItemList;
class SvxSearchDialog;
class SdrView;
class Dialog;
class SdrObject;
class SdrPageView;
class SwNaviImageButton;
class SwHlpImageButton;
class Outliner;
class SwView;
class SwEditWin;
class SwWrtShell;
class SwView_Impl;
namespace com{ namespace sun { namespace star {namespace view{ class XSelectionSupplier;}}}}
class XSelectionObject;
struct SwSearchOptions;
class FmFormShell;
class CommandEvent;
class InsCaptionOpt;
class SvGlobalName;

const long nLeftOfst = -370;
const long nScrollX  =   30;
const long nScrollY  =   30;

#define MINZOOM 20
#define MAXZOOM 600

#define MAX_MARKS 5


enum ShellModes
{
    SEL_TEXT,
    SEL_FRAME,
    SEL_GRAPHIC,
    SEL_OBJECT,
    SEL_DRAW,
    SEL_DRAW_CTRL,
    SEL_DRAW_FORM,
    SEL_DRAWTEXT,
    SEL_BEZIER,
    SEL_LIST_TEXT,
    SEL_TABLE_TEXT,
    SEL_TABLE_LIST_TEXT
};

/*--------------------------------------------------------------------
    Beschreibung:   Anwendung einer Vorlage
 --------------------------------------------------------------------*/

struct SwApplyTemplate
{
    union
    {
        SwTxtFmtColl* pTxtColl;
        SwPageDesc*   pPageDesc;
        SwFrmFmt*     pFrmFmt;
        SwCharFmt*    pCharFmt;
        SwNumRule*    pNumRule;
    } aColl;

    int eType;
    USHORT nColor;
    BOOL bUndo;

    SwApplyTemplate() :
        eType(0),
        nColor(0),
        bUndo(FALSE)
    {
        aColl.pTxtColl = 0;
    }
};


/*--------------------------------------------------------------------
    Beschreibung:   Sicht auf ein Dokument
 --------------------------------------------------------------------*/

class SwView: public SfxViewShell
{
    //Messehack (MA,MBA)
    friend void lcl_SelectShellForDrop( SwView &rView );

    friend class TestTemplateItem;
    friend class SwSpellWrapper;
    friend class SwHyphWrapper;
    friend class SwView_Impl;

    //suchen & ersetzen
    static SvxSearchDialog *pSrchDlg;
    static SvxSearchItem   *pSrchItem;

    static USHORT           nInsertCtrlState;
    static USHORT           nWebInsertCtrlState;
    static USHORT           nInsertObjectCtrlState;
    static USHORT           nInsertFieldCtrlState;
    static USHORT           nMoveType; // fuer Buttons unter dem Scrollbar (viewmdi)
    static BYTE             nActMark; // aktuelle Sprungmarke fuer unbenannte Merker

    static BOOL             bExtra;
    static BOOL             bFound;
    static BOOL             bJustOpened;

    static SearchAttrItemList* pSrchList;
    static SearchAttrItemList* pReplList;


    Timer               aTimer;         //Fuer verzoegerte ChgLnks waehrend
                                        //einer Aktion
    String              aPageStr;       //Statusanzeige, aktuelle Seite
    String              sSwViewData,
    //and the new cursor position if the user double click in the PagePreView
                        sNewCrsrPos;

    Point               aTabColFromDocPos;  //Verschieben von Tabellenspalten aus
                                            //aus dem Dokument heraus.
    Size                aDocSz;         // aktuelle Dokumentgroesse
    Rectangle           aVisArea;       // sichtbarer Bereich

    SwEditWin           *pEditWin;
    SwWrtShell          *pWrtShell;

    SfxShell            *pShell;        //aktuelle SubShell auf dem Dispatcher
    FmFormShell         *pFormShell;    // DB-FormShell

    SwView_Impl         *pViewImpl;     // Impl-Daten fuer UNO + Basic


    SwScrollbar         *pHScrollbar,   // MDI Bedienelemente
                        *pVScrollbar;

    Window              *pScrollFill;   // Dummy-Window zum Fllen der rechten
                                        // unteren Ecke, wenn beide Scrollbars
                                        // aktiv sind

    SvxRuler            *pHLineal,
                        *pVLineal;
    ImageButton         *pTogglePageBtn;

    SwHlpImageButton    *pPageUpBtn,
                        *pPageDownBtn;

    SwNaviImageButton   *pNaviBtn;
    SwGlossaryHdl       *pGlosHdl;          // Henkel Textbausteine
    SwDrawBase          *pDrawActual;

    const SwFrmFmt*     pLastTableFormat;

    int                 nSelectionType;

    // Aktueller Drawmode
    USHORT          nDrawSfxId;
    USHORT          nFormSfxId;

    BOOL            bCenterCrsr : 1,
                    bTopCrsr : 1,
                    bAllwaysShowSel : 1,
                    bTabColFromDoc : 1,
                    bSetTabColFromDoc : 1 ,
                    bAttrChgNotified : 1,
                    bAttrChgNotifiedWithRegistrations : 1,
                    bVerbsActive : 1,
                    bDrawRotate : 1,
                    bDrawSelMode : 1,
                    bShowAtResize : 1,
                    bInOuterResizePixel : 1,
                    bIsApi : 1;

    // Methoden fuers Suchen
    // Suchkontext setzen
    BOOL            SearchAndWrap(BOOL bApi = FALSE);
    BOOL            SearchAll(USHORT* pFound = 0);
    ULONG           FUNC_Search( const SwSearchOptions& rOptions );
    void            Replace();

    inline BOOL     IsDocumentBorder();

    BOOL            IsTextTool() const;

    // Bedienelemente verwalten anlegen
    void            CreateBtns();
    DECL_LINK( BtnPage, Button * );

    DECL_LINK( TimeoutHdl, Timer * );

    inline long     GetXScroll() const;
    inline long     GetYScroll() const;
           Point    AlignToPixel(const Point& rPt) const;
           void     CalcPt( Point* pPt,const Rectangle& rRect,
                            USHORT nRangeX = USHRT_MAX,
                            USHORT nRangeY = USHRT_MAX);

    BOOL            GetPageScrollUpOffset(SwTwips& rOff) const;
    BOOL            GetPageScrollDownOffset(SwTwips& rOff) const;

    // Scrollbar Movements
    long            PageUp();
    long            PageDown();
    long            PageUpCrsr(BOOL bSelect);
    long            PageDownCrsr(BOOL bSelect);
    long            PhyPageUp();
    long            PhyPageDown();

    int             _CreateScrollbar( int bHori );
    int             _KillScrollbar( int bHori );
    DECL_LINK( ScrollHdl, SwScrollbar * );
    DECL_LINK( EndScrollHdl, SwScrollbar * );
    BOOL            UpdateScrollbars();
    void            CalcVisArea( const Size &rPixelSz );

    int             _CreateVLineal();
    int             _KillVLineal();
    int             _CreateTab();
    int             _KillTab();
    void            CreatePageButtons(BOOL bShow);

    // Linguistik-Funktionen
    void            SpellDocument( const String* pStr, BOOL bAllRight = FALSE );
    void            HyphenateDocument();
    BOOL            IsDrawTextHyphenate();
    void            HyphenateDrawText();
    void            StartThesaurus();

    void            SpellStart( SvxSpellArea eSpell, BOOL bStartDone,
                                BOOL bEndDone );
    void            HyphStart( SvxSpellArea eSpell );
    void            SpellEnd();
    void            _SpellDocument( const String* pStr, BOOL bAllRight );
    BOOL            CheckSpecialCntnt();
    void            SpellKontext(BOOL bOn = TRUE)
                            { bCenterCrsr = bOn; bAllwaysShowSel = bOn; }

    // Methoden fuers Printing
    virtual SfxPrinter*     GetPrinter( BOOL bCreate = FALSE );
    virtual PrintDialog*    CreatePrintDialog( Window* pParent );
            SfxTabPage*     CreatePrintOptionsPage( Window* pParent,
                                                    const SfxItemSet& rSet);
    // fuer Readonly-Umschaltung
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    void            _CheckReadonlyState();
    void            _CheckReadonlySelection();

    // Methode fuer PageDesc drehen
    void            SwapPageMargin(const SwPageDesc&, SvxLRSpaceItem& rLR);

    void            _SetZoom( const Size &rEditSz,
                              SvxZoomType eZoomType,
                              short nFactor = 100,
                              BOOL bViewOnly = FALSE);
    void            CalcAndSetBorderPixel( SvBorder &rToFill, FASTBOOL bInner );

    void            ShowAtResize();

    virtual void    Move();


protected:

    SwView_Impl*    GetViewImpl() {return pViewImpl;}

    void ImpSetVerb( int nSelType );

    int             GetSelectionType() const { return nSelectionType; }
    void            SetSelectionType(int nSet) { nSelectionType = nSet;}

    // fuer die SwWebView
    void            SetShell( SfxShell* pS )            { pShell = pS; }
    void            SetFormShell( FmFormShell* pSh )    { pFormShell = pSh; }

    virtual void    SelectShell();

    virtual void    Activate(BOOL);
    virtual void    Deactivate(BOOL);
    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual Size    GetOptimalSizePixel() const;

    void            SetImageButtonColor(Color& rColor);

    const SwFrmFmt* GetLastTblFrmFmt() const {return pLastTableFormat;}
    void            SetLastTblFrmFmt(const SwFrmFmt* pSet) {pLastTableFormat = pSet;}

public:

    SFX_DECL_VIEWFACTORY(SwView);
    SFX_DECL_INTERFACE(SW_VIEWSHELL);
    TYPEINFO();

    SfxDispatcher   &GetDispatcher();

    virtual SdrView*        GetDrawView() const;
    virtual BOOL            HasUIFeature( ULONG nFeature );
    virtual void            ShowCursor( FASTBOOL bOn = TRUE );
    virtual ErrCode         DoVerb( long nVerb );

    virtual USHORT          SetPrinter( SfxPrinter* pNew,
                                        USHORT nDiff = SFX_PRINTER_ALL);
    virtual ErrCode         DoPrint( SfxPrinter *pPrinter, PrintDialog *pPrintDialog,
                                     BOOL bSilent );
    ShellModes              GetShellMode();

    com::sun::star::view::XSelectionSupplier*       GetUNOObject();

    String                  GetSelectionTextParam( BOOL bCompleteWords,
                                                   BOOL bEraseTrail );
    virtual BOOL            HasSelection( BOOL  bText ) const;
    virtual String          GetSelectionText( BOOL bCompleteWords = FALSE );
    virtual USHORT          PrepareClose( BOOL bUI = TRUE, BOOL bForBrowsing = FALSE );
    virtual void            MarginChanged();

    // Shell sofort wechseln ->  fuer GetSelectionObject
    void        StopShellTimer();
    // hier wird bei Bedarf SelectShell gerufen (getSelection)
    void        PrepareGetSelection();

    inline SwWrtShell&      GetWrtShell   () const { return *pWrtShell; }
    inline SwWrtShell*      GetWrtShellPtr() const { return  pWrtShell; }

    inline       SwEditWin &GetEditWin()        { return *pEditWin; }
    inline const SwEditWin &GetEditWin () const { return *pEditWin; }

#if defined WIN || defined WNT || defined UNX
    void ScannerEventHdl( const ::com::sun::star::lang::EventObject& rEventObject );
#endif

    //Handler fuer Textbausteine an die Textshell rausreichen, gfs. anlegen
    SwGlossaryHdl*          GetGlosHdl();

    inline const Rectangle& GetVisArea() const { return aVisArea; }

    BOOL            IsScroll(const Rectangle& rRect) const;
    void            Scroll( const Rectangle& rRect,
                            USHORT nRangeX = USHRT_MAX,
                            USHORT nRangeY = USHRT_MAX);

    long        SetVScrollMax(long lMax);
    long        SetHScrollMax(long lMax);

    DECL_LINK( SpellError, void * );
    BOOL            ExecSpellPopup( const Point& rPt );
    BOOL            ExecDrwTxtSpellPopup(const Point& rPt);

    void            SetTabColFromDocPos( const Point &rPt ) { aTabColFromDocPos = rPt; }
    void            SetTabColFromDoc( BOOL b ) { bTabColFromDoc = b; }
    BOOL            IsTabColFromDoc() const    { return bTabColFromDoc; }

            void    DocSzChgd( const Size& rNewSize );
    const   Size&   GetDocSz() const { return aDocSz; }
    virtual void    SetVisArea( const Rectangle&, BOOL bUpdateScrollbar = TRUE);
            void    SetVisArea( const Point&, BOOL bUpdateScrollbar = TRUE);
            void    CheckVisArea();

    static Dialog*  GetSearchDialog();

    static USHORT   GetMoveType();
    static void     SetMoveType(USHORT nSet);
    void            MoveNavigation(BOOL bNext);
    static void     SetActMark(BYTE nSet);

    BOOL            HandleWheelCommands( const CommandEvent& );

    // Rahmen einfuegen
    void            InsFrmMode(USHORT nCols);

    void            SetZoom( SvxZoomType eZoomType, short nFactor = 100, BOOL bViewOnly = FALSE);
    virtual void    SetZoomFactor( const Fraction &rX, const Fraction & );

    inline int      StatHScrollbar() const;
    inline int      CreateHScrollbar();
    inline int      KillHScrollbar();

    inline int      CreateVScrollbar();
    inline int      KillVScrollbar();
    inline int      StatVScrollbar() const;

    inline int      CreateVLineal();
    inline int      KillVLineal();
    int             StatVLineal() const { return pVLineal != 0; }
    void            ChangeVLinealMetric(FieldUnit eUnit);

    inline int      CreateTab();
    inline int      KillTab();
    int             StatTab() const { return ((Window*)pHLineal)->IsVisible(); }
    SvxRuler       *GetHLineal()    { return pHLineal; }
    SvxRuler       *GetVLineal()    { return pVLineal; }
    void            InvalidateRulerPos();
    void            ChangeTabMetric(FieldUnit eUnit);

        // Handler
    void            Execute(SfxRequest&);
    void            ExecPageMove(SfxRequest&);
    void            ExecStyle(SfxRequest&);
    void            ExecLingu(SfxRequest&);
    void            ExecDataBase(SfxRequest&);
    void            ExecDlg(SfxRequest&);
    void            ExecDlgExt(SfxRequest&);
    void            ExecDBDlg(SfxRequest &);
    void            ExecColl(SfxRequest&);
    void            ExecutePrint(SfxRequest&);
    void            ExecDraw(SfxRequest&);
    void            ExecTabWin(SfxRequest&);
    void            ExecuteStatusLine(SfxRequest&);
    DECL_LINK( ExecRulerClick, Ruler * );
    void            ExecSearch(SfxRequest&, BOOL bNoMessage = FALSE);

    void            StateSearch(SfxItemSet &);
    void            GetState(SfxItemSet&);
    void            StateStatusLine(SfxItemSet&);


    // Funktionen fuer Drawing
    void            SetDrawFuncPtr(SwDrawBase* pFuncPtr);
    inline SwDrawBase* GetDrawFuncPtr(BOOL bBuf = FALSE) const  { return pDrawActual; }
    void            GetDrawState(SfxItemSet &rSet);
    void            ExitDraw();
    inline BOOL     IsDrawRotate()      { return bDrawRotate; }
    inline void     FlipDrawRotate()    { bDrawRotate = !bDrawRotate; }
    inline BOOL     IsDrawSelMode()     { return bDrawSelMode; }
    void            SetSelDrawSlot();
    inline void     FlipDrawSelMode()   { bDrawSelMode = !bDrawSelMode; }
    void            NoRotate();     // Rotate-Mode abschalten
    BOOL            EnterDrawTextMode(const Point& aDocPos);
    void            LeaveDrawCreate()   { nDrawSfxId = nFormSfxId = USHRT_MAX; }
    BOOL            IsDrawMode()        { return (nDrawSfxId != USHRT_MAX || nFormSfxId != USHRT_MAX); }
    BOOL            IsFormMode() const;
    BOOL            IsBezierEditMode();
    BOOL            AreOnlyFormsSelected() const;
    BOOL            HasDrwObj(SdrObject *pSdrObj) const;
    BOOL            HasOnlyObj(SdrObject *pSdrObj, UINT32 eObjInventor) const;
    BOOL            BeginTextEdit(  SdrObject* pObj, SdrPageView* pPV=NULL,
                                    Window* pWin=NULL, BOOL bIsNewObj=FALSE);

    void            StateTabWin(SfxItemSet&);

    // Attribute haben sich geaendert
    DECL_LINK( AttrChangedNotify, SwWrtShell * );

    // Links bearbeiten
    void            EditLinkDlg();
    void            AutoCaption(const USHORT nType, const SvGlobalName *pOleId = 0);
    void            InsertCaption(const InsCaptionOpt *pOpt);

    // Async Aufruf durch Core
    void        UpdatePageNums(USHORT nPhyNum, USHORT nVirtNum, const String& rPgStr);

    String      GetPageStr(USHORT nPhyNum, USHORT nVirtNum, const String& rPgStr);

    // Shell rausreichen
                 SfxShell       *GetCurShell()  { return pShell; }
                 SwDocShell     *GetDocShell();
    inline const SwDocShell     *GetDocShell() const;
    inline       FmFormShell    *GetFormShell() { return pFormShell; }

    //damit in DToren der SubShells ggf. pShell zurueckgesetzt werden kann
    void ResetSubShell()    { pShell = 0; }

    virtual void    WriteUserData(String &, BOOL bBrowse = FALSE );
    virtual void    ReadUserData(const String &, BOOL bBrowse = FALSE );
    virtual void    ReadUserDataSequence ( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse );
    virtual void    WriteUserDataSequence ( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse );

    void SetCrsrAtTop( BOOL bFlag, BOOL bCenter = FALSE )
        { bTopCrsr = bFlag, bCenterCrsr = bCenter; }
    BOOL IsCrsrAtTop() const                    { return bTopCrsr; }
    BOOL IsCrsrAtCenter() const                 { return bCenterCrsr; }

    BOOL JumpToSwMark( const String& rMark );

    long InsertDoc( USHORT nSlotId, const String& rFileName,
                    const String& rFilterName, INT16 nVersion = 0 );

    SwView(SfxViewFrame* pFrame, SfxViewShell*);
    ~SwView();
};

// ----------------- inline Methoden ----------------------

inline int SwView::StatHScrollbar() const
{
    return  0 != pHScrollbar;
}
inline int SwView::CreateHScrollbar()
{
    return StatHScrollbar() ? 1 : _CreateScrollbar( TRUE );
}
inline int SwView::KillHScrollbar()
{
    return StatHScrollbar() ? _KillScrollbar( TRUE ) : 1;
}

inline int SwView::StatVScrollbar() const
{
    return  0 != pVScrollbar;
}
inline int SwView::CreateVScrollbar()
{
    return StatVScrollbar() ? 1 : _CreateScrollbar( FALSE );
}
inline int SwView::KillVScrollbar()
{
    return StatVScrollbar() ? _KillScrollbar( FALSE ) : 1;
}

inline int SwView::CreateVLineal()
{
    return StatVLineal() ? 1 : _CreateVLineal();
}
inline int SwView::KillVLineal()
{
    return StatVLineal() ? _KillVLineal() : 1;
}

inline int SwView::CreateTab()
{
    return StatTab() ? 1 : _CreateTab();
}
inline int  SwView::KillTab()
{
    return StatTab() ? _KillTab() : 1;
}

inline long SwView::GetXScroll() const
{
    return aVisArea.GetWidth() * nScrollX / 100L;
}

inline long SwView::GetYScroll() const
{
    return aVisArea.GetHeight() * nScrollY / 100L;
}

inline const SwDocShell *SwView::GetDocShell() const
{
    return ((SwView*)this)->GetDocShell();
}

//***********************************************************

SfxTabPage* CreatePrintOptionsPage( Window *pParent,
                                    const SfxItemSet &rOptions,
                                    BOOL bPreview);

#endif
