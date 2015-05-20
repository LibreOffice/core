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

#include "hintids.hxx"
#include "sw3convert.hxx"
#include <tools/resid.hxx>
#include <svl/macitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/linkmgr.hxx>

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
#define SWG_INETBROWSER 0x000e      // URL graph nodes, browse flag, formatter
#define SWG_NEWFIELDS   0x0200      // Fields with Numberformatter numberformat

using namespace nsSwDocInfoSubType;

struct OldFormats
{
    NfIndexTableOffset  eFormatIdx;
    sal_uInt16              nOldFormat;
};

static const OldFormats aOldDateFormat40[] =
{
    // Date fields
    { NF_DATE_SYSTEM_SHORT,         DFF_SSYS },     // Short system date
    { NF_DATE_SYSTEM_LONG,          DFF_LSYS },     // Long system date
    { NF_DATE_SYS_DDMMYY,           DFF_DMY },      // 06.10.64
    { NF_DATE_SYS_DDMMYYYY,         DFF_DMYY },     // 06.10.1964
    { NF_DATE_SYS_DMMMYY,           DFF_DMMY },     // 06. Oct 64
    { NF_DATE_SYS_DMMMYYYY,         DFF_DMMYY },    // 06. Oct 1964
    { NF_DATE_DIN_DMMMMYYYY,        DFF_DMMMYY },   // 06. October 1964
    { NF_DATE_DIN_DMMMMYYYY,        DFF_DMMMY },    // 06. October 64
    { NF_DATE_SYS_NNDMMMYY,         DFF_DDMMY },    // Tue, 06. Oct 64
    { NF_DATE_SYS_NNDMMMMYYYY,      DFF_DDMMMY },   // Tue, 06. October 64
    { NF_DATE_SYS_NNDMMMMYYYY,      DFF_DDMMMYY },  // Tue, 06. October 1964
    { NF_DATE_SYS_NNNNDMMMMYYYY,    DFF_DDDMMMYY }, // Tuesday, 06. October 1964
    { NF_DATE_SYS_NNNNDMMMMYYYY,    DFF_DDDMMMY },  // Tuesday, 06. October 64
    { NF_DATE_SYS_MMYY,             DFF_MY },       // 10.64
    { NF_DATE_DIN_MMDD,             DFF_MD },       // 10-06
    { NF_DATE_DIN_YYMMDD,           DFF_YMD },      // 64-10-06
    { NF_DATE_DIN_YYYYMMDD,         DFF_YYMD },     // 1964-10-06

    { NF_NUMERIC_START,             0  }            // End of table
};

static const OldFormats aOldDateFormat30[] =
{
    // Date fields
    { NF_DATE_SYSTEM_SHORT,         DFF_SSYS },     // Short system date
    { NF_DATE_SYSTEM_LONG,          DFF_LSYS },     // Long system date
    { NF_DATE_SYS_DDMMYY,           DFF_DMY },      // 06.10.64
    { NF_DATE_SYS_DDMMYYYY,         DFF_DMYY },     // 06.10.1964
    { NF_DATE_SYS_DMMMYY,           DFF_DMMY },     // 06. Oct 64
    { NF_DATE_SYS_DMMMYYYY,         4 /*DFF_DMMYY*/ },  // 06. Oct 1964
    { NF_DATE_DIN_DMMMMYYYY,        5 /*DFF_DMMMYY*/ }, // 06. October 1964
    { NF_DATE_DIN_DMMMMYYYY,        5 /*DFF_DMMMY*/ },  // 06. October 64
    { NF_DATE_SYS_NNDMMMMYYYY,      6 /*DFF_DDMMMYY*/ },// Tue, 06. October 1964
    { NF_DATE_SYS_NNDMMMYY,         6 /*DFF_DDMMY*/ },  // Tue, 06. Oct 64
    { NF_DATE_SYS_NNDMMMMYYYY,      6 /*DFF_DDMMMY*/ }, // Tue, 06. October 64
    { NF_DATE_SYS_NNNNDMMMMYYYY,    7 /*DFF_DDDMMMYY*/ }, // Tuesday, 06. October 1964
    { NF_DATE_SYS_NNNNDMMMMYYYY,    7 /*DFF_DDDMMMY*/ },  // Tuesday, 06. October 64
    { NF_DATE_SYS_MMYY,             2 /*DFF_MY*/ },     // 10.64
    { NF_DATE_DIN_MMDD,             DFF_MD },           // 10-06
    { NF_DATE_DIN_YYMMDD,           DFF_YMD },      // 64-10-06
    { NF_DATE_DIN_YYYYMMDD,         DFF_YYMD },     // 1964-10-06

    { NF_NUMERIC_START,             0  }            // End of table
};

