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

#ifndef SC_TABVWSH_HXX
#define SC_TABVWSH_HXX

#include <svtools/htmlcfg.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfac.hxx>
#include <editeng/svxenum.hxx>
#include "scdllapi.h"
#include "dbfunc.hxx"           // -> tabview
#include "target.hxx"
#include "rangelst.hxx"         // ScRangeListRef
#include "shellids.hxx"
#include "tabprotection.hxx" // for ScPasswordHash

class FmFormShell;
class SbxObject;
class SdrOle2Obj;
class SfxBindings;
class SfxChildWindow;
class SfxModelessDialog;
class SvxBorderLine;
class SvxBoxObjectRef;
class SvxNumberInfoItem;
struct SfxChildWinInfo;

class ScArea;
class ScAuditingShell;
class ScDrawShell;
class ScDrawTextObjectBar;
class ScEditShell;
class ScInputHandler;
class ScPivotShell;
class ScDrawFormShell;
class ScCellShell;
class ScOleObjectShell;
class ScGraphicShell;
class ScMediaShell;
class ScChartShell;
class ScPageBreakShell;
class ScDPObject;
class ScNavigatorSettings;

struct ScHeaderFieldData;

namespace com { namespace sun { namespace star { namespace frame {
    class XDispatchProviderInterceptor;
} } } }

namespace svx {
    class ExtrusionBar;
    class FontworkBar;
}

enum ObjectSelectionType
{
    OST_NONE,
    OST_Cell,
    OST_Editing,
    OST_DrawText,
    OST_Drawing,
    OST_DrawForm,
    OST_Pivot,
    OST_Auditing,
    OST_OleObject,
    OST_Chart,
    OST_Graphic,
    OST_Media
};

//==================================================================


class ScTabViewShell: public SfxViewShell, public ScDBFunc
{
private:
    static USHORT           nInsertCtrlState;
    static USHORT           nInsCellsCtrlState;
    static USHORT           nInsObjCtrlState;

    SvxHtmlOptions          aHTMLOpt;
    ObjectSelectionType     eCurOST;
    USHORT                  nDrawSfxId;
    USHORT                  nCtrlSfxId;
    USHORT                  nFormSfxId;
    String                  sDrawCustom;                // current custom shape type
    ScDrawShell*            pDrawShell;
    ScDrawTextObjectBar*    pDrawTextShell;
    ScEditShell*            pEditShell;
    ScPivotShell*           pPivotShell;
    ScAuditingShell*        pAuditingShell;
    ScDrawFormShell*        pDrawFormShell;
    ScCellShell*            pCellShell;
    ScOleObjectShell*       pOleObjectShell;
    ScChartShell*           pChartShell;
    ScGraphicShell*         pGraphicShell;
    ScMediaShell*           pMediaShell;
    ScPageBreakShell*       pPageBreakShell;
    svx::ExtrusionBar*      pExtrusionBarShell;
    svx::FontworkBar*       pFontworkBarShell;

    FmFormShell*            pFormShell;

    ScInputHandler*         pInputHandler;              // fuer OLE-Eingabezeile

    SvxBorderLine*          pCurFrameLine;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProviderInterceptor >
                            xDisProvInterceptor;

    Point                   aWinPos;

    ScTabViewTarget         aTarget;
    ScArea*                 pPivotSource;
    ScDPObject*             pDialogDPObject;

    ScNavigatorSettings*    pNavSettings;

    // used in first Activate
    BOOL                    bFirstActivate;

    BOOL                    bActiveDrawSh;
    BOOL                    bActiveDrawTextSh;
    BOOL                    bActivePivotSh;
    BOOL                    bActiveAuditingSh;
    BOOL                    bActiveDrawFormSh;
    BOOL                    bActiveOleObjectSh;
    BOOL                    bActiveChartSh;
    BOOL                    bActiveGraphicSh;
    BOOL                    bActiveMediaSh;
    BOOL                    bActiveEditSh;

    BOOL                    bFormShellAtTop;            // does the FormShell need to be on top?


