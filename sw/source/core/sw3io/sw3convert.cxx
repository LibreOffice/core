/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sw3convert.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 15:44:23 $
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



#include <stdlib.h>

#include <stdio.h>
#include "hintids.hxx"

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _LINKMGR_HXX
#include <sfx2/linkmgr.hxx>
#endif

#ifndef _SVSTDARR_USHORTS_DECL
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif

#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
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


//#include "poolfmt.hxx"        // fuer InSetExpField
//#include "poolfmt.hrc"        // fuer InSetExpField

//#if !defined(UNX) && !defined(MSC) && !defined(PPC) && !defined(CSET) && !defined(__MWERKS__) && !defined(WTC)

//#define FIELDFNTAB_SIZE 37
//#if FIELDFNTAB_SIZE != RES_FIELDS_END - RES_FIELDS_BEGIN
//#error Feld-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
//#endif

//#endif

//#define SWG_AUTHORITY_ENTRY_LCL   'E'

//sal_Char __FAR_DATA sSW3IO_FixedField[] = "FixedExport";
//sal_Char __FAR_DATA sSW3IO_AuthorityField[] = "AuthorityExport";
/* #108791# */
//sal_Char __FAR_DATA sSW3IO_DropDownField[] = "DropDownExport";

struct OldFormats
{
    NfIndexTableOffset  eFormatIdx;
    USHORT              nOldFormat;
};

static OldFormats aOldDateFmt40[] =
{
    // Datumsfelder:
    NF_DATE_SYSTEM_SHORT,           DFF_SSYS,       // Kurzes Systemdatum
    NF_DATE_SYSTEM_LONG,            DFF_LSYS,       // Langes Systemdatum
    NF_DATE_SYS_DDMMYY,             DFF_DMY,        // 06.10.64
    NF_DATE_SYS_DDMMYYYY,           DFF_DMYY,       // 06.10.1964
    NF_DATE_SYS_DMMMYY,             DFF_DMMY,       // 06. Okt 64
    NF_DATE_SYS_DMMMYYYY,           DFF_DMMYY,      // 06. Okt 1964
    NF_DATE_DIN_DMMMMYYYY,          DFF_DMMMYY,     // 06. Oktober 1964
    NF_DATE_DIN_DMMMMYYYY,          DFF_DMMMY,      // 06. Oktober 64
    NF_DATE_SYS_NNDMMMYY,           DFF_DDMMY,      // Di, 06. Okt 64
    NF_DATE_SYS_NNDMMMMYYYY,        DFF_DDMMMY,     // Di, 06. Oktober 64
    NF_DATE_SYS_NNDMMMMYYYY,        DFF_DDMMMYY,    // Di, 06. Oktober 1964
    NF_DATE_SYS_NNNNDMMMMYYYY,      DFF_DDDMMMYY,   // Dienstag, 06. Oktober 1964
    NF_DATE_SYS_NNNNDMMMMYYYY,      DFF_DDDMMMY,    // Dienstag, 06. Oktober 64
    NF_DATE_SYS_MMYY,               DFF_MY,         // 10.64
    NF_DATE_DIN_MMDD,               DFF_MD,         // 10-06
    NF_DATE_DIN_YYMMDD,             DFF_YMD,        // 64-10-06
    NF_DATE_DIN_YYYYMMDD,           DFF_YYMD,       // 1964-10-06

    NF_NUMERIC_START,               0               // Tabellenende
};

