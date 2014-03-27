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
#ifndef FORMULA_CONTROL_HELPER_HXX_INCLUDED
#define FORMULA_CONTROL_HELPER_HXX_INCLUDED

#include "formula/funcutl.hxx"
#include <svtools/svmedit.hxx>
namespace formula
{


// class ValWnd
class ValWnd : public Window
{
public:
            ValWnd( Window* pParent, const ResId& rId );

    void    SetValue( const OUString& rStrVal );

protected:
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;

private:
    OUString    aStrValue;
    Rectangle   aRectOut;
};


// class EditBox
class EditBox : public Control
{
private:

    MultiLineEdit*  pMEdit;
    Link            aSelChangedLink;
    Selection       aOldSel;
    sal_Bool            bMouseFlag;
                    DECL_LINK(ChangedHdl, void *);

protected:

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    SelectionChanged();
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;


public:
                    EditBox( Window* pParent, const ResId& rResId );

                    virtual ~EditBox();

    MultiLineEdit*  GetEdit() {return pMEdit;}

    void            SetSelChangedHdl( const Link& rLink ) { aSelChangedLink = rLink; }
    const Link&     GetSelChangedHdl() const { return aSelChangedLink; }

    void            UpdateOldSel();
};


// class ArgEdit

class ArgEdit : public RefEdit
{
public:
            ArgEdit( Window* pParent, const ResId& rResId );

    void    Init( ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                  ScrollBar& rArgSlider, sal_uInt16 nArgCount );

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

private:
    ArgEdit*    pEdPrev;
    ArgEdit*    pEdNext;
    ScrollBar*  pSlider;
    sal_uInt16      nArgs;
};



// class ArgInput

class ArgInput
{
private:

    Link            aFxClickLink;
    Link            aRefClickLink;
    Link            aFxFocusLink;
    Link            aRefFocusLink;
    Link            aEdFocusLink;
    Link            aEdModifyLink;

    FixedText*      pFtArg;
    ImageButton*    pBtnFx;
    ArgEdit*        pEdArg;
    RefButton*  pRefBtn;

    DECL_LINK(  FxBtnClickHdl, ImageButton* );
    DECL_LINK(  RefBtnClickHdl,RefButton* );
    DECL_LINK(  FxBtnFocusHdl, ImageButton* );
    DECL_LINK(  RefBtnFocusHdl,RefButton* );
    DECL_LINK(  EdFocusHdl, ArgEdit* );
    DECL_LINK(  EdModifyHdl,ArgEdit* );

protected:

    virtual void    FxClick();
    virtual void    RefClick();
    virtual void    FxFocus();
    virtual void    RefFocus();
    virtual void    EdFocus();
    virtual void    EdModify();

public:

    ArgInput();

    virtual ~ArgInput() {}

    void        InitArgInput (  FixedText*      pftArg,
                                ImageButton*    pbtnFx,
                                ArgEdit*        pedArg,
                                RefButton*  prefBtn);

    void        SetArgName(const OUString &aArg);
    OUString    GetArgName();
    void        SetArgNameFont(const Font&);

    void        SetArgVal(const OUString &aVal);
    OUString    GetArgVal();

    void        SetArgSelection (const Selection& rSel);
    void        ReplaceSelOfArg (const OUString& rStr);

    ArgEdit*    GetArgEdPtr() {return pEdArg;}


    void            SetFxClickHdl( const Link& rLink ) { aFxClickLink = rLink; }
    const Link&     GetFxClickHdl() const { return aFxClickLink; }

    void            SetRefClickHdl( const Link& rLink ) { aRefClickLink = rLink; }
    const Link&     GetRefClickHdl() const { return aRefClickLink; }

    void            SetFxFocusHdl( const Link& rLink ) { aFxFocusLink = rLink; }
    const Link&     GetFxFocusHdl() const { return aFxFocusLink; }

    void            SetRefFocusHdl( const Link& rLink ) { aRefFocusLink = rLink; }
    const Link&     GetRefFocusHdl() const { return aRefFocusLink; }

    void            SetEdFocusHdl( const Link& rLink ) { aEdFocusLink = rLink; }
    const Link&     GetEdFocusHdl() const { return aEdFocusLink; }

    void            SetEdModifyHdl( const Link& rLink ) { aEdModifyLink = rLink; }
    const Link&     GetEdModifyHdl() const { return aEdModifyLink; }

    void Hide();
    void Show();

    void UpdateAccessibleNames();
};

}
#endif // FORMULA_FORMULA_HELPER_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
