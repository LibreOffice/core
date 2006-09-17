/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chinese_translationdialog.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:09:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "chinese_translationdialog.hxx"
#include "chinese_translationdialog.hrc"
#include "resid.hxx"
#include "chinese_dictionarydialog.hxx"

#ifndef _COM_SUN_STAR_I18N_TEXTCONVERSIONOPTION_HPP_
#include <com/sun/star/i18n/TextConversionOption.hpp>
#endif
// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
// header for class SvtLinguConfigItem
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <svtools/linguprops.hxx>
#endif
#ifndef _SVX_HELPID_HRC
#include "helpid.hrc"
#endif

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
    , m_aFL_Direction( this, ResId( FL_DIRECTION ) )
    , m_aRB_To_Simplified( this, ResId( RB_TO_SIMPLIFIED ) )
    , m_aRB_To_Traditional( this, ResId( RB_TO_TRADITIONAL ) )
    , m_aCB_Use_Variants( this, ResId( CB_USE_VARIANTS ) )
    , m_aFL_Commonterms( this, ResId( FL_COMMONTERMS ) )
    , m_aCB_Translate_Commonterms( this, ResId( CB_TRANSLATE_COMMONTERMS ) )
    , m_aPB_Editterms( this, ResId( PB_EDITTERMS ) )
    , m_aFL_Bottomline( this, ResId( FL_BOTTOMLINE ) )
    , m_aBP_OK( this, ResId( PB_OK ) )
    , m_aBP_Cancel( this, ResId( PB_CANCEL ) )
    , m_aBP_Help( this, ResId( PB_HELP ) )
    , m_pDictionaryDialog(0)
{
    FreeResource();

    m_aRB_To_Simplified.SetHelpId( HID_SVX_CHINESE_TRANSLATION_RB_CONVERSION_TO_SIMPLIFIED );
    m_aRB_To_Traditional.SetHelpId( HID_SVX_CHINESE_TRANSLATION_RB_CONVERSION_TO_TRADITIONAL );
    m_aCB_Use_Variants.SetHelpId( HID_SVX_CHINESE_TRANSLATION_CB_USE_VARIANTS );

    SvtLinguConfig  aLngCfg;
    sal_Bool bValue;
    Any aAny( aLngCfg.GetProperty( rtl::OUString::createFromAscii( UPN_IS_DIRECTION_TO_SIMPLIFIED ) ) );
    aAny >>= bValue;
    if( bValue )
        m_aRB_To_Simplified.Check();
    else
        m_aRB_To_Traditional.Check();

    aAny = aLngCfg.GetProperty( rtl::OUString::createFromAscii( UPN_IS_USE_CHARACTER_VARIANTS ) );
    if( aAny >>= bValue )
        m_aCB_Use_Variants.Check( bValue );

    // #117820# (search for other occurences!)
    // disable and hide that checkbox until it is decided if it is needed or not.
    // If it is to be removed later the respective code needs to be removed as
    // well, otherwise we just have to remove the next lines again.
    m_aCB_Use_Variants.Check( sal_False );
    m_aCB_Use_Variants.Enable( sal_False );
    m_aCB_Use_Variants.Show( sal_False );

    aAny = aLngCfg.GetProperty( rtl::OUString::createFromAscii( UPN_IS_TRANSLATE_COMMON_TERMS ) );
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
// #117820# (search for other occurences!)
//    m_aCB_Use_Variants.Enable( m_aRB_To_Traditional.IsChecked() );
}

IMPL_LINK( ChineseTranslationDialog, DirectionHdl, void*, EMPTYARG )
{
    impl_UpdateVariantsCheckBox();
    return 0;
}

IMPL_LINK( ChineseTranslationDialog, CommonTermsHdl, void*, EMPTYARG )
{
// #117820# (search for other occurences!)
//    if( m_aCB_Translate_Commonterms.IsChecked() && m_aRB_To_Traditional.IsChecked() )
//        m_aCB_Use_Variants.Check( true );
    return 0;
}

IMPL_LINK( ChineseTranslationDialog, OkHdl, void*, EMPTYARG )
{
    //save settings to configuration
    SvtLinguConfig  aLngCfg;
    Any aAny;
    aAny <<= sal_Bool( !!m_aRB_To_Simplified.IsChecked() );
    aLngCfg.SetProperty( rtl::OUString::createFromAscii( UPN_IS_DIRECTION_TO_SIMPLIFIED ), aAny );
    aAny <<= sal_Bool( !!m_aCB_Use_Variants.IsChecked() );
    aLngCfg.SetProperty( rtl::OUString::createFromAscii( UPN_IS_USE_CHARACTER_VARIANTS ), aAny );
    aAny <<= sal_Bool( !!m_aCB_Translate_Commonterms.IsChecked() );
    aLngCfg.SetProperty( rtl::OUString::createFromAscii( UPN_IS_TRANSLATE_COMMON_TERMS ), aAny );

    EndDialog( RET_OK );
    return 0;
}

IMPL_LINK( ChineseTranslationDialog, DictionaryHdl, void*, EMPTYARG )
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
