/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    sal_Bool                bSilentFocus;       // for SilentGrabFocus()

    DECL_LINK( UpdateHdl, Timer* );

protected:
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        GetFocus();
    virtual void        LoseFocus();

public:
                        RefEdit( Window* _pParent,IControlReferenceHandler* pParent, const ResId& rResId );
                        RefEdit( Window* pParent, const ResId& rResId );
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

    void                SilentGrabFocus();  // does not update any references

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
