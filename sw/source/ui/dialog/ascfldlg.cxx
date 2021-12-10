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

#include <sal/config.h>

#include <utility>

#include <hintids.hxx>
#include <rtl/textenc.h>
#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/lingucfg.hxx>
#include <unotools/viewoptions.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/langitem.hxx>
#include <swtypes.hxx>
#include <ascfldlg.hxx>
#include <shellio.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>

#include <vcl/metric.hxx>

using namespace ::com::sun::star;

namespace
{

const sal_Unicode cDialogExtraDataClose = '}';
const char sDialogImpExtraData[] = "EncImpDlg:{";
const char sDialogExpExtraData[] = "EncExpDlg:{";
const sal_Int32 nDialogExtraDataLen = 11;      // 12345678901

}

SwAsciiFilterDlg::SwAsciiFilterDlg( weld::Window* pParent, SwDocShell& rDocSh,
                                    SvStream* pStream )
    : SfxDialogController(pParent, "modules/swriter/ui/asciifilterdialog.ui", "AsciiFilterDialog")
    , m_bSaveLineStatus(true)
    , m_xCharSetLB(new SvxTextEncodingBox(m_xBuilder->weld_combo_box("charset")))
    , m_xFontFT(m_xBuilder->weld_label("fontft"))
    , m_xFontLB(m_xBuilder->weld_combo_box("font"))
    , m_xLanguageFT(m_xBuilder->weld_label("languageft"))
    , m_xLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("language")))
    , m_xCRLF_RB(m_xBuilder->weld_radio_button("crlf"))
    , m_xCR_RB(m_xBuilder->weld_radio_button("cr"))
    , m_xLF_RB(m_xBuilder->weld_radio_button("lf"))
    , m_xIncludeBOM_CB(m_xBuilder->weld_check_button("includebom"))
{
    m_xFontLB->make_sorted();

    SwAsciiOptions aOpt;
    {
        SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
        if (aDlgOpt.Exists())
        {
            css::uno::Any aUserItem = aDlgOpt.GetUserItem("UserItem");
            aUserItem >>= m_sExtraData;
        }

        const SfxPoolItem* pItem;
        OUString sAsciiOptions;
        if( rDocSh.GetMedium() != nullptr &&
            rDocSh.GetMedium()->GetItemSet() != nullptr &&
            SfxItemState::SET == rDocSh.GetMedium()->GetItemSet()->GetItemState(
                SID_FILE_FILTEROPTIONS, true, &pItem ))
        {
            sAsciiOptions = static_cast<const SfxStringItem*>(pItem)->GetValue();
        }

        const OUString sFindNm = OUString::createFromAscii(
                                    pStream ? sDialogImpExtraData
                                              : sDialogExpExtraData);
        sal_Int32 nStt = m_sExtraData.indexOf( sFindNm );
        if( -1 != nStt )
        {
            nStt += nDialogExtraDataLen;
            sal_Int32 nEnd = m_sExtraData.indexOf( cDialogExtraDataClose, nStt );
            if( -1 != nEnd )
            {
                if(sAsciiOptions.isEmpty())
                    sAsciiOptions = m_sExtraData.copy(nStt, nEnd - nStt);
                nStt -= nDialogExtraDataLen;
                m_sExtraData = m_sExtraData.replaceAt(nStt, nEnd - nStt + 1, u"");
            }
        }
        if(!sAsciiOptions.isEmpty())
            aOpt.ReadUserData(sAsciiOptions);
    }

    // read the first chars and check the charset, (language - with L&H)
    if( pStream )
    {
        char aBuffer[ 4098 ];
        const sal_uInt64 nOldPos = pStream->Tell();
        const size_t nBytesRead = pStream->ReadBytes(aBuffer, 4096);
        pStream->Seek( nOldPos );

        if( nBytesRead <= 4096 )
        {
            aBuffer[ nBytesRead ] = '0';
            aBuffer[ nBytesRead+1 ] = '0';
        }

        bool bCR = false, bLF = false, bNullChar = false;
        for( sal_uInt64 nCnt = 0; nCnt < nBytesRead; ++nCnt )
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

            m_xLanguageLB->SetLanguageList( SvxLanguageListFlags::ALL, true );
            m_xLanguageLB->set_active_id(aOpt.GetLanguage());
        }

        {
            bool bDelPrinter = false;
            VclPtr<SfxPrinter> pPrt = pDoc ? pDoc->getIDocumentDeviceAccess().getPrinter(false) : nullptr;
            if( !pPrt )
            {
                auto pSet = std::make_unique<SfxItemSetFixed
                            <SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC>>( rDocSh.GetPool() );
                pPrt = VclPtr<SfxPrinter>::Create( std::move(pSet) );
                bDelPrinter = true;
            }

            // get the set of distinct available family names
            std::set< OUString > aFontNames;
            int nFontNames = pPrt->GetFontFaceCollectionCount();
            for( int i = 0; i < nFontNames; i++ )
            {
                FontMetric aFontMetric( pPrt->GetFontMetricFromCollection( i ) );
                aFontNames.insert( aFontMetric.GetFamilyName() );
            }

            // insert into listbox
            for( const auto& rFontName : aFontNames )
            {
                m_xFontLB->append_text(rFontName);
            }

            if( aOpt.GetFontName().isEmpty() )
            {
                LanguageType eLang = aOpt.GetLanguage();
                vcl::Font aTmpFont(OutputDevice::GetDefaultFont(DefaultFontType::FIXED, eLang, GetDefaultFontFlags::OnlyOne, pPrt));
                aOpt.SetFontName(aTmpFont.GetFamilyName());
            }

            m_xFontLB->set_active_text(aOpt.GetFontName());

            if( bDelPrinter )
                pPrt.disposeAndClear();
        }

        // hide the unused Controls for Export
        m_xIncludeBOM_CB->hide();
    }
    else
    {
        // hide the unused Controls for Export
        m_xFontFT->hide();
        m_xFontLB->hide();
        m_xLanguageFT->hide();
        m_xLanguageLB->hide();


        SetIncludeBOM(aOpt.GetIncludeBOM());
        m_xIncludeBOM_CB->save_state();
    }

    // initialize character set
    m_xCharSetLB->FillFromTextEncodingTable( pStream != nullptr );
    m_xCharSetLB->SelectTextEncoding( aOpt.GetCharSet()  );

    m_xCharSetLB->connect_changed( LINK( this, SwAsciiFilterDlg, CharSetSelHdl ));
    m_xCRLF_RB->connect_toggled( LINK( this, SwAsciiFilterDlg, LineEndHdl ));
    m_xLF_RB->connect_toggled( LINK( this, SwAsciiFilterDlg, LineEndHdl ));
    m_xCR_RB->connect_toggled( LINK( this, SwAsciiFilterDlg, LineEndHdl ));

    SetCRLF( aOpt.GetParaFlags() );

    m_xCRLF_RB->save_state();
    m_xLF_RB->save_state();
    m_xCR_RB->save_state();

    UpdateIncludeBOMSensitiveState();
}

