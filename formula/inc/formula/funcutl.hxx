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

#ifndef FORMULA_FUNCUTL_HXX
#define FORMULA_FUNCUTL_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include "formula/formuladllapi.h"

namespace formula {

class IControlReferenceHandler;

class FORMULA_DLLPUBLIC RefEdit : public Edit
{
private:
    Timer               aTimer;
    IControlReferenceHandler*      pAnyRefDlg;         // parent dialog

    DECL_LINK( UpdateHdl, void* );

protected:
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        GetFocus();
    virtual void        LoseFocus();

public:
                        RefEdit( Window* _pParent,IControlReferenceHandler* pParent, const ResId& rResId );
    virtual             ~RefEdit();

    void                SetRefString( const XubString& rStr );

    /**
     * Flag reference valid or invalid, which in turn changes the visual
     * appearance of the control accordingly.
     */
    void                SetRefValid(bool bValid);

    using Edit::SetText;
    virtual void        SetText( const XubString& rStr );
    virtual void        Modify();

    void                StartUpdateData();

    void                SetRefDialog( IControlReferenceHandler* pDlg );
    inline IControlReferenceHandler* GetRefDialog() { return pAnyRefDlg; }
};


//============================================================================

class FORMULA_DLLPUBLIC RefButton : public ImageButton
{
private:
    Image               aImgRefStart;   /// Start reference input
    Image               aImgRefDone;    /// Stop reference input
    IControlReferenceHandler*      pAnyRefDlg;     // parent dialog
    RefEdit*            pRefEdit;       // zugeordnetes Edit-Control

protected:
    virtual void        Click();
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        GetFocus();
    virtual void        LoseFocus();

public:
                        RefButton( Window* _pParent, const ResId& rResId);
                        RefButton( Window* _pParent, const ResId& rResId, RefEdit* pEdit ,IControlReferenceHandler* pDlg);

    void                SetReferences( IControlReferenceHandler* pDlg, RefEdit* pEdit );

    void                SetStartImage();
    void                SetEndImage();
    inline void         DoRef() { Click(); }
};

} // formula

#endif // FORMULA_FUNCUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
