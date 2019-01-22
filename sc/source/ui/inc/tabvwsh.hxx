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

#include <formula/errorcodes.hxx>
#include <svx/fmshell.hxx>
#include <sfx2/viewsh.hxx>
#include <editeng/svxenum.hxx>
#include <o3tl/deleter.hxx>
#include <scdllapi.h>
#include "dbfunc.hxx"
#include "target.hxx"
#include <shellids.hxx>
#include <tabprotection.hxx>

#include <memory>
#include <map>

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
    std::unique_ptr<ScDrawShell>         pDrawShell;
    std::unique_ptr<ScDrawTextObjectBar> pDrawTextShell;
    std::unique_ptr<ScEditShell>         pEditShell;
    std::unique_ptr<ScPivotShell>        pPivotShell;
    std::unique_ptr<ScAuditingShell>     pAuditingShell;
    std::unique_ptr<ScDrawFormShell>     pDrawFormShell;
    std::unique_ptr<ScCellShell>         pCellShell;
    std::unique_ptr<ScOleObjectShell>    pOleObjectShell;
    std::unique_ptr<ScChartShell>        pChartShell;
    std::unique_ptr<ScGraphicShell>      pGraphicShell;
    std::unique_ptr<ScMediaShell>        pMediaShell;
    std::unique_ptr<ScPageBreakShell>    pPageBreakShell;
    std::unique_ptr<svx::ExtrusionBar>   pExtrusionBarShell;
    std::unique_ptr<svx::FontworkBar>    pFontworkBarShell;

    std::unique_ptr<FmFormShell> pFormShell;

    std::unique_ptr<ScInputHandler, o3tl::default_delete<ScInputHandler>> mpInputHandler;              // for OLE input cell

    std::unique_ptr<::editeng::SvxBorderLine> pCurFrameLine;

    css::uno::Reference< css::frame::XDispatchProviderInterceptor >
                            xDisProvInterceptor;

    Point                   aWinPos;

    ScTabViewTarget         aTarget;
    std::unique_ptr<ScArea>     pPivotSource;
    std::unique_ptr<ScDPObject> pDialogDPObject;

    std::unique_ptr<ScNavigatorSettings> pNavSettings;

    // used in first Activate
    bool                    bFirstActivate;

    bool                    bActiveDrawSh;
    bool                    bActiveDrawTextSh;
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

    bool                    bInPrepareClose;
    bool                    bInDispose;

    sal_uInt16              nCurRefDlgId;

    std::unique_ptr<SfxBroadcaster> pAccessibilityBroadcaster;

    // ugly hack for Add button in ScNameDlg
    std::map<OUString, std::unique_ptr<ScRangeName>> m_RangeMap;
    bool    mbInSwitch;
    OUString   maName;
    OUString   maScope;

private:
    void    Construct( TriState nForceDesignMode );

    SfxShell*       GetMySubShell() const;

    void            DoReadUserData( const OUString& rData );
    void            DoReadUserDataSequence( const css::uno::Sequence< css::beans::PropertyValue >& rSettings );
    bool            IsSignatureLineSelected();
    bool            IsSignatureLineSigned();

    DECL_LINK( SimpleRefClose, const OUString*, void );
    DECL_LINK( SimpleRefDone, const OUString&, void );
    DECL_LINK( SimpleRefAborted, const OUString&, void );
    DECL_LINK( SimpleRefChange, const OUString&, void );
    DECL_LINK( FormControlActivated, LinkParamNone*, void );

