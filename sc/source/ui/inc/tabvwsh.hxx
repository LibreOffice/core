/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_TABVWSH_HXX
#define SC_TABVWSH_HXX

#include <svtools/htmlcfg.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfac.hxx>
#include <editeng/svxenum.hxx>
#include "scdllapi.h"
#include "dbfunc.hxx"
#include "target.hxx"
#include "rangelst.hxx"
#include "shellids.hxx"
#include "tabprotection.hxx"

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
    OUString                sDrawCustom;                // current custom shape type
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
    bool                    bFirstActivate;

    bool                    bActiveDrawSh;
    bool                    bActiveDrawTextSh;
    bool                    bActivePivotSh;
    bool                    bActiveAuditingSh;
    bool                    bActiveDrawFormSh;
    bool                    bActiveOleObjectSh;
    bool                    bActiveChartSh;
    bool                    bActiveGraphicSh;
    bool                    bActiveMediaSh;
    bool                    bActiveEditSh;

    bool                    bFormShellAtTop;            // does the FormShell need to be on top?


    bool                    bDontSwitch;                // Don't turn off EditShell
    bool                    bInFormatDialog;            // for GetSelectionText
    bool                    bPrintSelected;             // for result of SvxPrtQryBox

    bool                    bReadOnly;                  // to detect status changes

    bool                    bIsActive;

    SbxObject*              pScSbxObject;

    bool                    bChartAreaValid; // if chart is drawn
    bool                    bForceFocusOnCurCell; // #i123629#

    OUString                aEditChartName;
    ScRangeListRef          aChartSource;
    Rectangle               aChartPos;
    SCTAB                   nChartDestTab;
    sal_uInt16              nCurRefDlgId;

    SfxBroadcaster*         pAccessibilityBroadcaster;

    static const int        MASTERENUMCOMMANDS = 6;
    OUString                aCurrShapeEnumCommand[ MASTERENUMCOMMANDS ];

    // ugly hack for Add button in ScNameDlg
    boost::ptr_map<OUString, ScRangeName> maRangeMap;
    bool    mbInSwitch;
    OUString   maName;
    OUString   maScope;
private:
    void    Construct( sal_uInt8 nForceDesignMode = SC_FORCEMODE_NONE );

    SfxShell*       GetMySubShell() const;

    void            DoReadUserData( const OUString& rData );
    void          DoReadUserDataSequence( const ::com::sun::star::uno::Sequence<
                                     ::com::sun::star::beans::PropertyValue >& rSettings );

    DECL_LINK( SimpleRefClose, void* );
    DECL_LINK( SimpleRefDone, OUString* );
    DECL_LINK( SimpleRefAborted, OUString* );
    DECL_LINK( SimpleRefChange, OUString* );
    DECL_LINK( FormControlActivated, void* );