    BOOL                    bDontSwitch;                // EditShell nicht abschalten
    BOOL                    bInFormatDialog;            // fuer GetSelectionText
    BOOL                    bPrintSelected;             // for result of SvxPrtQryBox

    BOOL                    bReadOnly;                  // um Status-Aenderungen zu erkennen

    SbxObject*              pScSbxObject;

//UNUSED2008-05  BOOL                    bChartDlgIsEdit;            // Datenbereich aendern
    BOOL                    bChartAreaValid;            // wenn Chart aufgezogen wird
    String                  aEditChartName;
    ScRangeListRef          aChartSource;
    Rectangle               aChartPos;
    SCTAB                   nChartDestTab;
    USHORT                  nCurRefDlgId;

    SfxBroadcaster*         pAccessibilityBroadcaster;

    static const int        MASTERENUMCOMMANDS = 6;
    String                  aCurrShapeEnumCommand[ MASTERENUMCOMMANDS ];

private:
    void    Construct( BYTE nForceDesignMode = SC_FORCEMODE_NONE );

//UNUSED2008-05  void          SetMySubShell( SfxShell* pShell );
    SfxShell*       GetMySubShell() const;

    void            DoReadUserData( const String& rData );
    void          DoReadUserDataSequence( const ::com::sun::star::uno::Sequence<
                                     ::com::sun::star::beans::PropertyValue >& rSettings );

    DECL_LINK( SimpleRefClose, String* );
    DECL_LINK( SimpleRefDone, String* );
    DECL_LINK( SimpleRefAborted, String* );
    DECL_LINK( SimpleRefChange, String* );
    DECL_LINK( FormControlActivated, FmFormShell* );
    DECL_LINK( HtmlOptionsHdl, void * );

protected:
    virtual void    Activate(BOOL bMDI);
    virtual void    Deactivate(BOOL bMDI);
    virtual USHORT  PrepareClose( BOOL bUI = TRUE, BOOL bForBrowsing = FALSE );

    virtual void    ShowCursor(FASTBOOL bOn);

    virtual void    Move();     // Benachrichtigung

