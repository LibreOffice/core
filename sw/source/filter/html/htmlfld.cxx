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

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include "docsh.hxx"
#include <IDocumentFieldsAccess.hxx>
#include <svtools/htmltokn.h>
#include <svl/zformat.hxx>
#include <unotools/useroptions.hxx>
#include <fmtfld.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <htmlfld.hxx>
#include <swhtml.hxx>

using namespace nsSwDocInfoSubType;
using namespace ::com::sun::star;

struct HTMLNumFormatTableEntry
{
    const sal_Char *pName;
    NfIndexTableOffset eFormat;
};

static HTMLOptionEnum aHTMLFieldTypeTable[] =
{
    { OOO_STRING_SW_HTML_FT_author, RES_AUTHORFLD       },
    { OOO_STRING_SW_HTML_FT_sender, RES_EXTUSERFLD      },
    { "DATE",    RES_DATEFLD            },
    { "TIME",    RES_TIMEFLD            },
    { OOO_STRING_SW_HTML_FT_datetime,RES_DATETIMEFLD        },
    { OOO_STRING_SW_HTML_FT_page,   RES_PAGENUMBERFLD   },
    { OOO_STRING_SW_HTML_FT_docinfo, RES_DOCINFOFLD     },
    { OOO_STRING_SW_HTML_FT_docstat, RES_DOCSTATFLD     },
    { OOO_STRING_SW_HTML_FT_filename,RES_FILENAMEFLD        },
    { nullptr,                0                   }
};

static HTMLNumFormatTableEntry aHTMLDateFieldFormatTable[] =
{
    { "SSYS",       NF_DATE_SYSTEM_SHORT    },
    { "LSYS",       NF_DATE_SYSTEM_LONG     },
    { "DMY",        NF_DATE_SYS_DDMMYY,     },
    { "DMYY",       NF_DATE_SYS_DDMMYYYY,   },
    { "DMMY",       NF_DATE_SYS_DMMMYY,     },
    { "DMMYY",      NF_DATE_SYS_DMMMYYYY,   },
    { "DMMMY",      NF_DATE_DIN_DMMMMYYYY   },
    { "DMMMYY",         NF_DATE_DIN_DMMMMYYYY   },
    { "DDMMY",      NF_DATE_SYS_NNDMMMYY    },
    { "DDMMMY",         NF_DATE_SYS_NNDMMMMYYYY },
    { "DDMMMYY",    NF_DATE_SYS_NNDMMMMYYYY },
    { "DDDMMMY",    NF_DATE_SYS_NNNNDMMMMYYYY },
    { "DDDMMMYY",   NF_DATE_SYS_NNNNDMMMMYYYY },
    { "MY",             NF_DATE_SYS_MMYY        },
    { "MD",             NF_DATE_DIN_MMDD        },
    { "YMD",        NF_DATE_DIN_YYMMDD      },
    { "YYMD",       NF_DATE_DIN_YYYYMMDD    },
    { nullptr,                    NF_NUMERIC_START }
};

static HTMLNumFormatTableEntry aHTMLTimeFieldFormatTable[] =
{
    { "SYS",     NF_TIME_HHMMSS },
    { "SSMM24",      NF_TIME_HHMM },
    { "SSMM12",      NF_TIME_HHMMAMPM },
    { nullptr,                 NF_NUMERIC_START }
};

static HTMLOptionEnum aHTMLPageNumFieldFormatTable[] =
{
    { OOO_STRING_SW_HTML_FF_uletter,     SVX_NUM_CHARS_UPPER_LETTER },
    { OOO_STRING_SW_HTML_FF_lletter,     SVX_NUM_CHARS_LOWER_LETTER },
    { OOO_STRING_SW_HTML_FF_uroman,          SVX_NUM_ROMAN_UPPER },
    { OOO_STRING_SW_HTML_FF_lroman,          SVX_NUM_ROMAN_LOWER },
    { OOO_STRING_SW_HTML_FF_arabic,      SVX_NUM_ARABIC },
    { OOO_STRING_SW_HTML_FF_none,        SVX_NUM_NUMBER_NONE },
    { OOO_STRING_SW_HTML_FF_char,        SVX_NUM_CHAR_SPECIAL },
    { OOO_STRING_SW_HTML_FF_page,        SVX_NUM_PAGEDESC },
    { OOO_STRING_SW_HTML_FF_ulettern,    SVX_NUM_CHARS_UPPER_LETTER_N },
    { OOO_STRING_SW_HTML_FF_llettern,    SVX_NUM_CHARS_LOWER_LETTER_N },
    { nullptr,                     0 }
};

