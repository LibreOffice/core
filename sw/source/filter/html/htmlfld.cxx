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

struct HTMLNumFmtTblEntry
{
    const sal_Char *pName;
    NfIndexTableOffset eFmt;
};

static HTMLOptionEnum aHTMLFldTypeTable[] =
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
    { 0,                0                   }
};

static HTMLNumFmtTblEntry aHTMLDateFldFmtTable[] =
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
    { 0,                    NF_NUMERIC_START }
};

static HTMLNumFmtTblEntry aHTMLTimeFldFmtTable[] =
{
    { "SYS",     NF_TIME_HHMMSS },
    { "SSMM24",      NF_TIME_HHMM },
    { "SSMM12",      NF_TIME_HHMMAMPM },
    { 0,                 NF_NUMERIC_START }
};

static HTMLOptionEnum aHTMLPageNumFldFmtTable[] =
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
    { 0,                     0 }
};


static HTMLOptionEnum aHTMLExtUsrFldSubTable[] =
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
    { 0,                     0 }
};

static HTMLOptionEnum aHTMLAuthorFldFmtTable[] =
{
    { OOO_STRING_SW_HTML_FF_name,        AF_NAME },
    { OOO_STRING_SW_HTML_FF_shortcut,    AF_SHORTCUT },
    { 0,                     0 }
};

static HTMLOptionEnum aHTMLPageNumFldSubTable[] =
{
    { OOO_STRING_SW_HTML_FS_random,      PG_RANDOM },
    { OOO_STRING_SW_HTML_FS_next,        PG_NEXT },
    { OOO_STRING_SW_HTML_FS_prev,        PG_PREV },
    { 0,                     0  }
};

// UGLY: these are extensions of nsSwDocInfoSubType (in inc/docufld.hxx)
//       these are necessary for importing document info fields written by
//       older versions of OOo (< 3.0) which did not have DI_CUSTOM fields
    const SwDocInfoSubType DI_INFO1         =  DI_SUBTYPE_END + 1;
    const SwDocInfoSubType DI_INFO2         =  DI_SUBTYPE_END + 2;
    const SwDocInfoSubType DI_INFO3         =  DI_SUBTYPE_END + 3;
    const SwDocInfoSubType DI_INFO4         =  DI_SUBTYPE_END + 4;

static HTMLOptionEnum aHTMLDocInfoFldSubTable[] =
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
    { 0,                 0 }
};

static HTMLOptionEnum aHTMLDocInfoFldFmtTable[] =
{
    { OOO_STRING_SW_HTML_FF_author,      DI_SUB_AUTHOR },
    { OOO_STRING_SW_HTML_FF_time,    DI_SUB_TIME },
    { OOO_STRING_SW_HTML_FF_date,    DI_SUB_DATE },
    { 0,                 0 }
};

static HTMLOptionEnum aHTMLDocStatFldSubTable[] =
{
    { OOO_STRING_SW_HTML_FS_page,    DS_PAGE },
    { OOO_STRING_SW_HTML_FS_para,    DS_PARA },
    { OOO_STRING_SW_HTML_FS_word,    DS_WORD },
    { OOO_STRING_SW_HTML_FS_char,    DS_CHAR },
    { OOO_STRING_SW_HTML_FS_tbl,     DS_TBL },
    { OOO_STRING_SW_HTML_FS_grf,     DS_GRF },
    { OOO_STRING_SW_HTML_FS_ole,     DS_OLE },
    { 0,                 0 }
};

static HTMLOptionEnum aHTMLFileNameFldFmtTable[] =
{
    { OOO_STRING_SW_HTML_FF_name,       FF_NAME },
    { OOO_STRING_SW_HTML_FF_pathname,   FF_PATHNAME },
    { OOO_STRING_SW_HTML_FF_path,       FF_PATH },
    { OOO_STRING_SW_HTML_FF_name_noext, FF_NAME_NOEXT },
    { 0,                    0 }
};