    virtual void    AdjustPosSizePixel( const Point &rPos, const Size &rSize );     // alt

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );       // neu
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );

    virtual void    QueryObjAreaPixel( Rectangle& rRect ) const;

    virtual Size    GetOptimalSizePixel() const;

    virtual String  GetSelectionText( BOOL bWholeWord );
    virtual BOOL    HasSelection( BOOL bText ) const;
    virtual String  GetDescription() const;

    virtual void    WriteUserData(String &, BOOL bBrowse = FALSE);
    virtual void    ReadUserData(const String &, BOOL bBrowse = FALSE);
    virtual void    WriteUserDataSequence (::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence (const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

    virtual void    UIDeactivated( SfxInPlaceClient* pClient );

    virtual FASTBOOL KeyInput( const KeyEvent &rKeyEvent );
    virtual SdrView* GetDrawView() const;
    virtual void    EnterStandardMode();

public:
                    TYPEINFO_VISIBILITY(SC_DLLPUBLIC);

                    SFX_DECL_INTERFACE(SCID_TABVIEW_SHELL)
                    SFX_DECL_VIEWFACTORY(ScTabViewShell);


                    // -> Clone-Methode fuer Factory

//UNUSED2008-05  ScTabViewShell( SfxViewFrame*           pViewFrame,
//UNUSED2008-05                  const ScTabViewShell&   rWin );

                    // aus einer allgemeinen Shell konstruieren und
                    // soviel wie moeglich uebernehmen (SliderPos etc.):

                    ScTabViewShell( SfxViewFrame*           pViewFrame,
                                    SfxViewShell*           pOldSh );

    virtual         ~ScTabViewShell();

    Window*         GetDialogParent();

    bool            IsRefInputMode() const;
    void            ExecuteInputDirect();

    ScInputHandler* GetInputHandler() const;
    void            UpdateInputHandler( BOOL bForce = FALSE, BOOL bStopEditing = TRUE );
    void            UpdateInputHandlerCellAdjust( SvxCellHorJustify eJust );
    BOOL            TabKeyInput(const KeyEvent& rKEvt);
    BOOL            SfxKeyInput(const KeyEvent& rKEvt);

    void            SetActive();

    SvxBorderLine*  GetDefaultFrameLine() const { return pCurFrameLine; }
    void            SetDefaultFrameLine(const SvxBorderLine* pLine );

//UNUSED2008-05  void            ExecuteShowNIY( SfxRequest& rReq );
//UNUSED2008-05  void           StateDisabled( SfxItemSet& rSet );

    SC_DLLPUBLIC void           Execute( SfxRequest& rReq );
    SC_DLLPUBLIC void           GetState( SfxItemSet& rSet );

    void            ExecuteTable( SfxRequest& rReq );
    void            GetStateTable( SfxItemSet& rSet );

    void            WindowChanged();
    void            ExecDraw(SfxRequest&);
    void            ExecDrawIns(SfxRequest& rReq);
    void            GetDrawState(SfxItemSet &rSet);
    void            GetDrawInsState(SfxItemSet &rSet);
    void            ExecGallery(SfxRequest& rReq);      // StarGallery
    void            GetGalleryState(SfxItemSet& rSet);

    void            ExecChildWin(SfxRequest& rReq);
    void            GetChildWinState( SfxItemSet& rSet );

    void            ExecImageMap( SfxRequest& rReq );
    void            GetImageMapState( SfxItemSet& rSet );

    void            ExecTbx( SfxRequest& rReq );
    void            GetTbxState( SfxItemSet& rSet );

    void            ExecuteSave( SfxRequest& rReq );
    void            GetSaveState( SfxItemSet& rSet );
    void            ExecSearch( SfxRequest& rReq );

    void            ExecuteUndo(SfxRequest& rReq);
    void            GetUndoState(SfxItemSet &rSet);

    void            ExecuteSbx( SfxRequest& rReq );
    void            GetSbxState( SfxItemSet& rSet );

    void            ExecuteObject(SfxRequest& rReq);
    void            GetObjectState(SfxItemSet &rSet);

    void            ExecDrawOpt(SfxRequest& rReq);
    void            GetDrawOptState(SfxItemSet &rSet);


    void            SetDrawShell( BOOL bActive );
    void            SetDrawTextShell( BOOL bActive );

    void            SetPivotShell( BOOL bActive );
    ScArea*         GetPivotSource(){return pPivotSource;}
    void            SetPivotSource(ScArea* pSrc){pPivotSource=pSrc;}
    void            SetDialogDPObject( const ScDPObject* pObj );
    const ScDPObject* GetDialogDPObject() const { return pDialogDPObject; }

    BOOL            GetDontSwitch(){return bDontSwitch;}
    void            SetDontSwitch(BOOL bFlag){bDontSwitch=bFlag;}


    void            SetAuditShell( BOOL bActive );
    void            SetDrawFormShell( BOOL bActive );
    void            SetEditShell(EditView* pView, BOOL bActive );
    void            SetOleObjectShell( BOOL bActive );
    void            SetChartShell( BOOL bActive );
    void            SetGraphicShell( BOOL bActive );
    void            SetMediaShell( BOOL bActive );


    void            SetDrawShellOrSub();
    void            SetCurSubShell( ObjectSelectionType eOST, BOOL bForce = FALSE );

    void            SetFormShellAtTop( BOOL bSet );

    ObjectSelectionType GetCurObjectSelectionType();

    virtual ErrCode DoVerb(long nVerb);


    void            StopEditShell();
    BOOL            IsDrawTextShell() const;
    BOOL            IsAuditShell() const;

    void            SetDrawTextUndo( ::svl::IUndoManager* pUndoMgr );

    void            FillFieldData( ScHeaderFieldData& rData );

//UNUSED2008-05  void            ResetChartArea();
    void            SetChartArea( const ScRangeListRef& rSource, const Rectangle& rDest );
    BOOL            GetChartArea( ScRangeListRef& rSource, Rectangle& rDest, SCTAB& rTab ) const;

//UNUSED2008-05  BOOL            IsChartDlgEdit() const;
//UNUSED2008-05  void            SetChartDlgEdit(BOOL bFlag){bChartDlgIsEdit=bFlag;}

    void            SetEditChartName(const String& aStr){aEditChartName=aStr;}
//UNUSED2008-05  const String&   GetEditChartName() const;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ScNavigatorSettings*    GetNavigatorSettings();

    // Drucken:
    virtual SfxPrinter*     GetPrinter( BOOL bCreate = FALSE );
    virtual USHORT          SetPrinter( SfxPrinter* pNewPrinter,
                                          USHORT nDiffFlags = SFX_PRINTER_ALL, bool bIsApi=false );

    virtual PrintDialog*    CreatePrintDialog( Window* pParent );
    virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions );
    virtual void            PreparePrint( PrintDialog* pPrintDialog = NULL );
    virtual ErrCode         DoPrint( SfxPrinter *pPrinter,
                                         PrintDialog *pPrintDialog,
                                         BOOL bSilent, BOOL bIsAPI );
    virtual USHORT          Print( SfxProgress& rProgress, BOOL bIsAPI, PrintDialog* pPrintDialog = NULL );

    void            ConnectObject( SdrOle2Obj* pObj );
    BOOL            ActivateObject( SdrOle2Obj* pObj, long nVerb );

    void            DeactivateOle();

    SC_DLLPUBLIC static ScTabViewShell* GetActiveViewShell();
    SfxModelessDialog*  CreateRefDialog( SfxBindings* pB, SfxChildWindow* pCW,
                                        SfxChildWinInfo* pInfo,
                                        Window* pParent, USHORT nSlotId );

    void            UpdateOleZoom();
    inline SbxObject* GetScSbxObject() const
                    { return pScSbxObject; }
    inline void     SetScSbxObject( SbxObject* pOb )
                    { pScSbxObject = pOb; }

    FmFormShell*    GetFormShell() const    { return pFormShell; }

    void    InsertURL( const String& rName, const String& rURL, const String& rTarget,
                            USHORT nMode );
    void    InsertURLButton( const String& rName, const String& rURL, const String& rTarget,
                            const Point* pInsPos = NULL );
    void    InsertURLField( const String& rName, const String& rURL, const String& rTarget );

    BOOL    SelectObject( const String& rName );

    void    SetInFormatDialog(BOOL bFlag) {bInFormatDialog=bFlag;}
    BOOL    IsInFormatDialog() {return bInFormatDialog;}

    void    ForceMove()     { Move(); }

    void    MakeNumberInfoItem      ( ScDocument*            pDoc,
                                      ScViewData*            pViewData,
                                      SvxNumberInfoItem**    ppItem );

    void    UpdateNumberFormatter   ( ScDocument*               pDoc,
                                      const SvxNumberInfoItem&  rInfoItem );

    void    ExecuteCellFormatDlg    ( SfxRequest& rReq, USHORT nTabPage = 0xffff );

    BOOL    GetFunction( String& rFuncStr, sal_uInt16 nErrCode = 0 );

    void    StartSimpleRefDialog( const String& rTitle, const String& rInitVal,
                                    BOOL bCloseOnButtonUp, BOOL bSingleCell, BOOL bMultiSelection );
    void    StopSimpleRefDialog();

    void    SetCurRefDlgId( USHORT nNew );

    void    AddAccessibilityObject( SfxListener& rObject );
    void    RemoveAccessibilityObject( SfxListener& rObject );
    void    BroadcastAccessibility( const SfxHint &rHint );
    BOOL    HasAccessibilityObjects();

    bool    ExecuteRetypePassDlg(ScPasswordHash eDesiredHash);

    using ScTabView::ShowCursor;
};

//==================================================================


#endif

