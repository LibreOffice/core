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
#ifndef FORMULA_CONTROL_HELPER_HXX_INCLUDED
#define FORMULA_CONTROL_HELPER_HXX_INCLUDED

#include "formula/funcutl.hxx"
#include <svtools/svmedit.hxx>
namespace formula
{

//============================================================================
// class ValWnd
class ValWnd : public Window
{
public:
            ValWnd( Window* pParent, const ResId& rId );

    void    SetValue( const String& rStrVal );

protected:
    virtual void    Paint( const Rectangle& rRect );

private:
    String      aStrValue;
    Rectangle   aRectOut;
};

//============================================================================
// class EditBox
class EditBox : public Control
{
private:

    MultiLineEdit*  pMEdit;
    Link            aSelChangedLink;
    Selection       aOldSel;
    sal_Bool            bMouseFlag;
                    DECL_LINK( ChangedHdl, EditBox* );

protected:

    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual void    SelectionChanged();
    virtual void    Resize();
    virtual void    GetFocus();


public:
                    EditBox( Window* pParent,
                                WinBits nWinStyle = WB_LEFT | WB_BORDER );
                    EditBox( Window* pParent, const ResId& rResId );

                    virtual ~EditBox();

    MultiLineEdit*  GetEdit() {return pMEdit;}

    void            SetSelChangedHdl( const Link& rLink ) { aSelChangedLink = rLink; }
    const Link&     GetSelChangedHdl() const { return aSelChangedLink; }

    void            UpdateOldSel();
};

//============================================================================
// class ArgEdit

class ArgEdit : public RefEdit
{
public:
            ArgEdit( Window* pParent, const ResId& rResId );

    void    Init( ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                  ScrollBar& rArgSlider, sal_uInt16 nArgCount );

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt );

private:
    ArgEdit*    pEdPrev;
    ArgEdit*    pEdNext;
    ScrollBar*  pSlider;
    sal_uInt16      nArgs;
};


//============================================================================
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

    void        InitArgInput (  FixedText*      pftArg,
                                ImageButton*    pbtnFx,
                                ArgEdit*        pedArg,
                                RefButton*  prefBtn);

    void        SetArgName(const String &aArg);
    String      GetArgName();
    void        SetArgNameFont(const Font&);

    void        SetArgVal(const String &aVal);
    String      GetArgVal();

    void        SetArgSelection (const Selection& rSel );
    void        ReplaceSelOfArg (const String& rStr );

    Selection   GetArgSelection();


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

};

}
#endif // FORMULA_FORMULA_HELPER_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