static OldFormats aOldDateFmt30[] =
{
    // Datumsfelder:
    NF_DATE_SYSTEM_SHORT,           DFF_SSYS,       // Kurzes Systemdatum
    NF_DATE_SYSTEM_LONG,            DFF_LSYS,       // Langes Systemdatum
    NF_DATE_SYS_DDMMYY,             DFF_DMY,        // 06.10.64
    NF_DATE_SYS_DDMMYYYY,           DFF_DMYY,       // 06.10.1964
    NF_DATE_SYS_DMMMYY,             DFF_DMMY,       // 06. Okt 64
    NF_DATE_SYS_DMMMYYYY,           4 /*DFF_DMMYY*/,    // 06. Okt 1964
    NF_DATE_DIN_DMMMMYYYY,          5 /*DFF_DMMMYY*/,   // 06. Oktober 1964
    NF_DATE_DIN_DMMMMYYYY,          5 /*DFF_DMMMY*/,    // 06. Oktober 64
    NF_DATE_SYS_NNDMMMMYYYY,        6 /*DFF_DDMMMYY*/,  // Di, 06. Oktober 1964
    NF_DATE_SYS_NNDMMMYY,           6 /*DFF_DDMMY*/,    // Di, 06. Okt 64
    NF_DATE_SYS_NNDMMMMYYYY,        6 /*DFF_DDMMMY*/,   // Di, 06. Oktober 64
    NF_DATE_SYS_NNNNDMMMMYYYY,      7 /*DFF_DDDMMMYY*/, // Dienstag, 06. Oktober 1964
    NF_DATE_SYS_NNNNDMMMMYYYY,      7 /*DFF_DDDMMMY*/,  // Dienstag, 06. Oktober 64
    NF_DATE_SYS_MMYY,               2 /*DFF_MY*/,       // 10.64
    NF_DATE_DIN_MMDD,               DFF_MD,         // 10-06
    NF_DATE_DIN_YYMMDD,             DFF_YMD,        // 64-10-06
    NF_DATE_DIN_YYYYMMDD,           DFF_YYMD,       // 1964-10-06

    NF_NUMERIC_START,               0               // Tabellenende
};

static OldFormats aOldTimeFmt[] =
{
    // Zeitfelder:
    NF_TIME_HHMMSS,                 TF_SYSTEM,      // Systemzeit
    NF_TIME_HHMM,                   TF_SSMM_24,     // 23:25
    NF_TIME_HHMMAMPM,               TF_SSMM_12,     // 11:25 PM

    NF_NUMERIC_START,               0               // Tabellenende
};

static OldFormats aOldGetSetExpFmt40[] =
{
    NF_TEXT,                        VVF_CMD,        // Kommando anzeigen
    NF_TEXT,                        VVF_INVISIBLE,  // unsichtbar
    NF_PERCENT_INT,                 VVF_XXP,        // 1234%
    NF_PERCENT_DEC2,                VVF_XX_XXP,     // 1.234,56%
    NF_TEXT,                        VVF_CLEAR,      // ???

    NF_NUMBER_SYSTEM,               VVF_SYS,        // Zahlenformat aus der
                                                    // Systemeinstellung
    NF_NUMBER_INT,                  VVF_X,          // 1234
    NF_NUMBER_DEC2,                 VVF_X_X,        // 1234,5
    NF_NUMBER_DEC2,                 VVF_X_XX,       // 1245,56
    NF_NUMBER_1000DEC2,             VVF_XX_XX,      // 1.234,56
    NF_NUMBER_1000DEC2,             VVF_XX_X,       // 1.234,5
    NF_NUMBER_1000DEC2,             VVF_XX_XXX,     // 1.234,567
    NF_CURRENCY_1000DEC2,           VVF_SYS_CUR,    // W„hrungsformat aus der
                                                    // Systemeinstellung
                                                    // (1.234,00 DM)
    NF_CURRENCY_1000INT,            VVF_X_CUR,      // 1234 DM
    NF_CURRENCY_1000DEC2,           VVF_XX_XX_CUR,  // 1234,56 DM 1234,00 DM
    NF_CURRENCY_1000DEC2_DASHED,    VVF_XX_X0_CUR,  // 1234,56 DM 1234,-- DM
    NF_CURRENCY_1000INT,            VVF_CUR_X,      // DM 1234
    NF_CURRENCY_1000DEC2,           VVF_CUR_XX_XX,  // DM 1234,56 DM 1234,00
    NF_CURRENCY_1000DEC2_DASHED,    VVF_CUR_XX_X0,  // DM 1234,56 DM 1234,--

    NF_NUMERIC_START,               0               // Tabellenende
};

