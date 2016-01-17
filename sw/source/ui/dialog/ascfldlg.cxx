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

#include <hintids.hxx>
#include <rtl/textenc.h>
#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/lingucfg.hxx>
#include <fontcfg.hxx>
#include <swmodule.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/printer.hxx>
#include <editeng/flstitem.hxx>
#include <svx/dlgutil.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <swtypes.hxx>
#include <ascfldlg.hxx>
#include <shellio.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>

#include <dialog.hrc>

#include <vcl/metric.hxx>

using namespace ::com::sun::star;

namespace
{

const sal_Unicode cDialogExtraDataClose = '}';
const char sDialogImpExtraData[] = "EncImpDlg:{";
const char sDialogExpExtraData[] = "EncExpDlg:{";
const sal_Int32 nDialogExtraDataLen = 11;      // 12345678901

}

SwAsciiFilterDlg::SwAsciiFilterDlg( vcl::Window* pParent, SwDocShell& rDocSh,
                                    SvStream* pStream )
    : SfxModalDialog(pParent, "AsciiFilterDialog", "modules/swriter/ui/asciifilterdialog.ui")
    , m_bSaveLineStatus(true)
{
    get(m_pCharSetLB, "charset");
    m_pCharSetLB->SetStyle(m_pCharSetLB->GetStyle() | WB_SORT);
    get(m_pFontFT, "fontft");
    get(m_pFontLB, "font");
    m_pFontLB->SetStyle(m_pFontLB->GetStyle() | WB_SORT);
    get(m_pLanguageFT, "languageft");
    get(m_pLanguageLB, "language");
    m_pLanguageLB->SetStyle(m_pLanguageLB->GetStyle() | WB_SORT);
    get(m_pCRLF_RB, "crlf");
    get(m_pCR_RB, "cr");
    get(m_pLF_RB, "lf");

    SwAsciiOptions aOpt;
    {
        const OUString sFindNm = OUString::createFromAscii(
                                    pStream ? sDialogImpExtraData
                                              : sDialogExpExtraData);
        sal_Int32 nEnd, nStt = GetExtraData().indexOf( sFindNm );
        if( -1 != nStt )
        {
            nStt += nDialogExtraDataLen;
            nEnd = GetExtraData().indexOf( cDialogExtraDataClose, nStt );
            if( -1 != nEnd )
            {
                aOpt.ReadUserData( GetExtraData().copy( nStt, nEnd - nStt ));
                nStt -= nDialogExtraDataLen;
                GetExtraData() = GetExtraData().replaceAt( nStt, nEnd - nStt + 1, "" );
            }
        }
    }

    // read the first chars and check the charset, (language - with L&H)
    if( pStream )
    {
        char aBuffer[ 4098 ];
        const sal_uLong nOldPos = pStream->Tell();
        const sal_uLong nBytesRead = pStream->Read( aBuffer, 4096 );
        pStream->Seek( nOldPos );

        if( nBytesRead <= 4096 )
        {
            aBuffer[ nBytesRead ] = '0';
            aBuffer[ nBytesRead+1 ] = '0';
        }

        bool bCR = false, bLF = false, bNullChar = false;
        for( sal_uLong nCnt = 0; nCnt < nBytesRead; ++nCnt )
            switch( aBuffer[ nCnt ] )
            {
                case 0x0:   bNullChar = true; break;
                case 0xA:   bLF = true; break;
                case 0xD:   bCR = true; break;
                case 0xC:
                case 0x1A:
                case 0x9:   break;
                default:    break;
            }

        if( !bNullChar )
        {
            if( bCR )
            {
                if( bLF )
                {
                    aOpt.SetParaFlags( LINEEND_CRLF );
                }
                else
                {
                    aOpt.SetParaFlags( LINEEND_CR );
                }
            }
            else if( bLF )
            {
                aOpt.SetParaFlags( LINEEND_LF );
            }
        }

        const sal_uInt16 nAppScriptType = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
        SwDoc* pDoc = rDocSh.GetDoc();

        // initialize language
        {
            if( !aOpt.GetLanguage() )
            {
                if(pDoc)
                {
                    const sal_uInt16 nWhich = GetWhichOfScript( RES_CHRATR_LANGUAGE, nAppScriptType);
                    aOpt.SetLanguage( static_cast<const SvxLanguageItem&>(pDoc->
                                GetDefault( nWhich )).GetLanguage());
                }
                else
                {
                    SvtLinguOptions aLinguOpt;
                    SvtLinguConfig().GetOptions( aLinguOpt );
                    switch(nAppScriptType)
                    {
                        case css::i18n::ScriptType::ASIAN:
                            aOpt.SetLanguage(MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, css::i18n::ScriptType::ASIAN));
                        break;
                        case css::i18n::ScriptType::COMPLEX:
                            aOpt.SetLanguage(MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, css::i18n::ScriptType::COMPLEX));
                        break;
                        //SvtScriptType::LATIN:
                        default:
                            aOpt.SetLanguage(MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, css::i18n::ScriptType::LATIN));
                    }
                }
            }

            m_pLanguageLB->SetLanguageList( SvxLanguageListFlags::ALL, true );
            m_pLanguageLB->SelectLanguage( aOpt.GetLanguage() );
        }

        {
            bool bDelPrinter = false;
            VclPtr<SfxPrinter> pPrt = pDoc ? pDoc->getIDocumentDeviceAccess().getPrinter(false) : nullptr;
            if( !pPrt )
            {
                SfxItemSet* pSet = new SfxItemSet( rDocSh.GetPool(),
                            SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                            0 );
                pPrt = VclPtr<SfxPrinter>::Create( pSet );
                bDelPrinter = true;
            }

            // get the set of disctinct available family names
            std::set< OUString > aFontNames;
            int nFontNames = pPrt->GetDevFontCount();
            for( int i = 0; i < nFontNames; i++ )
            {
                vcl::FontInfo aInf( pPrt->GetDevFont( i ) );
                aFontNames.insert( aInf.GetFamilyName() );
            }

            // insert into listbox
            for( std::set< OUString >::const_iterator it = aFontNames.begin();
                 it != aFontNames.end(); ++it )
            {
                m_pFontLB->InsertEntry( *it );
            }

            if( aOpt.GetFontName().isEmpty() )
            {
                LanguageType eLang = aOpt.GetLanguage();
                vcl::Font aTmpFont(OutputDevice::GetDefaultFont(DefaultFontType::FIXED, eLang, GetDefaultFontFlags::OnlyOne, pPrt));
                aOpt.SetFontName(aTmpFont.GetFamilyName());
            }

            m_pFontLB->SelectEntry( aOpt.GetFontName() );

            if( bDelPrinter )
                pPrt.disposeAndClear();
        }

    }
    else
    {
        // hide the unused Controls for Export
        m_pFontFT->Hide();
        m_pFontLB->Hide();
        m_pLanguageFT->Hide();
        m_pLanguageLB->Hide();
    }

    // initialize character set
    m_pCharSetLB->FillFromTextEncodingTable( pStream != nullptr );
    m_pCharSetLB->SelectTextEncoding( aOpt.GetCharSet()  );

    m_pCharSetLB->SetSelectHdl( LINK( this, SwAsciiFilterDlg, CharSetSelHdl ));
    m_pCRLF_RB->SetToggleHdl( LINK( this, SwAsciiFilterDlg, LineEndHdl ));
    m_pLF_RB->SetToggleHdl( LINK( this, SwAsciiFilterDlg, LineEndHdl ));
    m_pCR_RB->SetToggleHdl( LINK( this, SwAsciiFilterDlg, LineEndHdl ));

    SetCRLF( aOpt.GetParaFlags() );

    m_pCRLF_RB->SaveValue();
    m_pLF_RB->SaveValue();
    m_pCR_RB->SaveValue();
}

