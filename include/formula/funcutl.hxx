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

#ifndef INCLUDED_FORMULA_FUNCUTL_HXX
#define INCLUDED_FORMULA_FUNCUTL_HXX

#include <formula/formuladllapi.h>
#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>

class KeyEvent;
struct ImplSVEvent;

namespace formula {

class IControlReferenceHandler;

class FORMULA_DLLPUBLIC RefEdit
{
protected:
    std::unique_ptr<weld::Entry> xEntry;

private:
    Idle aIdle;
    IControlReferenceHandler* pAnyRefDlg; // parent dialog
    weld::Label* pLabelWidget;
    ImplSVEvent* mpFocusInEvent;
    ImplSVEvent* mpFocusOutEvent;

    Link<RefEdit&,void> maGetFocusHdl;
    Link<RefEdit&,void> maLoseFocusHdl;
    Link<RefEdit&,void> maModifyHdl;
    Link<weld::Widget&,bool> maActivateHdl;

    DECL_LINK( UpdateHdl, Timer*, void );

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(GetFocusHdl, weld::Widget&, void);
    DECL_LINK(LoseFocusHdl, weld::Widget&, void);
    DECL_LINK(AsyncFocusInHdl, void*, void);
    DECL_LINK(AsyncFocusOutHdl, void*, void);
    DECL_LINK(Modify, weld::Entry&, void);

    void GetFocus();
    void LoseFocus();

protected:
    virtual bool KeyInput(const KeyEvent& rKEvt);

public:
    RefEdit(std::unique_ptr<weld::Entry> xControl);
    weld::Entry* GetWidget() const { return xEntry.get(); }
    virtual ~RefEdit();

    void SetRefString( const OUString& rStr );

    /**
     * Flag reference valid or invalid, which in turn changes the visual
     * appearance of the control accordingly.
     */
    void SetRefValid(bool bValid);

    void SetText(const OUString& rStr);
    OUString GetText() const
    {
        return xEntry->get_text();
    }

    void StartUpdateData();

    void SetReferences( IControlReferenceHandler* pDlg, weld::Label *pLabelWidget );

    void DoModify()
    {
        Modify(*xEntry);
    }

    void GrabFocus()
    {
        xEntry->grab_focus();
    }

    void SelectAll()
    {
        xEntry->select_region(0, -1);
    }

    void SetSelection(const Selection& rSelection)
    {
        xEntry->select_region(rSelection.Min(), rSelection.Max());
    }

    void SetCursorAtLast()
    {
        xEntry->set_position(-1);
    }

    Selection GetSelection() const
    {
        int nStartPos, nEndPos;
        xEntry->get_selection_bounds(nStartPos, nEndPos);
        return Selection(nStartPos, nEndPos);
    }

    weld::Label* GetLabelWidgetForShrinkMode()
    {
        return pLabelWidget;
    }

    void SaveValue()
    {
        xEntry->save_value();
    }

    bool IsValueChangedFromSaved() const
    {
        return xEntry->get_value_changed_from_saved();
    }

    void SetGetFocusHdl(const Link<RefEdit&,void>& rLink) { maGetFocusHdl = rLink; }
    void SetLoseFocusHdl(const Link<RefEdit&,void>& rLink) { maLoseFocusHdl = rLink; }
    void SetModifyHdl(const Link<RefEdit&,void>& rLink) { maModifyHdl = rLink; }
    const Link<RefEdit&,void>& GetModifyHdl() const { return maModifyHdl; }
    void SetActivateHdl(const Link<weld::Widget&,bool>& rLink) { maActivateHdl = rLink; }
};

class FORMULA_DLLPUBLIC RefButton
{
private:
    std::unique_ptr<weld::Button> xButton;
    IControlReferenceHandler* pAnyRefDlg;   // parent dialog
    RefEdit*              pRefEdit;     // associated Edit-Control
    Link<RefButton&,void> maGetFocusHdl;
    Link<RefButton&,void> maLoseFocusHdl;
    Link<weld::Widget&,bool> maActivateHdl;
    Link<RefButton&,void> maClickHdl;

    DECL_LINK(Click, weld::Button&, void);
    DECL_LINK(KeyInput, const KeyEvent&, bool);
    DECL_LINK(GetFocus, weld::Widget&, void);
    DECL_LINK(LoseFocus, weld::Widget&, void);

public:
    RefButton(std::unique_ptr<weld::Button> xControl);
    weld::Button* GetWidget() const { return xButton.get(); }
    ~RefButton();
    void SetReferences(IControlReferenceHandler* pDlg, RefEdit* pEdit);
    void SetStartImage();
    void SetEndImage();
    void DoRef()
    {
        Click(*xButton);
    }
    void SetGetFocusHdl(const Link<RefButton&,void>& rLink) { maGetFocusHdl = rLink; }
    void SetLoseFocusHdl(const Link<RefButton&,void>& rLink) { maLoseFocusHdl = rLink; }
    void SetActivateHdl(const Link<weld::Widget&,bool>& rLink) { maActivateHdl = rLink; }
    void SetClickHdl(const Link<RefButton&,void>& rLink) { maClickHdl = rLink; }
};

} // formula

#endif // INCLUDED_FORMULA_FUNCUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
