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

#pragma once

#include <set>
#include <vector>
#include <memory>
#include <vcl/customweld.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <sfx2/childwin.hxx>
#include <svl/lstner.hxx>
#include <svtools/stringtransfer.hxx>
#include <vcl/window.hxx>
#include <formula/opcode.hxx>
#include <svx/weldeditview.hxx>

namespace com::sun::star::accessibility { class XAccessible; }

class EditView;
class ScAccessibleEditLineTextData;
class ScAccessibleEditObject;
class ScEditEngineDefaulter;
class ScTextWndGroup;
class ScInputBarGroup;
class ScInputHandler;
class ScTabViewShell;
struct EENotify;

class ScTextWndBase
{
public:
    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) = 0;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) = 0;
    virtual void            SetTextString( const OUString& rString, bool bKitUpdate ) = 0;
    virtual const OUString& GetTextString() const = 0;
    virtual void            StartEditEngine() = 0;
    virtual void            StopEditEngine( bool bAll ) = 0;
    virtual EditView*       GetEditView() const = 0;
    virtual bool            HasEditView() const = 0;
    virtual void            MakeDialogEditView() = 0;
    virtual void            SetFormulaMode( bool bSet ) = 0;
    virtual bool            IsInputActive() = 0;
    virtual void            TextGrabFocus() = 0;
    virtual tools::Long            GetNumLines() const = 0;
    virtual ~ScTextWndBase() {}
};

class ScTextWnd : public WeldEditView
                , public ScTextWndBase
{
public:
    ScTextWnd(ScTextWndGroup& rParent, ScTabViewShell* pViewSh);
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~ScTextWnd() override;

    virtual void            SetTextString( const OUString& rString, bool bKitUpdate ) override;
    virtual const OUString& GetTextString() const override;

    bool                    IsInputActive() override;
    virtual EditView*       GetEditView() const override;
    virtual bool            HasEditView() const override;

    const OutputDevice&     GetEditViewDevice() const;

                        // for function autopilots
    virtual void            MakeDialogEditView() override;

    virtual void            StartEditEngine() override;
    virtual void            StopEditEngine( bool bAll ) override;

    virtual void            TextGrabFocus() override;

    virtual void            StyleUpdated() override;

    // Triggered if scroll bar state should change
    virtual void EditViewScrollStateChange() override;

    virtual void            SetFormulaMode( bool bSet ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) override;

    virtual void            Resize() override;

    int GetPixelHeightForLines(tools::Long nLines);
    int GetEditEngTxtHeight() const;

    virtual tools::Long GetNumLines() const override;
    void SetNumLines(tools::Long nLines);
    tools::Long GetLastNumExpandedLines() const { return mnLastExpandedLines; }
    void SetLastNumExpandedLines(tools::Long nLastExpandedLines) { mnLastExpandedLines = nLastExpandedLines; }

    void DoScroll();

    DECL_LINK(ModifyHdl, LinkParamNone*, void);
    DECL_LINK(EditStatusHdl, EditStatus&, void);

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;

    virtual bool    MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual bool    Command( const CommandEvent& rCEvt ) override;
    virtual bool    KeyInput(const KeyEvent& rKEvt) override;
    virtual bool    CanFocus() const override;
    virtual void    GetFocus() override;

    virtual bool    StartDrag() override;

private:
    void            ImplInitSettings();
    void            UpdateAutoCorrFlag();

    void SetScrollBarRange();

    void InitEditEngine();

    void UpdateFocus();

    rtl::Reference<svt::OStringTransferable> m_xHelper;

    typedef ::std::vector< ScAccessibleEditLineTextData* > AccTextDataVector;

    rtl::Reference<ScAccessibleEditObject> pAcc;

    OUString    aString;
    vcl::Font   aTextFont;
    AccTextDataVector maAccTextDatas;   // #i105267# text data may be cloned, remember all copies
    bool        bIsRTL;
    bool        bIsInsertMode;
    bool        bFormulaMode;

    // #102710#; this flag should be true if a key input or a command is handled
    // it prevents the call of InputChanged in the ModifyHandler of the EditEngine
    bool        bInputMode;

    ScTabViewShell* mpViewShell;
    ScTextWndGroup& mrGroupBar;
    tools::Long mnLastExpandedLines;
    bool mbInvalidate;
};

class ScPosWnd final : public InterimItemWindow, public SfxListener        // Display position
{
private:
    std::unique_ptr<weld::ComboBox> m_xWidget;

    ImplSVEvent* m_nAsyncGetFocusId;

    OUString aPosStr;
    void* nTipVisible;
    bool bFormulaMode;
    std::set<OUString> aRangeNames;

public:
    ScPosWnd(vcl::Window* pParent, ScTabViewShell* pViewSh);
    virtual         ~ScPosWnd() override;
    virtual void    dispose() override;

    void            SetPos( const OUString& rPosStr );        // Displayed Text
    void            SetFormulaMode( bool bSet );

    static OUString createLocalRangeName(std::u16string_view rName, std::u16string_view rTableName);

private:
    DECL_LINK(OnAsyncGetFocus, void*, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);
    DECL_LINK(ModifyHdl, weld::ComboBox&, void);
    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

private:
    void            FillRangeNames(bool initialize = false);
    void            FillFunctions();
    void            DoEnter();
    void            HideTip();

    void            ReleaseFocus_Impl();
};

class ScTextWndGroup : public ScTextWndBase
{
public:
    ScTextWndGroup(ScInputBarGroup& pParent, ScTabViewShell* pViewSh);
    virtual ~ScTextWndGroup() override;