SwAsciiFilterDlg::~SwAsciiFilterDlg()
{
    disposeOnce();
}

void SwAsciiFilterDlg::dispose()
{
    m_pCharSetLB.clear();
    m_pFontFT.clear();
    m_pFontLB.clear();
    m_pLanguageFT.clear();
    m_pLanguageLB.clear();
    m_pCRLF_RB.clear();
    m_pCR_RB.clear();
    m_pLF_RB.clear();
    SfxModalDialog::dispose();
}


void SwAsciiFilterDlg::FillOptions( SwAsciiOptions& rOptions )
{
    sal_uLong nCCode = m_pCharSetLB->GetSelectTextEncoding();
    OUString sFont;
    LanguageType nLng = 0;
    if( m_pFontLB->IsVisible() )
    {
        sFont = m_pFontLB->GetSelectEntry();
        nLng = m_pLanguageLB->GetSelectLanguage();
    }

    rOptions.SetFontName( sFont );
    rOptions.SetCharSet( rtl_TextEncoding( nCCode ) );
    rOptions.SetLanguage( sal_uInt16( nLng ) );
    rOptions.SetParaFlags( GetCRLF() );

    // save the user settings
    OUString sData;
    rOptions.WriteUserData( sData );
    if (!sData.isEmpty())
    {
        const OUString sFindNm = OUString::createFromAscii(
                                    m_pFontLB->IsVisible() ? sDialogImpExtraData
                                              : sDialogExpExtraData);
        sal_Int32 nEnd, nStt = GetExtraData().indexOf( sFindNm );
        if( -1 != nStt )
        {
            // called twice, so remove "old" settings
            nEnd = GetExtraData().indexOf( cDialogExtraDataClose,
                                            nStt + nDialogExtraDataLen );
            if( -1 != nEnd )
                GetExtraData() = GetExtraData().replaceAt( nStt, nEnd - nStt + 1, "" );
        }
        GetExtraData() += sFindNm + sData + OUStringLiteral1<cDialogExtraDataClose>();
    }
}

