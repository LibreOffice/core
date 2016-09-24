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
#include <vcl/msgbox.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include "helpid.hrc"


namespace textconversiondlgs
{


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

ChineseTranslationDialog::ChineseTranslationDialog( vcl::Window* pParent )
    : ModalDialog(pParent, "ChineseConversionDialog", "svx/ui/chineseconversiondialog.ui")
    , m_pDictionaryDialog(nullptr)
{
    get(m_pBP_OK, "ok");
    get(m_pPB_Editterms, "editterms");
    get(m_pRB_To_Simplified, "tosimplified");
    get(m_pRB_To_Traditional, "totraditional");
    get(m_pCB_Translate_Commonterms, "commonterms");

    SvtLinguConfig  aLngCfg;
    bool bValue = false;
    Any aAny( aLngCfg.GetProperty( OUString( UPN_IS_DIRECTION_TO_SIMPLIFIED ) ) );
    aAny >>= bValue;
    if( bValue )
        m_pRB_To_Simplified->Check();
    else
        m_pRB_To_Traditional->Check();

    aAny = aLngCfg.GetProperty( OUString( UPN_IS_TRANSLATE_COMMON_TERMS ) );
    if( aAny >>= bValue )
        m_pCB_Translate_Commonterms->Check( bValue );

    m_pPB_Editterms->SetClickHdl( LINK( this, ChineseTranslationDialog, DictionaryHdl ) );
    m_pBP_OK->SetClickHdl( LINK( this, ChineseTranslationDialog, OkHdl ) );
}

ChineseTranslationDialog::~ChineseTranslationDialog()
{
    disposeOnce();
}

void ChineseTranslationDialog::dispose()
{
    if(m_pDictionaryDialog)
    {
        if(m_pDictionaryDialog->IsInExecute())
            m_pDictionaryDialog->EndDialog();
    }
    m_pDictionaryDialog.disposeAndClear();
    m_pRB_To_Simplified.clear();
    m_pRB_To_Traditional.clear();
    m_pCB_Translate_Commonterms.clear();
    m_pPB_Editterms.clear();
    m_pBP_OK.clear();
    ModalDialog::dispose();
}

void ChineseTranslationDialog::getSettings( bool& rbDirectionToSimplified
                                          , bool& rbTranslateCommonTerms ) const
{
    rbDirectionToSimplified = m_pRB_To_Simplified->IsChecked();
    rbTranslateCommonTerms = m_pCB_Translate_Commonterms->IsChecked();
}

IMPL_LINK_NOARG_TYPED(ChineseTranslationDialog, OkHdl, Button*, void)
{
    //save settings to configuration
    SvtLinguConfig  aLngCfg;
    Any aAny;
    aAny <<= m_pRB_To_Simplified->IsChecked();
    aLngCfg.SetProperty( OUString( UPN_IS_DIRECTION_TO_SIMPLIFIED ), aAny );
    aAny <<= m_pCB_Translate_Commonterms->IsChecked();
    aLngCfg.SetProperty( OUString( UPN_IS_TRANSLATE_COMMON_TERMS ), aAny );

    EndDialog( RET_OK );
}

IMPL_LINK_NOARG_TYPED(ChineseTranslationDialog, DictionaryHdl, Button*, void)
{
    if( !m_pDictionaryDialog )
    {
        m_pDictionaryDialog = VclPtr<ChineseDictionaryDialog>::Create(this);
    }
    if( m_pDictionaryDialog )
    {
        if( m_pDictionaryDialog->IsInExecute() )
        {
            if( !m_pDictionaryDialog->IsReallyVisible() )
            {
                m_pDictionaryDialog->ToTop();
                m_pDictionaryDialog->GrabFocusToFirstControl();
            }
        }
        else
        {
            sal_Int32 nTextConversionOptions = i18n::TextConversionOption::NONE;
            if( !m_pCB_Translate_Commonterms->IsChecked() )
                nTextConversionOptions = nTextConversionOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;
            m_pDictionaryDialog->setDirectionAndTextConversionOptions( m_pRB_To_Simplified->IsChecked(), nTextConversionOptions );
            m_pDictionaryDialog->Execute();
        }
    }
}


} //end namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
