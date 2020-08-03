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

#include <formula/funcutl.hxx>

namespace formula
{

class ParaWin;


class ArgEdit : public RefEdit
{
public:
    ArgEdit(std::unique_ptr<weld::Entry> xControl);

    void    Init(ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                 weld::ScrolledWindow& rArgSlider,
                 ParaWin& rParaWin, sal_uInt16 nArgCount);

protected:
    virtual bool KeyInput(const KeyEvent& rKEvt) override;

private:
    ArgEdit* pEdPrev;
    ArgEdit* pEdNext;
    weld::ScrolledWindow* pSlider;
    ParaWin* pParaWin;
    sal_uInt16 nArgs;
};



class ArgInput final
{
private:
    Link<ArgInput&,void>          aFxClickLink;
    Link<ArgInput&,void>          aFxFocusLink;
    Link<ArgInput&,void>          aEdFocusLink;
    Link<ArgInput&,void>          aEdModifyLink;

    weld::Label*pFtArg;
    weld::Button* pBtnFx;
    ArgEdit* pEdArg;
    RefButton* pRefBtn;

    DECL_LINK( FxBtnClickHdl, weld::Button&, void );
    DECL_LINK( FxBtnFocusHdl, weld::Widget&, void );
    DECL_LINK( EdFocusHdl, RefEdit&, void );
    DECL_LINK( EdModifyHdl, RefEdit&, void );

public:

    ArgInput();

    void        InitArgInput(weld::Label* pftArg,
                             weld::Button* pbtnFx,
                             ArgEdit* pedArg,
                             RefButton* prefBtn);

    void        SetArgName(const OUString &aArg);
    OUString    GetArgName() const;
    void        SetArgNameFont(const vcl::Font&);

    void        SetArgVal(const OUString &aVal);
    OUString    GetArgVal() const;

    void        SelectAll();

    ArgEdit*    GetArgEdPtr() { return pEdArg; }


    void            SetFxClickHdl( const Link<ArgInput&,void>& rLink ) { aFxClickLink = rLink; }

    void            SetFxFocusHdl( const Link<ArgInput&,void>& rLink ) { aFxFocusLink = rLink; }

    void            SetEdFocusHdl( const Link<ArgInput&,void>& rLink ) { aEdFocusLink = rLink; }

    void            SetEdModifyHdl( const Link<ArgInput&,void>& rLink ) { aEdModifyLink = rLink; }

    void Hide();
    void Show();

    void UpdateAccessibleNames();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