protected:
    virtual void    Activate(bool bMDI) override;
    virtual void    Deactivate(bool bMDI) override;
    virtual bool    PrepareClose( bool bUI = true ) override;

    virtual void    ShowCursor(bool bOn) override;

    virtual void    Move() override;     // notification

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize, bool inplaceEditModeChange ) override; // new
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize ) override;
    virtual void    SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY ) override;

    virtual void    QueryObjAreaPixel( tools::Rectangle& rRect ) const override;

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

    virtual         ~ScTabViewShell() override;

    vcl::Window* GetDialogParent();

    bool            IsRefInputMode() const;
    void            ExecuteInputDirect();

    const ScInputHandler* GetInputHandler() const { return mpInputHandler.get(); }
    ScInputHandler* GetInputHandler() { return mpInputHandler.get(); }
    void            UpdateInputHandler( bool bForce = false, bool bStopEditing = true );
    void            UpdateInputHandlerCellAdjust( SvxCellHorJustify eJust );
    bool            TabKeyInput(const KeyEvent& rKEvt);
    bool            SfxKeyInput(const KeyEvent& rKEvt);

    void            SetActive();

    ::editeng::SvxBorderLine*   GetDefaultFrameLine() const { return pCurFrameLine.get(); }
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
    void            ExecGallery(const SfxRequest& rReq);

    void            ExecChildWin(const SfxRequest& rReq);

    void            ExecImageMap( SfxRequest& rReq );
    void            GetImageMapState( SfxItemSet& rSet );

    void            ExecuteSave( SfxRequest& rReq );
    void            GetSaveState( SfxItemSet& rSet );
    void            ExecSearch( SfxRequest& rReq );

    void            ExecuteUndo(SfxRequest& rReq);
    void            GetUndoState(SfxItemSet &rSet);

    void            ExecuteObject(const SfxRequest& rReq);
    void            GetObjectState(SfxItemSet &rSet);

    void            ExecDrawOpt(const SfxRequest& rReq);
    void            GetDrawOptState(SfxItemSet &rSet);

    void            UpdateDrawShell();
    void            SetDrawShell( bool bActive );
    void            SetDrawTextShell( bool bActive );

    void            SetPivotShell( bool bActive );
    void            SetDialogDPObject( std::unique_ptr<ScDPObject> pObj );
    const ScDPObject* GetDialogDPObject() const { return pDialogDPObject.get(); }

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

    void            SetDrawTextUndo( SfxUndoManager* pUndoMgr );

    void            FillFieldData( ScHeaderFieldData& rData );

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ScNavigatorSettings*    GetNavigatorSettings();

    // Drucken:
    virtual SfxPrinter*     GetPrinter( bool bCreate = false ) override;
    virtual sal_uInt16      SetPrinter( SfxPrinter* pNewPrinter,
                                          SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL ) override;

    virtual bool            HasPrintOptionsPage() const override;
    virtual VclPtr<SfxTabPage> CreatePrintOptionsPage(TabPageParent pParent, const SfxItemSet &rOptions) override;

    void            ConnectObject( const SdrOle2Obj* pObj );
    void            ActivateObject( SdrOle2Obj* pObj, long nVerb );

    void            DeactivateOle();

    static ScTabViewShell* GetActiveViewShell();
    VclPtr<SfxModelessDialog> CreateRefDialog( SfxBindings* pB, SfxChildWindow* pCW,
                                               const SfxChildWinInfo* pInfo,
                                               vcl::Window* pParent, sal_uInt16 nSlotId );

    void            UpdateOleZoom();

    virtual const FmFormShell* GetFormShell() const override { return pFormShell.get(); }
    virtual       FmFormShell* GetFormShell()       override { return pFormShell.get(); }

    void    InsertURL( const OUString& rName, const OUString& rURL, const OUString& rTarget,
                            sal_uInt16 nMode );
    void    InsertURLButton( const OUString& rName, const OUString& rURL, const OUString& rTarget,
                            const Point* pInsPos );
    void    InsertURLField( const OUString& rName, const OUString& rURL, const OUString& rTarget );

    bool    SelectObject( const OUString& rName );

    void    SetInFormatDialog(bool bFlag) {bInFormatDialog=bFlag;}

    void    ForceMove()     { Move(); }

    static SvxNumberInfoItem* MakeNumberInfoItem( ScDocument* pDoc, const ScViewData* pViewData );

    static void UpdateNumberFormatter( const SvxNumberInfoItem&  rInfoItem );

    void    ExecuteCellFormatDlg( SfxRequest& rReq, const OString &rTabPage);

    bool    GetFunction( OUString& rFuncStr, FormulaError nErrCode );

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
    OUString GetFormula(const ScAddress& rAddress);
    bool    UseSubTotal(ScRangeList* pRangeList);
    const   OUString DoAutoSum(bool& rRangeFinder, bool& rSubTotal);

    // ugly hack to call Define Names from Manage Names
    void    SwitchBetweenRefDialogs(SfxModelessDialog* pDialog);
    // #i123629#
    bool    GetForceFocusOnCurCell() const { return bForceFocusOnCurCell; }
    void SetForceFocusOnCurCell(bool bFlag) { bForceFocusOnCurCell=bFlag; }
    /// See SfxViewShell::getPart().
    int getPart() const override;
    /// See SfxViewShell::afterCallbackRegistered().
    void afterCallbackRegistered() override;
    /// See SfxViewShell::NotifyCursor().
    void NotifyCursor(SfxViewShell* pViewShell) const override;
    /// Emits a LOK_CALLBACK_INVALIDATE_HEADER for all views whose current tab is equal to nCurrentTabIndex
    static void notifyAllViewsHeaderInvalidation(HeaderType eHeaderType, SCTAB nCurrentTabIndex);
    static void notifyAllViewsHeaderInvalidation(bool Columns, SCTAB nCurrentTabIndex);
    static bool isAnyEditViewInRange(bool bColumns, SCCOLROW nStart, SCCOLROW nEnd);
    css::uno::Reference<css::drawing::XShapes> getSelectedXShapes();
    static  css::uno::Reference<css::datatransfer::XTransferable2> GetClipData(vcl::Window* pWin);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
