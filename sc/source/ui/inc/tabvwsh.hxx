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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TABVWSH_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TABVWSH_HXX

#include <svx/fmshell.hxx>
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

#include <memory>
#include <map>

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

class SC_DLLPUBLIC ScTabViewShell: public SfxViewShell, public ScDBFunc
{
private:
    ObjectSelectionType     eCurOST;
    sal_uInt16              nDrawSfxId;
    sal_uInt16              nFormSfxId;
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

    css::uno::Reference< css::frame::XDispatchProviderInterceptor >
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

    bool                    bReadOnly;                  // to detect status changes

    bool                    bIsActive;

    bool                    bForceFocusOnCurCell; // #i123629#

    ScRangeListRef          aChartSource;
    Rectangle               aChartPos;
    SCTAB                   nChartDestTab;
    sal_uInt16              nCurRefDlgId;

    SfxBroadcaster*         pAccessibilityBroadcaster;

    // ugly hack for Add button in ScNameDlg
    std::map<OUString, std::unique_ptr<ScRangeName>> m_RangeMap;
    bool    mbInSwitch;
    OUString   maName;
    OUString   maScope;

private:
    void    Construct( TriState nForceDesignMode = TRISTATE_INDET );

    SfxShell*       GetMySubShell() const;

    void            DoReadUserData( const OUString& rData );
    void            DoReadUserDataSequence( const css::uno::Sequence< css::beans::PropertyValue >& rSettings );

    DECL_LINK_TYPED( SimpleRefClose, const OUString*, void );
    DECL_LINK_TYPED( SimpleRefDone, const OUString&, void );
    DECL_LINK_TYPED( SimpleRefAborted, const OUString&, void );
    DECL_LINK_TYPED( SimpleRefChange, const OUString&, void );
    DECL_LINK_TYPED( FormControlActivated, LinkParamNone*, void );

protected:
    virtual void    Activate(bool bMDI) override;
    virtual void    Deactivate(bool bMDI) override;
    virtual bool    PrepareClose( bool bUI = true ) override;

    virtual void    ShowCursor(bool bOn) override;

    virtual void    Move() override;     // Benachrichtigung

    virtual void    AdjustPosSizePixel( const Point &rPos, const Size &rSize ) override;     // alt

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize ) override;       // neu
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize ) override;
    virtual void    SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY ) override;

    virtual void    QueryObjAreaPixel( Rectangle& rRect ) const override;

    virtual OUString GetSelectionText( bool bWholeWord = false ) override;
    virtual bool     HasSelection( bool bText = true ) const override;

    virtual void    WriteUserData(OUString &, bool bBrowse = false) override;
    virtual void    ReadUserData(const OUString &, bool bBrowse = false) override;
    virtual void    WriteUserDataSequence (css::uno::Sequence < css::beans::PropertyValue >& ) override;
    virtual void    ReadUserDataSequence (const css::uno::Sequence < css::beans::PropertyValue >& ) override;

    virtual void    UIDeactivated( SfxInPlaceClient* pClient ) override;

    virtual bool    KeyInput( const KeyEvent &rKeyEvent ) override;
    virtual SdrView* GetDrawView() const override;

