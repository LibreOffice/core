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
#ifndef INCLUDED_FORMULA_SOURCE_UI_DLG_CONTROLHELPER_HXX
#define INCLUDED_FORMULA_SOURCE_UI_DLG_CONTROLHELPER_HXX

#include "formula/funcutl.hxx"
#include <svtools/svmedit.hxx>
namespace formula
{

// class EditBox
class EditBox : public Control
{
private:

    VclPtr<MultiLineEdit>  pMEdit;
    Link<EditBox&,void>    aSelChangedLink;
    Selection              aOldSel;
    bool                   bMouseFlag;

    DECL_LINK_TYPED(ChangedHdl, void *, void);

protected:

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    void            SelectionChanged();
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;


public:
                    EditBox( vcl::Window* pParent, WinBits nBits );

                    virtual ~EditBox();
    virtual void    dispose() SAL_OVERRIDE;

    MultiLineEdit*  GetEdit() {return pMEdit;}

    void            SetSelChangedHdl( const Link<EditBox&,void>& rLink ) { aSelChangedLink = rLink; }

    void            UpdateOldSel();
};


// class ArgEdit

class ArgEdit : public RefEdit
{
public:
            ArgEdit( vcl::Window* pParent, WinBits nBits );
    virtual ~ArgEdit();
    virtual void dispose() SAL_OVERRIDE;

    void    Init( ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                  ScrollBar& rArgSlider, sal_uInt16 nArgCount );

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

private:
    VclPtr<ArgEdit>    pEdPrev;
    VclPtr<ArgEdit>    pEdNext;
    VclPtr<ScrollBar>  pSlider;
    sal_uInt16      nArgs;
};



// class ArgInput

class ArgInput
{
private:
    Link<ArgInput&,void>          aFxClickLink;
    Link<ArgInput&,void>          aFxFocusLink;
    Link<>          aEdFocusLink;
    Link<>          aEdModifyLink;

    VclPtr<FixedText>      pFtArg;
    VclPtr<PushButton>     pBtnFx;
    VclPtr<ArgEdit>        pEdArg;
    VclPtr<RefButton>      pRefBtn;

    DECL_LINK_TYPED( FxBtnClickHdl, Button*, void );
    DECL_LINK(  FxBtnFocusHdl, ImageButton* );
    DECL_LINK(  EdFocusHdl, ArgEdit* );
    DECL_LINK(  EdModifyHdl,ArgEdit* );

protected:

    void    FxClick();
    void    FxFocus();
    void    EdFocus();
    void    EdModify();

public:

    ArgInput();

    virtual ~ArgInput() {}

    void        InitArgInput (  FixedText*      pftArg,
                                PushButton*    pbtnFx,
                                ArgEdit*        pedArg,
                                RefButton*  prefBtn);

    void        SetArgName(const OUString &aArg);
    OUString    GetArgName();
    void        SetArgNameFont(const vcl::Font&);

    void        SetArgVal(const OUString &aVal);
    OUString    GetArgVal();

    void        SetArgSelection (const Selection& rSel);

    ArgEdit*    GetArgEdPtr() {return pEdArg;}


    void            SetFxClickHdl( const Link<ArgInput&,void>& rLink ) { aFxClickLink = rLink; }

    void            SetFxFocusHdl( const Link<ArgInput&,void>& rLink ) { aFxFocusLink = rLink; }

    void            SetEdFocusHdl( const Link<>& rLink ) { aEdFocusLink = rLink; }

    void            SetEdModifyHdl( const Link<>& rLink ) { aEdModifyLink = rLink; }

    void Hide();
    void Show();

    void UpdateAccessibleNames();
};

}
#endif // FORMULA_FORMULA_HELPER_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
