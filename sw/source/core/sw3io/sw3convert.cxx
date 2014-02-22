/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "hintids.hxx"
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


#define SWG_INETBROWSER 0x000e      
#define SWG_NEWFIELDS   0x0200      

using namespace nsSwDocInfoSubType;

struct OldFormats
{
    NfIndexTableOffset  eFormatIdx;
    sal_uInt16              nOldFormat;
};

static const OldFormats aOldDateFmt40[] =
{
    
    { NF_DATE_SYSTEM_SHORT,         DFF_SSYS },     
    { NF_DATE_SYSTEM_LONG,          DFF_LSYS },     
    { NF_DATE_SYS_DDMMYY,               DFF_DMY },        
    { NF_DATE_SYS_DDMMYYYY,         DFF_DMYY },     
    { NF_DATE_SYS_DMMMYY,               DFF_DMMY },     
    { NF_DATE_SYS_DMMMYYYY,         DFF_DMMYY },        
    { NF_DATE_DIN_DMMMMYYYY,            DFF_DMMMYY },       
    { NF_DATE_DIN_DMMMMYYYY,            DFF_DMMMY },        
    { NF_DATE_SYS_NNDMMMYY,         DFF_DDMMY },        
    { NF_DATE_SYS_NNDMMMMYYYY,      DFF_DDMMMY },       
    { NF_DATE_SYS_NNDMMMMYYYY,      DFF_DDMMMYY },  
    { NF_DATE_SYS_NNNNDMMMMYYYY,        DFF_DDDMMMYY }, 
    { NF_DATE_SYS_NNNNDMMMMYYYY,        DFF_DDDMMMY },  
    { NF_DATE_SYS_MMYY,             DFF_MY },           
    { NF_DATE_DIN_MMDD,             DFF_MD },           
    { NF_DATE_DIN_YYMMDD,               DFF_YMD },      
    { NF_DATE_DIN_YYYYMMDD,         DFF_YYMD },     

    { NF_NUMERIC_START,             0  }                
};

static const OldFormats aOldDateFmt30[] =
{
    
    { NF_DATE_SYSTEM_SHORT,         DFF_SSYS },     
    { NF_DATE_SYSTEM_LONG,          DFF_LSYS },     
    { NF_DATE_SYS_DDMMYY,               DFF_DMY },        
    { NF_DATE_SYS_DDMMYYYY,         DFF_DMYY },     
    { NF_DATE_SYS_DMMMYY,               DFF_DMMY },     
    { NF_DATE_SYS_DMMMYYYY,         4 /*DFF_DMMYY*/ },  
    { NF_DATE_DIN_DMMMMYYYY,            5 /*DFF_DMMMYY*/ }, 
    { NF_DATE_DIN_DMMMMYYYY,            5 /*DFF_DMMMY*/ },  
    { NF_DATE_SYS_NNDMMMMYYYY,      6 /*DFF_DDMMMYY*/ },    
    { NF_DATE_SYS_NNDMMMYY,         6 /*DFF_DDMMY*/ },  
    { NF_DATE_SYS_NNDMMMMYYYY,      6 /*DFF_DDMMMY*/ }, 
    { NF_DATE_SYS_NNNNDMMMMYYYY,        7 /*DFF_DDDMMMYY*/ },   
    { NF_DATE_SYS_NNNNDMMMMYYYY,        7 /*DFF_DDDMMMY*/ },    
    { NF_DATE_SYS_MMYY,             2 /*DFF_MY*/ },     
    { NF_DATE_DIN_MMDD,             DFF_MD },           
    { NF_DATE_DIN_YYMMDD,               DFF_YMD },      
    { NF_DATE_DIN_YYYYMMDD,         DFF_YYMD },     

    { NF_NUMERIC_START,             0  }                
};

static const OldFormats aOldTimeFmt[] =
{
    
    { NF_TIME_HHMMSS,                   TF_SYSTEM },        
    { NF_TIME_HHMM,                 TF_SSMM_24 },       
    { NF_TIME_HHMMAMPM,             TF_SSMM_12 },       

    { NF_NUMERIC_START,             0 }             
};

static const OldFormats aOldGetSetExpFmt40[] =
{
    { NF_TEXT,                      VVF_CMD },      
    { NF_TEXT,                      VVF_INVISIBLE },    
    { NF_PERCENT_INT,                   VVF_XXP },      
    { NF_PERCENT_DEC2,              VVF_XX_XXP },       
    { NF_TEXT,                      VVF_CLEAR },        

    { NF_NUMBER_SYSTEM,             VVF_SYS },      
                                                    
    { NF_NUMBER_INT,                    VVF_X },            
    { NF_NUMBER_DEC2,                   VVF_X_X },          
    { NF_NUMBER_DEC2,                   VVF_X_XX },     
    { NF_NUMBER_1000DEC2,               VVF_XX_XX },    
    { NF_NUMBER_1000DEC2,               VVF_XX_X },     
    { NF_NUMBER_1000DEC2,               VVF_XX_XXX },       
    { NF_CURRENCY_1000DEC2,         VVF_SYS_CUR },  
                                                    
                                                    
    { NF_CURRENCY_1000INT,          VVF_X_CUR },        
    { NF_CURRENCY_1000DEC2,         VVF_XX_XX_CUR },  
    { NF_CURRENCY_1000DEC2_DASHED,  VVF_XX_X0_CUR },  
    { NF_CURRENCY_1000INT,          VVF_CUR_X },    
    { NF_CURRENCY_1000DEC2,         VVF_CUR_XX_XX },  
    { NF_CURRENCY_1000DEC2_DASHED,  VVF_CUR_XX_X0 },  

    { NF_NUMERIC_START,                 0  }                
};

static const OldFormats aOldGetSetExpFmt30[] =
{
    { NF_TEXT,                      VVF_CMD },      
    { NF_TEXT,                      VVF_INVISIBLE },    
    { NF_PERCENT_INT,                   VVF_XXP },      
    { NF_PERCENT_DEC2,              VVF_XX_XXP },       
    { NF_TEXT,                      VVF_CLEAR },        

    { NF_NUMBER_SYSTEM,             0x0020 },       
                                                    
    { NF_NUMBER_INT,                    0x0080 },           
    { NF_NUMBER_1000DEC2,               0x0100 },           
    { NF_NUMBER_DEC2,                   0x0100 },           
    { NF_NUMBER_DEC2,                   0x0100 },       
    { NF_NUMBER_1000DEC2,               0x0100 },           
    { NF_NUMBER_1000DEC2,               0x0100 },           
    { NF_CURRENCY_1000DEC2,         0x0200 },           
                                                    
                                                    
    { NF_CURRENCY_1000INT,          0x1000 },           
    { NF_CURRENCY_1000DEC2,         0x1000 },       
    { NF_CURRENCY_1000DEC2_DASHED,  0x1000 },       
    { NF_CURRENCY_1000INT,          0x1000 },           
    { NF_CURRENCY_1000DEC2,         0x1000 },       
    { NF_CURRENCY_1000DEC2_DASHED,  0x1000 },       

    { NF_NUMERIC_START,                 0  }                
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
