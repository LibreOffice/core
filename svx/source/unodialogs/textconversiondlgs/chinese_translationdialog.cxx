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


#include "chinese_translationdialog.hxx"
#include "chinese_dictionarydialog.hxx"
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>

namespace textconversiondlgs
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

ChineseTranslationDialog::ChineseTranslationDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "svx/ui/chineseconversiondialog.ui", "ChineseConversionDialog")
    , m_xBP_OK(m_xBuilder->weld_button("ok"))
    , m_xPB_Editterms(m_xBuilder->weld_button("editterms"))
    , m_xRB_To_Simplified(m_xBuilder->weld_radio_button("tosimplified"))
    , m_xRB_To_Traditional(m_xBuilder->weld_radio_button("totraditional"))
    , m_xCB_Translate_Commonterms(m_xBuilder->weld_check_button("commonterms"))
{
    SvtLinguConfig  aLngCfg;
    bool bValue = false;
    Any aAny( aLngCfg.GetProperty( OUString( UPN_IS_DIRECTION_TO_SIMPLIFIED ) ) );
    aAny >>= bValue;
    if( bValue )
        m_xRB_To_Simplified->set_active(true);
    else
        m_xRB_To_Traditional->set_active(true);

    aAny = aLngCfg.GetProperty( OUString( UPN_IS_TRANSLATE_COMMON_TERMS ) );
    if( aAny >>= bValue )
        m_xCB_Translate_Commonterms->set_active( bValue );

    m_xPB_Editterms->connect_clicked( LINK( this, ChineseTranslationDialog, DictionaryHdl ) );
    m_xBP_OK->connect_clicked( LINK( this, ChineseTranslationDialog, OkHdl ) );
}

ChineseTranslationDialog::~ChineseTranslationDialog()
{
}

void ChineseTranslationDialog::getSettings( bool& rbDirectionToSimplified
                                          , bool& rbTranslateCommonTerms ) const
{
    rbDirectionToSimplified = m_xRB_To_Simplified->get_active();
    rbTranslateCommonTerms = m_xCB_Translate_Commonterms->get_active();
}

IMPL_LINK_NOARG(ChineseTranslationDialog, OkHdl, weld::Button&, void)
{
    //save settings to configuration
    SvtLinguConfig  aLngCfg;
    Any aAny;
    aAny <<= m_xRB_To_Simplified->get_active();
    aLngCfg.SetProperty( OUString( UPN_IS_DIRECTION_TO_SIMPLIFIED ), aAny );
    aAny <<= m_xCB_Translate_Commonterms->get_active();
    aLngCfg.SetProperty( OUString( UPN_IS_TRANSLATE_COMMON_TERMS ), aAny );

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ChineseTranslationDialog, DictionaryHdl, weld::Button&, void)
{
    if( !m_xDictionaryDialog )
        m_xDictionaryDialog.reset(new ChineseDictionaryDialog(m_xDialog.get()));
    sal_Int32 nTextConversionOptions = i18n::TextConversionOption::NONE;
    if (!m_xCB_Translate_Commonterms->get_active())
        nTextConversionOptions = nTextConversionOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;
    m_xDictionaryDialog->setDirectionAndTextConversionOptions(m_xRB_To_Simplified->get_active(), nTextConversionOptions);
    m_xDictionaryDialog->run();
}


} //end namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
