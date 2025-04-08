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

#include <svx/weldeditview.hxx>
#include <vcl/idle.hxx>

class SmDocShell;
class SmViewShell;
class EditView;
class EditEngine;
class EditStatus;
class SmCmdBoxWindow;
class Timer;

void SmGetLeftSelectionPart(const ESelection& rSelection, sal_Int32& nPara, sal_uInt16& nPos);

class SmEditWindow;

class SmEditTextWindow : public WeldEditView
{
private:
    SmEditWindow& mrEditWindow;

    Idle aModifyIdle;
    Idle aCursorMoveIdle;

    ESelection aOldSelection;

    DECL_LINK(ModifyTimerHdl, Timer*, void);
    DECL_LINK(CursorMoveTimerHdl, Timer*, void);
    DECL_LINK(EditStatusHdl, EditStatus&, void);

public:
    SmEditTextWindow(SmEditWindow& rEditWindow);
    virtual ~SmEditTextWindow() override;

    virtual EditEngine* GetEditEngine() const override;

    virtual void EditViewScrollStateChange() override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual bool KeyInput(const KeyEvent& rKeyEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rEvt) override;
    virtual bool Command(const CommandEvent& rCEvt) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual void StyleUpdated() override;

    void SetText(const OUString& rText);
    void InsertText(const OUString& rText);
    void SelNextMark();
    ESelection GetSelection() const;
    void UserPossiblyChangedText();
    void Flush();
    void UpdateStatus(bool bSetDocModified);
    void StartCursorMove();
};

class SmEditWindow final
{
    SmCmdBoxWindow& rCmdBox;
    std::unique_ptr<weld::ScrolledWindow> mxScrolledWindow;
    std::unique_ptr<SmEditTextWindow> mxTextControl;
    std::unique_ptr<weld::CustomWeld> mxTextControlWin;

    DECL_LINK(ScrollHdl, weld::ScrolledWindow&, void);

    void CreateEditView(weld::Builder& rBuilder);

public:
    SmEditWindow(SmCmdBoxWindow& rMyCmdBoxWin, weld::Builder& rBuilder);
    ~SmEditWindow() COVERITY_NOEXCEPT_FALSE;

    weld::Window* GetFrameWeld() const;

    SmDocShell* GetDoc();
    SmViewShell* GetView();
    EditView* GetEditView() const;
    EditEngine* GetEditEngine();
    SmCmdBoxWindow& GetCmdBox() const { return rCmdBox; }

    void SetText(const OUString& rText);
    OUString GetText() const;
    void Flush();
    void GrabFocus();

    css::uno::Reference<css::datatransfer::clipboard::XClipboard> GetClipboard() const
    {
        return mxTextControl->GetClipboard();
    }

    ESelection GetSelection() const;
    void SetSelection(const ESelection& rSel);
    void UpdateStatus();

    bool IsEmpty() const;
    bool IsSelected() const;
    bool IsAllSelected() const;
    void SetScrollBarRanges();
    tools::Rectangle AdjustScrollBars();
    void InvalidateSlots();
    void Cut();
    void Copy();
    void Paste();
    void Delete();
    void SelectAll();
    void InsertText(const OUString& rText);
    void MarkError(const Point& rPos);
    void SelNextMark();
    void SelPrevMark();

    void DeleteEditView();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