static HTMLOptionEnum aHTMLExtUsrFieldSubTable[] =
{
    { OOO_STRING_SW_HTML_FS_company,         EU_COMPANY },
    { OOO_STRING_SW_HTML_FS_firstname,   EU_FIRSTNAME },
    { OOO_STRING_SW_HTML_FS_name,        EU_NAME },
    { OOO_STRING_SW_HTML_FS_shortcut,    EU_SHORTCUT },
    { OOO_STRING_SW_HTML_FS_street,      EU_STREET },
    { OOO_STRING_SW_HTML_FS_country,      EU_COUNTRY },
    { OOO_STRING_SW_HTML_FS_zip,          EU_ZIP },
    { OOO_STRING_SW_HTML_FS_city,         EU_CITY },
    { OOO_STRING_SW_HTML_FS_title,        EU_TITLE },
    { OOO_STRING_SW_HTML_FS_position,     EU_POSITION },
    { OOO_STRING_SW_HTML_FS_pphone,       EU_PHONE_PRIVATE },
    { OOO_STRING_SW_HTML_FS_cphone,       EU_PHONE_COMPANY },
    { OOO_STRING_SW_HTML_FS_fax,          EU_FAX },
    { OOO_STRING_SW_HTML_FS_email,        EU_EMAIL },
    { OOO_STRING_SW_HTML_FS_state,        EU_STATE },
    { nullptr,                     0 }
};

static HTMLOptionEnum aHTMLAuthorFieldFormatTable[] =
{
    { OOO_STRING_SW_HTML_FF_name,        AF_NAME },
    { OOO_STRING_SW_HTML_FF_shortcut,    AF_SHORTCUT },
    { nullptr,                     0 }
};

static HTMLOptionEnum aHTMLPageNumFieldSubTable[] =
{
    { OOO_STRING_SW_HTML_FS_random,      PG_RANDOM },
    { OOO_STRING_SW_HTML_FS_next,        PG_NEXT },
    { OOO_STRING_SW_HTML_FS_prev,        PG_PREV },
    { nullptr,                     0  }
};

// UGLY: these are extensions of nsSwDocInfoSubType (in inc/docufld.hxx)
//       these are necessary for importing document info fields written by
//       older versions of OOo (< 3.0) which did not have DI_CUSTOM fields
    const SwDocInfoSubType DI_INFO1         =  DI_SUBTYPE_END + 1;
    const SwDocInfoSubType DI_INFO2         =  DI_SUBTYPE_END + 2;
    const SwDocInfoSubType DI_INFO3         =  DI_SUBTYPE_END + 3;
    const SwDocInfoSubType DI_INFO4         =  DI_SUBTYPE_END + 4;

static HTMLOptionEnum aHTMLDocInfoFieldSubTable[] =
{
    { OOO_STRING_SW_HTML_FS_title,   DI_TITEL },
    { OOO_STRING_SW_HTML_FS_theme,   DI_THEMA },
    { OOO_STRING_SW_HTML_FS_keys,    DI_KEYS },
    { OOO_STRING_SW_HTML_FS_comment,  DI_COMMENT },
    { "INFO1",   DI_INFO1 },
    { "INFO2",   DI_INFO2 },
    { "INFO3",   DI_INFO3 },
    { "INFO4",   DI_INFO4 },
    { OOO_STRING_SW_HTML_FS_custom,      DI_CUSTOM },
    { OOO_STRING_SW_HTML_FS_create,      DI_CREATE },
    { OOO_STRING_SW_HTML_FS_change,      DI_CHANGE },
    { nullptr,                 0 }
};

static HTMLOptionEnum aHTMLDocInfoFieldFormatTable[] =
{
    { OOO_STRING_SW_HTML_FF_author,      DI_SUB_AUTHOR },
    { OOO_STRING_SW_HTML_FF_time,    DI_SUB_TIME },
    { OOO_STRING_SW_HTML_FF_date,    DI_SUB_DATE },
    { nullptr,                 0 }
};