sal_uInt16 SwHTMLParser::GetNumType( const String& rStr, sal_uInt16 nDfltType )
{
    sal_uInt16 nType = nDfltType;
    const HTMLOptionEnum *pOptEnums = aHTMLPageNumFldFmtTable;
    while( pOptEnums->pName )
        if( !rStr.EqualsIgnoreCaseAscii( pOptEnums->pName ) )
            pOptEnums++;
        else
            break;

    if( pOptEnums->pName )
        nType = pOptEnums->nValue;

    return nType;
}


void SwHTMLParser::NewField()
{
    sal_Bool bKnownType = sal_False, bFixed = sal_False,
         bHasNumFmt = sal_False, bHasNumValue = sal_False;
    sal_uInt16 nType = 0;
    String aValue, aNumFmt, aNumValue, aName;
    const HTMLOption *pSubOption=0, *pFmtOption=0;

    const HTMLOptions& rHTMLOptions = GetOptions();
    size_t i;

    for ( i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_TYPE:
            bKnownType = rOption.GetEnum( nType, aHTMLFldTypeTable );
            break;
        case HTML_O_SUBTYPE:
            pSubOption = &rOption;
            break;
        case HTML_O_FORMAT:
            pFmtOption = &rOption;
            break;
        case HTML_O_NAME:
            aName = rOption.GetString();
            break;
        case HTML_O_VALUE:
            aValue = rOption.GetString();
            break;
        case HTML_O_SDNUM:
            aNumFmt = rOption.GetString();
            bHasNumFmt = sal_True;
            break;
        case HTML_O_SDVAL:
            aNumValue = rOption.GetString();
            bHasNumValue = sal_True;
            break;
        case HTML_O_SDFIXED:
            bFixed = sal_True;
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
        const String& rUser = aOpt.GetFullName();
        SwDocShell *pDocShell(pDoc->GetDocShell());
        OSL_ENSURE(pDocShell, "no SwDocShell");
        if (pDocShell) {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                pDocShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps(
                xDPS->getDocumentProperties());
            OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");
            const String& rChanged = xDocProps->getModifiedBy();
            const String& rCreated = xDocProps->getAuthor();
            if( !rUser.Len() ||
                (rChanged.Len() ? rUser != rChanged : rUser != rCreated) )
                bFixed = sal_True;
        }
    }

    sal_uInt16 nWhich = nType;
    if( RES_DATEFLD==nType || RES_TIMEFLD==nType )
        nWhich = RES_DATETIMEFLD;

    SwFieldType* pType = pDoc->GetSysFldType( nWhich );
    SwField *pFld = 0;
    sal_Bool bInsOnEndTag = sal_False;

    switch( (RES_FIELDS)nType )
    {
    case RES_EXTUSERFLD:
        if( pSubOption )
        {
            sal_uInt16 nSub;
            sal_uLong nFmt = 0;
            if( bFixed )
            {
                nFmt |= AF_FIXED;
                bInsOnEndTag = sal_True;
            }
            if( pSubOption->GetEnum( nSub, aHTMLExtUsrFldSubTable ) )
                pFld = new SwExtUserField( (SwExtUserFieldType*)pType,
                                           nSub, nFmt );
        }
        break;

    case RES_AUTHORFLD:
        {
            sal_uInt16 nFmt = AF_NAME;
            if( pFmtOption )
                pFmtOption->GetEnum( nFmt, aHTMLAuthorFldFmtTable );
            if( bFixed )
            {
                nFmt |= AF_FIXED;
                bInsOnEndTag = sal_True;
            }

            pFld = new SwAuthorField( (SwAuthorFieldType *)pType, nFmt );
        }
        break;

    case RES_DATEFLD:
    case RES_TIMEFLD:
        {
            sal_uLong nNumFmt = 0;
            sal_uLong nTime = Time( Time::SYSTEM ).GetTime(), nDate = Date( Date::SYSTEM ).GetDate();
            sal_uInt16 nSub = 0;
            sal_Bool bValidFmt = sal_False;
            HTMLNumFmtTblEntry * pFmtTbl;

            if( RES_DATEFLD==nType )
            {
                nSub = DATEFLD;
                pFmtTbl = aHTMLDateFldFmtTable;
                if( aValue.Len() )
                    nDate = (sal_uLong)aValue.ToInt32();
            }
            else
            {
                nSub = TIMEFLD;
                pFmtTbl = aHTMLTimeFldFmtTable;
                if( aValue.Len() )
                    nTime = (sal_uLong)aValue.ToInt32();
            }
            if( aValue.Len() )
                nSub |= FIXEDFLD;

            SvNumberFormatter *pFormatter = pDoc->GetNumberFormatter();
            if( pFmtOption )
            {
                const String& rFmt = pFmtOption->GetString();
                for( sal_uInt16 k = 0; pFmtTbl[k].pName; k++ )
                {
                    if( rFmt.EqualsIgnoreCaseAscii( pFmtTbl[k].pName ) )
                    {
                        nNumFmt = pFormatter->GetFormatIndex(
                                        pFmtTbl[k].eFmt, LANGUAGE_SYSTEM);
                        bValidFmt = sal_True;
                        break;
                    }
                }
            }
            if( !bValidFmt )
                nNumFmt = pFormatter->GetFormatIndex( pFmtTbl[i].eFmt,
                                                      LANGUAGE_SYSTEM);

            pFld = new SwDateTimeField( (SwDateTimeFieldType *)pType,
                                          nSub, nNumFmt );

            if (nSub & FIXEDFLD)
                ((SwDateTimeField *)pFld)->SetDateTime( DateTime(Date(nDate), Time(nTime)) );
        }
        break;

    case RES_DATETIMEFLD:
        if( bHasNumFmt )
        {
            sal_uInt16 nSub = 0;

            SvNumberFormatter *pFormatter = pDoc->GetNumberFormatter();
            sal_uInt32 nNumFmt;
            LanguageType eLang;
            double dValue = GetTableDataOptionsValNum(
                                nNumFmt, eLang, aNumValue, aNumFmt,
                                *pDoc->GetNumberFormatter() );
            short nFmtType = pFormatter->GetType( nNumFmt );
            switch( nFmtType )
            {
            case NUMBERFORMAT_DATE: nSub = DATEFLD; break;
            case NUMBERFORMAT_TIME: nSub = TIMEFLD; break;
            }

            if( nSub )
            {
                if( bHasNumValue )
                    nSub |= FIXEDFLD;

                pFld = new SwDateTimeField( (SwDateTimeFieldType *)pType,
                                                  nSub, nNumFmt );
                if( bHasNumValue )
                    ((SwDateTimeField *)pFld)->SetValue( dValue );
            }
        }
        break;

    case RES_PAGENUMBERFLD:
        if( pSubOption )
        {
            sal_uInt16 nSub;
            if( pSubOption->GetEnum( nSub, aHTMLPageNumFldSubTable ) )
            {
                sal_uInt16 nFmt = SVX_NUM_PAGEDESC;
                if( pFmtOption )
                    pFmtOption->GetEnum( nFmt, aHTMLPageNumFldFmtTable );

                short nOff = 0;

                if( (SvxExtNumType)nFmt!=SVX_NUM_CHAR_SPECIAL && aValue.Len() )
                    nOff = (short)aValue.ToInt32();
                else if( (SwPageNumSubType)nSub == PG_NEXT  )
                    nOff = 1;
                else if( (SwPageNumSubType)nSub == PG_PREV  )
                    nOff = -1;

                if( (SvxExtNumType)nFmt==SVX_NUM_CHAR_SPECIAL &&
                    (SwPageNumSubType)nSub==PG_RANDOM )
                    nFmt = SVX_NUM_PAGEDESC;

                pFld = new SwPageNumberField( (SwPageNumberFieldType *)pType, nSub, nFmt, nOff );
                if( (SvxExtNumType)nFmt==SVX_NUM_CHAR_SPECIAL )
                    ((SwPageNumberField *)pFld)->SetUserString( aValue );
            }
        }
        break;

    case RES_DOCINFOFLD:
        if( pSubOption )
        {
            sal_uInt16 nSub;
            if( pSubOption->GetEnum( nSub, aHTMLDocInfoFldSubTable ) )
            {
                sal_uInt16 nExtSub = 0;
                if( DI_CREATE==(SwDocInfoSubType)nSub ||
                    DI_CHANGE==(SwDocInfoSubType)nSub )
                {
                    nExtSub = DI_SUB_AUTHOR;
                    if( pFmtOption )
                        pFmtOption->GetEnum( nExtSub, aHTMLDocInfoFldFmtTable );
                    nSub |= nExtSub;
                }

                sal_uInt32 nNumFmt = 0;
                double dValue = 0;
                if( bHasNumFmt && (DI_SUB_DATE==nExtSub || DI_SUB_TIME==nExtSub) )
                {
                    LanguageType eLang;
                    dValue = GetTableDataOptionsValNum(
                                    nNumFmt, eLang, aNumValue, aNumFmt,
                                    *pDoc->GetNumberFormatter() );
                    bFixed &= bHasNumValue;
                }
                else
                    bHasNumValue = sal_False;

                if( nSub >= DI_INFO1 && nSub <= DI_INFO4 && aName.Len() == 0 )
                {
                    // backward compatibility for OOo 2:
                    // map to names stored in AddMetaUserDefined
                    aName = m_InfoNames[nSub - DI_INFO1];
                    nSub = DI_CUSTOM;
                }

                if( bFixed )
                {
                    nSub |= DI_SUB_FIXED;
                    bInsOnEndTag = sal_True;
                }

                pFld = new SwDocInfoField( (SwDocInfoFieldType *)pType,
                                             nSub, aName, nNumFmt );
                if( bHasNumValue )
                    ((SwDocInfoField*)pFld)->SetValue( dValue );
            }
        }
        break;

    case RES_DOCSTATFLD:
        if( pSubOption )
        {
            sal_uInt16 nSub;
            if( pSubOption->GetEnum( nSub, aHTMLDocStatFldSubTable ) )
            {
                sal_uInt16 nFmt = SVX_NUM_ARABIC;
                if( pFmtOption )
                    pFmtOption->GetEnum( nFmt, aHTMLPageNumFldFmtTable );
                pFld = new SwDocStatField( (SwDocStatFieldType *)pType,
                                             nSub, nFmt );
                bUpdateDocStat |= (DS_PAGE != nFmt);
            }
        }
        break;

    case RES_FILENAMEFLD:
        {
            sal_uInt16 nFmt = FF_NAME;
            if( pFmtOption )
                pFmtOption->GetEnum( nFmt, aHTMLFileNameFldFmtTable );
            if( bFixed )
            {
                nFmt |= FF_FIXED;
                bInsOnEndTag = sal_True;
            }

            pFld = new SwFileNameField( (SwFileNameFieldType *)pType, nFmt );
        }
        break;
    default:
        ;
    }

    if( pFld )
    {
        if( bInsOnEndTag )
        {
            pField = pFld;
        }
        else
        {
            pDoc->InsertPoolItem( *pPam, SwFmtFld(*pFld), 0 );
            delete pFld;
        }
        bInField = sal_True;
    }
}