void SwAsciiFilterDlg::SetCRLF( LineEnd eEnd )
{
    switch( eEnd )
    {
    case LINEEND_CR:    m_pCR_RB->Check();     break;
    case LINEEND_CRLF:  m_pCRLF_RB->Check();   break;
    case LINEEND_LF:    m_pLF_RB->Check();     break;
    }
}

LineEnd SwAsciiFilterDlg::GetCRLF() const
{
    LineEnd eEnd;
    if( m_pCR_RB->IsChecked() )
        eEnd = LINEEND_CR;
    else if( m_pLF_RB->IsChecked() )
        eEnd = LINEEND_LF;
    else
        eEnd = LINEEND_CRLF;
    return eEnd;
}

IMPL_LINK_TYPED( SwAsciiFilterDlg, CharSetSelHdl, ListBox&, rListBox, void )
{
    SvxTextEncodingBox* pBox = static_cast<SvxTextEncodingBox*>(&rListBox);
    LineEnd eOldEnd = GetCRLF(), eEnd = (LineEnd)-1;
    LanguageType nLng = m_pFontLB->IsVisible()
                    ? m_pLanguageLB->GetSelectLanguage()
                    : LANGUAGE_SYSTEM,
                nOldLng = nLng;

    rtl_TextEncoding nChrSet = pBox->GetSelectTextEncoding();
    if( nChrSet == osl_getThreadTextEncoding() )
        eEnd = GetSystemLineEnd();
    else
    {
        switch( nChrSet )
        {
        case RTL_TEXTENCODING_MS_1252:
#ifdef UNX
            eEnd = LINEEND_LF;
#else
            eEnd = LINEEND_CRLF;                // ANSI
#endif
            break;

        case RTL_TEXTENCODING_APPLE_ROMAN:      // MAC
            eEnd = LINEEND_CR;
            break;

        case RTL_TEXTENCODING_IBM_850:          // DOS
            eEnd = LINEEND_CRLF;
            break;

        case RTL_TEXTENCODING_APPLE_ARABIC:
        case RTL_TEXTENCODING_APPLE_CENTEURO:
        case RTL_TEXTENCODING_APPLE_CROATIAN:
        case RTL_TEXTENCODING_APPLE_CYRILLIC:
        case RTL_TEXTENCODING_APPLE_DEVANAGARI:
        case RTL_TEXTENCODING_APPLE_FARSI:
        case RTL_TEXTENCODING_APPLE_GREEK:
        case RTL_TEXTENCODING_APPLE_GUJARATI:
        case RTL_TEXTENCODING_APPLE_GURMUKHI:
        case RTL_TEXTENCODING_APPLE_HEBREW:
        case RTL_TEXTENCODING_APPLE_ICELAND:
        case RTL_TEXTENCODING_APPLE_ROMANIAN:
        case RTL_TEXTENCODING_APPLE_THAI:
        case RTL_TEXTENCODING_APPLE_TURKISH:
        case RTL_TEXTENCODING_APPLE_UKRAINIAN:
        case RTL_TEXTENCODING_APPLE_CHINSIMP:
        case RTL_TEXTENCODING_APPLE_CHINTRAD:
        case RTL_TEXTENCODING_APPLE_JAPANESE:
        case RTL_TEXTENCODING_APPLE_KOREAN:
            eEnd = LINEEND_CR;
            break;
        }
    }

    m_bSaveLineStatus = false;
    if( eEnd != (LineEnd)-1 )       // changed?
    {
        if( eOldEnd != eEnd )
            SetCRLF( eEnd );
    }
    else
    {
        // restore old user choice (not the automatic!)
        m_pCRLF_RB->Check( m_pCRLF_RB->GetSavedValue() );
        m_pCR_RB->Check( m_pCR_RB->GetSavedValue() );
        m_pLF_RB->Check( m_pLF_RB->GetSavedValue() );
    }
    m_bSaveLineStatus = true;

    if( nOldLng != nLng && m_pFontLB->IsVisible() )
        m_pLanguageLB->SelectLanguage( nLng );
}

IMPL_LINK_TYPED( SwAsciiFilterDlg, LineEndHdl, RadioButton&, rBtn, void )
{
    if( m_bSaveLineStatus )
        rBtn.SaveValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
