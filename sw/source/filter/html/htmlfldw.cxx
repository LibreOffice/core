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

#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <comphelper/string.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <fmtfld.hxx>
#include <doc.hxx>
#include <breakit.hxx>
#include <ndtxt.hxx>
#include <txtfld.hxx>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include "htmlfld.hxx"
#include "wrthtml.hxx"
#include <rtl/strbuf.hxx>
#include "css1atr.hxx"

using namespace nsSwDocInfoSubType;

const sal_Char *SwHTMLWriter::GetNumFormat( sal_uInt16 nFormat )
{
    const sal_Char *pFormatStr = nullptr;

    switch( static_cast<SvxNumType>(nFormat) )
    {
    case SVX_NUM_CHARS_UPPER_LETTER:    pFormatStr = OOO_STRING_SW_HTML_FF_uletter;    break;
    case SVX_NUM_CHARS_LOWER_LETTER:    pFormatStr = OOO_STRING_SW_HTML_FF_lletter;    break;
    case SVX_NUM_ROMAN_UPPER:           pFormatStr = OOO_STRING_SW_HTML_FF_uroman;         break;
    case SVX_NUM_ROMAN_LOWER:           pFormatStr = OOO_STRING_SW_HTML_FF_lroman;         break;
    case SVX_NUM_ARABIC:                pFormatStr = OOO_STRING_SW_HTML_FF_arabic;     break;
    case SVX_NUM_NUMBER_NONE:           pFormatStr = OOO_STRING_SW_HTML_FF_none;       break;
    case SVX_NUM_CHAR_SPECIAL:          pFormatStr = OOO_STRING_SW_HTML_FF_char;       break;
    case SVX_NUM_PAGEDESC:              pFormatStr = OOO_STRING_SW_HTML_FF_page;       break;
    case SVX_NUM_CHARS_UPPER_LETTER_N:  pFormatStr = OOO_STRING_SW_HTML_FF_ulettern;   break;
    case SVX_NUM_CHARS_LOWER_LETTER_N:  pFormatStr = OOO_STRING_SW_HTML_FF_llettern;   break;
    default:
        ;
    }

    return pFormatStr;
}