void SwHTMLParser::EndField()
{
    if( pField )
    {
        switch( pField->Which() )
        {
        case RES_DOCINFOFLD:
            OSL_ENSURE( ((SwDocInfoField*)pField)->IsFixed(),
                    "DokInfo-Feld haette nicht gemerkt werden muessen" );
            ((SwDocInfoField*)pField)->SetExpansion( aContents );
            break;

        case RES_EXTUSERFLD:
            OSL_ENSURE( ((SwExtUserField*)pField)->IsFixed(),
                    "ExtUser-Feld haette nicht gemerkt werden muessen" );
            ((SwExtUserField*)pField)->SetExpansion( aContents );
            break;

        case RES_AUTHORFLD:
            OSL_ENSURE( ((SwAuthorField*)pField)->IsFixed(),
                    "Author-Feld haette nicht gemerkt werden muessen" );
            ((SwAuthorField*)pField)->SetExpansion( aContents );
            break;

        case RES_FILENAMEFLD:
            OSL_ENSURE( ((SwFileNameField*)pField)->IsFixed(),
                    "FileName-Feld haette nicht gemerkt werden muessen" );
            ((SwFileNameField*)pField)->SetExpansion( aContents );
            break;
        }

        pDoc->InsertPoolItem( *pPam, SwFmtFld(*pField), 0 );
        delete pField;
        pField = 0;
    }

    bInField = sal_False;
    aContents = "";
}

