/*************************************************************************
 *
 *  $RCSfile: htmlfldw.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:54:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/



#pragma hdrstop
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif
#include "fldbas.hxx"
#include "docufld.hxx"
#include "flddat.hxx"
#include "htmlfld.hxx"
#include "wrthtml.hxx"

//-----------------------------------------------------------------------

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_author, "AUTHOR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_sender, "SENDER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_date, "DATE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_time, "TIME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_datetime, "DATETIME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_page, "PAGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_docinfo, "DOCINFO" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_docstat, "DOCSTAT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FT_filename, "FILENAME" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_company, "COMPANY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_firstname, "FIRSTNAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_name, "NAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_shortcut, "SHORTCUT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_street, "STREET" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_country, "COUNTRY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_zip, "ZIP" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_city, "CITY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_title, "TITLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_position, "POSITION" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_pphone, "PPHONE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_cphone, "CPHONE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_fax, "FAX" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_email, "EMAIL" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_state, "STATE" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_random, "RANDOM" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_next, "NEXT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_prev, "PREV" );

//sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_title, "TITLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_theme, "THEME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_keys, "KEYS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_comment, "COMMENT" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_info1, "INFO1" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_info2, "INFO2" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_info3, "INFO3" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_info4, "INFO4" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_create, "CREATE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_change, "CHANGE" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_page, "PAGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_para, "PARAGRAPH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_word, "WORD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_char, "CHAR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_tbl, "TABLE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_grf, "GRAPHIC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FS_ole, "OLE" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_name, "NAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_shortcut, "SHORTCUT" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_ssys, "SSYS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_lsys, "LSYS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_dmy, "DMY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_dmyy, "DMYY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_dmmy, "DMMY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_dmmyy, "DMMYY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_dmmmy, "DMMMY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_dmmmyy, "DMMMYY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_ddmmy, "DDMMY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_ddmmmy, "DDMMMY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_ddmmmyy, "DDMMMYY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_dddmmmy, "DDDMMMY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_dddmmmyy, "DDDMMMYY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_my, "MY" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_md, "MD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_ymd, "YMD" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_yymd, "YYMD" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_sys, "SYS" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_ssmm24, "SSMM24" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_ssmm12, "SSMM12" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_uletter, "ULETTER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_lletter, "LLETTER" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_uroman, "UROMAN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_lroman, "LROMAN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_arabic, "ARABIC" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_none, "NONE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_char, "CHAR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_page, "PAGE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_ulettern, "ULETTERN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_llettern, "LLETTERN" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_author, "AUTHOR" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_time, "TIME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_date, "DATE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_all, "ALL" );

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_pathname, "PATHNAME" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_path, "PATH" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_FF_name_noext, "NAME-NOEXT" );


const sal_Char *SwHTMLWriter::GetNumFormat( USHORT nFmt )
{
    const sal_Char *pFmtStr = 0;

    switch( (SvxExtNumType)nFmt )
    {
    case SVX_NUM_CHARS_UPPER_LETTER:    pFmtStr = sHTML_FF_uletter;     break;
    case SVX_NUM_CHARS_LOWER_LETTER:    pFmtStr = sHTML_FF_lletter;     break;
    case SVX_NUM_ROMAN_UPPER:           pFmtStr = sHTML_FF_uroman;      break;
    case SVX_NUM_ROMAN_LOWER:           pFmtStr = sHTML_FF_lroman;      break;
    case SVX_NUM_ARABIC:                pFmtStr = sHTML_FF_arabic;      break;
    case SVX_NUM_NUMBER_NONE:           pFmtStr = sHTML_FF_none;        break;
    case SVX_NUM_CHAR_SPECIAL:          pFmtStr = sHTML_FF_char;        break;
    case SVX_NUM_PAGEDESC:              pFmtStr = sHTML_FF_page;        break;
    case SVX_NUM_CHARS_UPPER_LETTER_N:  pFmtStr = sHTML_FF_ulettern;    break;
    case SVX_NUM_CHARS_LOWER_LETTER_N:  pFmtStr = sHTML_FF_llettern;    break;
    }

    return pFmtStr;
}

extern BOOL lcl_css1atr_equalFontItems( const SfxPoolItem& r1, const SfxPoolItem& r2 );
static Writer& OutHTML_SwField( Writer& rWrt, const SwField* pFld,
                                 const SwTxtNode& rTxtNd, xub_StrLen nFldPos )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const SwFieldType* pFldTyp = pFld->GetTyp();
    USHORT nField = pFldTyp->Which();
    ULONG nFmt = pFld->GetFormat();

    const sal_Char *pTypeStr=0, // TYPE
                      *pSubStr=0,   // SUBTYPE
                   *pFmtStr=0;  // FORMAT (SW)
    String aValue;              // VALUE (SW)
    BOOL bNumFmt=FALSE;         // SDNUM (Number-Formatter-Format)
    BOOL bNumValue=FALSE;       // SDVAL (Number-Formatter-Value)
    double dNumValue;           // SDVAL (Number-Formatter-Value)
    BOOL bFixed=FALSE;          // SDFIXED

    switch( nField )
    {
        case RES_EXTUSERFLD:
            pTypeStr = sHTML_FT_sender;
            switch( (SwExtUserSubType)pFld->GetSubType() )
            {
                case EU_COMPANY:    pSubStr = sHTML_FS_company;     break;
                case EU_FIRSTNAME:  pSubStr = sHTML_FS_firstname;   break;
                case EU_NAME:       pSubStr = sHTML_FS_name;        break;
                case EU_SHORTCUT:   pSubStr = sHTML_FS_shortcut;    break;
                case EU_STREET:     pSubStr = sHTML_FS_street;      break;
                case EU_COUNTRY:    pSubStr = sHTML_FS_country;     break;
                case EU_ZIP:        pSubStr = sHTML_FS_zip;         break;
                case EU_CITY:       pSubStr = sHTML_FS_city;        break;
                case EU_TITLE:      pSubStr = sHTML_FS_title;       break;
                case EU_POSITION:   pSubStr = sHTML_FS_position;    break;
                case EU_PHONE_PRIVATE:  pSubStr = sHTML_FS_pphone;      break;
                case EU_PHONE_COMPANY:  pSubStr = sHTML_FS_cphone;      break;
                case EU_FAX:        pSubStr = sHTML_FS_fax;         break;
                case EU_EMAIL:      pSubStr = sHTML_FS_email;       break;
                case EU_STATE:      pSubStr = sHTML_FS_state;       break;
            }
            ASSERT( pSubStr, "ubekannter Subtyp fuer SwExtUserField" );
            bFixed = ((const SwExtUserField*)pFld)->IsFixed();
            break;

        case RES_AUTHORFLD:
            pTypeStr = sHTML_FT_author;
            switch( (SwAuthorFormat)nFmt & 0xff)
            {
                case AF_NAME:     pFmtStr = sHTML_FF_name;      break;
                case AF_SHORTCUT:  pFmtStr = sHTML_FF_shortcut; break;
            }
            ASSERT( pFmtStr, "ubekanntes Format fuer SwAuthorField" );
            bFixed = ((const SwAuthorField*)pFld)->IsFixed();
            break;

        case RES_DATETIMEFLD:
            pTypeStr = sHTML_FT_datetime;
            bNumFmt = TRUE;
            if( ((SwDateTimeField*)pFld)->IsFixed() )
            {
                bNumValue = TRUE;
                dNumValue = ((SwDateTimeField*)pFld)->GetValue();
            }
            break;

        case RES_PAGENUMBERFLD:
            {
                pTypeStr = sHTML_FT_page;
                SwPageNumSubType eSubType = (SwPageNumSubType)pFld->GetSubType();
                switch( eSubType )
                {
                    case PG_RANDOM:     pSubStr = sHTML_FS_random;      break;
                    case PG_NEXT:       pSubStr = sHTML_FS_next;        break;
                    case PG_PREV:       pSubStr = sHTML_FS_prev;        break;
                }
                ASSERT( pSubStr, "ubekannter Subtyp fuer SwPageNumberField" );
                pFmtStr = SwHTMLWriter::GetNumFormat( (SvxExtNumType)nFmt );

                if( (SvxExtNumType)nFmt==SVX_NUM_CHAR_SPECIAL )
                {
                    aValue = ((const SwPageNumberField *)pFld)->GetUserString();
                }
                else
                {
                    const String& rValue = pFld->GetPar2();
                    short nValue = (short)rValue.ToInt32();
                    if( (eSubType == PG_NEXT && nValue!=1) ||
                        (eSubType == PG_PREV && nValue!=-1) ||
                        (eSubType == PG_RANDOM && nValue!=0) )
                    {
                        aValue = rValue;
                    }
                }
            }
            break;
        case RES_DOCINFOFLD:
            {
                USHORT nSubType = pFld->GetSubType();
                pTypeStr = sHTML_FT_docinfo;
                USHORT nExtSubType = nSubType & 0x0f00;
                nSubType &= 0x00ff;

                switch( nSubType )
                {
                    case DI_TITEL:      pSubStr = sHTML_FS_title;   break;
                    case DI_THEMA:      pSubStr = sHTML_FS_theme;   break;
                    case DI_KEYS:       pSubStr = sHTML_FS_keys;    break;
                    case DI_COMMENT:    pSubStr = sHTML_FS_comment; break;
                    case DI_INFO1:      pSubStr = sHTML_FS_info1;   break;
                    case DI_INFO2:      pSubStr = sHTML_FS_info2;   break;
                    case DI_INFO3:      pSubStr = sHTML_FS_info3;   break;
                    case DI_INFO4:      pSubStr = sHTML_FS_info4;   break;
                    case DI_CREATE:     pSubStr = sHTML_FS_create;  break;
                    case DI_CHANGE:     pSubStr = sHTML_FS_change;  break;
                    default:            pTypeStr = 0;               break;
                }
                if( DI_CREATE == nSubType || DI_CHANGE == nSubType )
                {
                    switch( nExtSubType )
                    {
                        case DI_SUB_AUTHOR:
                            pFmtStr = sHTML_FF_author;
                            break;
                        case DI_SUB_TIME:
                            pFmtStr = sHTML_FF_time;
                            bNumFmt = TRUE;
                            break;
                        case DI_SUB_DATE:
                            pFmtStr = sHTML_FF_date;
                            bNumFmt = TRUE;
                            break;
                    }
                }
                bFixed = ((const SwDocInfoField*)pFld)->IsFixed();
                if( bNumFmt )
                {
                    if( bFixed )
                    {
                        // Fuer ein fixes Feld och den Num-Value ausgeben.
                        // Fixe Felder ohne Zahlenformate sollte es
                        // eigentlich nicht geben. ASSERT ist unten.
                        dNumValue = ((const SwDocInfoField*)pFld)->GetValue();
                        bNumValue = TRUE;
                    }
                    else if( !nFmt  )
                    {
                        // Nicht fixe Felder muessen kein Zahlenformat haben,
                        // wenn sie aus 4.0-Dokumenten stammen.
                        bNumFmt = FALSE;
                    }
                }
            }
            break;

        case RES_DOCSTATFLD:
            {
                pTypeStr = sHTML_FT_docstat;
                USHORT nSubType = pFld->GetSubType();
                switch( nSubType )
                {
                    case DS_PAGE:       pSubStr = sHTML_FS_page;    break;
                    case DS_PARA:       pSubStr = sHTML_FS_para;    break;
                    case DS_WORD:       pSubStr = sHTML_FS_word;    break;
                    case DS_CHAR:       pSubStr = sHTML_FS_char;    break;
                    case DS_TBL:        pSubStr = sHTML_FS_tbl;     break;
                    case DS_GRF:        pSubStr = sHTML_FS_grf;     break;
                    case DS_OLE:        pSubStr = sHTML_FS_ole;     break;
                    default:            pTypeStr = 0;               break;
                }
                pFmtStr = SwHTMLWriter::GetNumFormat( (SvxExtNumType)nFmt );
            }
            break;

        case RES_FILENAMEFLD:
            pTypeStr = sHTML_FT_filename;
            switch( (SwFileNameFormat)(nFmt & ~FF_FIXED) )
            {
                case FF_NAME:       pFmtStr = sHTML_FF_name;        break;
                case FF_PATHNAME:   pFmtStr = sHTML_FF_pathname;    break;
                case FF_PATH:       pFmtStr = sHTML_FF_path;        break;
                case FF_NAME_NOEXT: pFmtStr = sHTML_FF_name_noext;  break;
            }
            bFixed = ((const SwFileNameField*)pFld)->IsFixed();
            ASSERT( pFmtStr, "unbekanntes Format fuer SwFileNameField" );
            break;
    }

    // <SDFIELD>-Tag ausgeben
    if( pTypeStr )
    {
        ByteString sOut( '<' );
        ((((sOut += sHTML_sdfield) += ' ') += sHTML_O_type) += '=')
            += pTypeStr;
        if( pSubStr )
            (((sOut += ' ') += sHTML_O_subtype) += '=') += pSubStr;
        if( pFmtStr )
            (((sOut += ' ') += sHTML_O_format) += '=') += pFmtStr;
        if( aValue.Len() )
        {
            ((sOut += ' ') += sHTML_O_value) += "=\"";
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rWrt.Strm(), aValue, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
            sOut = '\"';
        }
        if( bNumFmt )
        {
            ASSERT( nFmt, "Zahlenformat ist 0" );
            sOut = HTMLOutFuncs::CreateTableDataOptionsValNum( sOut,
                        bNumValue, dNumValue, nFmt,
                        *rHTMLWrt.pDoc->GetNumberFormatter(),
                        rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );

        }
        if( bFixed )
            (sOut += ' ') += sHTML_O_sdfixed;
        sOut += '>';
        rWrt.Strm() << sOut.GetBuffer();
    }

    // Inhalt des Feldes ausgeben
    String sExpand( pFld->Expand() );
    sal_Bool bNeedsCJKProcessing = sal_False;
    if( sExpand.Len() )
    {
        sal_uInt16 nScriptType = pBreakIt->xBreak->getScriptType( sExpand, 0 );
        xub_StrLen nPos = (xub_StrLen)pBreakIt->xBreak->endOfScript( sExpand, 0,
                                                          nScriptType );

        sal_uInt16 nScript =
            SwHTMLWriter::GetCSS1ScriptForScriptType( nScriptType );
        if( nPos < sExpand.Len() || nScript != rHTMLWrt.nCSS1Script )
        {
            bNeedsCJKProcessing = sal_True;
        }
    }

    if( bNeedsCJKProcessing )
    {
        SfxItemSet aScriptItemSet( rWrt.pDoc->GetAttrPool(),
                                   RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                                   RES_CHRATR_POSTURE, RES_CHRATR_POSTURE,
                                   RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT,
                                   RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_WEIGHT,
                                   0 );
        rTxtNd.GetAttr( aScriptItemSet, nFldPos, nFldPos+1 );

        sal_uInt16 aWesternWhichIds[4] =
            { RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
              RES_CHRATR_POSTURE, RES_CHRATR_WEIGHT };
        sal_uInt16 aCJKWhichIds[4] =
            { RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE,
              RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT };
        sal_uInt16 aCTLWhichIds[4] =
            { RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE,
              RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT };

        sal_uInt16 *pRefWhichIds;
        switch( rHTMLWrt.nCSS1Script )
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

        xub_StrLen nPos = 0;
        do
        {
            sal_uInt16 nScriptType = pBreakIt->xBreak->getScriptType( sExpand, nPos );
            sal_uInt16 nScript =
                SwHTMLWriter::GetCSS1ScriptForScriptType( nScriptType );
            xub_StrLen nEndPos = (xub_StrLen)pBreakIt->xBreak->endOfScript(
                                    sExpand, nPos, nScriptType );
            if( nScript != rHTMLWrt.nCSS1Script )
            {
                sal_uInt16 *pWhichIds;
                switch( nScript )
                {
                case CSS1_OUTMODE_WESTERN:  pWhichIds = aWesternWhichIds; break;
                case CSS1_OUTMODE_CJK:      pWhichIds = aCJKWhichIds; break;
                case CSS1_OUTMODE_CTL:      pWhichIds = aCTLWhichIds; break;
                }

                rHTMLWrt.bTagOn = TRUE;
                const SfxPoolItem *aItems[5];
                sal_uInt16 nItems = 0;
                for( sal_uInt16 i=0; i<4; i++ )
                {
                    const SfxPoolItem *pRefItem =
                        aScriptItemSet.GetItem( pRefWhichIds[i] );
                    const SfxPoolItem *pItem =
                        aScriptItemSet.GetItem( pWhichIds[i] );
                    if( pRefItem && pItem &&
                        !(0==i ? lcl_css1atr_equalFontItems( *pRefItem, *pItem )
                               : *pRefItem == *pItem) )
                    {
                        Out( aHTMLAttrFnTab, *pItem, rHTMLWrt );
                        aItems[nItems++] = pItem;
                    }
                }

                HTMLOutFuncs::Out_String( rWrt.Strm(), sExpand.Copy( nPos, nEndPos ),
                    rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );

                rHTMLWrt.bTagOn = FALSE;
                while( nItems )
                    Out( aHTMLAttrFnTab, *aItems[--nItems], rHTMLWrt );

            }
            else
            {
                HTMLOutFuncs::Out_String( rWrt.Strm(), sExpand.Copy( nPos, nEndPos ),
                    rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
            }
            nPos = nEndPos;
        }
        while( nPos < sExpand.Len() );
    }
    else
    {
        HTMLOutFuncs::Out_String( rWrt.Strm(), sExpand,
              rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
    }

    // Off-Tag ausgeben
    if( pTypeStr )
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_sdfield, FALSE );

    return rWrt;
}


Writer& OutHTML_SwFmtFld( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwFmtFld & rFld = (SwFmtFld&)rHt;
    const SwField* pFld = rFld.GetFld();
    const SwFieldType* pFldTyp = pFld->GetTyp();

    if( RES_SETEXPFLD == pFldTyp->Which() &&
        (GSE_STRING & pFld->GetSubType()) )
    {
        int bOn = FALSE;
        if( pFldTyp->GetName().EqualsAscii("HTML_ON" ) )
            bOn = TRUE;
        else if( !pFldTyp->GetName().EqualsAscii( "HTML_OFF" ) )
            return rWrt;

        String rTxt( pFld->GetPar2() );
        rTxt.EraseLeadingChars().EraseTrailingChars();
        rWrt.Strm() << '<';
        if( !bOn )
            rWrt.Strm() << '/';
        // TODO: HTML-Tags are written without entitities, that for, characters
        // not contained in the destination encoding are lost!
        ByteString sTmp( rTxt, ((SwHTMLWriter&)rWrt).eDestEnc );
        rWrt.Strm() << sTmp.GetBuffer() << '>';
    }
    else if( RES_POSTITFLD == pFldTyp->Which() )
    {
        // Kommentare werden im ANSI-Zeichensetz, aber mit System-Zeilen-
        // Umbruechen gesschrieben.
        const String& rComment = pFld->GetPar2();
        BOOL bWritten = FALSE;

        if( (rComment.Len() >= 6 && '<' == rComment.GetChar(0) &&
            '>' == rComment.GetChar(rComment.Len()-1) &&
            rComment.Copy( 1, 4 ).EqualsIgnoreCaseAscii(sHTML_meta)) ||
              (rComment.Len() >= 7 &&
             rComment.Copy( 0, 4 ).EqualsAscii( "<!--" ) &&
             rComment.Copy( rComment.Len()-3, 3 ).EqualsAscii( "-->" )) )
        {
            // META-Tags direkt ausgeben
            String sComment( rComment );
            sComment.ConvertLineEnd( GetSystemLineEnd() );
            // TODO: HTML-Tags are written without entitities, that for,
            // characters not contained in the destination encoding are lost!
            ByteString sTmp( sComment, ((SwHTMLWriter&)rWrt).eDestEnc );
            rWrt.Strm() << sTmp.GetBuffer();
            bWritten = TRUE;
        }
        else if( rComment.Len() >= 7 &&
                 '>' == rComment.GetChar(rComment.Len()-1) &&
                 rComment.Copy(0,5).EqualsIgnoreCaseAscii("HTML:") )
        {
            String sComment( rComment.Copy(5) );
            sComment.EraseLeadingChars();
            if( '<' == sComment.GetChar(0) )
            {
                sComment.ConvertLineEnd( GetSystemLineEnd() );
                // TODO: HTML-Tags are written without entitities, that for,
                // characters not contained in the destination encoding are
                // lost!
                ByteString sTmp( sComment, ((SwHTMLWriter&)rWrt).eDestEnc );
                rWrt.Strm() << sTmp.GetBuffer();
                bWritten = TRUE;
            }

        }

        if( !bWritten )
        {
            ByteString sOut( '<' );

            String sComment( rComment );
            sComment.ConvertLineEnd( GetSystemLineEnd() );
            // TODO: ???
            (((sOut += sHTML_comment) += ' ')
                += ByteString( sComment, ((SwHTMLWriter&)rWrt).eDestEnc ))
                += " -->";
            rWrt.Strm() << sOut.GetBuffer();
        }
    }
    else if( RES_SCRIPTFLD == pFldTyp->Which() )
    {
        SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine( TRUE );

        BOOL bURL = ((const SwScriptField *)pFld)->IsCodeURL();
        const String& rType = pFld->GetPar1();
        String aContents, aURL;
        if(bURL)
            aURL = pFld->GetPar2();
        else
            aContents = pFld->GetPar2();

        // sonst ist es der Script-Inhalt selbst. Da nur noh JavaScript
        // in Feldern landet, muss es sich um JavaSrript handeln ...:)
        HTMLOutFuncs::OutScript( rWrt.Strm(), aContents, rType, JAVASCRIPT,
                                 aURL, 0, 0, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );

        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine( TRUE );
    }
    else
    {
        const SwTxtFld *pTxtFld = rFld.GetTxtFld();
        ASSERT( pTxtFld, "Where is the txt fld?" );
        if( pTxtFld )
            OutHTML_SwField( rWrt, pFld, pTxtFld->GetTxtNode(),
                             *pTxtFld->GetStart()  );
    }
    return rWrt;
}


