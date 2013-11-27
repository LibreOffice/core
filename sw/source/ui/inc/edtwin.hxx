/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _EDTWIN_HXX
#define _EDTWIN_HXX

#include <svx/svdobj.hxx>
#include <tools/link.hxx>
#include <vcl/timer.hxx>
#include <vcl/window.hxx>
#include <svtools/transfer.hxx>
#include <swevent.hxx>
#define _SVSTDARR_STRINGSISORTDTOR
#include <svl/svstdarr.hxx>
#include <svx/sdrobjectfactory.hxx>

class   SwWrtShell;
class   SwView;
class   SwRect;
class   ViewShell;
class   SwAnchorMarker;
class   SdrObject;
class   SwShadowCursor;
class   DataChangedEvent;
class   SvxAutoCorrCfg;
class   SvxAutoCorrect;
class   SwPaM;
struct  SwApplyTemplate;
struct  QuickHelpData;
class SdrDropMarkerOverlay;

/*--------------------------------------------------------------------
    Beschreibung:   Eingabe-Fenster
 --------------------------------------------------------------------*/

class SwEditWin: public Window,
                public DropTargetHelper, public DragSourceHelper
{
friend void     ScrollMDI(ViewShell* pVwSh, const SwRect&,
                          sal_uInt16 nRangeX, sal_uInt16 nRangeY);
friend sal_Bool     IsScrollMDI(ViewShell* pVwSh, const SwRect&);

friend void     SizeNotify(ViewShell* pVwSh, const Size &);

friend void     PageNumNotify(  ViewShell* pVwSh,
                                sal_uInt16 nPhyNum,
                                sal_uInt16 nVirtNum,
                                const String& rPg );

    static  QuickHelpData* pQuickHlpData;

    static  sal_Bool    bReplaceQuote;
    static  long    nDDStartPosX, nDDStartPosY;

    static  Color   aTextColor;     //Textfarbe, fuer die Giesskanne
    static  sal_Bool    bTransparentBackColor; // Hintergrund transparent
    static  Color   aTextBackColor; //Texthintergrundfarbe, fuer die Giesskanne

    /*
     * Timer und Handler fuer das Weiterscrollen, wenn der
     * Mauspointer innerhalb eines Drag-Vorgangs ausserhalb des
     * EditWin stehen bleibt.  In regelmaessigen Intervallen wird
     * die Selektion in Richtung der Mausposition vergroessert.
     */
    AutoTimer       aTimer;
    // Timer fuer verschachtelte KeyInputs (z.B. fuer Tabellen)
    Timer           aKeyInputTimer;
    // timer for ANY-KeyInut question without a following KeyInputEvent
    Timer           aKeyInputFlushTimer;

    String          aInBuffer;
    LanguageType    eBufferLanguage;
    Point           aStartPos;
    Point           aMovePos;
    Point           aRszMvHdlPt;
    Timer           aTemplateTimer;

    // Type/Objecte ueber dem der MousePointer steht
    SwCallMouseEvent aSaveCallEvent;

    SwApplyTemplate     *pApplyTempl;
    SwAnchorMarker      *pAnchorMarker; // zum Verschieben eines Ankers

    SdrDropMarkerOverlay    *pUserMarker;
    SdrObject               *pUserMarkerObj;
    SwShadowCursor          *pShadCrsr;
    Point                   *pRowColumnSelectionStart; // save position where table row/column selection has been started

    SwView         &rView;

    int             aActHitType;    // aktueller Mauspointer

    sal_uLong           m_nDropFormat;   //Format aus dem letzten QueryDrop
    sal_uInt16          m_nDropAction;   //Action aus dem letzten QueryDrop
    sal_uInt16          m_nDropDestination;  //Ziel aus dem letzten QueryDrop

    sal_uInt16          eBezierMode;
    sal_uInt16          nInsFrmColCount; //Spaltenzahl fuer interaktiven Rahmen
    SdrObjectCreationInfo meSdrObjectCreationInfo;
    sal_Bool            bLinkRemoved    : 1,
                    bMBPressed      : 1,
                    bInsDraw        : 1,
                    bInsFrm         : 1,
                    bIsInMove       : 1,
                    bIsInDrag       : 1, //StartExecuteDrag nich doppelt ausfuehren
                    bOldIdle        : 1, //Zum abschalten des Idle'ns
                    bOldIdleSet     : 1, //waehrend QeueryDrop
                    bTblInsDelMode  : 1, //
                    bTblIsInsMode   : 1, //
                    bTblIsColMode   : 1, //
                    bChainMode      : 1, //Rahmen verbinden
                    bWasShdwCrsr    : 1, //ShadowCrsr war im MouseButtonDown an
                    bLockInput      : 1, //Lock waehrend die Rechenleiste aktiv ist
                    bIsRowDrag      : 1, //selection of rows is used, in combination with pRowColumnSelectionStart
                    /** #i42732# display status of font size/name depending on either the input language or the
                        selection position depending on what has changed lately
                     */
                    bUseInputLanguage: 1,
                    bObjectSelect   : 1;


    sal_uInt16          nKS_NUMDOWN_Count; // #i23725#
    sal_uInt16          nKS_NUMINDENTINC_Count;

    void            LeaveArea(const Point &);
    void            JustifyAreaTimer();
    inline void     EnterArea();

    void            RstMBDownFlags();

    void            ChangeFly( sal_uInt8 nDir, sal_Bool bWeb = sal_False );
    void            ChangeDrawing( sal_uInt8 nDir );

    sal_Bool            EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos);
    sal_Bool            RulerColumnDrag( const MouseEvent& rMEvt, sal_Bool bVerticalMode);

    //Hilfsfunktionen fuer D&D
    void            DropCleanup();
    void            CleanupDropUserMarker();
    sal_uInt16          GetDropDestination( const Point& rPixPnt,
                                        SdrObject ** ppObj = 0 );
    //select the object/cursor at the mouse position of the context menu request
    sal_Bool            SelectMenuPosition(SwWrtShell& rSh, const Point& rMousePos );

    /*
     * Handler fuer das Weiterscrollen, wenn der Mauspointer innerhalb eines
     * Drag-Vorgangs ausserhalb des EditWin stehen bleibt. In regelmaessigen
     * Intervallen wird die Selektion in Richtung der Mausposition
     * vergroessert.
     */
    DECL_LINK( TimerHandler, Timer * );
    void            StartDDTimer();
    void            StopDDTimer(SwWrtShell *, const Point &);
    DECL_LINK( DDHandler, Timer * );

    // timer for ANY-KeyInut question without a following KeyInputEvent
    DECL_LINK( KeyInputFlushHandler, Timer * );

    // Timer fuer verschachtelte KeyInputs (z.B. fuer Tabellen)
    DECL_LINK( KeyInputTimerHandler, Timer * );

    // Timer fuer das ApplyTemplates per Maus (verkapptes Drag&Drop)
    DECL_LINK( TemplateTimerHdl, Timer* );

    using OutputDevice::GetTextColor;

