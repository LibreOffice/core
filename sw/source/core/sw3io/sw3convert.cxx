/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <stdlib.h>

#include <stdio.h>
#include "hintids.hxx"
#include <tools/resid.hxx>
#include <svl/macitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/linkmgr.hxx>

#ifndef _SVSTDARR_USHORTS_DECL
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <fmtinfmt.hxx>
#include <fmtfld.hxx>
#include <frmatr.hxx>
#include "doc.hxx"
#include "docary.hxx"
#include "fldbas.hxx"
#include "flddat.hxx"
#include "docufld.hxx"
#include "chpfld.hxx"
#include "ddefld.hxx"
#include "expfld.hxx"
#include "reffld.hxx"
#include "usrfld.hxx"
#include "dbfld.hxx"
#include "txtfld.hxx"
#include "ndtxt.hxx"

//really used ids from old sw3ids.hxx
#define SWG_INETBROWSER 0x000e      // URL-Grf-Nodes, Browse-Flag, Format-Ums.
#define SWG_NEWFIELDS   0x0200      // Felder mit Numberformatter-Zahlenformat
#define SWG_EXPORT40    0x0110      // Version fuer 4.0-Export ab 5.0

using namespace nsSwDocInfoSubType;

struct OldFormats
{
    NfIndexTableOffset  eFormatIdx;
    sal_uInt16              nOldFormat;
};

static OldFormats aOldDateFmt40[] =
{
    // Datumsfelder:
    { NF_DATE_SYSTEM_SHORT,         DFF_SSYS },     // Kurzes Systemdatum
    { NF_DATE_SYSTEM_LONG,          DFF_LSYS },     // Langes Systemdatum
    { NF_DATE_SYS_DDMMYY,               DFF_DMY },        // 06.10.64
    { NF_DATE_SYS_DDMMYYYY,         DFF_DMYY },     // 06.10.1964
    { NF_DATE_SYS_DMMMYY,               DFF_DMMY },     // 06. Okt 64
    { NF_DATE_SYS_DMMMYYYY,         DFF_DMMYY },        // 06. Okt 1964
    { NF_DATE_DIN_DMMMMYYYY,            DFF_DMMMYY },       // 06. Oktober 1964
    { NF_DATE_DIN_DMMMMYYYY,            DFF_DMMMY },        // 06. Oktober 64
    { NF_DATE_SYS_NNDMMMYY,         DFF_DDMMY },        // Di, 06. Okt 64
    { NF_DATE_SYS_NNDMMMMYYYY,      DFF_DDMMMY },       // Di, 06. Oktober 64
    { NF_DATE_SYS_NNDMMMMYYYY,      DFF_DDMMMYY },  // Di, 06. Oktober 1964
    { NF_DATE_SYS_NNNNDMMMMYYYY,        DFF_DDDMMMYY }, // Dienstag, 06. Oktober 1964
    { NF_DATE_SYS_NNNNDMMMMYYYY,        DFF_DDDMMMY },  // Dienstag, 06. Oktober 64
    { NF_DATE_SYS_MMYY,             DFF_MY },           // 10.64
    { NF_DATE_DIN_MMDD,             DFF_MD },           // 10-06
    { NF_DATE_DIN_YYMMDD,               DFF_YMD },      // 64-10-06
    { NF_DATE_DIN_YYYYMMDD,         DFF_YYMD },     // 1964-10-06

    { NF_NUMERIC_START,             0  }                // Tabellenende
};

