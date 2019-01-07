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
#include <memory>
#include <vcl/toolbox.hxx>
#include <sfx2/childwin.hxx>
#include <svl/lstner.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/window.hxx>
#include <vcl/transfer.hxx>

class EditView;
class ScAccessibleEditLineTextData;
class ScEditEngineDefaulter;
class ScInputBarGroup;
class ScInputHandler;
class ScTabViewShell;
struct EENotify;

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
    virtual bool            HasEditView() const = 0;
    virtual void            MakeDialogEditView() = 0;
    virtual void            SetFormulaMode( bool bSet ) = 0;
    virtual bool            IsInputActive() = 0;
    virtual void            TextGrabFocus() = 0;
};

class ScTextWnd : public ScTextWndBase, public DragSourceHelper     // edit window
{
public:
    ScTextWnd(ScInputBarGroup* pParent, ScTabViewShell* pViewSh);
    virtual         ~ScTextWnd() override;
    virtual void    dispose() override;

    virtual void            SetTextString( const OUString& rString ) override;
    virtual const OUString& GetTextString() const override;

    bool                    IsInputActive() override;
    virtual EditView*       GetEditView() override;
    virtual bool            HasEditView() const override;

                        // for function autopilots
    virtual void            MakeDialogEditView() override;

    virtual void            StartEditEngine() override;
    virtual void            StopEditEngine( bool bAll ) override;

    virtual void            TextGrabFocus() override;

    virtual void            DataChanged(const DataChangedEvent& rDCEvt) override;

    virtual void            SetFormulaMode( bool bSet ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;

    virtual void            Resize() override;

    long GetPixelHeightForLines(long nLines);
    long GetEditEngTxtHeight();

    long GetNumLines() { return mnLines; }
    void SetNumLines(long nLines);
    long GetLastNumExpandedLines() { return mnLastExpandedLines; }

    void DoScroll();

    DECL_LINK(NotifyHdl, EENotify&, void);
    DECL_LINK(ModifyHdl, LinkParamNone*, void);

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    KeyInput(const KeyEvent& rKEvt) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

    virtual OUString  GetText() const override;

private:
    void            ImplInitSettings();
    void            UpdateAutoCorrFlag();

    void SetScrollBarRange();

    void InitEditEngine();

    typedef ::std::vector< ScAccessibleEditLineTextData* > AccTextDataVector;

    OUString    aString;
    vcl::Font   aTextFont;
    std::unique_ptr<ScEditEngineDefaulter> mpEditEngine; // only created when needed
    std::unique_ptr<EditView> mpEditView;
    AccTextDataVector maAccTextDatas;   // #i105267# text datas may be cloned, remember all copies
    bool        bIsRTL;
    bool        bIsInsertMode;
    bool        bFormulaMode;

    // #102710#; this flag should be true if a key input or a command is handled
    // it prevents the call of InputChanged in the ModifyHandler of the EditEngine
    bool        bInputMode;

    ScTabViewShell* mpViewShell;
    ScInputBarGroup& mrGroupBar;
    long mnLines;
    long mnLastExpandedLines;
    long mnBorderHeight;
    bool mbInvalidate;
};

class ScPosWnd : public ComboBox, public SfxListener        // Display position
{
private:
    OUString        aPosStr;
    void*           nTipVisible;
    bool            bFormulaMode;

public:
                    ScPosWnd( vcl::Window* pParent );
    virtual         ~ScPosWnd() override;
    virtual void    dispose() override;

    void            SetPos( const OUString& rPosStr );        // Displayed Text
    void            SetFormulaMode( bool bSet );

protected:
    virtual void    Select() override;
    virtual void    Modify() override;

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

private:
    void            FillRangeNames();
    void            FillFunctions();
    void            DoEnter();
    void            HideTip();

    void            ReleaseFocus_Impl();
};

class ScInputBarGroup : public ScTextWndBase
{

public:
                    ScInputBarGroup( vcl::Window* Parent, ScTabViewShell* pViewSh );
    virtual         ~ScInputBarGroup() override;
    virtual void    dispose() override;
    virtual void    InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;
    virtual void    RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;
    void            SetTextString( const OUString& rString ) override;
    void            StartEditEngine() override;
    virtual EditView* GetEditView() override;
    virtual bool HasEditView() const override;
    virtual void    Resize() override;
    virtual const OUString&   GetTextString() const override;
    virtual void            StopEditEngine( bool bAll ) override;
    virtual void            TextGrabFocus() override;
    void            SetFormulaMode( bool bSet ) override;
    void            MakeDialogEditView() override;
    bool            IsInputActive() override;
    ScrollBar&      GetScrollBar() { return *maScrollbar; }
    void            IncrementVerticalSize();
    void            DecrementVerticalSize();
    long            GetNumLines() { return maTextWnd->GetNumLines(); }
    long            GetVertOffset() { return  mnVertOffset; }

private:
    void            TriggerToolboxLayout();

    VclPtr<ScTextWnd> maTextWnd;
    VclPtr<ImageButton> maButton;
    VclPtr<ScrollBar> maScrollbar;
    long            mnVertOffset;

    DECL_LINK( ClickHdl, Button*, void );
    DECL_LINK( Impl_ScrollHdl, ScrollBar*, void );
};

class ScInputWindow final : public ToolBox                        // Parent toolbox
{
public:
                    ScInputWindow( vcl::Window* pParent, const SfxBindings* pBind );
    virtual         ~ScInputWindow() override;
    virtual void    dispose() override;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    Select() override;

    void            SetFuncString( const OUString& rString, bool bDoEdit = true );
    void            SetPosString( const OUString& rStr );
    void            SetTextString( const OUString& rString );

    void            SetOkCancelMode();
    void            SetSumAssignMode();
    void            EnableButtons( bool bEnable );

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

private:
    bool IsPointerAtResizePos();

    VclPtr<ScPosWnd>  aWndPos;
    VclPtr<ScTextWndBase> pRuntimeWindow;
    ScTextWndBase&  aTextWindow;
    ScInputHandler* pInputHdl;
    long            mnMaxY;
    bool            bIsOkCancelMode;
    bool            bInResize;
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
