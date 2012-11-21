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
#include "chinese_translationdialog.hrc"
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
    : ModalDialog( pParent, TextConversionDlgs_ResId( DLG_CHINESETRANSLATION ) )
    , m_aFL_Direction( this, TextConversionDlgs_ResId( FL_DIRECTION ) )
    , m_aRB_To_Simplified( this, TextConversionDlgs_ResId( RB_TO_SIMPLIFIED ) )
    , m_aRB_To_Traditional( this, TextConversionDlgs_ResId( RB_TO_TRADITIONAL ) )
    , m_aCB_Use_Variants( this, TextConversionDlgs_ResId( CB_USE_VARIANTS ) )
    , m_aFL_Commonterms( this, TextConversionDlgs_ResId( FL_COMMONTERMS ) )
    , m_aCB_Translate_Commonterms( this, TextConversionDlgs_ResId( CB_TRANSLATE_COMMONTERMS ) )
    , m_aPB_Editterms( this, TextConversionDlgs_ResId( PB_EDITTERMS ) )
    , m_aFL_Bottomline( this, TextConversionDlgs_ResId( T_FL_BOTTOMLINE ) )
    , m_aBP_OK( this, TextConversionDlgs_ResId( PB_OK ) )
    , m_aBP_Cancel( this, TextConversionDlgs_ResId( PB_CANCEL ) )
    , m_aBP_Help( this, TextConversionDlgs_ResId( PB_HELP ) )
    , m_pDictionaryDialog(0)
{
    FreeResource();

    m_aRB_To_Simplified.SetHelpId( HID_SVX_CHINESE_TRANSLATION_RB_CONVERSION_TO_SIMPLIFIED );
    m_aRB_To_Traditional.SetHelpId( HID_SVX_CHINESE_TRANSLATION_RB_CONVERSION_TO_TRADITIONAL );
    m_aCB_Use_Variants.SetHelpId( HID_SVX_CHINESE_TRANSLATION_CB_USE_VARIANTS );

    SvtLinguConfig  aLngCfg;
    sal_Bool bValue = sal_Bool();
    Any aAny( aLngCfg.GetProperty( rtl::OUString( UPN_IS_DIRECTION_TO_SIMPLIFIED ) ) );
    aAny >>= bValue;
    if( bValue )
        m_aRB_To_Simplified.Check();
    else
        m_aRB_To_Traditional.Check();

    aAny = aLngCfg.GetProperty( rtl::OUString( UPN_IS_USE_CHARACTER_VARIANTS ) );
    if( aAny >>= bValue )
        m_aCB_Use_Variants.Check( bValue );

    // #117820# (search for other occurrences!)
    // disable and hide that checkbox until it is decided if it is needed or not.
    // If it is to be removed later the respective code needs to be removed as
    // well, otherwise we just have to remove the next lines again.
    m_aCB_Use_Variants.Check( sal_False );
    m_aCB_Use_Variants.Enable( sal_False );
    m_aCB_Use_Variants.Show( sal_False );

    aAny = aLngCfg.GetProperty( rtl::OUString( UPN_IS_TRANSLATE_COMMON_TERMS ) );
    if( aAny >>= bValue )
        m_aCB_Translate_Commonterms.Check( bValue );

    m_aPB_Editterms.SetClickHdl( LINK( this, ChineseTranslationDialog, DictionaryHdl ) );
    m_aRB_To_Simplified.SetClickHdl( LINK( this, ChineseTranslationDialog, DirectionHdl ) );
    m_aRB_To_Traditional.SetClickHdl( LINK( this, ChineseTranslationDialog, DirectionHdl ) );
    m_aCB_Translate_Commonterms.SetClickHdl( LINK( this, ChineseTranslationDialog, CommonTermsHdl ) );
    m_aBP_OK.SetClickHdl( LINK( this, ChineseTranslationDialog, OkHdl ) );

    impl_UpdateVariantsCheckBox();
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
                                          , sal_Bool& rbUseCharacterVariants
                                          , sal_Bool& rbTranslateCommonTerms ) const
{
    rbDirectionToSimplified = m_aRB_To_Simplified.IsChecked();
    rbUseCharacterVariants = m_aCB_Use_Variants.IsChecked();
    rbTranslateCommonTerms = m_aCB_Translate_Commonterms.IsChecked();
}

void ChineseTranslationDialog::impl_UpdateVariantsCheckBox()
{
// #117820# (search for other occurrences!)
//    m_aCB_Use_Variants.Enable( m_aRB_To_Traditional.IsChecked() );
}

IMPL_LINK_NOARG(ChineseTranslationDialog, DirectionHdl)
{
    impl_UpdateVariantsCheckBox();
    return 0;
}

IMPL_LINK_NOARG(ChineseTranslationDialog, CommonTermsHdl)
{
// #117820# (search for other occurrences!)
//    if( m_aCB_Translate_Commonterms.IsChecked() && m_aRB_To_Traditional.IsChecked() )
//        m_aCB_Use_Variants.Check( true );
    return 0;
}

IMPL_LINK_NOARG(ChineseTranslationDialog, OkHdl)
{
    //save settings to configuration
    SvtLinguConfig  aLngCfg;
    Any aAny;
    aAny <<= sal_Bool( !!m_aRB_To_Simplified.IsChecked() );
    aLngCfg.SetProperty( rtl::OUString( UPN_IS_DIRECTION_TO_SIMPLIFIED ), aAny );
    aAny <<= sal_Bool( !!m_aCB_Use_Variants.IsChecked() );
    aLngCfg.SetProperty( rtl::OUString( UPN_IS_USE_CHARACTER_VARIANTS ), aAny );
    aAny <<= sal_Bool( !!m_aCB_Translate_Commonterms.IsChecked() );
    aLngCfg.SetProperty( rtl::OUString( UPN_IS_TRANSLATE_COMMON_TERMS ), aAny );

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
            if( !m_aCB_Translate_Commonterms.IsChecked() )
                nTextConversionOptions = nTextConversionOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;
            if( m_aCB_Use_Variants.IsChecked() )
                nTextConversionOptions = nTextConversionOptions | i18n::TextConversionOption::USE_CHARACTER_VARIANTS;

            m_pDictionaryDialog->setDirectionAndTextConversionOptions( m_aRB_To_Simplified.IsChecked(), nTextConversionOptions );
            m_pDictionaryDialog->Execute();
        }
    }
    return 0;
}

//.............................................................................
} //end namespace
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