static OldFormats aOldDateFmt30[] =
{
    // Datumsfelder:
    { NF_DATE_SYSTEM_SHORT,         DFF_SSYS },     // Kurzes Systemdatum
    { NF_DATE_SYSTEM_LONG,          DFF_LSYS },     // Langes Systemdatum
    { NF_DATE_SYS_DDMMYY,               DFF_DMY },        // 06.10.64
    { NF_DATE_SYS_DDMMYYYY,         DFF_DMYY },     // 06.10.1964
    { NF_DATE_SYS_DMMMYY,               DFF_DMMY },     // 06. Okt 64
    { NF_DATE_SYS_DMMMYYYY,         4 /*DFF_DMMYY*/ },  // 06. Okt 1964
    { NF_DATE_DIN_DMMMMYYYY,            5 /*DFF_DMMMYY*/ }, // 06. Oktober 1964
    { NF_DATE_DIN_DMMMMYYYY,            5 /*DFF_DMMMY*/ },  // 06. Oktober 64
    { NF_DATE_SYS_NNDMMMMYYYY,      6 /*DFF_DDMMMYY*/ },    // Di, 06. Oktober 1964
    { NF_DATE_SYS_NNDMMMYY,         6 /*DFF_DDMMY*/ },  // Di, 06. Okt 64
    { NF_DATE_SYS_NNDMMMMYYYY,      6 /*DFF_DDMMMY*/ }, // Di, 06. Oktober 64
    { NF_DATE_SYS_NNNNDMMMMYYYY,        7 /*DFF_DDDMMMYY*/ },   // Dienstag, 06. Oktober 1964
    { NF_DATE_SYS_NNNNDMMMMYYYY,        7 /*DFF_DDDMMMY*/ },    // Dienstag, 06. Oktober 64
    { NF_DATE_SYS_MMYY,             2 /*DFF_MY*/ },     // 10.64
    { NF_DATE_DIN_MMDD,             DFF_MD },           // 10-06
    { NF_DATE_DIN_YYMMDD,               DFF_YMD },      // 64-10-06
    { NF_DATE_DIN_YYYYMMDD,         DFF_YYMD },     // 1964-10-06

    { NF_NUMERIC_START,             0  }                // Tabellenende
};

static OldFormats aOldTimeFmt[] =
{
    // Zeitfelder:
    { NF_TIME_HHMMSS,                   TF_SYSTEM },        // Systemzeit
    { NF_TIME_HHMM,                 TF_SSMM_24 },       // 23:25
    { NF_TIME_HHMMAMPM,             TF_SSMM_12 },       // 11:25 PM

    { NF_NUMERIC_START,             0 }             // Tabellenende
};

static OldFormats aOldGetSetExpFmt40[] =
{
    { NF_TEXT,                      VVF_CMD },      // Kommando anzeigen
    { NF_TEXT,                      VVF_INVISIBLE },    // unsichtbar
    { NF_PERCENT_INT,                   VVF_XXP },      // 1234%
    { NF_PERCENT_DEC2,              VVF_XX_XXP },       // 1.234,56%
    { NF_TEXT,                      VVF_CLEAR },        // ???

    { NF_NUMBER_SYSTEM,             VVF_SYS },      // Zahlenformat aus der
                                                    // Systemeinstellung
    { NF_NUMBER_INT,                    VVF_X },            // 1234
    { NF_NUMBER_DEC2,                   VVF_X_X },          // 1234,5
    { NF_NUMBER_DEC2,                   VVF_X_XX },     // 1245,56
    { NF_NUMBER_1000DEC2,               VVF_XX_XX },    // 1.234,56
    { NF_NUMBER_1000DEC2,               VVF_XX_X },     // 1.234,5
    { NF_NUMBER_1000DEC2,               VVF_XX_XXX },       // 1.234,567
    { NF_CURRENCY_1000DEC2,         VVF_SYS_CUR },  // W?hrungsformat aus der
                                                    // Systemeinstellung
                                                    // (1.234,00 DM)
    { NF_CURRENCY_1000INT,          VVF_X_CUR },        // 1234 DM
    { NF_CURRENCY_1000DEC2,         VVF_XX_XX_CUR },  // 1234,56 DM 1234,00 DM
    { NF_CURRENCY_1000DEC2_DASHED,  VVF_XX_X0_CUR },  // 1234,56 DM 1234,-- DM
    { NF_CURRENCY_1000INT,          VVF_CUR_X },    // DM 1234
    { NF_CURRENCY_1000DEC2,         VVF_CUR_XX_XX },  // DM 1234,56 DM 1234,00
    { NF_CURRENCY_1000DEC2_DASHED,  VVF_CUR_XX_X0 },  // DM 1234,56 DM 1234,--

    { NF_NUMERIC_START,                 0  }                // Tabellenende
};