static HTMLOptionEnum aHTMLDocStatFieldSubTable[] =
{
    { OOO_STRING_SW_HTML_FS_page,    DS_PAGE },
    { OOO_STRING_SW_HTML_FS_para,    DS_PARA },
    { OOO_STRING_SW_HTML_FS_word,    DS_WORD },
    { OOO_STRING_SW_HTML_FS_char,    DS_CHAR },
    { OOO_STRING_SW_HTML_FS_tbl,     DS_TBL },
    { OOO_STRING_SW_HTML_FS_grf,     DS_GRF },
    { OOO_STRING_SW_HTML_FS_ole,     DS_OLE },
    { nullptr,                 0 }
};

static HTMLOptionEnum aHTMLFileNameFieldFormatTable[] =
{
    { OOO_STRING_SW_HTML_FF_name,       FF_NAME },
    { OOO_STRING_SW_HTML_FF_pathname,   FF_PATHNAME },
    { OOO_STRING_SW_HTML_FF_path,       FF_PATH },
    { OOO_STRING_SW_HTML_FF_name_noext, FF_NAME_NOEXT },
    { nullptr,                    0 }
};

sal_uInt16 SwHTMLParser::GetNumType( const OUString& rStr, sal_uInt16 nDfltType )
{
    sal_uInt16 nType = nDfltType;
    const HTMLOptionEnum *pOptEnums = aHTMLPageNumFieldFormatTable;
    while( pOptEnums->pName )
        if( !rStr.equalsIgnoreAsciiCaseAscii( pOptEnums->pName ) )
            pOptEnums++;
        else
            break;

    if( pOptEnums->pName )
        nType = pOptEnums->nValue;

    return nType;
}

