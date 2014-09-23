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
    HyphenEdit*         m_pWordEdit;
    PushButton*         m_pLeftBtn;
    PushButton*         m_pRightBtn;
    PushButton*         m_pOkBtn;
    PushButton*         m_pContBtn;
    PushButton*         m_pDelBtn;
    PushButton*         m_pHyphAll;
    CloseButton*        m_pCloseBtn;
    OUString            aLabel;
    SvxSpellWrapper*    pHyphWrapper;
    css::uno::Reference< css::linguistic2::XHyphenator >        xHyphenator;
    css::uno::Reference< css::linguistic2::XPossibleHyphens >   xPossHyph;
    OUString            aEditWord;      // aEditWord and aWordEdit.GetText() differ only by the character for the current selected hyphenation position
    OUString            aActWord;           // actual word to be hyphenated
    LanguageType        nActLanguage;       // and its language
    sal_uInt16          nMaxHyphenationPos; // right most valid hyphenation pos
    sal_uInt16          nHyphPos;
    sal_uInt16          nOldPos;
    sal_Int32           nHyphenationPositionsOffset;
    bool            bBusy;


    void            EnableLRBtn_Impl();
    OUString        EraseUnusableHyphens_Impl( css::uno::Reference< css::linguistic2::XPossibleHyphens >  &rxPossHyph, sal_uInt16 nMaxHyphenationPos );

    void            InitControls_Impl();
    void            ContinueHyph_Impl( sal_uInt16 nInsPos = 0 );
    sal_uInt16      GetHyphIndex_Impl();

    DECL_LINK(Left_Impl, void *);
    DECL_LINK(Right_Impl, void *);
    DECL_LINK(CutHdl_Impl, void *);
    DECL_LINK(ContinueHdl_Impl, void *);
    DECL_LINK(DeleteHdl_Impl, void *);
    DECL_LINK( HyphenateAllHdl_Impl, Button* );
    DECL_LINK(CancelHdl_Impl, void *);
    DECL_LINK(GetFocusHdl_Impl, void *);

public:
    SvxHyphenWordDialog( const OUString &rWord, LanguageType nLang,
                         vcl::Window* pParent,
                         css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                         SvxSpellWrapper* pWrapper );
    virtual ~SvxHyphenWordDialog();

    void            SetWindowTitle( LanguageType nLang );
    void            SelLeft();
    void            SelRight();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