SwAsciiFilterDlg::~SwAsciiFilterDlg()
{
    SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetUserItem("UserItem", uno::makeAny(m_sExtraData));
}

void SwAsciiFilterDlg::FillOptions( SwAsciiOptions& rOptions )
{
    sal_uLong nCCode = m_xCharSetLB->GetSelectTextEncoding();
    OUString sFont;
    LanguageType nLng = LANGUAGE_SYSTEM;
    if (m_xFontLB->get_visible())
    {
        sFont = m_xFontLB->get_active_text();
        nLng = m_xLanguageLB->get_active_id();
    }

    rOptions.SetFontName( sFont );
    rOptions.SetCharSet( rtl_TextEncoding( nCCode ) );
    rOptions.SetLanguage( nLng );
    rOptions.SetParaFlags( GetCRLF() );
    rOptions.SetIncludeBOM( GetIncludeBOM() );

    // save the user settings
    OUString sData;
    rOptions.WriteUserData( sData );
    if (sData.isEmpty())
        return;

    const OUString sFindNm = OUString::createFromAscii(
                                m_xFontLB->get_visible() ? sDialogImpExtraData
                                          : sDialogExpExtraData);
    sal_Int32 nStt = m_sExtraData.indexOf( sFindNm );
    if( -1 != nStt )
    {
        // called twice, so remove "old" settings
        sal_Int32 nEnd = m_sExtraData.indexOf( cDialogExtraDataClose,
                                        nStt + nDialogExtraDataLen );
        if( -1 != nEnd )
            m_sExtraData = m_sExtraData.replaceAt( nStt, nEnd - nStt + 1, u"" );
    }
    m_sExtraData += sFindNm + sData + OUStringChar(cDialogExtraDataClose);
}