static OldFormats aOldGetSetExpFmt30[] =
{
    NF_TEXT,                        VVF_CMD,        // Kommando anzeigen
    NF_TEXT,                        VVF_INVISIBLE,  // unsichtbar
    NF_PERCENT_INT,                 VVF_XXP,        // 1234%
    NF_PERCENT_DEC2,                VVF_XX_XXP,     // 1.234,56%
    NF_TEXT,                        VVF_CLEAR,      // ???

    NF_NUMBER_SYSTEM,               0x0020,         // Zahlenformat aus der
                                                    // Systemeinstellung
    NF_NUMBER_INT,                  0x0080,         // 1234
    NF_NUMBER_1000DEC2,             0x0100,         // 1.234,56
    NF_NUMBER_DEC2,                 0x0100,         // 1234,5
    NF_NUMBER_DEC2,                 0x0100,         // 1245,56
    NF_NUMBER_1000DEC2,             0x0100,         // 1.234,5
    NF_NUMBER_1000DEC2,             0x0100,         // 1.234,567
    NF_CURRENCY_1000DEC2,           0x0200,         // W„hrungsformat aus der
                                                    // Systemeinstellung
                                                    // (1.234,00 DM)
    NF_CURRENCY_1000INT,            0x1000,         // 1234 DM
    NF_CURRENCY_1000DEC2,           0x1000,         // 1234,56 DM 1234,00 DM
    NF_CURRENCY_1000DEC2_DASHED,    0x1000,         // 1234,56 DM 1234,-- DM
    NF_CURRENCY_1000INT,            0x1000,         // DM 1234
    NF_CURRENCY_1000DEC2,           0x1000,         // DM 1234,56 DM 1234,00
    NF_CURRENCY_1000DEC2_DASHED,    0x1000,         // DM 1234,56 DM 1234,--

    NF_NUMERIC_START,               0               // Tabellenende
};

