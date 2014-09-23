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

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <formula/formuladllapi.h>

namespace formula {

class IControlReferenceHandler;

class FORMULA_DLLPUBLIC RefEdit : public Edit
{
private:
    Timer                     aTimer;
    IControlReferenceHandler* pAnyRefDlg; // parent dialog
    vcl::Window*                   pLabelWidget;

    DECL_LINK( UpdateHdl, void* );

protected:
    virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void GetFocus() SAL_OVERRIDE;
    virtual void LoseFocus() SAL_OVERRIDE;

public:
    RefEdit( vcl::Window* _pParent,IControlReferenceHandler* pParent, vcl::Window* pShrinkModeLabel, const ResId& rResId );
    RefEdit( vcl::Window* _pParent, vcl::Window* pShrinkModeLabel, WinBits nStyle = WB_BORDER );

    virtual ~RefEdit();

    void SetRefString( const OUString& rStr );

    /**
     * Flag reference valid or invalid, which in turn changes the visual
     * appearance of the control accordingly.
     */
    void SetRefValid(bool bValid);

    using Edit::SetText;

    void         SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual void Modify() SAL_OVERRIDE;

    void         StartUpdateData();

    void         SetReferences( IControlReferenceHandler* pDlg, vcl::Window *pLabelWidget );

    IControlReferenceHandler* GetRefDialog()
    {
        return pAnyRefDlg;
    }

    vcl::Window*      GetLabelWidgetForShrinkMode()
    {
        return pLabelWidget;
    }
};


class FORMULA_DLLPUBLIC RefButton : public ImageButton
{
private:
    Image                     aImgRefStart; // Start reference input
    Image                     aImgRefDone;  // Stop reference input
    OUString                  aShrinkQuickHelp;
    OUString                  aExpandQuickHelp;
    IControlReferenceHandler* pAnyRefDlg;   // parent dialog
    RefEdit*                  pRefEdit;     // zugeordnetes Edit-Control

protected:
    virtual void Click() SAL_OVERRIDE;
    virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void GetFocus() SAL_OVERRIDE;
    virtual void LoseFocus() SAL_OVERRIDE;

public:
    RefButton(vcl::Window* _pParent, WinBits nStyle = 0);
    void SetReferences( IControlReferenceHandler* pDlg, RefEdit* pEdit );
    void SetStartImage();
    void SetEndImage();
    void DoRef()
    {
        Click();
    }
};

} // formula

#endif // INCLUDED_FORMULA_FUNCUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
