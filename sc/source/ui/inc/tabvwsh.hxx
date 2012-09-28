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

#include <boost/ptr_container/ptr_map.hpp>

class FmFormShell;
class SbxObject;
class SdrOle2Obj;
class SfxBindings;
class SfxChildWindow;
class SfxModelessDialog;
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
class ScRangeName;

struct ScHeaderFieldData;

namespace editeng { class SvxBorderLine; }

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
    static sal_uInt16           nInsertCtrlState;
    static sal_uInt16           nInsCellsCtrlState;
    static sal_uInt16           nInsObjCtrlState;

    SvxHtmlOptions          aHTMLOpt;
    ObjectSelectionType     eCurOST;
    sal_uInt16                  nDrawSfxId;
    sal_uInt16                  nCtrlSfxId;
    sal_uInt16                  nFormSfxId;
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

    ScInputHandler*         pInputHandler;              // for OLE input cell

    ::editeng::SvxBorderLine*           pCurFrameLine;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProviderInterceptor >
                            xDisProvInterceptor;

    Point                   aWinPos;

    ScTabViewTarget         aTarget;
    ScArea*                 pPivotSource;
    ScDPObject*             pDialogDPObject;

    ScNavigatorSettings*    pNavSettings;

    // used in first Activate
    sal_Bool                    bFirstActivate;

    sal_Bool                    bActiveDrawSh;
    sal_Bool                    bActiveDrawTextSh;
    sal_Bool                    bActivePivotSh;
    sal_Bool                    bActiveAuditingSh;
    sal_Bool                    bActiveDrawFormSh;
    sal_Bool                    bActiveOleObjectSh;
    sal_Bool                    bActiveChartSh;
    sal_Bool                    bActiveGraphicSh;
    sal_Bool                    bActiveMediaSh;
    sal_Bool                    bActiveEditSh;

    sal_Bool                    bFormShellAtTop;            // does the FormShell need to be on top?


    sal_Bool                    bDontSwitch;                // Don't turn off EditShell
    sal_Bool                    bInFormatDialog;            // for GetSelectionText
    sal_Bool                    bPrintSelected;             // for result of SvxPrtQryBox

    sal_Bool                    bReadOnly;                  // to detect status changes

    SbxObject*              pScSbxObject;

    sal_Bool                    bChartAreaValid;            // if chart is drawn
    String                  aEditChartName;
    ScRangeListRef          aChartSource;
    Rectangle               aChartPos;
    SCTAB                   nChartDestTab;
    sal_uInt16                  nCurRefDlgId;

    SfxBroadcaster*         pAccessibilityBroadcaster;

    static const int        MASTERENUMCOMMANDS = 6;
    String                  aCurrShapeEnumCommand[ MASTERENUMCOMMANDS ];

    // ugly hack for Add button in ScNameDlg
    boost::ptr_map<rtl::OUString, ScRangeName> maRangeMap;
    bool    mbInSwitch;
    rtl::OUString   maName;
    rtl::OUString   maScope;

private:
    void    Construct( sal_uInt8 nForceDesignMode = SC_FORCEMODE_NONE );

    SfxShell*       GetMySubShell() const;

    void            DoReadUserData( const String& rData );
    void          DoReadUserDataSequence( const ::com::sun::star::uno::Sequence<
                                     ::com::sun::star::beans::PropertyValue >& rSettings );

    DECL_LINK( SimpleRefClose, void* );
    DECL_LINK( SimpleRefDone, String* );
    DECL_LINK( SimpleRefAborted, String* );
    DECL_LINK( SimpleRefChange, String* );
    DECL_LINK( FormControlActivated, void* );

protected:
    virtual void    Activate(sal_Bool bMDI);
    virtual void    Deactivate(sal_Bool bMDI);
    virtual sal_uInt16  PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = false );

    virtual void    ShowCursor(bool bOn);

    virtual void    Move();     // Benachrichtigung

    virtual void    AdjustPosSizePixel( const Point &rPos, const Size &rSize );     // alt

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );       // neu
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );

    virtual void    QueryObjAreaPixel( Rectangle& rRect ) const;

    virtual Size    GetOptimalSizePixel() const;

    virtual String  GetSelectionText( sal_Bool bWholeWord );
    virtual sal_Bool    HasSelection( sal_Bool bText ) const;
    virtual String  GetDescription() const;

    virtual void    WriteUserData(String &, sal_Bool bBrowse = false);
    virtual void    ReadUserData(const String &, sal_Bool bBrowse = false);
    virtual void    WriteUserDataSequence (::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = false );
    virtual void    ReadUserDataSequence (const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = false );

    virtual void    UIDeactivated( SfxInPlaceClient* pClient );

    virtual bool    KeyInput( const KeyEvent &rKeyEvent );
    virtual SdrView* GetDrawView() const;