static OldFormats aOldGetSetExpFmt30[] =
{
    { NF_TEXT,                      VVF_CMD },      // Kommando anzeigen
    { NF_TEXT,                      VVF_INVISIBLE },    // unsichtbar
    { NF_PERCENT_INT,                   VVF_XXP },      // 1234%
    { NF_PERCENT_DEC2,              VVF_XX_XXP },       // 1.234,56%
    { NF_TEXT,                      VVF_CLEAR },        // ???

    { NF_NUMBER_SYSTEM,             0x0020 },       // Zahlenformat aus der
                                                    // Systemeinstellung
    { NF_NUMBER_INT,                    0x0080 },           // 1234
    { NF_NUMBER_1000DEC2,               0x0100 },           // 1.234,56
    { NF_NUMBER_DEC2,                   0x0100 },           // 1234,5
    { NF_NUMBER_DEC2,                   0x0100 },       // 1245,56
    { NF_NUMBER_1000DEC2,               0x0100 },           // 1.234,5
    { NF_NUMBER_1000DEC2,               0x0100 },           // 1.234,567
    { NF_CURRENCY_1000DEC2,         0x0200 },           // W?hrungsformat aus der
                                                    // Systemeinstellung
                                                    // (1.234,00 DM)
    { NF_CURRENCY_1000INT,          0x1000 },           // 1234 DM
    { NF_CURRENCY_1000DEC2,         0x1000 },       // 1234,56 DM 1234,00 DM
    { NF_CURRENCY_1000DEC2_DASHED,  0x1000 },       // 1234,56 DM 1234,-- DM
    { NF_CURRENCY_1000INT,          0x1000 },           // DM 1234
    { NF_CURRENCY_1000DEC2,         0x1000 },       // DM 1234,56 DM 1234,00
    { NF_CURRENCY_1000DEC2_DASHED,  0x1000 },       // DM 1234,56 DM 1234,--

    { NF_NUMERIC_START,                 0  }                // Tabellenende
};