    virtual void            InsertAccessibleTextData(ScAccessibleEditLineTextData& rTextData) override;
    virtual EditView*       GetEditView() const override;
    const OutputDevice&     GetEditViewDevice() const;
    Point                   GetCursorScreenPixelPos(bool bBelowLine);
    tools::Long             GetLastNumExpandedLines() const;
    void SetLastNumExpandedLines(tools::Long nLastExpandedLines);
    virtual tools::Long     GetNumLines() const override;
    int                     GetPixelHeightForLines(tools::Long nLines);
    weld::ScrolledWindow&   GetScrollWin();
    virtual const OUString& GetTextString() const override;
    virtual bool            HasEditView() const override;
    virtual bool            IsInputActive() override;
    virtual void            MakeDialogEditView() override;
    virtual void            RemoveAccessibleTextData(ScAccessibleEditLineTextData& rTextData) override;
    void                    SetScrollPolicy();
    void                    SetNumLines(tools::Long nLines);
    virtual void            SetFormulaMode(bool bSet) override;
    virtual void            SetTextString(const OUString& rString, bool bKitUpdate) override;
    virtual void            StartEditEngine() override;
    virtual void            StopEditEngine(bool bAll) override;
    virtual void            TextGrabFocus() override;

    vcl::Window&            GetVclParent() { return mrParent; }

private:
    std::unique_ptr<ScTextWnd> mxTextWnd;
    std::unique_ptr<weld::ScrolledWindow> mxScrollWin;
    std::unique_ptr<weld::CustomWeld> mxTextWndWin;
    vcl::Window& mrParent;

    DECL_LINK(Impl_ScrollHdl, weld::ScrolledWindow&, void);
};

class ScInputBarGroup : public InterimItemWindow
                      , public ScTextWndBase
{
public:
                            ScInputBarGroup(vcl::Window* Parent, ScTabViewShell* pViewSh);
    virtual                 ~ScInputBarGroup() override;
    virtual void            dispose() override;
    virtual void            InsertAccessibleTextData(ScAccessibleEditLineTextData& rTextData) override;
    virtual void            RemoveAccessibleTextData(ScAccessibleEditLineTextData& rTextData) override;
    void                    SetTextString(const OUString& rString, bool bKitUpdate) override;
    void                    StartEditEngine() override;
    virtual EditView*       GetEditView() const override;
    virtual bool            HasEditView() const override;
    Point                   GetCursorScreenPixelPos(bool bBelowLine);
    virtual void            Resize() override;
    virtual void            DataChanged(const DataChangedEvent& rDCEvt) override;
    virtual const OUString& GetTextString() const override;
    virtual void            StopEditEngine(bool bAll) override;
    virtual void            TextGrabFocus() override;
    void                    SetFormulaMode(bool bSet) override;
    void                    MakeDialogEditView() override;
    bool                    IsInputActive() override;
    void                    IncrementVerticalSize();
    void                    DecrementVerticalSize();
    void                    NumLinesChanged();
    virtual tools::Long     GetNumLines() const override { return mxTextWndGroup->GetNumLines(); }

    int GetPixelHeightForLines(tools::Long nLines) const
    {
        return mxTextWndGroup->GetPixelHeightForLines(nLines);
    }

    weld::Builder&          GetBuilder() { return *m_xBuilder; }

private:
    void            TriggerToolboxLayout();
    void            SetBackgrounds();

    std::unique_ptr<weld::Container> mxBackground;
    std::unique_ptr<ScTextWndGroup> mxTextWndGroup;
    std::unique_ptr<weld::Button> mxButtonUp;
    std::unique_ptr<weld::Button> mxButtonDown;

    DECL_LINK(ClickHdl, weld::Button&, void);
};

class ScInputWindow final : public ToolBox                        // Parent toolbox
{
public:
                    ScInputWindow( vcl::Window* pParent, const SfxBindings* pBind );
    virtual         ~ScInputWindow() override;
    virtual void    dispose() override;

    virtual void    PixelInvalidate(const tools::Rectangle* pRectangle) override;
    virtual void    SetSizePixel( const Size& rNewSize ) override;
    virtual void    Resize() override;
    virtual void    Select() override;

    void            SetFuncString( const OUString& rString, bool bDoEdit = true );
    void            SetPosString( const OUString& rStr );
    void            SetTextString(const OUString& rString, bool bKitUpdate);

    void            SetOkCancelMode();
    void            SetSumAssignMode();
    void            EnableButtons( bool bEnable );
    /// Update Input bar after the number of lines was changed externally
    void            NumLinesChanged();

    void            StartFormula();
    void            SetFormulaMode( bool bSet );

    bool            IsInputActive();
    EditView*       GetEditView();
    vcl::Window*    GetEditWindow();
    Point           GetCursorScreenPixelPos(bool bBelowLine = false);

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

    void            NotifyLOKClient();

    void MenuHdl(std::u16string_view command);
    DECL_LINK( DropdownClickHdl, ToolBox*, void );

    void            AutoSum( bool& bRangeFinder, bool& bSubTotal, OpCode eCode );

private:
    bool IsPointerAtResizePos();

    VclPtr<ScPosWnd>  aWndPos;
    VclPtr<ScInputBarGroup> mxTextWindow;
    ScInputHandler* pInputHdl;
    ScTabViewShell* mpViewShell;
    tools::Long     mnMaxY;
    tools::Long     mnStandardItemHeight;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