void SwHTMLParser::NewField()
{
    bool bKnownType = false, bFixed = false,
         bHasNumFormat = false, bHasNumValue = false;
    sal_uInt16 nType = 0;
    OUString aValue, aNumFormat, aNumValue, aName;
    const HTMLOption *pSubOption=nullptr, *pFormatOption=nullptr;

    const HTMLOptions& rHTMLOptions = GetOptions();
    size_t i;

    for ( i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_TYPE:
            bKnownType = rOption.GetEnum( nType, aHTMLFieldTypeTable );
            break;
        case HTML_O_SUBTYPE:
            pSubOption = &rOption;
            break;
        case HTML_O_FORMAT:
            pFormatOption = &rOption;
            break;
        case HTML_O_NAME:
            aName = rOption.GetString();
            break;
        case HTML_O_VALUE:
            aValue = rOption.GetString();
            break;
        case HTML_O_SDNUM:
            aNumFormat = rOption.GetString();
            bHasNumFormat = true;
            break;
        case HTML_O_SDVAL:
            aNumValue = rOption.GetString();
            bHasNumValue = true;
            break;
        case HTML_O_SDFIXED:
            bFixed = true;
            break;
        }
    }

    if( !bKnownType )
        return;

    // Autor und Absender werden nur als als variables Feld eingefuegt,
    // wenn man das Dok selbst als letztes geaendert hat oder es noch
    // niemend geandert hat und man das Dok erstellt hat. Sonst
    // wird ein Fixed-Feld daraus gemacht.
    if( !bFixed &&
        (RES_EXTUSERFLD == (RES_FIELDS)nType ||
         RES_AUTHORFLD == (RES_FIELDS)nType) )
    {
        SvtUserOptions aOpt;
        const OUString& rUser = aOpt.GetFullName();
        SwDocShell *pDocShell(m_pDoc->GetDocShell());
        OSL_ENSURE(pDocShell, "no SwDocShell");
        if (pDocShell) {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                pDocShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps(
                xDPS->getDocumentProperties());
            OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");
            const OUString& rChanged = xDocProps->getModifiedBy();
            const OUString& rCreated = xDocProps->getAuthor();
            if( rUser.isEmpty() ||
                (!rChanged.isEmpty() ? rUser != rChanged : rUser != rCreated) )
                bFixed = true;
        }
    }

    sal_uInt16 nWhich = nType;
    if( RES_DATEFLD==nType || RES_TIMEFLD==nType )
        nWhich = RES_DATETIMEFLD;

    SwFieldType* pType = m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( nWhich );
    SwField *pNewField = nullptr;
    bool bInsOnEndTag = false;

    switch( (RES_FIELDS)nType )
    {
    case RES_EXTUSERFLD:
        if( pSubOption )
        {
            sal_uInt16 nSub;
            sal_uLong nFormat = 0;
            if( bFixed )
            {
                nFormat |= AF_FIXED;
                bInsOnEndTag = true;
            }
            if( pSubOption->GetEnum( nSub, aHTMLExtUsrFieldSubTable ) )
                pNewField = new SwExtUserField(static_cast<SwExtUserFieldType*>(pType), nSub, nFormat);
        }
        break;

    case RES_AUTHORFLD:
        {
            sal_uInt16 nFormat = AF_NAME;
            if( pFormatOption )
                pFormatOption->GetEnum( nFormat, aHTMLAuthorFieldFormatTable );
            if( bFixed )
            {
                nFormat |= AF_FIXED;
                bInsOnEndTag = true;
            }

            pNewField = new SwAuthorField(static_cast<SwAuthorFieldType*>(pType), nFormat);
        }
        break;

    case RES_DATEFLD:
    case RES_TIMEFLD:
        {
            sal_uLong nNumFormat = 0;
            sal_Int64 nTime = tools::Time( tools::Time::SYSTEM ).GetTime();
            sal_Int32 nDate = Date( Date::SYSTEM ).GetDate();
            sal_uInt16 nSub = 0;
            bool bValidFormat = false;
            HTMLNumFormatTableEntry * pFormatTable;

            if( RES_DATEFLD==nType )
            {
                nSub = DATEFLD;
                pFormatTable = aHTMLDateFieldFormatTable;
                if( !aValue.isEmpty() )
                    nDate = aValue.toInt32();
            }
            else
            {
                nSub = TIMEFLD;
                pFormatTable = aHTMLTimeFieldFormatTable;
                if( !aValue.isEmpty() )
                    nTime = (sal_uLong)aValue.toInt32();
            }
            if( !aValue.isEmpty() )
                nSub |= FIXEDFLD;

            SvNumberFormatter *pFormatter = m_pDoc->GetNumberFormatter();
            if( pFormatOption )
            {
                const OUString& rFormat = pFormatOption->GetString();
                for( int k = 0; pFormatTable[k].pName; ++k )
                {
                    if( rFormat.equalsIgnoreAsciiCaseAscii( pFormatTable[k].pName ) )
                    {
                        nNumFormat = pFormatter->GetFormatIndex(
                                        pFormatTable[k].eFormat, LANGUAGE_SYSTEM);
                        bValidFormat = true;
                        break;
                    }
                }
            }
            if( !bValidFormat )
                nNumFormat = pFormatter->GetFormatIndex( pFormatTable[i].eFormat,
                                                      LANGUAGE_SYSTEM);

            pNewField = new SwDateTimeField(static_cast<SwDateTimeFieldType *>(pType), nSub, nNumFormat);

            if (nSub & FIXEDFLD)
                static_cast<SwDateTimeField *>(pNewField)->SetDateTime(DateTime(Date(nDate), tools::Time(nTime)));
        }
        break;

    case RES_DATETIMEFLD:
        if( bHasNumFormat )
        {
            sal_uInt16 nSub = 0;

            SvNumberFormatter *pFormatter = m_pDoc->GetNumberFormatter();
            sal_uInt32 nNumFormat;
            LanguageType eLang;
            double dValue = GetTableDataOptionsValNum(
                                nNumFormat, eLang, aNumValue, aNumFormat,
                                *m_pDoc->GetNumberFormatter() );
            short nFormatType = pFormatter->GetType( nNumFormat );
            switch( nFormatType )
            {
            case css::util::NumberFormat::DATE: nSub = DATEFLD; break;
            case css::util::NumberFormat::TIME: nSub = TIMEFLD; break;
            }

            if( nSub )
            {
                if( bHasNumValue )
                    nSub |= FIXEDFLD;

                pNewField = new SwDateTimeField(static_cast<SwDateTimeFieldType *>(pType), nSub, nNumFormat);
                if (bHasNumValue)
                    static_cast<SwDateTimeField *>(pNewField)->SetValue(dValue);
            }
        }
        break;

    case RES_PAGENUMBERFLD:
        if( pSubOption )
        {
            sal_uInt16 nSub;
            if( pSubOption->GetEnum( nSub, aHTMLPageNumFieldSubTable ) )
            {
                sal_uInt16 nFormat = SVX_NUM_PAGEDESC;
                if( pFormatOption )
                    pFormatOption->GetEnum( nFormat, aHTMLPageNumFieldFormatTable );

                short nOff = 0;

                if( (SvxExtNumType)nFormat!=SVX_NUM_CHAR_SPECIAL && !aValue.isEmpty() )
                    nOff = (short)aValue.toInt32();
                else if( (SwPageNumSubType)nSub == PG_NEXT  )
                    nOff = 1;
                else if( (SwPageNumSubType)nSub == PG_PREV  )
                    nOff = -1;

                if( (SvxExtNumType)nFormat==SVX_NUM_CHAR_SPECIAL &&
                    (SwPageNumSubType)nSub==PG_RANDOM )
                    nFormat = SVX_NUM_PAGEDESC;

                pNewField = new SwPageNumberField(static_cast<SwPageNumberFieldType*>(pType), nSub, nFormat, nOff);
                if ((SvxExtNumType)nFormat == SVX_NUM_CHAR_SPECIAL)
                    static_cast<SwPageNumberField *>(pNewField)->SetUserString(aValue);
            }
        }
        break;

    case RES_DOCINFOFLD:
        if( pSubOption )
        {
            sal_uInt16 nSub;
            if( pSubOption->GetEnum( nSub, aHTMLDocInfoFieldSubTable ) )
            {
                sal_uInt16 nExtSub = 0;
                if( DI_CREATE==(SwDocInfoSubType)nSub ||
                    DI_CHANGE==(SwDocInfoSubType)nSub )
                {
                    nExtSub = DI_SUB_AUTHOR;
                    if( pFormatOption )
                        pFormatOption->GetEnum( nExtSub, aHTMLDocInfoFieldFormatTable );
                    nSub |= nExtSub;
                }

                sal_uInt32 nNumFormat = 0;
                double dValue = 0;
                if( bHasNumFormat && (DI_SUB_DATE==nExtSub || DI_SUB_TIME==nExtSub) )
                {
                    LanguageType eLang;
                    dValue = GetTableDataOptionsValNum(
                                    nNumFormat, eLang, aNumValue, aNumFormat,
                                    *m_pDoc->GetNumberFormatter() );
                    bFixed &= bHasNumValue;
                }
                else
                    bHasNumValue = false;

                if( nSub >= DI_INFO1 && nSub <= DI_INFO4 && aName.isEmpty() )
                {
                    // backward compatibility for OOo 2:
                    // map to names stored in AddMetaUserDefined
                    aName = m_InfoNames[nSub - DI_INFO1];
                    nSub = DI_CUSTOM;
                }

                if( bFixed )
                {
                    nSub |= DI_SUB_FIXED;
                    bInsOnEndTag = true;
                }

                pNewField = new SwDocInfoField(static_cast<SwDocInfoFieldType *>(pType), nSub, aName, nNumFormat);
                if (bHasNumValue)
                    static_cast<SwDocInfoField*>(pNewField)->SetValue(dValue);
            }
        }
        break;

    case RES_DOCSTATFLD:
        if( pSubOption )
        {
            sal_uInt16 nSub;
            if( pSubOption->GetEnum( nSub, aHTMLDocStatFieldSubTable ) )
            {
                sal_uInt16 nFormat = SVX_NUM_ARABIC;
                if( pFormatOption )
                    pFormatOption->GetEnum( nFormat, aHTMLPageNumFieldFormatTable );
                pNewField = new SwDocStatField(static_cast<SwDocStatFieldType*>(pType), nSub, nFormat);
                m_bUpdateDocStat |= (DS_PAGE != nFormat);
            }
        }
        break;

    case RES_FILENAMEFLD:
        {
            sal_uInt16 nFormat = FF_NAME;
            if( pFormatOption )
                pFormatOption->GetEnum( nFormat, aHTMLFileNameFieldFormatTable );
            if( bFixed )
            {
                nFormat |= FF_FIXED;
                bInsOnEndTag = true;
            }

            pNewField = new SwFileNameField(static_cast<SwFileNameFieldType*>(pType), nFormat);
        }
        break;
    default:
        ;
    }

    if (pNewField)
    {
        if (bInsOnEndTag)
        {
            m_pField = pNewField;
        }
        else
        {
            m_pDoc->getIDocumentContentOperations().InsertPoolItem(*m_pPam, SwFormatField(*pNewField));
            delete pNewField;
        }
        m_bInField = true;
    }
}

