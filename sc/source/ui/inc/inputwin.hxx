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

#ifndef INCLUDED_SC_SOURCE_UI_INC_INPUTWIN_HXX
#define INCLUDED_SC_SOURCE_UI_INC_INPUTWIN_HXX

#include <vector>
#include <vcl/toolbox.hxx>
#include <sfx2/childwin.hxx>
#include <svl/lstner.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/window.hxx>
#include <svtools/transfer.hxx>

class ScEditEngineDefaulter;
class EditView;
struct ESelection;
class ScInputHandler;
class ScAccessibleEditLineTextData;
struct EENotify;
class ScRangeList;
class ScTabViewShell;
class Accelerator;

class ScTextWndBase : public vcl::Window
{
public:
    ScTextWndBase( vcl::Window* pParent,  WinBits nStyle );
    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) = 0;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) = 0;
    virtual void            SetTextString( const OUString& rString ) = 0;
    virtual const OUString& GetTextString() const = 0;
    virtual void            StartEditEngine() = 0;
    virtual void            StopEditEngine( bool bAll ) = 0;
    virtual EditView*       GetEditView() = 0;
    virtual void            MakeDialogEditView() = 0;
    virtual void            SetFormulaMode( bool bSet ) = 0;
    virtual bool            IsInputActive() = 0;
    virtual void            TextGrabFocus() = 0;
};

class ScTextWnd : public ScTextWndBase, public DragSourceHelper     // edit window
{
public:
                    ScTextWnd( vcl::Window* pParent, ScTabViewShell* pViewSh );
    virtual         ~ScTextWnd();
    virtual void    dispose() override;

    virtual void            SetTextString( const OUString& rString ) override;
    virtual const OUString& GetTextString() const override;

    bool                     IsInputActive() override;
    virtual EditView*       GetEditView() override;

                        // for function autopilots
    virtual void            MakeDialogEditView() override;

    virtual void            StartEditEngine() override;
    virtual void            StopEditEngine( bool bAll ) override;

    virtual void            TextGrabFocus() override;

    virtual void             DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void            SetFormulaMode( bool bSet ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() override;

    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;

    DECL_LINK_TYPED( NotifyHdl, LinkParamNone*, void );

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    Resize() override;

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    KeyInput(const KeyEvent& rKEvt) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

    virtual OUString  GetText() const override;

    void            ImplInitSettings();
    void            UpdateAutoCorrFlag();

    ScTabViewShell* GetViewShell() { return mpViewShell;}

    typedef ::std::vector< ScAccessibleEditLineTextData* > AccTextDataVector;

    OUString    aString;
    vcl::Font   aTextFont;
    ScEditEngineDefaulter*  pEditEngine;            // only created when needed
    EditView*   pEditView;
    AccTextDataVector maAccTextDatas;   // #i105267# text datas may be cloned, remember all copies
    bool        bIsRTL;
    bool        bIsInsertMode;
    bool        bFormulaMode;

    // #102710#; this flag should be true if a key input or a command is handled
    // it prevents the call of InputChanged in the ModifyHandler of the EditEngine
    bool        bInputMode;

private:
    ScTabViewShell* mpViewShell;
};

class ScPosWnd : public ComboBox, public SfxListener        // Display position
{
private:
    OUString        aPosStr;
    Accelerator*    pAccel;
    sal_uLong       nTipVisible;
    bool            bFormulaMode;

public:
                    ScPosWnd( vcl::Window* pParent );
    virtual         ~ScPosWnd();
    virtual void    dispose() override;

    void            SetPos( const OUString& rPosStr );        // Displayed Text
    void            SetFormulaMode( bool bSet );

protected:
    virtual void    Select() override;
    virtual void    Modify() override;

    virtual bool    Notify( NotifyEvent& rNEvt ) override;

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

private:
    void            FillRangeNames();
    void            FillFunctions();
    void            DoEnter();
    void            HideTip();

    void            ReleaseFocus_Impl();
};

class ScInputBarGroup;

class ScMultiTextWnd : public ScTextWnd
{
public:
    ScMultiTextWnd( ScInputBarGroup* pParent, ScTabViewShell* pViewSh );
    virtual ~ScMultiTextWnd();
    virtual void StartEditEngine() override;
    virtual void StopEditEngine( bool bAll ) override;
    virtual void Resize() override;
    virtual EditView*  GetEditView() override;
    long GetPixelHeightForLines( long nLines );
    long GetEditEngTxtHeight();

