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
#include <tools/wintypes.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/idle.hxx>
#include <vcl/image.hxx>
#include <vcl/weld.hxx>
#include <vcl/vclptr.hxx>

class KeyEvent;

namespace vcl {
    class Window;
}

namespace formula {

class IControlReferenceHandler;

class FORMULA_DLLPUBLIC RefEdit : public Edit
{
private:
    Idle                      aIdle;
    IControlReferenceHandler* pAnyRefDlg; // parent dialog
    VclPtr<vcl::Window>       pLabelWidget;

    DECL_LINK( UpdateHdl, Timer*, void );

protected:
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;

public:
    RefEdit( vcl::Window* _pParent, vcl::Window* pShrinkModeLabel, WinBits nStyle = WB_BORDER );

    virtual ~RefEdit() override;
    virtual void dispose() override;

    void SetRefString( const OUString& rStr );

    /**
     * Flag reference valid or invalid, which in turn changes the visual
     * appearance of the control accordingly.
     */
    void SetRefValid(bool bValid);

    using Edit::SetText;

    void         SetText( const OUString& rStr ) override;
    virtual void Modify() override;

    void         StartUpdateData();

    void         SetReferences( IControlReferenceHandler* pDlg, vcl::Window *pLabelWidget );

    vcl::Window*      GetLabelWidgetForShrinkMode()
    {
        return pLabelWidget;
    }
};

class FORMULA_DLLPUBLIC WeldRefEdit
{
private:
    std::unique_ptr<weld::Entry> xEntry;
    Idle aIdle;
    IControlReferenceHandler* pAnyRefDlg; // parent dialog
    weld::Label* pLabelWidget;
    Link<WeldRefEdit&,void> maGetFocusHdl;
    Link<WeldRefEdit&,void> maLoseFocusHdl;
    Link<WeldRefEdit&,void> maModifyHdl;
    Link<weld::Widget&,bool> maActivateHdl;

    DECL_LINK( UpdateHdl, Timer*, void );

protected:
    DECL_LINK(KeyInput, const KeyEvent&, bool);
    DECL_LINK(GetFocus, weld::Widget&, void);
    DECL_LINK(LoseFocus, weld::Widget&, void);
    DECL_LINK(Modify, weld::Entry&, void);

public:
    WeldRefEdit(std::unique_ptr<weld::Entry> xControl);
    weld::Widget* GetWidget() const { return xEntry.get(); }
    ~WeldRefEdit();

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

    weld::Label* GetLabelWidgetForShrinkMode()
    {
        return pLabelWidget;
    }

    void SetGetFocusHdl(const Link<WeldRefEdit&,void>& rLink) { maGetFocusHdl = rLink; }
    void SetLoseFocusHdl(const Link<WeldRefEdit&,void>& rLink) { maLoseFocusHdl = rLink; }
    void SetModifyHdl(const Link<WeldRefEdit&,void>& rLink) { maModifyHdl = rLink; }
    void SetActivateHdl(const Link<weld::Widget&,bool>& rLink) { maActivateHdl = rLink; }
};

class FORMULA_DLLPUBLIC RefButton : public ImageButton
{
private:
    Image                     aImgRefStart; // Start reference input
    Image                     aImgRefDone;  // Stop reference input
    OUString                  aShrinkQuickHelp;
    OUString                  aExpandQuickHelp;
    IControlReferenceHandler* pAnyRefDlg;   // parent dialog
    VclPtr<RefEdit>                  pRefEdit;     // zugeordnetes Edit-Control

protected:
    virtual void Click() override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;

public:
    RefButton(vcl::Window* _pParent, WinBits nStyle);
    virtual ~RefButton() override;
    virtual void dispose() override;
    void SetReferences( IControlReferenceHandler* pDlg, RefEdit* pEdit );
    void SetStartImage();
    void SetEndImage();
    void DoRef()
    {
        Click();
    }
};

class FORMULA_DLLPUBLIC WeldRefButton
{
private:
    std::unique_ptr<weld::Button> xButton;
    IControlReferenceHandler* pAnyRefDlg;   // parent dialog
    WeldRefEdit*              pRefEdit;     // associated Edit-Control
    Link<WeldRefButton&,void> maGetFocusHdl;
    Link<WeldRefButton&,void> maLoseFocusHdl;
    Link<weld::Widget&,bool> maActivateHdl;

protected:
    DECL_LINK(Click, weld::Button&, void);
    DECL_LINK(KeyInput, const KeyEvent&, bool);
    DECL_LINK(GetFocus, weld::Widget&, void);
    DECL_LINK(LoseFocus, weld::Widget&, void);

public:
    WeldRefButton(std::unique_ptr<weld::Button> xControl);
    weld::Widget* GetWidget() const { return xButton.get(); }
    ~WeldRefButton();
    void SetReferences(IControlReferenceHandler* pDlg, WeldRefEdit* pEdit);
    void SetStartImage();
    void SetEndImage();
    void DoRef()
    {
        Click(*xButton);
    }
    void SetGetFocusHdl(const Link<WeldRefButton&,void>& rLink) { maGetFocusHdl = rLink; }
    void SetLoseFocusHdl(const Link<WeldRefButton&,void>& rLink) { maLoseFocusHdl = rLink; }
    void SetActivateHdl(const Link<weld::Widget&,bool>& rLink) { maActivateHdl = rLink; }
};


} // formula

#endif // INCLUDED_FORMULA_FUNCUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
