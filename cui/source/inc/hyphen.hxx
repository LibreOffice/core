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

#include <memory>

#include <vcl/weld.hxx>
#include <sfx2/basedlgs.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>

class SvxSpellWrapper;

class SvxHyphenWordDialog : public SfxDialogController
{
    OUString            m_aLabel;
    SvxSpellWrapper     *const m_pHyphWrapper;
    css::uno::Reference< css::linguistic2::XHyphenator >        m_xHyphenator;
    css::uno::Reference< css::linguistic2::XPossibleHyphens >   m_xPossHyph;
    OUString            m_aEditWord;      // aEditWord and aWordEdit.GetText() differ only by the character for the current selected hyphenation position
    OUString            m_aActWord;           // actual word to be hyphenated
    LanguageType        m_nActLanguage;       // and its language
    sal_Int16           m_nMaxHyphenationPos; // right most valid hyphenation pos
    sal_Int32           m_nOldPos;
    sal_Int32           m_nHyphenationPositionsOffset;
    int                 m_nWordEditWidth;
    bool                m_bBusy;

    std::unique_ptr<weld::Entry> m_xWordEdit;
    std::unique_ptr<weld::Button> m_xLeftBtn;
    std::unique_ptr<weld::Button> m_xRightBtn;
    std::unique_ptr<weld::Button> m_xOkBtn;
    std::unique_ptr<weld::Button> m_xContBtn;
    std::unique_ptr<weld::Button> m_xDelBtn;
    std::unique_ptr<weld::Button> m_xHyphAll;
    std::unique_ptr<weld::Button> m_xCloseBtn;

    void            EnableLRBtn_Impl();
    OUString        EraseUnusableHyphens_Impl();

    void            InitControls_Impl();
    void            ContinueHyph_Impl( sal_Int32 nInsPos = -1 ); // continue by default

    void            select_region(int nStart, int nEnd);

    DECL_LINK(Left_Impl, weld::Button&, void);
    DECL_LINK(Right_Impl, weld::Button&, void);
    DECL_LINK(CutHdl_Impl, weld::Button&, void);
    DECL_LINK(ContinueHdl_Impl, weld::Button&, void);
    DECL_LINK(DeleteHdl_Impl, weld::Button&, void);
    DECL_LINK(HyphenateAllHdl_Impl, weld::Button&, void);
    DECL_LINK(CancelHdl_Impl, weld::Button&, void);
    DECL_LINK(GetFocusHdl_Impl, weld::Widget&, void);
    DECL_LINK(CursorChangeHdl_Impl, weld::Entry&, void);

public:
    SvxHyphenWordDialog(const OUString &rWord, LanguageType nLang,
                        weld::Window* pParent,
                        css::uno::Reference<css::linguistic2::XHyphenator> const &xHyphen,
                        SvxSpellWrapper* pWrapper);
    virtual ~SvxHyphenWordDialog() override;

    void            SetWindowTitle( LanguageType nLang );
    bool            SelLeft();
    bool            SelRight();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