public:
                    TYPEINFO_VISIBILITY(SC_DLLPUBLIC);

                    SFX_DECL_INTERFACE(SCID_TABVIEW_SHELL)
                    SFX_DECL_VIEWFACTORY(ScTabViewShell);


                    /** -> Clone Method for Factory
                        Created from a general shell and inherit as much as possible */
                    ScTabViewShell( SfxViewFrame*           pViewFrame,
                                    SfxViewShell*           pOldSh );

    virtual         ~ScTabViewShell();

    SC_DLLPUBLIC Window* GetDialogParent();

    bool            IsRefInputMode() const;
    void            ExecuteInputDirect();

    ScInputHandler* GetInputHandler() const;
    void            UpdateInputHandler( sal_Bool bForce = false, sal_Bool bStopEditing = sal_True );
    void            UpdateInputHandlerCellAdjust( SvxCellHorJustify eJust );
    sal_Bool            TabKeyInput(const KeyEvent& rKEvt);
    sal_Bool            SfxKeyInput(const KeyEvent& rKEvt);

    void            SetActive();

    ::editeng::SvxBorderLine*   GetDefaultFrameLine() const { return pCurFrameLine; }
    void            SetDefaultFrameLine(const ::editeng::SvxBorderLine* pLine );

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

    void            UpdateDrawShell();
    void            SetDrawShell( sal_Bool bActive );
    void            SetDrawTextShell( sal_Bool bActive );

    void            SetPivotShell( sal_Bool bActive );
    ScArea*         GetPivotSource(){return pPivotSource;}
    void            SetPivotSource(ScArea* pSrc){pPivotSource=pSrc;}
    void            SetDialogDPObject( const ScDPObject* pObj );
    const ScDPObject* GetDialogDPObject() const { return pDialogDPObject; }

    sal_Bool            GetDontSwitch(){return bDontSwitch;}
    void            SetDontSwitch(sal_Bool bFlag){bDontSwitch=bFlag;}


    void            SetAuditShell( sal_Bool bActive );
    void            SetDrawFormShell( sal_Bool bActive );
    void            SetEditShell(EditView* pView, sal_Bool bActive );
    void            SetOleObjectShell( sal_Bool bActive );
    void            SetChartShell( sal_Bool bActive );
    void            SetGraphicShell( sal_Bool bActive );
    void            SetMediaShell( sal_Bool bActive );


    void            SetDrawShellOrSub();
    void            SetCurSubShell( ObjectSelectionType eOST, sal_Bool bForce = false );

    void            SetFormShellAtTop( sal_Bool bSet );

    ObjectSelectionType GetCurObjectSelectionType();

    virtual ErrCode DoVerb(long nVerb);


    void            StopEditShell();
    sal_Bool            IsDrawTextShell() const;
    sal_Bool            IsAuditShell() const;

    void            SetDrawTextUndo( ::svl::IUndoManager* pUndoMgr );

    void            FillFieldData( ScHeaderFieldData& rData );

    void            SetChartArea( const ScRangeListRef& rSource, const Rectangle& rDest );
    sal_Bool            GetChartArea( ScRangeListRef& rSource, Rectangle& rDest, SCTAB& rTab ) const;

    void            SetEditChartName(const String& aStr){aEditChartName=aStr;}

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ScNavigatorSettings*    GetNavigatorSettings();

    // Drucken:
    virtual SfxPrinter*     GetPrinter( sal_Bool bCreate = false );
    virtual sal_uInt16          SetPrinter( SfxPrinter* pNewPrinter,
                                          sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsApi=false );

    virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions );

    void            ConnectObject( SdrOle2Obj* pObj );
    sal_Bool            ActivateObject( SdrOle2Obj* pObj, long nVerb );

    void            DeactivateOle();

    SC_DLLPUBLIC static ScTabViewShell* GetActiveViewShell();
    SfxModelessDialog*  CreateRefDialog( SfxBindings* pB, SfxChildWindow* pCW,
                                        SfxChildWinInfo* pInfo,
                                        Window* pParent, sal_uInt16 nSlotId );

    void            UpdateOleZoom();
    inline SbxObject* GetScSbxObject() const
                    { return pScSbxObject; }
    inline void     SetScSbxObject( SbxObject* pOb )
                    { pScSbxObject = pOb; }

    FmFormShell*    GetFormShell() const    { return pFormShell; }

    void    InsertURL( const String& rName, const String& rURL, const String& rTarget,
                            sal_uInt16 nMode );
    void    InsertURLButton( const String& rName, const String& rURL, const String& rTarget,
                            const Point* pInsPos = NULL );
    void    InsertURLField( const String& rName, const String& rURL, const String& rTarget );

    sal_Bool    SelectObject( const String& rName );

    void    SetInFormatDialog(sal_Bool bFlag) {bInFormatDialog=bFlag;}
    sal_Bool    IsInFormatDialog() {return bInFormatDialog;}

    void    ForceMove()     { Move(); }

    void    MakeNumberInfoItem      ( ScDocument*            pDoc,
                                      ScViewData*            pViewData,
                                      SvxNumberInfoItem**    ppItem );

    void    UpdateNumberFormatter   ( const SvxNumberInfoItem&  rInfoItem );

    void    ExecuteCellFormatDlg    ( SfxRequest& rReq, sal_uInt16 nTabPage = 0xffff );

    sal_Bool    GetFunction( String& rFuncStr, sal_uInt16 nErrCode = 0 );

    void    StartSimpleRefDialog( const String& rTitle, const String& rInitVal,
                                    sal_Bool bCloseOnButtonUp, sal_Bool bSingleCell, sal_Bool bMultiSelection );
    void    StopSimpleRefDialog();

    void    SetCurRefDlgId( sal_uInt16 nNew );

    void    AddAccessibilityObject( SfxListener& rObject );
    void    RemoveAccessibilityObject( SfxListener& rObject );
    void    BroadcastAccessibility( const SfxHint &rHint );
    sal_Bool    HasAccessibilityObjects();

    bool    ExecuteRetypePassDlg(ScPasswordHash eDesiredHash);

    using ScTabView::ShowCursor;

    // ugly hack to call Define Names from Manage Names
    void    SwitchBetweenRefDialogs(SfxModelessDialog* pDialog);
};

//==================================================================


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