void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                USHORT& rSubType, ULONG &rFmt,
                                USHORT nVersion )
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
                rSubType = SUB_OWN_FMT;
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
                    rSubType = SUB_INVISIBLE;
                    rFmt = 0;
                }
                else if( rFmt == VVF_CMD )
                {
                    rSubType = SUB_CMD;
                    rFmt = 0;
                }
                else
                {
                    // Kleiner Hack: Bei Numernkreisen wird das
                    // unkonvertierte Format noch benoetigt. Wir merken es
                    // uns voruebergehend mal im Subtyp, sofern es
                    // ueberhaupt als entsprechendes Format in Frage kommt.
                    if( RES_SETEXPFLD==rWhich &&
                        rFmt >= (USHORT)SVX_NUM_CHARS_UPPER_LETTER &&
                        rFmt <= (USHORT)SVX_NUM_BITMAP )
                    {
                        rSubType = (USHORT)rFmt;
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
        USHORT i = 0;

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
void sw3io_ConvertToOldField( const SwField* pFld, USHORT& rWhich,
                              ULONG& rFmt, ULONG nFFVersion )
{
    const OldFormats *pOldFmt = 0L;
    ULONG nOldFmt = rFmt;

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
                USHORT nSubType = ((SwDateTimeField*) pFld)->GetSubType();
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
                USHORT nSubType = pFld->GetSubType();

                if (nSubType & SUB_INVISIBLE)
                    rFmt = VVF_INVISIBLE;
                else if (nSubType & SUB_CMD)
                    rFmt = VVF_CMD;
                else if( !(GSE_SEQ & nSubType) )
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
                // case REF_UPDOWN:
                // case REF_PAGE_PGDESC:
                    rFmt = REF_PAGE;
                    break;
                }
            }
            break;
    }

    if( pOldFmt && nOldFmt )
    {
        USHORT i = 0;

        SvNumberFormatter *pFormatter = ((SwValueField*)pFld)->GetDoc()->GetNumberFormatter();
        const SvNumberformat* pEntry = pFormatter->GetEntry( nOldFmt );

        if( pEntry )
        {
            while( pOldFmt[i].eFormatIdx != NF_NUMERIC_START ||
                   pOldFmt[i].nOldFormat )
            {
                ULONG nKey = pFormatter->GetFormatIndex(
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
void lcl_sw3io__ConvertNumTabStop( SvxTabStopItem& rTStop, long nOffset )
{
    for( USHORT n = 0; n < rTStop.Count(); ++n )
    {
        SvxTabStop& rTab = (SvxTabStop&)rTStop[ n ];
        if( SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment() &&
            rTab.GetTabPos() )
        {
            rTab.GetTabPos() += nOffset;
        }
    }
}
void lcl_sw3io__ConvertNumTabStop( SwTxtNode& rTxtNd, long nOffset,
                                   BOOL bDeep )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rTxtNd.GetSwAttrSet().GetItemState( RES_PARATR_TABSTOP,
                                                            bDeep, &pItem ) )
    {
        SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
        lcl_sw3io__ConvertNumTabStop( aTStop, nOffset );
        rTxtNd.SwCntntNode::SetAttr( aTStop );
    }
}

void lcl_sw3io_ChkHiddenExp( String& rCond )
{
    // die Expression wurde bei 4.0 Export einmal gedreht, beim erneuten
    // Einlesen sollte diese nicht noch mal gedreht werden.
    xub_StrLen nLen = rCond.Len(), nPos = nLen, nCnt = 1;
    if( 3 < nPos-- && ')' == rCond.GetChar( nPos ) &&
        '!' == rCond.GetChar( nPos = 0 ) && '(' == rCond.GetChar( ++nPos ))
    {
        // dann teste mal ob es dann eine komplette Klammerung ist
        --nLen; ++nPos;
        nCnt = 0;
        while( nPos < nLen )
            switch( rCond.GetChar( nPos++ ) )
            {
            case '(':       ++nCnt;     break;
            case ')':       if( !nCnt-- )
                                nPos = nLen;
                            break;
            }
    }

    if( !nCnt )
        rCond = rCond.Copy( 2, rCond.Len() - 3);
    else
        rCond.InsertAscii( "!(", 0 ) += ')';
}

void lcl_sw3io__ConvertNumLRSpace( SwTxtNode& rTxtNd, const SwNumRule& rNumRule,
                                   BYTE nLevel, BOOL bTabStop )
{
    if( nLevel == NO_NUMBERING )
        return;

    nLevel = GetRealLevel( nLevel );
    USHORT nNumLSpace = rNumRule.Get( nLevel ).GetAbsLSpace();

    // Wenn im Absatz der Einzug eingestellt ist, dann will
    // man den wieder Erreichen, sonst den der NumRule.
    const SfxPoolItem *pItem;
    const SvxLRSpaceItem *pParaLRSpace = 0;
    const SfxItemSet *pAttrSet = rTxtNd.GetpSwAttrSet();
    if( pAttrSet &&
        SFX_ITEM_SET == pAttrSet->GetItemState( RES_LR_SPACE, FALSE, &pItem ) )
        pParaLRSpace = (const SvxLRSpaceItem *)pItem;
    USHORT nWishLSpace = pParaLRSpace ? pParaLRSpace->GetTxtLeft() : nNumLSpace;

    // Dazu muss man den folgenden Wert im Absatz einstellen
    const SvxLRSpaceItem& rCollLRSpace = rTxtNd.GetAnyFmtColl().GetLRSpace();
    USHORT nOldLSpace = pParaLRSpace ? pParaLRSpace->GetTxtLeft()
                                     : rCollLRSpace.GetTxtLeft();
    USHORT nNewLSpace;
    if( rNumRule.IsAbsSpaces() )
        nNewLSpace = rCollLRSpace.GetTxtLeft();
    else
        nNewLSpace = nWishLSpace > nNumLSpace ? nWishLSpace - nNumLSpace : 0U;

    // Wenn der neue Wert zufaellig der der Vorlage ist und der
    // rechte Einzug auch mit dem der Vorlage ueberseinstimmt,
    // dann braucht bzw. darf man das Absatz-Attribut nicht
    // setzen, sonst muss man es setzen.
    if( nNewLSpace == rCollLRSpace.GetTxtLeft() &&
        (!pParaLRSpace || pParaLRSpace->GetRight() == rCollLRSpace.GetRight()) )
    {
        if( pParaLRSpace )
            rTxtNd.ResetAttr( RES_LR_SPACE );
    }
    else
    {
        if( !pParaLRSpace )
            pParaLRSpace = &rCollLRSpace;
        SvxLRSpaceItem aLRSpace( *pParaLRSpace );
        short nFirst = aLRSpace.GetTxtFirstLineOfst();
        if( nFirst < 0 && (USHORT)-nFirst > nNewLSpace )
            aLRSpace.SetTxtFirstLineOfst( -(short)nNewLSpace );
        aLRSpace.SetTxtLeft( nNewLSpace );
        ((SwCntntNode&)rTxtNd).SetAttr( aLRSpace );
    }

    if( bTabStop && nOldLSpace != nNewLSpace )
        lcl_sw3io__ConvertNumTabStop( rTxtNd,
                                      (long)nOldLSpace - (long)nNewLSpace,
                                      TRUE );
}

