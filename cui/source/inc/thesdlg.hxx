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

#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include <vcl/idle.hxx>
#include <sfx2/basedlgs.hxx>

#include <memory>
#include <stack>

class SvxThesaurusDialog : public SfxDialogController
{
    Idle                    m_aModifyIdle;

    css::uno::Reference< css::linguistic2::XThesaurus >   xThesaurus;
    OUString                aLookUpText;
    LanguageType            nLookUpLanguage;
    std::stack< OUString >  aLookUpHistory;
    bool                    m_bWordFound;

    std::unique_ptr<weld::Button> m_xLeftBtn;
    std::unique_ptr<weld::ComboBox> m_xWordCB;
    std::unique_ptr<weld::TreeView> m_xAlternativesCT;
    std::unique_ptr<weld::Label> m_xNotFound;
    std::unique_ptr<weld::Entry> m_xReplaceEdit;
    std::unique_ptr<weld::ComboBox> m_xLangLB;
    std::unique_ptr<weld::Button> m_xReplaceBtn;

public:
    virtual ~SvxThesaurusDialog() override;

    // Handler
    DECL_LINK( ReplaceBtnHdl_Impl, weld::Button&, void );
    DECL_LINK( LeftBtnHdl_Impl, weld::Button&, void );
    DECL_LINK( LanguageHdl_Impl, weld::ComboBox&, void );
    DECL_LINK( WordSelectHdl_Impl, weld::ComboBox&, void );
    DECL_LINK( AlternativesSelectHdl_Impl, weld::TreeView&, void );
    DECL_LINK( AlternativesDoubleClickHdl_Impl, weld::TreeView&, bool );
    DECL_LINK( SelectFirstHdl_Impl, void*, void );
    DECL_LINK( ReplaceEditHdl_Impl, weld::Entry&, void );
    DECL_LINK( ModifyTimer_Hdl, Timer *, void );
    DECL_LINK( KeyInputHdl, const KeyEvent&, bool );

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XMeaning > >
            queryMeanings_Impl( OUString& rTerm, const css::lang::Locale& rLocale, const css::beans::PropertyValues& rProperties );

    bool    UpdateAlternativesBox_Impl();
    void    LookUp( const OUString &rText );
    void    LookUp_Impl();

public:
    SvxThesaurusDialog(weld::Window* pParent,
                       css::uno::Reference< css::linguistic2::XThesaurus > const & xThesaurus,
                       const OUString &rWord, LanguageType nLanguage);
    void            SetWindowTitle( LanguageType nLanguage );
    OUString        GetWord() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
