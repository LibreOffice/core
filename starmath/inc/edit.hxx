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
#ifndef INCLUDED_STARMATH_INC_EDIT_HXX
#define INCLUDED_STARMATH_INC_EDIT_HXX

#include <vcl/window.hxx>
#include <vcl/idle.hxx>
#include <vcl/transfer.hxx>
#include <editeng/editdata.hxx>
#include <memory>

class SmDocShell;
class SmViewShell;
class EditView;
class EditEngine;
class EditStatus;
class ScrollBar;
class ScrollBarBox;
class DataChangedEvent;
class SmCmdBoxWindow;
class SmEditAccessible;
class CommandEvent;
class Timer;

namespace svtools
{
class ColorConfig;
}

void SmGetLeftSelectionPart(const ESelection& rSelection, sal_Int32& nPara, sal_uInt16& nPos);

class SmEditWindow final : public vcl::Window, public DropTargetHelper
{
    rtl::Reference<SmEditAccessible> mxAccessible;

    SmCmdBoxWindow& rCmdBox;
    std::unique_ptr<EditView> pEditView;
    VclPtr<ScrollBar> pHScrollBar;
    VclPtr<ScrollBar> pVScrollBar;
    VclPtr<ScrollBarBox> pScrollBox;
    Idle aModifyIdle;
    Idle aCursorMoveIdle;
    ESelection aOldSelection;

    virtual void KeyInput(const KeyEvent& rKEvt) override;
    virtual void Command(const CommandEvent& rCEvt) override;

    DECL_LINK(ModifyTimerHdl, Timer*, void);
    DECL_LINK(CursorMoveTimerHdl, Timer*, void);

    virtual void ApplySettings(vcl::RenderContext&) override;
    virtual void DataChanged(const DataChangedEvent&) override;
    virtual void Resize() override;
    virtual void MouseMove(const MouseEvent& rEvt) override;
    virtual void MouseButtonUp(const MouseEvent& rEvt) override;
    virtual void MouseButtonDown(const MouseEvent& rEvt) override;

    virtual OUString GetSurroundingText() const override;
    virtual Selection GetSurroundingTextSelection() const override;
    virtual bool DeleteSurroundingText(const Selection& rSelection) override;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    DECL_LINK(EditStatusHdl, EditStatus&, void);
    DECL_LINK(ScrollHdl, ScrollBar*, void);

    void CreateEditView();
    tools::Rectangle AdjustScrollBars();
    void SetScrollBarRanges();
    void InitScrollBars();
    void InvalidateSlots();
    void UpdateStatus(bool bSetDocModified);
    void UserPossiblyChangedText();

public:
    explicit SmEditWindow(SmCmdBoxWindow& rMyCmdBoxWin);
    virtual ~SmEditWindow() override;
    virtual void dispose() override;

    SmDocShell* GetDoc();
    SmViewShell* GetView();
    EditView* GetEditView();
    EditEngine* GetEditEngine();

    // Window
    virtual void SetText(const OUString& rText) override;
    virtual OUString GetText() const override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;

    ESelection GetSelection() const;
    void SetSelection(const ESelection& rSel);

    bool IsEmpty() const;
    bool IsSelected() const;
    bool IsAllSelected() const;
    void Cut();
    void Copy();
    void Paste();
    void Delete();
    void SelectAll();
    void InsertText(const OUString& rText);
    void MarkError(const Point& rPos);
    void SelNextMark();
    void SelPrevMark();
    static bool HasMark(const OUString& rText);

    void Flush() override;
    void DeleteEditView();

    bool HandleWheelCommands(const CommandEvent& rCEvt);
    static bool IsInlineEditEnabled();
    void StartCursorMove();

    // for Accessibility
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    using Window::GetAccessible;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
