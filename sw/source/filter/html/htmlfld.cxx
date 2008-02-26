/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlfld.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:17:04 $
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
#include "precompiled_sw.hxx"



#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentInfo.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>

#include "docsh.hxx"

#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif

#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _HTMLFLD_HXX
#include <htmlfld.hxx>
#endif
#ifndef _SWHTML_HXX
#include <swhtml.hxx>
#endif

using namespace nsSwDocInfoSubType;
using namespace ::com::sun::star;

struct HTMLNumFmtTblEntry
{
    const sal_Char *pName;
    NfIndexTableOffset eFmt;
};

static HTMLOptionEnum __FAR_DATA aHTMLFldTypeTable[] =
{
    { sHTML_FT_author,  RES_AUTHORFLD       },
    { sHTML_FT_sender,  RES_EXTUSERFLD      },
    { sHTML_FT_date,    RES_DATEFLD         },
    { sHTML_FT_time,    RES_TIMEFLD         },
    { sHTML_FT_datetime,RES_DATETIMEFLD     },
    { sHTML_FT_page,    RES_PAGENUMBERFLD   },
    { sHTML_FT_docinfo, RES_DOCINFOFLD      },
    { sHTML_FT_docstat, RES_DOCSTATFLD      },
    { sHTML_FT_filename,RES_FILENAMEFLD     },
    { 0,                0                   }
};

static HTMLNumFmtTblEntry __FAR_DATA aHTMLDateFldFmtTable[] =
{
    { sHTML_FF_ssys,        NF_DATE_SYSTEM_SHORT    },
    { sHTML_FF_lsys,        NF_DATE_SYSTEM_LONG     },
    { sHTML_FF_dmy,         NF_DATE_SYS_DDMMYY,     },
    { sHTML_FF_dmyy,        NF_DATE_SYS_DDMMYYYY,   },
    { sHTML_FF_dmmy,        NF_DATE_SYS_DMMMYY,     },
    { sHTML_FF_dmmyy,       NF_DATE_SYS_DMMMYYYY,   },
    { sHTML_FF_dmmmy,       NF_DATE_DIN_DMMMMYYYY   },
    { sHTML_FF_dmmmyy,      NF_DATE_DIN_DMMMMYYYY   },
    { sHTML_FF_ddmmy,       NF_DATE_SYS_NNDMMMYY    },
    { sHTML_FF_ddmmmy,      NF_DATE_SYS_NNDMMMMYYYY },
    { sHTML_FF_ddmmmyy,     NF_DATE_SYS_NNDMMMMYYYY },
    { sHTML_FF_dddmmmy,     NF_DATE_SYS_NNNNDMMMMYYYY },
    { sHTML_FF_dddmmmyy,    NF_DATE_SYS_NNNNDMMMMYYYY },
    { sHTML_FF_my,          NF_DATE_SYS_MMYY        },
    { sHTML_FF_md,          NF_DATE_DIN_MMDD        },
    { sHTML_FF_ymd,         NF_DATE_DIN_YYMMDD      },
    { sHTML_FF_yymd,        NF_DATE_DIN_YYYYMMDD    },
    { 0,                    NF_NUMERIC_START }
};

static HTMLNumFmtTblEntry __FAR_DATA aHTMLTimeFldFmtTable[] =
{
    { sHTML_FF_sys,      NF_TIME_HHMMSS },
    { sHTML_FF_ssmm24,   NF_TIME_HHMM },
    { sHTML_FF_ssmm12,   NF_TIME_HHMMAMPM },
    { 0,                 NF_NUMERIC_START }
};

static HTMLOptionEnum __FAR_DATA aHTMLPageNumFldFmtTable[] =
{
    { sHTML_FF_uletter,      SVX_NUM_CHARS_UPPER_LETTER },
    { sHTML_FF_lletter,      SVX_NUM_CHARS_LOWER_LETTER },
    { sHTML_FF_uroman,       SVX_NUM_ROMAN_UPPER },
    { sHTML_FF_lroman,       SVX_NUM_ROMAN_LOWER },
    { sHTML_FF_arabic,       SVX_NUM_ARABIC },
    { sHTML_FF_none,         SVX_NUM_NUMBER_NONE },
    { sHTML_FF_char,         SVX_NUM_CHAR_SPECIAL },
    { sHTML_FF_page,         SVX_NUM_PAGEDESC },
    { sHTML_FF_ulettern,     SVX_NUM_CHARS_UPPER_LETTER_N },
    { sHTML_FF_llettern,     SVX_NUM_CHARS_LOWER_LETTER_N },
    { 0,                     0 }
};