static const OldFormats aOldTimeFormat[] =
{
    // Time fields
    { NF_TIME_HHMMSS,               TF_SYSTEM },    // System time
    { NF_TIME_HHMM,                 TF_SSMM_24 },   // 23:25
    { NF_TIME_HHMMAMPM,             TF_SSMM_12 },   // 11:25 PM

    { NF_NUMERIC_START,             0 }             // End of table
};

static const OldFormats aOldGetSetExpFormat40[] =
{
    { NF_TEXT,                      VVF_CMD },      // Show command
    { NF_TEXT,                      VVF_INVISIBLE },// Invisible
    { NF_PERCENT_INT,               VVF_XXP },      // 1234%
    { NF_PERCENT_DEC2,              VVF_XX_XXP },   // 1.234,56%
    { NF_TEXT,                      VVF_CLEAR },    // ???

    { NF_NUMBER_SYSTEM,             VVF_SYS },      // Number format from system preferences
    { NF_NUMBER_INT,                VVF_X },        // 1234
    { NF_NUMBER_DEC2,               VVF_X_X },      // 1234,5
    { NF_NUMBER_DEC2,               VVF_X_XX },     // 1245,56
    { NF_NUMBER_1000DEC2,           VVF_XX_XX },    // 1.234,56
    { NF_NUMBER_1000DEC2,           VVF_XX_X },     // 1.234,5
    { NF_NUMBER_1000DEC2,           VVF_XX_XXX },   // 1.234,567
    { NF_CURRENCY_1000DEC2,         VVF_SYS_CUR },  // Currency format from system settings
                                                    // (1.234,00 EUR)
    { NF_CURRENCY_1000INT,          VVF_X_CUR },    // 1234 EUR
    { NF_CURRENCY_1000DEC2,         VVF_XX_XX_CUR },// 1234,56 EUR 1234,00 EUR
    { NF_CURRENCY_1000DEC2_DASHED,  VVF_XX_X0_CUR },// 1234,56 EUR 1234,-- EUR
    { NF_CURRENCY_1000INT,          VVF_CUR_X },    // EUR 1234
    { NF_CURRENCY_1000DEC2,         VVF_CUR_XX_XX },// EUR 1234,56 EUR 1234,00
    { NF_CURRENCY_1000DEC2_DASHED,  VVF_CUR_XX_X0 },// EUR 1234,56 EUR 1234,--

    { NF_NUMERIC_START,                 0  }        // End of table
};