SW_DLLPUBLIC void sw3io_ConvertFromOldField( SwDoc& rDoc, sal_uInt16& rWhich,
                                sal_uInt16& rSubType, sal_uLong &rFmt,
                                sal_uInt16 nVersion )
{
    const OldFormats *pOldFmt = 0L;

    switch( rWhich )
    {
        case RES_DATEFLD:
        case RES_FIXDATEFLD:
            if( nVersion < SWG_NEWFIELDS )
            {
                rSubType = DATEFLD;
                if( RES_FIXDATEFLD == rWhich )
                    rSubType |= FIXEDFLD;
                rWhich = RES_DATETIMEFLD;
                pOldFmt = nVersion<SWG_INETBROWSER ? aOldDateFmt30
                                                   : aOldDateFmt40;
            }
            break;

        case RES_TIMEFLD:
        case RES_FIXTIMEFLD:
            if( nVersion < SWG_NEWFIELDS )
            {
                rSubType = TIMEFLD;
                if( RES_FIXTIMEFLD == rWhich )
                    rSubType |= FIXEDFLD;
                rWhich = RES_DATETIMEFLD;
                pOldFmt = aOldTimeFmt;
            }
            break;

        case RES_DBFLD:
            if( nVersion < SWG_NEWFIELDS )
            {
                rSubType = nsSwExtendedSubType::SUB_OWN_FMT;
                pOldFmt = nVersion<SWG_INETBROWSER ? aOldGetSetExpFmt30
                                                    : aOldGetSetExpFmt40;
            }
            break;

        case RES_TABLEFLD:
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        case RES_USERFLD:
            if( nVersion < SWG_NEWFIELDS )
            {
                if( rFmt == VVF_INVISIBLE )
                {
                    rSubType = nsSwExtendedSubType::SUB_INVISIBLE;
                    rFmt = 0;
                }
                else if( rFmt == VVF_CMD )
                {
                    rSubType = nsSwExtendedSubType::SUB_CMD;
                    rFmt = 0;
                }
                else
                {
                    // Kleiner Hack: Bei Numernkreisen wird das
                    // unkonvertierte Format noch benoetigt. Wir merken es
                    // uns voruebergehend mal im Subtyp, sofern es
                    // ueberhaupt als entsprechendes Format in Frage kommt.
                    if( RES_SETEXPFLD==rWhich &&
                        /*rFmt >= (sal_uInt16)SVX_NUM_CHARS_UPPER_LETTER && always true*/
                        rFmt <= (sal_uInt16)SVX_NUM_BITMAP )
                    {
                        rSubType = (sal_uInt16)rFmt;
                    }
                    pOldFmt = nVersion<SWG_INETBROWSER ? aOldGetSetExpFmt30
                                                       : aOldGetSetExpFmt40;
                }
            }
            break;
        case RES_DOCINFOFLD:
            if( nVersion < SWG_NEWFIELDS )
            {
                switch( rFmt )
                {
                case RF_AUTHOR: rSubType = DI_SUB_AUTHOR;   break;
                case RF_TIME:   rSubType = DI_SUB_TIME; break;
                case RF_DATE:   rSubType = DI_SUB_DATE; break;
                case RF_ALL:    rSubType = DI_SUB_DATE; break;
                }
                rFmt = 0;
            }
            break;
    }

    if( pOldFmt )
    {
        SvNumberFormatter *pFormatter = rDoc.GetNumberFormatter();
        sal_uInt16 i = 0;

        while( pOldFmt[i].eFormatIdx != NF_NUMERIC_START ||
               pOldFmt[i].nOldFormat)
        {
            if( rFmt == pOldFmt[i].nOldFormat )
            {
                rFmt = pFormatter->GetFormatIndex(pOldFmt[i].eFormatIdx, LANGUAGE_SYSTEM);
                break;
            }
            i++;
        }
    }
}
SW_DLLPUBLIC void sw3io_ConvertToOldField( const SwField* pFld, sal_uInt16& rWhich,
                              sal_uLong& rFmt, sal_uLong nFFVersion )
{
    const OldFormats *pOldFmt = 0L;
    sal_uLong nOldFmt = rFmt;

    switch( rWhich )
    {
        case RES_DOCINFOFLD:
            if( SOFFICE_FILEFORMAT_40 >= nFFVersion )
            {
                switch (pFld->GetSubType() & 0xff00)
                {
                case DI_SUB_AUTHOR: rFmt = RF_AUTHOR;   break;
                case DI_SUB_TIME:   rFmt = RF_TIME;     break;
                case DI_SUB_DATE:   rFmt = RF_DATE;     break;
                }
            }
            break;

        case RES_DATETIMEFLD:
            if( SOFFICE_FILEFORMAT_40 >= nFFVersion )
            {
                sal_uInt16 nSubType = ((SwDateTimeField*) pFld)->GetSubType();
                switch( nSubType )
                {
                case DATEFLD:           rWhich = RES_DATEFLD;       break;
                case TIMEFLD:           rWhich = RES_TIMEFLD;       break;
                case DATEFLD|FIXEDFLD:  rWhich = RES_FIXDATEFLD;    break;
                case TIMEFLD|FIXEDFLD:  rWhich = RES_FIXTIMEFLD;    break;
                }

                if( nSubType & DATEFLD )
                {
                    rFmt = DFF_DMY;
                    pOldFmt = aOldDateFmt40;
                }
                else
                {
                    rFmt = TF_SYSTEM;
                    pOldFmt = aOldTimeFmt;
                }
            }
            break;

        case RES_DBFLD:
        case RES_TABLEFLD:
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        case RES_USERFLD:
            if( SOFFICE_FILEFORMAT_40 >= nFFVersion )
            {
                sal_uInt16 nSubType = pFld->GetSubType();

                if (nSubType & nsSwExtendedSubType::SUB_INVISIBLE)
                    rFmt = VVF_INVISIBLE;
                else if (nSubType & nsSwExtendedSubType::SUB_CMD)
                    rFmt = VVF_CMD;
                else if( !(nsSwGetSetExpType::GSE_SEQ & nSubType) )
                {
                    pOldFmt = aOldGetSetExpFmt40;
                    rFmt = VVF_SYS;
                }
            }
            break;

        case RES_GETREFFLD:
            if( SOFFICE_FILEFORMAT_31 == nFFVersion )
            {
                switch( rFmt )
                {
                case REF_PAGE:
                case REF_CHAPTER:
                case REF_CONTENT:
                    break;

                default:
                    rFmt = REF_PAGE;
                    break;
                }
            }
            break;
    }

    if( pOldFmt && nOldFmt )
    {
        SvNumberFormatter *pFormatter = ((SwValueField*)pFld)->GetDoc()->GetNumberFormatter();
        const SvNumberformat* pEntry = pFormatter->GetEntry( nOldFmt );

        if( pEntry )
        {
            sal_uInt16 i = 0;
            while( pOldFmt[i].eFormatIdx != NF_NUMERIC_START ||
                   pOldFmt[i].nOldFormat )
            {
                sal_uLong nKey = pFormatter->GetFormatIndex(
                            pOldFmt[i].eFormatIdx, pEntry->GetLanguage() );

                if( nOldFmt == nKey )
                {
                    rFmt = pOldFmt[i].nOldFormat;
                    break;
                }
                i++;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