static HTMLOptionEnum __FAR_DATA aHTMLExtUsrFldSubTable[] =
{
    { sHTML_FS_company,      EU_COMPANY },
    { sHTML_FS_firstname,    EU_FIRSTNAME },
    { sHTML_FS_name,         EU_NAME },
    { sHTML_FS_shortcut,     EU_SHORTCUT },
    { sHTML_FS_street,       EU_STREET },
    { sHTML_FS_country,      EU_COUNTRY },
    { sHTML_FS_zip,          EU_ZIP },
    { sHTML_FS_city,         EU_CITY },
    { sHTML_FS_title,        EU_TITLE },
    { sHTML_FS_position,     EU_POSITION },
    { sHTML_FS_pphone,       EU_PHONE_PRIVATE },
    { sHTML_FS_cphone,       EU_PHONE_COMPANY },
    { sHTML_FS_fax,          EU_FAX },
    { sHTML_FS_email,        EU_EMAIL },
    { sHTML_FS_state,        EU_STATE },
    { 0,                     0 }
};

static HTMLOptionEnum __FAR_DATA aHTMLAuthorFldFmtTable[] =
{
    { sHTML_FF_name,         AF_NAME },
    { sHTML_FF_shortcut,     AF_SHORTCUT },
    { 0,                     0 }
};

static HTMLOptionEnum __FAR_DATA aHTMLPageNumFldSubTable[] =
{
    { sHTML_FS_random,       PG_RANDOM },
    { sHTML_FS_next,         PG_NEXT },
    { sHTML_FS_prev,         PG_PREV },
    { 0,                     0  }
};

// UGLY: these are extensions of nsSwDocInfoSubType (in inc/docufld.hxx)
//       these are necessary for importing document info fields written by
//       older versions of OOo (< 3.0) which did not have DI_CUSTOM fields
    const SwDocInfoSubType DI_INFO1         =  DI_SUBTYPE_END + 1;
    const SwDocInfoSubType DI_INFO2         =  DI_SUBTYPE_END + 2;
    const SwDocInfoSubType DI_INFO3         =  DI_SUBTYPE_END + 3;
    const SwDocInfoSubType DI_INFO4         =  DI_SUBTYPE_END + 4;

static HTMLOptionEnum __FAR_DATA aHTMLDocInfoFldSubTable[] =
{
    { sHTML_FS_title,    DI_TITEL },
    { sHTML_FS_theme,    DI_THEMA },
    { sHTML_FS_keys,     DI_KEYS },
    { sHTML_FS_comment,  DI_COMMENT },
    { sHTML_FS_info1,    DI_INFO1 },
    { sHTML_FS_info2,    DI_INFO2 },
    { sHTML_FS_info3,    DI_INFO3 },
    { sHTML_FS_info4,    DI_INFO4 },
    { sHTML_FS_custom,   DI_CUSTOM },
    { sHTML_FS_create,   DI_CREATE },
    { sHTML_FS_change,   DI_CHANGE },
    { 0,                 0 }
};

static HTMLOptionEnum __FAR_DATA aHTMLDocInfoFldFmtTable[] =
{
    { sHTML_FF_author,   DI_SUB_AUTHOR },
    { sHTML_FF_time,     DI_SUB_TIME },
    { sHTML_FF_date,     DI_SUB_DATE },
    { 0,                 0 }
};

static HTMLOptionEnum __FAR_DATA aHTMLDocStatFldSubTable[] =
{
    { sHTML_FS_page,     DS_PAGE },
    { sHTML_FS_para,     DS_PARA },
    { sHTML_FS_word,     DS_WORD },
    { sHTML_FS_char,     DS_CHAR },
    { sHTML_FS_tbl,      DS_TBL },
    { sHTML_FS_grf,      DS_GRF },
    { sHTML_FS_ole,      DS_OLE },
    { 0,                 0 }
};

static HTMLOptionEnum __FAR_DATA aHTMLFileNameFldFmtTable[] =
{
    { sHTML_FF_name,        FF_NAME },
    { sHTML_FF_pathname,    FF_PATHNAME },
    { sHTML_FF_path,        FF_PATH },
    { sHTML_FF_name_noext,  FF_NAME_NOEXT },
    { 0,                    0 }
};

/*  */