static Writer& OutHTML_SwField( Writer& rWrt, const SwField* pField,
                                 const SwTextNode& rTextNd, sal_Int32 nFieldPos )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    const SwFieldType* pFieldTyp = pField->GetTyp();
    SwFieldIds nField = pFieldTyp->Which();
    sal_uLong nFormat = pField->GetFormat();

    const sal_Char *pTypeStr=nullptr, // TYPE
                      *pSubStr=nullptr,   // SUBTYPE
                   *pFormatStr=nullptr;  // FORMAT (SW)
    OUString aValue;              // VALUE (SW)
    bool bNumFormat=false;         // SDNUM (Number-Formatter-Format)
    bool bNumValue=false;       // SDVAL (Number-Formatter-Value)
    double dNumValue = 0.0;     // SDVAL (Number-Formatter-Value)
    bool bFixed=false;          // SDFIXED
    OUString aName;               // NAME (CUSTOM)

    switch( nField )
    {
        case SwFieldIds::ExtUser:
            pTypeStr = OOO_STRING_SW_HTML_FT_sender;
            switch( static_cast<SwExtUserSubType>(pField->GetSubType()) )
            {
                case EU_COMPANY:    pSubStr = OOO_STRING_SW_HTML_FS_company;        break;
                case EU_FIRSTNAME:  pSubStr = OOO_STRING_SW_HTML_FS_firstname;  break;
                case EU_NAME:       pSubStr = OOO_STRING_SW_HTML_FS_name;       break;
                case EU_SHORTCUT:   pSubStr = OOO_STRING_SW_HTML_FS_shortcut;   break;
                case EU_STREET:     pSubStr = OOO_STRING_SW_HTML_FS_street;     break;
                case EU_COUNTRY:    pSubStr = OOO_STRING_SW_HTML_FS_country;     break;
                case EU_ZIP:        pSubStr = OOO_STRING_SW_HTML_FS_zip;         break;
                case EU_CITY:       pSubStr = OOO_STRING_SW_HTML_FS_city;        break;
                case EU_TITLE:      pSubStr = OOO_STRING_SW_HTML_FS_title;       break;
                case EU_POSITION:   pSubStr = OOO_STRING_SW_HTML_FS_position;    break;
                case EU_PHONE_PRIVATE:  pSubStr = OOO_STRING_SW_HTML_FS_pphone;      break;
                case EU_PHONE_COMPANY:  pSubStr = OOO_STRING_SW_HTML_FS_cphone;      break;
                case EU_FAX:        pSubStr = OOO_STRING_SW_HTML_FS_fax;         break;
                case EU_EMAIL:      pSubStr = OOO_STRING_SW_HTML_FS_email;       break;
                case EU_STATE:      pSubStr = OOO_STRING_SW_HTML_FS_state;       break;
                default:
                    ;
            }
            OSL_ENSURE( pSubStr, "unknown sub type for SwExtUserField" );
            bFixed = static_cast<const SwExtUserField*>(pField)->IsFixed();
            break;

        case SwFieldIds::Author:
            pTypeStr = OOO_STRING_SW_HTML_FT_author;
            switch( static_cast<SwAuthorFormat>(nFormat) & 0xff)
            {
                case AF_NAME:     pFormatStr = OOO_STRING_SW_HTML_FF_name;     break;
                case AF_SHORTCUT:  pFormatStr = OOO_STRING_SW_HTML_FF_shortcut;    break;
            }
            OSL_ENSURE( pFormatStr, "unknown format for SwAuthorField" );
            bFixed = static_cast<const SwAuthorField*>(pField)->IsFixed();
            break;

        case SwFieldIds::DateTime:
            pTypeStr = OOO_STRING_SW_HTML_FT_datetime;
            bNumFormat = true;
            if( static_cast<const SwDateTimeField*>(pField)->IsFixed() )
            {
                bNumValue = true;
                dNumValue = static_cast<const SwDateTimeField*>(pField)->GetValue();
            }
            break;

        case SwFieldIds::PageNumber:
            {
                pTypeStr = OOO_STRING_SW_HTML_FT_page;
                SwPageNumSubType eSubType = static_cast<SwPageNumSubType>(pField->GetSubType());
                switch( eSubType )
                {
                    case PG_RANDOM:     pSubStr = OOO_STRING_SW_HTML_FS_random;     break;
                    case PG_NEXT:       pSubStr = OOO_STRING_SW_HTML_FS_next;       break;
                    case PG_PREV:       pSubStr = OOO_STRING_SW_HTML_FS_prev;       break;
                }
                OSL_ENSURE( pSubStr, "unknown sub type for SwPageNumberField" );
                pFormatStr = SwHTMLWriter::GetNumFormat( static_cast< sal_uInt16 >(nFormat) );

                if( static_cast<SvxNumType>(nFormat)==SVX_NUM_CHAR_SPECIAL )
                {
                    aValue = static_cast<const SwPageNumberField *>(pField)->GetUserString();
                }
                else
                {
                    const OUString& rValue = pField->GetPar2();
                    short nValue = static_cast<short>(rValue.toInt32());
                    if( (eSubType == PG_NEXT && nValue!=1) ||
                        (eSubType == PG_PREV && nValue!=-1) ||
                        (eSubType == PG_RANDOM && nValue!=0) )
                    {
                        aValue = rValue;
                    }
                }
            }
            break;
        case SwFieldIds::DocInfo:
            {
                sal_uInt16 nSubType = pField->GetSubType();
                pTypeStr = OOO_STRING_SW_HTML_FT_docinfo;
                sal_uInt16 nExtSubType = nSubType & 0x0f00;
                nSubType &= 0x00ff;

                switch( nSubType )
                {
                    case DI_TITLE:      pSubStr = OOO_STRING_SW_HTML_FS_title;  break;
                    case DI_THEMA:      pSubStr = OOO_STRING_SW_HTML_FS_theme;  break;
                    case DI_KEYS:       pSubStr = OOO_STRING_SW_HTML_FS_keys;   break;
                    case DI_COMMENT:    pSubStr = OOO_STRING_SW_HTML_FS_comment; break;
                    case DI_CREATE:     pSubStr = OOO_STRING_SW_HTML_FS_create;     break;
                    case DI_CHANGE:     pSubStr = OOO_STRING_SW_HTML_FS_change;     break;
                    case DI_CUSTOM:     pSubStr = OOO_STRING_SW_HTML_FS_custom;     break;
                    default:            pTypeStr = nullptr;               break;
                }

                if( DI_CUSTOM == nSubType ) {
                    aName = static_cast<const SwDocInfoField*>(pField)->GetName();
                }

                if( DI_CREATE == nSubType || DI_CHANGE == nSubType )
                {
                    switch( nExtSubType )
                    {
                        case DI_SUB_AUTHOR:
                            pFormatStr = OOO_STRING_SW_HTML_FF_author;
                            break;
                        case DI_SUB_TIME:
                            pFormatStr = OOO_STRING_SW_HTML_FF_time;
                            bNumFormat = true;
                            break;
                        case DI_SUB_DATE:
                            pFormatStr = OOO_STRING_SW_HTML_FF_date;
                            bNumFormat = true;
                            break;
                    }
                }
                bFixed = static_cast<const SwDocInfoField*>(pField)->IsFixed();
                if( bNumFormat )
                {
                    if( bFixed )
                    {
                        // For a fixed field output the num value too.
                        // Fixed fields without number format shouldn't
                        // exist. See below for OSL_ENSURE().
                        dNumValue = static_cast<const SwDocInfoField*>(pField)->GetValue();
                        bNumValue = true;
                    }
                    else if( !nFormat  )
                    {
                        // Non-fixed fields may not have a number format, when
                        // they come from a 4.0-document.
                        bNumFormat = false;
                    }
                }
            }
            break;

        case SwFieldIds::DocStat:
            {
                pTypeStr = OOO_STRING_SW_HTML_FT_docstat;
                sal_uInt16 nSubType = pField->GetSubType();
                switch( nSubType )
                {
                    case DS_PAGE:       pSubStr = OOO_STRING_SW_HTML_FS_page;   break;
                    case DS_PARA:       pSubStr = OOO_STRING_SW_HTML_FS_para;   break;
                    case DS_WORD:       pSubStr = OOO_STRING_SW_HTML_FS_word;   break;
                    case DS_CHAR:       pSubStr = OOO_STRING_SW_HTML_FS_char;   break;
                    case DS_TBL:        pSubStr = OOO_STRING_SW_HTML_FS_tbl;    break;
                    case DS_GRF:        pSubStr = OOO_STRING_SW_HTML_FS_grf;    break;
                    case DS_OLE:        pSubStr = OOO_STRING_SW_HTML_FS_ole;    break;
                    default:            pTypeStr = nullptr;               break;
                }
                pFormatStr = SwHTMLWriter::GetNumFormat( static_cast< sal_uInt16 >(nFormat) );
            }
            break;

        case SwFieldIds::Filename:
            pTypeStr = OOO_STRING_SW_HTML_FT_filename;
            switch( static_cast<SwFileNameFormat>(nFormat & ~FF_FIXED) )
            {
                case FF_NAME:       pFormatStr = OOO_STRING_SW_HTML_FF_name;       break;
                case FF_PATHNAME:   pFormatStr = OOO_STRING_SW_HTML_FF_pathname;   break;
                case FF_PATH:       pFormatStr = OOO_STRING_SW_HTML_FF_path;       break;
                case FF_NAME_NOEXT: pFormatStr = OOO_STRING_SW_HTML_FF_name_noext; break;
                default:
                    ;
            }
            bFixed = static_cast<const SwFileNameField*>(pField)->IsFixed();
            OSL_ENSURE( pFormatStr, "unknown format for SwFileNameField" );
            break;
        default: break;
    }

    // <SDFIELD>-Tag ausgeben
    if( pTypeStr )
    {
        OStringBuffer sOut;
        sOut.append('<');
        sOut.append(OOO_STRING_SVTOOLS_HTML_sdfield).append(' ').
            append(OOO_STRING_SVTOOLS_HTML_O_type).append('=').
            append(pTypeStr);
        if( pSubStr )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_subtype).
                append('=').append(pSubStr);
        }
        if( pFormatStr )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_format).
                append('=').append(pFormatStr);
        }
        if( !aName.isEmpty() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_name).
                append("=\"");
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rWrt.Strm(), aName, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            sOut.append('\"');
        }
        if( !aValue.isEmpty() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_value).
                append("=\"");
            rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rWrt.Strm(), aValue, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            sOut.append('\"');
        }
        if( bNumFormat )
        {
            OSL_ENSURE( nFormat, "number format is 0" );
            sOut.append(HTMLOutFuncs::CreateTableDataOptionsValNum(
                bNumValue, dNumValue, nFormat,
                *rHTMLWrt.m_pDoc->GetNumberFormatter(), rHTMLWrt.m_eDestEnc,
                &rHTMLWrt.m_aNonConvertableCharacters));
        }
        if( bFixed )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_sdfixed);
        }
        sOut.append('>');
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    }

    // output content of the field
    OUString const sExpand( pField->ExpandField(true, nullptr) );
    bool bNeedsCJKProcessing = false;
    if( !sExpand.isEmpty() )
    {
        sal_uInt16 nScriptType = g_pBreakIt->GetBreakIter()->getScriptType( sExpand, 0 );
        sal_Int32 nPos = g_pBreakIt->GetBreakIter()->endOfScript( sExpand, 0,
                                                          nScriptType );

        sal_uInt16 nScript =
            SwHTMLWriter::GetCSS1ScriptForScriptType( nScriptType );
        if( (nPos < sExpand.getLength() && nPos >= 0) || nScript != rHTMLWrt.m_nCSS1Script )
        {
            bNeedsCJKProcessing = true;
        }
    }

    if( bNeedsCJKProcessing )
    {
        //sequence of (start, end) property ranges we want to
        //query
        SfxItemSet aScriptItemSet( rWrt.m_pDoc->GetAttrPool(),
                                   svl::Items<RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                                   RES_CHRATR_POSTURE, RES_CHRATR_POSTURE,
                                   RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT,
                                   RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_WEIGHT>{} );
        rTextNd.GetParaAttr(aScriptItemSet, nFieldPos, nFieldPos+1);

        sal_uInt16 aWesternWhichIds[4] =
            { RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
              RES_CHRATR_POSTURE, RES_CHRATR_WEIGHT };
        sal_uInt16 aCJKWhichIds[4] =
            { RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE,
              RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT };
        sal_uInt16 aCTLWhichIds[4] =
            { RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE,
              RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT };

        sal_uInt16 *pRefWhichIds = nullptr;
        switch( rHTMLWrt.m_nCSS1Script )
        {
        case CSS1_OUTMODE_WESTERN:
            pRefWhichIds = aWesternWhichIds;
            break;
        case CSS1_OUTMODE_CJK:
            pRefWhichIds = aCJKWhichIds;
            break;
        case CSS1_OUTMODE_CTL:
            pRefWhichIds = aCTLWhichIds;
            break;
        }

        sal_Int32 nPos = 0;
        do
        {
            sal_uInt16 nScriptType = g_pBreakIt->GetBreakIter()->getScriptType( sExpand, nPos );
            sal_uInt16 nScript =
                SwHTMLWriter::GetCSS1ScriptForScriptType( nScriptType );
            sal_Int32 nEndPos = g_pBreakIt->GetBreakIter()->endOfScript(
                                    sExpand, nPos, nScriptType );
            sal_Int32 nChunkLen = nEndPos - nPos;
            if( nScript != CSS1_OUTMODE_ANY_SCRIPT &&
                /* #108791# */ nScript != rHTMLWrt.m_nCSS1Script )
            {
                sal_uInt16 *pWhichIds = nullptr;
                switch( nScript )
                {
                case CSS1_OUTMODE_WESTERN:  pWhichIds = aWesternWhichIds; break;
                case CSS1_OUTMODE_CJK:      pWhichIds = aCJKWhichIds; break;
                case CSS1_OUTMODE_CTL:      pWhichIds = aCTLWhichIds; break;
                }

                rHTMLWrt.m_bTagOn = true;

                const SfxPoolItem *aItems[5];
                int nItems = 0;

                assert(pWhichIds && pRefWhichIds);
                if (pWhichIds && pRefWhichIds)
                {
                    for( int i=0; i<4; i++ )
                    {
                        const SfxPoolItem *pRefItem =
                            aScriptItemSet.GetItem( pRefWhichIds[i] );
                        const SfxPoolItem *pItem =
                            aScriptItemSet.GetItem( pWhichIds[i] );
                        if( pRefItem && pItem &&
                            !(0==i ? swhtml_css1atr_equalFontItems( *pRefItem, *pItem )
                                   : *pRefItem == *pItem) )
                        {
                            Out( aHTMLAttrFnTab, *pItem, rHTMLWrt );
                            aItems[nItems++] = pItem;
                        }
                    }
                }

                HTMLOutFuncs::Out_String( rWrt.Strm(), sExpand.copy( nPos, nChunkLen ),
                    rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );

                rHTMLWrt.m_bTagOn = false;
                while( nItems )
                    Out( aHTMLAttrFnTab, *aItems[--nItems], rHTMLWrt );

            }
            else
            {
                HTMLOutFuncs::Out_String( rWrt.Strm(), sExpand.copy( nPos, nChunkLen ),
                    rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
            }
            nPos = nEndPos;
        }
        while( nPos < sExpand.getLength() );
    }
    else
    {
        HTMLOutFuncs::Out_String( rWrt.Strm(), sExpand,
              rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
    }

    // Off-Tag ausgeben
    if( pTypeStr )
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_sdfield, false );

    return rWrt;
}