static const OldFormats aOldGetSetExpFormat30[] =
{
    { NF_TEXT,                      VVF_CMD },      // Show command
    { NF_TEXT,                      VVF_INVISIBLE },// Invisible
    { NF_PERCENT_INT,                   VVF_XXP },  // 1234%
    { NF_PERCENT_DEC2,              VVF_XX_XXP },   // 1.234,56%
    { NF_TEXT,                      VVF_CLEAR },    // ???

    { NF_NUMBER_SYSTEM,             0x0020 },       // Number format from system preferences
    { NF_NUMBER_INT,                    0x0080 },   // 1234
    { NF_NUMBER_1000DEC2,               0x0100 },   // 1.234,56
    { NF_NUMBER_DEC2,                   0x0100 },   // 1234,5
    { NF_NUMBER_DEC2,                   0x0100 },   // 1245,56
    { NF_NUMBER_1000DEC2,               0x0100 },   // 1.234,5
    { NF_NUMBER_1000DEC2,               0x0100 },   // 1.234,567
    { NF_CURRENCY_1000DEC2,         0x0200 },       // Currency format from system settings
                                                    // (1.234,00 EUR)
    { NF_CURRENCY_1000INT,          0x1000 },       // 1234 EUR
    { NF_CURRENCY_1000DEC2,         0x1000 },       // 1234,56 EUR 1234,00 EUR
    { NF_CURRENCY_1000DEC2_DASHED,  0x1000 },       // 1234,56 EUR 1234,-- EUR
    { NF_CURRENCY_1000INT,          0x1000 },       // EUR 1234
    { NF_CURRENCY_1000DEC2,         0x1000 },       // EUR 1234,56 EUR 1234,00
    { NF_CURRENCY_1000DEC2_DASHED,  0x1000 },       // EUR 1234,56 EUR 1234,--

    { NF_NUMERIC_START,                 0  }        // End of table
};

void sw3io_ConvertFromOldField( SwDoc& rDoc, sal_uInt16& rWhich,
                                sal_uInt16& rSubType, sal_uLong &rFormat,
                                sal_uInt16 nVersion )
{
    const OldFormats *pOldFormat = 0L;

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
                pOldFormat = nVersion<SWG_INETBROWSER ? aOldDateFormat30
                                                   : aOldDateFormat40;
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
                pOldFormat = aOldTimeFormat;
            }
            break;

        case RES_DBFLD:
            if( nVersion < SWG_NEWFIELDS )
            {
                rSubType = nsSwExtendedSubType::SUB_OWN_FMT;
                pOldFormat = nVersion<SWG_INETBROWSER ? aOldGetSetExpFormat30
                                                    : aOldGetSetExpFormat40;
            }
            break;

        case RES_TABLEFLD:
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        case RES_USERFLD:
            if( nVersion < SWG_NEWFIELDS )
            {
                if( rFormat == VVF_INVISIBLE )
                {
                    rSubType = nsSwExtendedSubType::SUB_INVISIBLE;
                    rFormat = 0;
                }
                else if( rFormat == VVF_CMD )
                {
                    rSubType = nsSwExtendedSubType::SUB_CMD;
                    rFormat = 0;
                }
                else
                {
                    // Small hack: We still need the unconverted format
                    // when handling number groups. We keep it temporarily
                    // in the subtype; if it's a valid format in the first
                    // place.
                    if( RES_SETEXPFLD==rWhich &&
                        /*rFormat >= (sal_uInt16)SVX_NUM_CHARS_UPPER_LETTER && always true*/
                        rFormat <= (sal_uInt16)SVX_NUM_BITMAP )
                    {
                        rSubType = (sal_uInt16)rFormat;
                    }
                    pOldFormat = nVersion<SWG_INETBROWSER ? aOldGetSetExpFormat30
                                                       : aOldGetSetExpFormat40;
                }
            }
            break;
        case RES_DOCINFOFLD:
            if( nVersion < SWG_NEWFIELDS )
            {
                switch( rFormat )
                {
                case RF_AUTHOR: rSubType = DI_SUB_AUTHOR;   break;
                case RF_TIME:   rSubType = DI_SUB_TIME; break;
                case RF_DATE:   rSubType = DI_SUB_DATE; break;
                case RF_ALL:    rSubType = DI_SUB_DATE; break;
                }
                rFormat = 0;
            }
            break;
    }

    if( pOldFormat )
    {
        SvNumberFormatter *pFormatter = rDoc.GetNumberFormatter();
        sal_uInt16 i = 0;

        while( pOldFormat[i].eFormatIdx != NF_NUMERIC_START ||
               pOldFormat[i].nOldFormat)
        {
            if( rFormat == pOldFormat[i].nOldFormat )
            {
                rFormat = pFormatter->GetFormatIndex(pOldFormat[i].eFormatIdx, LANGUAGE_SYSTEM);
                break;
            }
            i++;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