    void DoScroll();
    virtual void SetTextString( const OUString& rString ) override;
    void SetNumLines( long nLines );
    long GetNumLines() { return mnLines; }
    long GetLastNumExpandedLines() { return mnLastExpandedLines; }
protected:
    void SetScrollBarRange();
    void InitEditEngine();

    virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    DECL_LINK_TYPED( NotifyHdl, EENotify&, void );
    DECL_LINK_TYPED( ModifyHdl, LinkParamNone*, void );
private:
    ScInputBarGroup& mrGroupBar;
    long mnLines;
    long mnLastExpandedLines;
    long mnBorderHeight;
    bool mbInvalidate;
};

class ScInputBarGroup : public ScTextWndBase
{

public:
                    ScInputBarGroup( vcl::Window* Parent, ScTabViewShell* pViewSh );
    virtual         ~ScInputBarGroup();
    virtual void    dispose() override;
    virtual void    InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;
    virtual void    RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;
    void            SetTextString( const OUString& rString ) override;
    void            StartEditEngine() override;
    EditView*       GetEditView() override;
    virtual void    Resize() override;
    virtual const OUString&   GetTextString() const override;
    virtual void            StopEditEngine( bool bAll ) override;
    virtual void            TextGrabFocus() override;
    void            SetFormulaMode( bool bSet ) override;
    void            MakeDialogEditView() override;
    bool            IsInputActive() override;
    ScrollBar&      GetScrollBar() { return *aScrollBar.get(); }
    void            IncrementVerticalSize();
    void            DecrementVerticalSize();
    long            GetNumLines() { return aMultiTextWnd->GetNumLines(); }
    long            GetVertOffset() { return  nVertOffset; }
private:
    void            TriggerToolboxLayout();
    VclPtr<ScMultiTextWnd>  aMultiTextWnd;
    VclPtr<ImageButton>     aButton;
    VclPtr<ScrollBar>       aScrollBar;
    long            nVertOffset;
    DECL_LINK_TYPED( ClickHdl, Button*, void );
    DECL_LINK_TYPED( Impl_ScrollHdl, ScrollBar*, void );

};

class ScInputWindow : public ToolBox                        // Parent toolbox
{
public:
                    ScInputWindow( vcl::Window* pParent, SfxBindings* pBind );
    virtual         ~ScInputWindow();
    virtual void    dispose() override;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    Select() override;

    void            SetFuncString( const OUString& rString, bool bDoEdit = true );
    void            SetPosString( const OUString& rStr );
    void            SetTextString( const OUString& rString );

    void            SetOkCancelMode();
    void            SetSumAssignMode();
    void            EnableButtons( bool bEnable = true );

    void            SetFormulaMode( bool bSet );

    bool            IsInputActive();
    EditView*       GetEditView();

    void            TextGrabFocus();
    void            TextInvalidate();
    void            SwitchToTextWin();

    void            PosGrabFocus();

    // For function autopilots
    void            MakeDialogEditView();

    void            StopEditEngine( bool bAll );

    void            SetInputHandler( ScInputHandler* pNew );

    ScInputHandler* GetInputHandler(){ return pInputHdl;}

    void            StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    bool            IsMultiLineInput() { return mbIsMultiLine; }
protected:
    virtual void    SetText( const OUString& rString ) override;
    virtual OUString  GetText() const override;

    static bool UseSubTotal( ScRangeList* pRangeList );
    bool IsPointerAtResizePos();
private:
    VclPtr<ScPosWnd>  aWndPos;
    VclPtr<ScTextWndBase> pRuntimeWindow;
    ScTextWndBase&  aTextWindow;
    ScInputHandler* pInputHdl;
    OUString        aTextOk;
    OUString        aTextCancel;
    OUString        aTextSum;
    OUString        aTextEqual;
    long            mnMaxY;
    bool            bIsOkCancelMode;
    bool            bInResize;
    bool            mbIsMultiLine;
};

class ScInputWindowWrapper : public SfxChildWindow
{
public:
            ScInputWindowWrapper( vcl::Window*           pParent,
                                  sal_uInt16            nId,
                                  SfxBindings*      pBindings,
                                  SfxChildWinInfo*  pInfo );

    SFX_DECL_CHILDWINDOW_WITHID(ScInputWindowWrapper);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