public:
                    SFX_DECL_INTERFACE(SCID_TABVIEW_SHELL)
                    SFX_DECL_VIEWFACTORY(ScTabViewShell);

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
                    /** -> Clone Method for Factory
                        Created from a general shell and inherit as much as possible */
                    ScTabViewShell( SfxViewFrame*           pViewFrame,
                                    SfxViewShell*           pOldSh );

    virtual         ~ScTabViewShell();

    vcl::Window* GetDialogParent();

    bool            IsRefInputMode() const;
    void            ExecuteInputDirect();

    ScInputHandler* GetInputHandler() const { return pInputHandler;}
    void            UpdateInputHandler( bool bForce = false, bool bStopEditing = true );
    void            UpdateInputHandlerCellAdjust( SvxCellHorJustify eJust );
    bool            TabKeyInput(const KeyEvent& rKEvt);
    bool            SfxKeyInput(const KeyEvent& rKEvt);

    void            SetActive();

    ::editeng::SvxBorderLine*   GetDefaultFrameLine() const { return pCurFrameLine; }
    void            SetDefaultFrameLine(const ::editeng::SvxBorderLine* pLine );

    void           Execute( SfxRequest& rReq );
    void           GetState( SfxItemSet& rSet );

    void            ExecuteTable( SfxRequest& rReq );
    void            GetStateTable( SfxItemSet& rSet );

    void            WindowChanged();
    void            ExecDraw(SfxRequest&);
    void            ExecDrawIns(SfxRequest& rReq);
    void            GetDrawState(SfxItemSet &rSet);
    void            GetDrawInsState(SfxItemSet &rSet);
    void            ExecGallery(SfxRequest& rReq);

    void            ExecChildWin(SfxRequest& rReq);

    void            ExecImageMap( SfxRequest& rReq );
    void            GetImageMapState( SfxItemSet& rSet );

    void            ExecuteSave( SfxRequest& rReq );
    void            GetSaveState( SfxItemSet& rSet );
    void            ExecSearch( SfxRequest& rReq );

    void            ExecuteUndo(SfxRequest& rReq);
    void            GetUndoState(SfxItemSet &rSet);

    static void     ExecuteSbx( SfxRequest& rReq );
    static void     GetSbxState( SfxItemSet& rSet );

    void            ExecuteObject(SfxRequest& rReq);
    void            GetObjectState(SfxItemSet &rSet);

    void            ExecDrawOpt(SfxRequest& rReq);
    void            GetDrawOptState(SfxItemSet &rSet);

    void            UpdateDrawShell();
    void            SetDrawShell( bool bActive );
    void            SetDrawTextShell( bool bActive );

    void            SetPivotShell( bool bActive );
    void            SetDialogDPObject( const ScDPObject* pObj );
    const ScDPObject* GetDialogDPObject() const { return pDialogDPObject; }

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

    ObjectSelectionType GetCurObjectSelectionType() { return eCurOST; }

    virtual ErrCode DoVerb(long nVerb) override;

    void            StopEditShell();
    bool            IsDrawTextShell() const;
    bool            IsAuditShell() const;

    void            SetDrawTextUndo( ::svl::IUndoManager* pUndoMgr );

    void            FillFieldData( ScHeaderFieldData& rData );

    bool            GetChartArea( ScRangeListRef& rSource, Rectangle& rDest, SCTAB& rTab ) const;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ScNavigatorSettings*    GetNavigatorSettings();

    // Drucken:
    virtual SfxPrinter*     GetPrinter( bool bCreate = false ) override;
    virtual sal_uInt16      SetPrinter( SfxPrinter* pNewPrinter,
                                          SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL ) override;

    virtual bool            HasPrintOptionsPage() const override;
    virtual VclPtr<SfxTabPage> CreatePrintOptionsPage( vcl::Window *pParent, const SfxItemSet &rOptions ) override;

    void            ConnectObject( SdrOle2Obj* pObj );
    void            ActivateObject( SdrOle2Obj* pObj, long nVerb );

    void            DeactivateOle();

    static ScTabViewShell* GetActiveViewShell();
    VclPtr<SfxModelessDialog> CreateRefDialog( SfxBindings* pB, SfxChildWindow* pCW,
                                               SfxChildWinInfo* pInfo,
                                               vcl::Window* pParent, sal_uInt16 nSlotId );

    void            UpdateOleZoom();

    virtual const FmFormShell* GetFormShell() const override { return pFormShell; }
    virtual       FmFormShell* GetFormShell()       override { return pFormShell; }

    void    InsertURL( const OUString& rName, const OUString& rURL, const OUString& rTarget,
                            sal_uInt16 nMode );
    void    InsertURLButton( const OUString& rName, const OUString& rURL, const OUString& rTarget,
                            const Point* pInsPos = nullptr );
    void    InsertURLField( const OUString& rName, const OUString& rURL, const OUString& rTarget );

    bool    SelectObject( const OUString& rName );

    void    SetInFormatDialog(bool bFlag) {bInFormatDialog=bFlag;}

    void    ForceMove()     { Move(); }

    static SvxNumberInfoItem* MakeNumberInfoItem( ScDocument* pDoc, ScViewData* pViewData );

    static void UpdateNumberFormatter( const SvxNumberInfoItem&  rInfoItem );

    void    ExecuteCellFormatDlg( SfxRequest& rReq, const OString &rTabPage = OString());

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
    /// See SfxViewShell::getPart().
    int getPart() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
