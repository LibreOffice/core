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
#include "resid.hxx"
#include "chinese_dictionarydialog.hxx"
#include <com/sun/star/i18n/TextConversionOption.hpp>
// header for define RET_OK
#include <vcl/msgbox.hxx>
// header for class SvtLinguConfigItem
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include "helpid.hrc"

//disable compiler warning C4355: 'this' : used in base member initializer list
#ifdef _MSC_VER
#  pragma warning (disable : 4355)
#endif

//.............................................................................
namespace textconversiondlgs
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

ChineseTranslationDialog::ChineseTranslationDialog( Window* pParent )
    : ModalDialog(pParent, "ChineseConversionDialog", "svx/ui/chineseconversiondialog.ui")
    , m_pDictionaryDialog(0)
{
    get(m_pBP_OK, "ok");
    get(m_pPB_Editterms, "editterms");
    get(m_pRB_To_Simplified, "tosimplified");
    get(m_pRB_To_Traditional, "totraditional");
    get(m_pCB_Translate_Commonterms, "commonterms");

    m_pRB_To_Simplified->SetHelpId( HID_SVX_CHINESE_TRANSLATION_RB_CONVERSION_TO_SIMPLIFIED );
    m_pRB_To_Traditional->SetHelpId( HID_SVX_CHINESE_TRANSLATION_RB_CONVERSION_TO_TRADITIONAL );

    SvtLinguConfig  aLngCfg;
    sal_Bool bValue = sal_Bool();
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
    if(m_pDictionaryDialog)
    {
        if(m_pDictionaryDialog->IsInExecute())
            m_pDictionaryDialog->EndDialog();
        delete m_pDictionaryDialog;
    }
}

void ChineseTranslationDialog::getSettings( sal_Bool& rbDirectionToSimplified
                                          , sal_Bool& rbTranslateCommonTerms ) const
{
    rbDirectionToSimplified = m_pRB_To_Simplified->IsChecked();
    rbTranslateCommonTerms = m_pCB_Translate_Commonterms->IsChecked();
}

IMPL_LINK_NOARG(ChineseTranslationDialog, OkHdl)
{
    //save settings to configuration
    SvtLinguConfig  aLngCfg;
    Any aAny;
    aAny <<= sal_Bool( !!m_pRB_To_Simplified->IsChecked() );
    aLngCfg.SetProperty( OUString( UPN_IS_DIRECTION_TO_SIMPLIFIED ), aAny );
    aAny <<= sal_Bool( !!m_pCB_Translate_Commonterms->IsChecked() );
    aLngCfg.SetProperty( OUString( UPN_IS_TRANSLATE_COMMON_TERMS ), aAny );

    EndDialog( RET_OK );
    return 0;
}

IMPL_LINK_NOARG(ChineseTranslationDialog, DictionaryHdl)
{
    if( !m_pDictionaryDialog )
    {
        m_pDictionaryDialog = new ChineseDictionaryDialog(this);
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
    return 0;
}

//.............................................................................
} //end namespace
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