protected:

    virtual void    DataChanged( const DataChangedEvent& );
    virtual void    PrePaint();
    virtual void    Paint( const Rectangle& rRect );
    virtual void    KeyInput(const KeyEvent &rKEvt);

    virtual void    GetFocus();
    virtual void    LoseFocus();


    virtual void    MouseMove(const MouseEvent& rMEvt);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt);
    virtual void    RequestHelp(const HelpEvent& rEvt);

    virtual void    Command( const CommandEvent& rCEvt );

                                // Drag & Drop Interface
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    virtual XubString GetSurroundingText() const;
    virtual Selection GetSurroundingTextSelection() const;

    void    ShowAutoTextCorrectQuickHelp( const String& rWord, SvxAutoCorrCfg* pACfg,
                                SvxAutoCorrect* pACorr, sal_Bool bFromIME = sal_False );
public:

    void            UpdatePointer(const Point &, sal_uInt16 nButtons = 0);

    sal_Bool            IsDrawSelMode();
    sal_Bool            IsDrawAction()                  { return (bInsDraw); }
    void            SetDrawAction(sal_Bool bFlag)       { bInsDraw = bFlag; }

    void            SetObjectSelect( sal_Bool bVal )    { bObjectSelect = bVal; }
    sal_Bool            IsObjectSelect() const          { return bObjectSelect; }

    inline const SdrObjectCreationInfo& getSdrObjectCreationInfo() const { return meSdrObjectCreationInfo; }
    inline void setSdrObjectCreationInfo(const SdrObjectCreationInfo& eSdrObjectCreationInfo) { meSdrObjectCreationInfo = eSdrObjectCreationInfo; SetObjectSelect( sal_False ); }
    void                StdDrawMode( SdrObjKind eSdrObjectKind, sal_Bool bObjSelect );

    sal_Bool            IsFrmAction()                   { return (bInsFrm); }
    inline sal_uInt16   GetBezierMode()                 { return eBezierMode; }
    void            SetBezierMode(sal_uInt16 eBezMode)  { eBezierMode = eBezMode; }
    void            EnterDrawTextMode(const basegfx::B2DPoint& aDocPos); // DrawTextEditMode einschalten
    void            InsFrm(sal_uInt16 nCols);
    void            StopInsFrm();
    sal_uInt16          GetFrmColCount() const {return nInsFrmColCount;} //Spaltenzahl fuer interaktiven Rahmen


    void            SetChainMode( sal_Bool bOn );
    sal_Bool            IsChainMode() const             { return bChainMode; }

    void            FlushInBuffer();
    sal_Bool            IsInputSequenceCheckingRequired( const String &rText, const SwPaM& rCrsr ) const;


    static  void    SetReplaceQuote(sal_Bool bOn = sal_True) { bReplaceQuote = bOn; }
    static  sal_Bool    IsReplaceQuote() { return bReplaceQuote; }

    void             SetApplyTemplate(const SwApplyTemplate &);
    SwApplyTemplate* GetApplyTemplate() const { return pApplyTempl; }

    void            StartExecuteDrag();
    void            DragFinished();
    sal_uInt16          GetDropAction() const { return m_nDropAction; }
    sal_uLong           GetDropFormat() const { return m_nDropFormat; }

    Color           GetTextColor() { return aTextColor; }

    void            SetTextColor(const Color& rCol ) { aTextColor = rCol; }

    Color           GetTextBackColor()
                                            { return aTextBackColor; }
    void            SetTextBackColor(const Color& rCol )
                                            { aTextBackColor = rCol; }
    void            SetTextBackColorTransparent(sal_Bool bSet)
                                    { bTransparentBackColor = bSet; }
    sal_Bool            IsTextBackColorTransparent()
                                    { return bTransparentBackColor; }
    void            LockKeyInput(sal_Bool bSet){bLockInput = bSet;}

    const SwView &GetView() const { return rView; }
          SwView &GetView()       { return rView; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    // Tipfenster loeschen
    static void ClearTip();

    static inline long GetDDStartPosX() { return nDDStartPosX; }
    static inline long GetDDStartPosY() { return nDDStartPosY; }

    static void _InitStaticData();
    static void _FinitStaticData();

    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    void StopQuickHelp();

    // --> OD 2005-02-18 #i42921# - add parameter <bVerticalMode>
    sal_Bool RulerMarginDrag( const MouseEvent& rMEvt,
                                     const bool bVerticalMode );
    // <--

    /** #i42732# display status of font size/name depending on either the input
        language or the selection position depending on what has changed lately
     */
    void        SetUseInputLanguage( sal_Bool bNew );
    sal_Bool    IsUseInputLanguage() const { return bUseInputLanguage; }

    SwEditWin(Window *pParent, SwView &);
    virtual ~SwEditWin();
    //IAccessibility2 Implementation 2009-----
    virtual void SwitchView();
    //-----IAccessibility2 Implementation 2009
};

#endif