USHORT SwHTMLParser::GetNumType( const String& rStr, USHORT nDfltType )
{
    USHORT nType = nDfltType;
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
    BOOL bKnownType = FALSE, bFixed = FALSE,
         bHasNumFmt = FALSE, bHasNumValue = FALSE;
    USHORT nType = 0;
    String aValue, aNumFmt, aNumValue, aName;
    const HTMLOption *pSubOption=0, *pFmtOption=0;

    const HTMLOptions *pHTMLOptions = GetOptions();
    USHORT i;

    for( i = pHTMLOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pHTMLOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_TYPE:
            bKnownType = pOption->GetEnum( nType, aHTMLFldTypeTable );
            break;
        case HTML_O_SUBTYPE:
            pSubOption = pOption;
            break;
        case HTML_O_FORMAT:
            pFmtOption = pOption;
            break;
        case HTML_O_NAME:
            aName = pOption->GetString();
            break;
        case HTML_O_VALUE:
            aValue = pOption->GetString();
            break;
        case HTML_O_SDNUM:
            aNumFmt = pOption->GetString();
            bHasNumFmt = TRUE;
            break;
        case HTML_O_SDVAL:
            aNumValue = pOption->GetString();
            bHasNumValue = TRUE;
            break;
        case HTML_O_SDFIXED:
            bFixed = TRUE;
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
        DBG_ASSERT(pDocShell, "no SwDocShell");
        if (pDocShell) {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                pDocShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps(
                xDPS->getDocumentProperties());
            DBG_ASSERT(xDocProps.is(), "Doc has no DocumentProperties");
            const String& rChanged = xDocProps->getModifiedBy();
            const String& rCreated = xDocProps->getAuthor();
            if( !rUser.Len() ||
                (rChanged.Len() ? rUser != rChanged : rUser != rCreated) )
                bFixed = TRUE;
        }
    }

    USHORT nWhich = nType;
    if( RES_DATEFLD==nType || RES_TIMEFLD==nType )
        nWhich = RES_DATETIMEFLD;

    SwFieldType* pType = pDoc->GetSysFldType( nWhich );
    SwField *pFld = 0;
    BOOL bInsOnEndTag = FALSE;

    switch( (RES_FIELDS)nType )
    {
    case RES_EXTUSERFLD:
        if( pSubOption )
        {
            USHORT nSub;
            ULONG nFmt = 0;
            if( bFixed )
            {
                nFmt |= AF_FIXED;
                bInsOnEndTag = TRUE;
            }
            if( pSubOption->GetEnum( nSub, aHTMLExtUsrFldSubTable ) )
                pFld = new SwExtUserField( (SwExtUserFieldType*)pType,
                                           nSub, nFmt );
        }
        break;

    case RES_AUTHORFLD:
        {
            USHORT nFmt = AF_NAME;
            if( pFmtOption )
                pFmtOption->GetEnum( nFmt, aHTMLAuthorFldFmtTable );
            if( bFixed )
            {
                nFmt |= AF_FIXED;
                bInsOnEndTag = TRUE;
            }

            pFld = new SwAuthorField( (SwAuthorFieldType *)pType, nFmt );
        }
        break;

    case RES_DATEFLD:
    case RES_TIMEFLD:
        {
            ULONG nNumFmt = 0;
            ULONG nTime = Time().GetTime(), nDate = Date().GetDate();
            USHORT nSub = 0;
            BOOL bValidFmt = FALSE;
            HTMLNumFmtTblEntry * pFmtTbl;

            if( RES_DATEFLD==nType )
            {
                nSub = DATEFLD;
                pFmtTbl = aHTMLDateFldFmtTable;
                if( aValue.Len() )
                    nDate = (ULONG)aValue.ToInt32();
            }
            else
            {
                nSub = TIMEFLD;
                pFmtTbl = aHTMLTimeFldFmtTable;
                if( aValue.Len() )
                    nTime = (ULONG)aValue.ToInt32();
            }
            if( aValue.Len() )
                nSub |= FIXEDFLD;

            SvNumberFormatter *pFormatter = pDoc->GetNumberFormatter();
            if( pFmtOption )
            {
                const String& rFmt = pFmtOption->GetString();
                for( USHORT k = 0; pFmtTbl[k].pName; k++ )
                {
                    if( rFmt.EqualsIgnoreCaseAscii( pFmtTbl[k].pName ) )
                    {
                        nNumFmt = pFormatter->GetFormatIndex(
                                        pFmtTbl[k].eFmt, LANGUAGE_SYSTEM);
                        bValidFmt = TRUE;
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
            USHORT nSub = 0;

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
            USHORT nSub;
            if( pSubOption->GetEnum( nSub, aHTMLPageNumFldSubTable ) )
            {
                USHORT nFmt = SVX_NUM_PAGEDESC;
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
            USHORT nSub;
            if( pSubOption->GetEnum( nSub, aHTMLDocInfoFldSubTable ) )
            {
                USHORT nExtSub = 0;
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
                    bHasNumValue = FALSE;

                if( nSub >= DI_INFO1 && nSub <= DI_INFO4 && aName.Len() == 0 )
                {
                    // backward compatibility: map to names from document info
                    SfxObjectShell* pShell = pDoc->GetDocShell();
                    DBG_ASSERT(pShell, "no object shell");
                    if (pShell) {
                        uno::Reference<document::XDocumentInfoSupplier> xDIS(
                            pShell->GetModel(), uno::UNO_QUERY_THROW);
                        uno::Reference<document::XDocumentInfo> xDocInfo
                            = xDIS->getDocumentInfo();
                        aName = xDocInfo->getUserFieldName(nSub - DI_INFO1);
                    }
                    nSub = DI_CUSTOM;
                }

                if( bFixed )
                {
                    nSub |= DI_SUB_FIXED;
                    bInsOnEndTag = TRUE;
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
            USHORT nSub;
            if( pSubOption->GetEnum( nSub, aHTMLDocStatFldSubTable ) )
            {
                USHORT nFmt = SVX_NUM_ARABIC;
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
            USHORT nFmt = FF_NAME;
            if( pFmtOption )
                pFmtOption->GetEnum( nFmt, aHTMLFileNameFldFmtTable );
            if( bFixed )
            {
                nFmt |= FF_FIXED;
                bInsOnEndTag = TRUE;
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
            pDoc->Insert( *pPam, SwFmtFld(*pFld), 0 );
            delete pFld;
        }
        bInField = TRUE;
    }
}

void SwHTMLParser::EndField()
{
    if( pField )
    {
        switch( pField->Which() )
        {
        case RES_DOCINFOFLD:
            ASSERT( ((SwDocInfoField*)pField)->IsFixed(),
                    "DokInfo-Feld haette nicht gemerkt werden muessen" );
            ((SwDocInfoField*)pField)->SetExpansion( aContents );
            break;

        case RES_EXTUSERFLD:
            ASSERT( ((SwExtUserField*)pField)->IsFixed(),
                    "ExtUser-Feld haette nicht gemerkt werden muessen" );
            ((SwExtUserField*)pField)->SetExpansion( aContents );
            break;

        case RES_AUTHORFLD:
            ASSERT( ((SwAuthorField*)pField)->IsFixed(),
                    "Author-Feld haette nicht gemerkt werden muessen" );
            ((SwAuthorField*)pField)->SetExpansion( aContents );
            break;

        case RES_FILENAMEFLD:
            ASSERT( ((SwFileNameField*)pField)->IsFixed(),
                    "FileName-Feld haette nicht gemerkt werden muessen" );
            ((SwFileNameField*)pField)->SetExpansion( aContents );
            break;
        }

        pDoc->Insert( *pPam, SwFmtFld(*pField), 0 );
        delete pField;
        pField = 0;
    }

    bInField = FALSE;
    aContents.Erase();
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
    BOOL bEmpty = aContents.Len() == 0;
    if( !bEmpty )
        aContents += '\n';

    aContents += aToken;
    if( bEmpty && pTag )
    {
        String aTmp( aContents );
        aContents.AssignAscii( "HTML: <" );
        aContents.AppendAscii( pTag );
        aContents.Append( '>' );
        aContents.Append( aTmp );
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
    BOOL bMoveFwd = FALSE;
    if( nPos>0 && pTxtNd && ' '==pTxtNd->GetTxt().GetChar(nPos-1) )
    {
        bMoveFwd = TRUE;

        ULONG nNodeIdx = pPam->GetPoint()->nNode.GetIndex();
        xub_StrLen nIdx = pPam->GetPoint()->nContent.GetIndex();
        for( USHORT i = aSetAttrTab.Count(); i > 0; )
        {
            _HTMLAttr *pAttr = aSetAttrTab[--i];
            if( pAttr->GetSttParaIdx() != nNodeIdx ||
                pAttr->GetSttCnt() != nIdx )
                break;

            if( RES_TXTATR_FIELD == pAttr->pItem->Which() &&
                RES_SCRIPTFLD == ((const SwFmtFld *)pAttr->pItem)->GetFld()
                                                        ->GetTyp()->Which() )
            {
                bMoveFwd = FALSE;
                break;
            }
        }

        if( bMoveFwd )
            pPam->Move( fnMoveBackward );
    }

    SwPostItField aPostItFld(
                    (SwPostItFieldType*)pDoc->GetSysFldType( RES_POSTITFLD ),
                    aEmptyStr, aComment, DateTime() );
    InsertAttr( SwFmtFld( aPostItFld ) );

    if( bMoveFwd )
        pPam->Move( fnMoveForward );
}