Writer& OutHTML_SwFormatField( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFormatField & rField = static_cast<const SwFormatField&>(rHt);
    const SwField* pField = rField.GetField();
    const SwFieldType* pFieldTyp = pField->GetTyp();

    if( SwFieldIds::SetExp == pFieldTyp->Which() &&
        (nsSwGetSetExpType::GSE_STRING & pField->GetSubType()) )
    {
        const bool bOn = pFieldTyp->GetName() == "HTML_ON";
        if (!bOn && pFieldTyp->GetName() != "HTML_OFF")
            return rWrt;

        OUString rText(comphelper::string::strip(pField->GetPar2(), ' '));
        rWrt.Strm().WriteChar( '<' );
        if( !bOn )
            rWrt.Strm().WriteChar( '/' );
        // TODO: HTML-Tags are written without entities, that for, characters
        // not contained in the destination encoding are lost!
        OString sTmp(OUStringToOString(rText,
            static_cast<SwHTMLWriter&>(rWrt).m_eDestEnc));
        rWrt.Strm().WriteCharPtr( sTmp.getStr() ).WriteChar( '>' );
    }
    else if( SwFieldIds::Postit == pFieldTyp->Which() )
    {
        // Comments will be written in ANSI character set, but with system
        // line breaks.
        const OUString& rComment = pField->GetPar2();
        bool bWritten = false;

        if( (rComment.getLength() >= 6 && rComment.startsWith("<") && rComment.endsWith(">") &&
             rComment.copy( 1, 4 ).equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_meta) ) ||
            (rComment.getLength() >= 7 &&
             rComment.startsWith( "<!--" ) &&
             rComment.endsWith( "-->" )) )
        {
            // directly output META tags
            OUString sComment(convertLineEnd(rComment, GetSystemLineEnd()));
            // TODO: HTML-Tags are written without entities, that for,
            // characters not contained in the destination encoding are lost!
            OString sTmp(OUStringToOString(sComment,
                static_cast<SwHTMLWriter&>(rWrt).m_eDestEnc));
            rWrt.Strm().WriteCharPtr( sTmp.getStr() );
            bWritten = true;
        }
        else if( rComment.getLength() >= 7 &&
                 rComment.endsWith(">") &&
                 rComment.startsWithIgnoreAsciiCase( "HTML:" ) )
        {
            OUString sComment(comphelper::string::stripStart(rComment.copy(5), ' '));
            if( '<' == sComment[0] )
            {
                sComment = convertLineEnd(sComment, GetSystemLineEnd());
                // TODO: HTML-Tags are written without entities, that for,
                // characters not contained in the destination encoding are
                // lost!
                OString sTmp(OUStringToOString(sComment,
                    static_cast<SwHTMLWriter&>(rWrt).m_eDestEnc));
                rWrt.Strm().WriteCharPtr( sTmp.getStr() );
                bWritten = true;
            }

        }

        if( !bWritten )
        {
            OUString sComment(convertLineEnd(rComment, GetSystemLineEnd()));
            OStringBuffer sOut;
            // TODO: ???
            sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_comment)
                .append(' ').append(OUStringToOString(sComment,
                    static_cast<SwHTMLWriter&>(rWrt).m_eDestEnc)).append(" -->");
            rWrt.Strm().WriteCharPtr( sOut.getStr() );
        }
    }
    else if( SwFieldIds::Script == pFieldTyp->Which() )
    {
        SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
        if( rHTMLWrt.m_bLFPossible )
            rHTMLWrt.OutNewLine( true );

        bool bURL = static_cast<const SwScriptField *>(pField)->IsCodeURL();
        const OUString& rType = pField->GetPar1();
        OUString aContents, aURL;
        if(bURL)
            aURL = pField->GetPar2();
        else
            aContents = pField->GetPar2();

        // otherwise is the script content itself. Since only JavaScript
        // is in fields, it must be JavaScript ...:)
        HTMLOutFuncs::OutScript( rWrt.Strm(), rWrt.GetBaseURL(), aContents, rType, JAVASCRIPT,
                                 aURL, nullptr, nullptr, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );

        if( rHTMLWrt.m_bLFPossible )
            rHTMLWrt.OutNewLine( true );
    }
    else
    {
        const SwTextField *pTextField = rField.GetTextField();
        OSL_ENSURE( pTextField, "Where is the txt fld?" );
        if( pTextField )
            OutHTML_SwField( rWrt, pField, pTextField->GetTextNode(),
                             pTextField->GetStart()  );
    }
    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