void SwHTMLParser::EndField()
{
    if( m_pField )
    {
        switch( m_pField->Which() )
        {
        case RES_DOCINFOFLD:
            OSL_ENSURE( static_cast<SwDocInfoField*>(m_pField)->IsFixed(),
                    "DokInfo-Feld haette nicht gemerkt werden muessen" );
            static_cast<SwDocInfoField*>(m_pField)->SetExpansion( m_aContents );
            break;

        case RES_EXTUSERFLD:
            OSL_ENSURE( static_cast<SwExtUserField*>(m_pField)->IsFixed(),
                    "ExtUser-Feld haette nicht gemerkt werden muessen" );
            static_cast<SwExtUserField*>(m_pField)->SetExpansion( m_aContents );
            break;

        case RES_AUTHORFLD:
            OSL_ENSURE( static_cast<SwAuthorField*>(m_pField)->IsFixed(),
                    "Author-Feld haette nicht gemerkt werden muessen" );
            static_cast<SwAuthorField*>(m_pField)->SetExpansion( m_aContents );
            break;

        case RES_FILENAMEFLD:
            OSL_ENSURE( static_cast<SwFileNameField*>(m_pField)->IsFixed(),
                    "FileName-Feld haette nicht gemerkt werden muessen" );
            static_cast<SwFileNameField*>(m_pField)->SetExpansion( m_aContents );
            break;
        }

        m_pDoc->getIDocumentContentOperations().InsertPoolItem( *m_pPam, SwFormatField(*m_pField) );
        delete m_pField;
        m_pField = nullptr;
    }

    m_bInField = false;
    m_aContents.clear();
}