void SwAsciiFilterDlg::SetCRLF( LineEnd eEnd )
{
    switch (eEnd)
    {
        case LINEEND_CR:
            m_xCR_RB->set_active(true);
            break;
        case LINEEND_CRLF:
            m_xCRLF_RB->set_active(true);
            break;
        case LINEEND_LF:
            m_xLF_RB->set_active(true);
            break;
    }
}

LineEnd SwAsciiFilterDlg::GetCRLF() const
{
    LineEnd eEnd;
    if(m_xCR_RB->get_active())
        eEnd = LINEEND_CR;
    else if (m_xLF_RB->get_active())
        eEnd = LINEEND_LF;
    else
        eEnd = LINEEND_CRLF;
    return eEnd;
}

void SwAsciiFilterDlg::SetIncludeBOM( bool bIncludeBOM )
{
    m_xIncludeBOM_CB->set_state(bIncludeBOM ? TRISTATE_TRUE : TRISTATE_FALSE);
}

bool SwAsciiFilterDlg::GetIncludeBOM() const
{
    return m_xIncludeBOM_CB->get_state() != TRISTATE_FALSE;
}

void SwAsciiFilterDlg::UpdateIncludeBOMSensitiveState()
{
    if (!m_xIncludeBOM_CB->get_visible())
        return;

    switch (m_xCharSetLB->GetSelectTextEncoding())
    {
        case RTL_TEXTENCODING_UTF8:
        case RTL_TEXTENCODING_UCS2:
            m_xIncludeBOM_CB->set_sensitive(true);
            break;
        default:
            m_xIncludeBOM_CB->set_sensitive(false);
            break;
    }
}

IMPL_LINK_NOARG(SwAsciiFilterDlg, CharSetSelHdl, weld::ComboBox&, void)
{
    LineEnd eOldEnd = GetCRLF(), eEnd = LineEnd(-1);
    LanguageType nLng = m_xFontLB->get_visible()
                    ? m_xLanguageLB->get_active_id()
                    : LANGUAGE_SYSTEM,
                nOldLng = nLng;

    rtl_TextEncoding nChrSet = m_xCharSetLB->GetSelectTextEncoding();
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
    if( eEnd != LineEnd(-1) )       // changed?
    {
        if( eOldEnd != eEnd )
            SetCRLF( eEnd );
    }
    else
    {
        // restore old user choice (not the automatic!)
        m_xCRLF_RB->set_state(m_xCRLF_RB->get_saved_state());
        m_xCR_RB->set_state(m_xCR_RB->get_saved_state());
        m_xLF_RB->set_state(m_xLF_RB->get_saved_state());
    }
    m_bSaveLineStatus = true;

    if (nOldLng != nLng && m_xFontLB->get_visible())
        m_xLanguageLB->set_active_id(nLng);

    UpdateIncludeBOMSensitiveState();
}

IMPL_LINK(SwAsciiFilterDlg, LineEndHdl, weld::Toggleable&, rBtn, void)
{
    if (m_bSaveLineStatus)
        rBtn.save_state();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
