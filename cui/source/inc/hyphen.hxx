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
#ifndef INCLUDED_CUI_SOURCE_INC_HYPHEN_HXX
#define INCLUDED_CUI_SOURCE_INC_HYPHEN_HXX

#include <memory>

#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>

class SvxSpellWrapper;

class HyphenEdit : public Edit
{
public:
    HyphenEdit(vcl::Window* pParent);

protected:
    virtual void KeyInput(const KeyEvent &rKEvt) SAL_OVERRIDE;
};

class SvxHyphenWordDialog : public SfxModalDialog
{
    VclPtr<HyphenEdit>         m_pWordEdit;
    VclPtr<PushButton>         m_pLeftBtn;
    VclPtr<PushButton>         m_pRightBtn;
    VclPtr<PushButton>         m_pOkBtn;
    VclPtr<PushButton>         m_pContBtn;
    VclPtr<PushButton>         m_pDelBtn;
    VclPtr<PushButton>         m_pHyphAll;
    VclPtr<CloseButton>        m_pCloseBtn;
    OUString            m_aLabel;
    SvxSpellWrapper     *const m_pHyphWrapper;
    css::uno::Reference< css::linguistic2::XHyphenator >        m_xHyphenator;
    css::uno::Reference< css::linguistic2::XPossibleHyphens >   m_xPossHyph;
    OUString            m_aEditWord;      // aEditWord and aWordEdit.GetText() differ only by the character for the current selected hyphenation position
    OUString            m_aActWord;           // actual word to be hyphenated
    LanguageType        m_nActLanguage;       // and its language
    sal_Int16           m_nMaxHyphenationPos; // right most valid hyphenation pos
    sal_uInt16          m_nHyphPos;
    sal_Int32           m_nOldPos;
    sal_Int32           m_nHyphenationPositionsOffset;
    bool                m_bBusy;

    void            EnableLRBtn_Impl();
    OUString        EraseUnusableHyphens_Impl();

    void            InitControls_Impl();
    void            ContinueHyph_Impl( sal_Int32 nInsPos = -1 ); // continue by default
    sal_uInt16      GetHyphIndex_Impl();

    DECL_LINK_TYPED(Left_Impl, Button*, void);
    DECL_LINK_TYPED(Right_Impl, Button*, void);
    DECL_LINK_TYPED(CutHdl_Impl, Button*, void);
    DECL_LINK_TYPED(ContinueHdl_Impl, Button*, void);
    DECL_LINK_TYPED(DeleteHdl_Impl, Button*, void);
    DECL_LINK_TYPED( HyphenateAllHdl_Impl, Button*, void );
    DECL_LINK_TYPED(CancelHdl_Impl, Button*, void);
    DECL_LINK_TYPED(GetFocusHdl_Impl, Control&, void);

public:
    SvxHyphenWordDialog( const OUString &rWord, LanguageType nLang,
                         vcl::Window* pParent,
                         css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                         SvxSpellWrapper* pWrapper );
    virtual ~SvxHyphenWordDialog();
    virtual void    dispose() SAL_OVERRIDE;

    void            SetWindowTitle( LanguageType nLang );
    void            SelLeft();
    void            SelRight();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