void SwHTMLParser::InsertFieldText()
{
    if( m_pField )
    {
        // das aktuelle Textstueck an den Text anhaengen
        m_aContents += aToken;
    }
}

void SwHTMLParser::InsertCommentText( const sal_Char *pTag )
{
    bool bEmpty = m_aContents.isEmpty();
    if( !bEmpty )
        m_aContents += "\n";

    m_aContents += aToken;
    if( bEmpty && pTag )
    {
        OUString aTmp( m_aContents );
        m_aContents = "HTML: <" + OUString( *pTag ) + ">" + aTmp;
    }
}

void SwHTMLParser::InsertComment( const OUString& rComment, const sal_Char *pTag )
{
    OUString aComment( rComment );
    if( pTag )
    {
        aComment += "</";
        aComment += OUString::createFromAscii(pTag);
        aComment += ">";
    }

    // MIB 24.06.97: Wenn ein PostIt nach einen Space eingefuegt
    // werden soll, fuegen wir es vor dem Space ein. Dann gibt es
    // weniger Probleme beim Formatieren (bug #40483#)
    const sal_Int32 nPos = m_pPam->GetPoint()->nContent.GetIndex();
    SwTextNode *pTextNd = m_pPam->GetNode().GetTextNode();
    bool bMoveFwd = false;
    if (nPos>0 && pTextNd && (' ' == pTextNd->GetText()[nPos-1]))
    {
        bMoveFwd = true;

        sal_uLong nNodeIdx = m_pPam->GetPoint()->nNode.GetIndex();
        const sal_Int32 nIdx = m_pPam->GetPoint()->nContent.GetIndex();
        for( auto i = m_aSetAttrTab.size(); i > 0; )
        {
            _HTMLAttr *pAttr = m_aSetAttrTab[--i];
            if( pAttr->GetSttParaIdx() != nNodeIdx ||
                pAttr->GetSttCnt() != nIdx )
                break;

            if( RES_TXTATR_FIELD == pAttr->pItem->Which() &&
                RES_SCRIPTFLD == static_cast<const SwFormatField *>(pAttr->pItem)->GetField()->GetTyp()->Which() )
            {
                bMoveFwd = false;
                break;
            }
        }

        if( bMoveFwd )
            m_pPam->Move( fnMoveBackward );
    }

    SwPostItField aPostItField(
                    static_cast<SwPostItFieldType*>(m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_POSTITFLD )),
                    aEmptyOUStr, aComment, aEmptyOUStr, aEmptyOUStr, DateTime( DateTime::SYSTEM ) );
    InsertAttr( SwFormatField( aPostItField ) );

    if( bMoveFwd )
        m_pPam->Move( fnMoveForward );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