protected:
    virtual void    Activate(bool bMDI);
    virtual void    Deactivate(bool bMDI);
    virtual bool    PrepareClose( sal_Bool bUI = sal_True ) SAL_OVERRIDE;

    virtual void    ShowCursor(bool bOn);

    virtual void    Move();     // Benachrichtigung

    virtual void    AdjustPosSizePixel( const Point &rPos, const Size &rSize );     // alt

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );       // neu
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );

    virtual void    QueryObjAreaPixel( Rectangle& rRect ) const;

    virtual OUString GetSelectionText( bool bWholeWord );
    virtual sal_Bool    HasSelection( sal_Bool bText ) const;
    virtual OUString GetDescription() const;

    virtual void    WriteUserData(OUString &, bool bBrowse = false);
    virtual void    ReadUserData(const OUString &, bool bBrowse = false);
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
    void            UpdateInputHandler( bool bForce = false, bool bStopEditing = true );
    void            UpdateInputHandlerCellAdjust( SvxCellHorJustify eJust );
    bool            TabKeyInput(const KeyEvent& rKEvt);
    bool            SfxKeyInput(const KeyEvent& rKEvt);

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
    void            SetDrawShell( bool bActive );
    void            SetDrawTextShell( bool bActive );

    void            SetPivotShell( bool bActive );
    ScArea*         GetPivotSource(){return pPivotSource;}
    void            SetPivotSource(ScArea* pSrc){pPivotSource=pSrc;}
    void            SetDialogDPObject( const ScDPObject* pObj );
    const ScDPObject* GetDialogDPObject() const { return pDialogDPObject; }

    bool            GetDontSwitch(){return bDontSwitch;}
    void            SetDontSwitch(bool bFlag){bDontSwitch=bFlag;}


    void            SetAuditShell( bool bActive );
    void            SetDrawFormShell( bool bActive );
    void            SetEditShell(EditView* pView, bool bActive );
    void            SetOleObjectShell( bool bActive );
    void            SetChartShell( bool bActive );
    void            SetGraphicShell( bool bActive );
    void            SetMediaShell( bool bActive );


    void            SetDrawShellOrSub();
    void            SetCurSubShell( ObjectSelectionType eOST, bool bForce = false );

    void            SetFormShellAtTop( bool bSet );

    ObjectSelectionType GetCurObjectSelectionType();

    virtual ErrCode DoVerb(long nVerb);


    void            StopEditShell();
    bool            IsDrawTextShell() const;
    bool            IsAuditShell() const;

    void            SetDrawTextUndo( ::svl::IUndoManager* pUndoMgr );

    void            FillFieldData( ScHeaderFieldData& rData );

    void            SetChartArea( const ScRangeListRef& rSource, const Rectangle& rDest );
    bool            GetChartArea( ScRangeListRef& rSource, Rectangle& rDest, SCTAB& rTab ) const;

    void            SetEditChartName(const OUString& aStr){aEditChartName=aStr;}

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ScNavigatorSettings*    GetNavigatorSettings();

    // Drucken:
    virtual SfxPrinter*     GetPrinter( sal_Bool bCreate = false );
    virtual sal_uInt16      SetPrinter( SfxPrinter* pNewPrinter,
                                          sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsApi=false );

    virtual bool            HasPrintOptionsPage() const;
    virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions );

    void            ConnectObject( SdrOle2Obj* pObj );
    bool            ActivateObject( SdrOle2Obj* pObj, long nVerb );

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

    void    InsertURL( const OUString& rName, const OUString& rURL, const OUString& rTarget,
                            sal_uInt16 nMode );
    void    InsertURLButton( const OUString& rName, const OUString& rURL, const OUString& rTarget,
                            const Point* pInsPos = NULL );
    void    InsertURLField( const OUString& rName, const OUString& rURL, const OUString& rTarget );

    bool    SelectObject( const OUString& rName );

    void    SetInFormatDialog(bool bFlag) {bInFormatDialog=bFlag;}
    bool    IsInFormatDialog() {return bInFormatDialog;}

    void    ForceMove()     { Move(); }

    SvxNumberInfoItem* MakeNumberInfoItem( ScDocument* pDoc, ScViewData* pViewData );

    void    UpdateNumberFormatter   ( const SvxNumberInfoItem&  rInfoItem );

    void    ExecuteCellFormatDlg    ( SfxRequest& rReq, const OString &rTabPage = OString());

    bool    GetFunction( OUString& rFuncStr, sal_uInt16 nErrCode = 0 );

    void    StartSimpleRefDialog( const OUString& rTitle, const OUString& rInitVal,
                                    bool bCloseOnButtonUp, bool bSingleCell, bool bMultiSelection );
    void    StopSimpleRefDialog();

    void    SetCurRefDlgId( sal_uInt16 nNew );

    void    AddAccessibilityObject( SfxListener& rObject );
    void    RemoveAccessibilityObject( SfxListener& rObject );
    void    BroadcastAccessibility( const SfxHint &rHint );
    bool    HasAccessibilityObjects();

    bool    ExecuteRetypePassDlg(ScPasswordHash eDesiredHash);

    using ScTabView::ShowCursor;

    bool IsActive() const { return bIsActive; }
    OUString GetFormula(ScAddress& rAddress);

    // ugly hack to call Define Names from Manage Names
    void    SwitchBetweenRefDialogs(SfxModelessDialog* pDialog);
    // #i123629#
    bool    GetForceFocusOnCurCell() const { return bForceFocusOnCurCell; }
    void SetForceFocusOnCurCell(bool bFlag) { bForceFocusOnCurCell=bFlag; }
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