void SwHTMLParser::InsertFieldText()
{
    if( pField )
    {
        // das aktuelle Textstueck an den Text anhaengen
        aContents += aToken;
    }
}

void SwHTMLParser::InsertCommentText( const sal_Char *pTag )
{
    sal_Bool bEmpty = aContents.isEmpty();
    if( !bEmpty )
        aContents += "\n";

    aContents += aToken;
    if( bEmpty && pTag )
    {
        OUString aTmp( aContents );
        aContents = "HTML: <" + OUString( *pTag ) + ">" + aTmp;
    }
}

void SwHTMLParser::InsertComment( const String& rComment, const sal_Char *pTag )
{
    String aComment( rComment );
    if( pTag )
    {
        aComment.AppendAscii( "</" );
        aComment.AppendAscii( pTag );
        aComment.Append( '>' );
    }

    // MIB 24.06.97: Wenn ein PostIt nach einen Space eingefuegt
    // werden soll, fuegen wir es vor dem Space ein. Dann gibt es
    // weniger Probleme beim Formatieren (bug #40483#)
    xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
    SwTxtNode *pTxtNd = pPam->GetNode()->GetTxtNode();
    sal_Bool bMoveFwd = sal_False;
    if (nPos>0 && pTxtNd && (' ' == pTxtNd->GetTxt()[nPos-1]))
    {
        bMoveFwd = sal_True;

        sal_uLong nNodeIdx = pPam->GetPoint()->nNode.GetIndex();
        xub_StrLen nIdx = pPam->GetPoint()->nContent.GetIndex();
        for( sal_uInt16 i = aSetAttrTab.size(); i > 0; )
        {
            _HTMLAttr *pAttr = aSetAttrTab[--i];
            if( pAttr->GetSttParaIdx() != nNodeIdx ||
                pAttr->GetSttCnt() != nIdx )
                break;

            if( RES_TXTATR_FIELD == pAttr->pItem->Which() &&
                RES_SCRIPTFLD == ((const SwFmtFld *)pAttr->pItem)->GetFld()
                                                        ->GetTyp()->Which() )
            {
                bMoveFwd = sal_False;
                break;
            }
        }

        if( bMoveFwd )
            pPam->Move( fnMoveBackward );
    }

    SwPostItField aPostItFld(
                    (SwPostItFieldType*)pDoc->GetSysFldType( RES_POSTITFLD ),
                    aEmptyStr, aComment, aEmptyStr, aEmptyStr, DateTime( DateTime::SYSTEM ) );
    InsertAttr( SwFmtFld( aPostItFld ) );

    if( bMoveFwd )
        pPam->Move( fnMoveForward );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
