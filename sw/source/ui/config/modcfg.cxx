/*************************************************************************
 *
 *  $RCSfile: modcfg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:33 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _MAILENUM_HXX //autogen
#include <goodies/mailenum.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _WORDSEL_HXX
#include <svtools/wordsel.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _FACTORY_HXX //autogen
#include <so3/factory.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _ITABENUM_HXX
#include <itabenum.hxx>
#endif
#ifndef _CFGID_H
#include <cfgid.h>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _CFGSTR_HRC
#include <cfgstr.hrc>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif


#define VERSION_01      1
#define VERSION_02      2
#define VERSION_03      3
#define VERSION_04      4
#define VERSION_05      5
#define VERSION_06      6
#define VERSION_07      7
#define VERSION_08      8
#define VERSION_09      9
#define VERSION_10      10
#define VERSION_11      11
#define VERSION_12      12
#define VERSION_13      13
#define VERSION_14      14      //nFldUpdateFlags
#define VERSION_15      15      //Insert-Flags auch fuer HTML
#define VERSION_16      16      //Insert-Flags auch fuer HTML
#define VERSION_17      17      //Preview flags AutoText + Index
#define VERSION_18      18      //bDefaultFontsInCurrDocOnly
#define MODCFG_VERSION  VERSION_18

SV_IMPL_PTRARR_SORT(InsCapOptArr, InsCaptionOptPtr)

/* -----------------03.11.98 13:46-------------------
 *
 * --------------------------------------------------*/

InsCaptionOpt* InsCaptionOptArr::Find(const SwCapObjType eType, const SvGlobalName *pOleId) const
{
    for (USHORT i = 0; i < Count(); i++ )
    {
        InsCaptionOpt* pObj = GetObject(i);
        if (pObj->GetObjType() == eType &&
            (eType != OLE_CAP ||
             (pOleId &&
              (pObj->GetOleId() == *pOleId ||
               SvFactory::GetAutoConvertTo(pObj->GetOleId()) == *pOleId))))
            return pObj;
    }

    return 0;
}

/* -----------------03.11.98 15:05-------------------
 *
 * --------------------------------------------------*/

const InsCaptionOpt* SwModuleOptions::GetCapOption(BOOL bHTML, const SwCapObjType eType, const SvGlobalName *pOleId)
{
    return bHTML ? aHTMLCapOptions.Find(eType, pOleId) : aCapOptions.Find(eType, pOleId);
}

/* -----------------03.11.98 15:05-------------------
 *
 * --------------------------------------------------*/

BOOL SwModuleOptions::SetCapOption(BOOL bHTML, const InsCaptionOpt* pOpt)
{
    BOOL bRet = FALSE;

    if (pOpt)
    {
        InsCaptionOptArr& rArr = bHTML ? aHTMLCapOptions : aCapOptions;
        InsCaptionOpt *pObj = rArr.Find(pOpt->GetObjType(), &pOpt->GetOleId());

        if (pObj)
        {
            *pObj = *pOpt;
            SetDefault(FALSE);
        }
        else
            rArr.Insert(new InsCaptionOpt(*pOpt));

        SetDefault(FALSE);
        bRet = TRUE;
    }

    return bRet;
}

/*-----------------13.01.97 12.44-------------------

--------------------------------------------------*/

int SwModuleOptions::Load(SvStream& rStream)
{
    rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
    int nRet;
    SetDefault(FALSE);
    UINT16 nVersion;
    rStream >> nVersion;
    if(nVersion >= VERSION_01)
    {
        UINT32 nColor;
        UINT16 nVal;
        rStream >>  nVal;
        nDefTab = nVal;

        rStream >>  nVal;
        nTblHMove = nVal;

        rStream >>  nVal;
        nTblVMove = nVal;

        rStream >> nVal;
        nTblHInsert = nVal;

        rStream >> nVal;
        nTblVInsert = nVal;

        BYTE bVal;
        rStream >> bVal;
        eUserMetric = (FieldUnit)bVal;
        if (nVersion >= VERSION_08)
        {
            rStream >> bVal;
            eWebUserMetric = (FieldUnit)bVal;
        }


        rStream >> bVal;
        eTblChgMode = (TblChgMode)bVal;

        if ( nVersion >= VERSION_02 )
        {
            rStream >> bVal;
            bGrfToGalleryAsLnk = BOOL(bVal);
        }
        if ( nVersion >= VERSION_03 )
        {
            rStream >> bVal;
            nMailingFormats = bVal;

            rStream >> bVal;
            bSinglePrintJob = BOOL(bVal);

            rStream >> bVal;
            bNumAlignSize = BOOL(bVal);
        }

        if ( nVersion >= VERSION_04 )
        {
            rStream >> bVal;
            bNameFromColumn = bVal;

            rStream.ReadByteString(sMailingPath, eEncoding);
            rStream.ReadByteString(sMailName, eEncoding);
        }
        if ( nVersion >= VERSION_05 )
        {

            rStream >> nVal;    aInsertAttr.nItemId = nVal;
            rStream >> nVal;    aInsertAttr.nAttr = nVal;
            rStream >> nColor;  aInsertAttr.nColor = nColor;

            rStream >> nVal;    aDeletedAttr.nItemId = nVal;
            rStream >> nVal;    aDeletedAttr.nAttr = nVal;
            rStream >> nColor;  aDeletedAttr.nColor = nColor;

            rStream >> nVal;    nMarkAlign = nVal;
            rStream >> nColor;  aMarkColor.SetColor(nColor);
        }

        if( nVersion >= VERSION_06 )
        {
            rStream >> nVal;    aFormatAttr.nItemId = nVal;
            rStream >> nVal;    aFormatAttr.nAttr = nVal;
            rStream >> nColor;  aFormatAttr.nColor = nColor;
        }

        if( nVersion >= VERSION_07 )
            rStream.ReadByteString(sWordDelimiter, eEncoding);

        if (nVersion >= VERSION_09)
        {
            rStream >> bVal;    bInsWithCaption = bVal;
            if (nVersion == VERSION_09)
                rStream >> bVal;    // Flag gibts nicht mehr

            if (nVersion < VERSION_13)
            {   // Flags gibts nicht mehr
                rStream >> bVal;
                rStream >> bVal;
            }

            rStream >> nVal;
            InsCaptionOpt aOpt;

            for (USHORT i = 0; i < nVal; i++)
            {
                rStream >> aOpt;
                aCapOptions.Insert(new InsCaptionOpt(aOpt));
            }
        }

        if (nVersion >= VERSION_10)
        {
            if (nVersion < VERSION_13)
                rStream >> bVal;    // Flag gibts nicht mehr

            if (nVersion >= VERSION_11)
            {
                rStream >> bVal;    bInsTblFormatNum = bVal;
                rStream >> bVal;    bInsTblAlignNum = bVal;
                rStream >> nVal;    nLinkMode = nVal;

                if (nVersion == VERSION_12)
                    rStream >> bVal;    // Flag gibts nicht mehr

                if (nVersion >= VERSION_13)
                {
                    rStream >> nVal;    nInsTblFlags = nVal;
                }
                if(nVersion >= VERSION_14)
                {
                    rStream>> nVal; nFldUpdateFlags = nVal;
                }
                else
                    nFldUpdateFlags = AUTOUPD_OFF;
                if(nVersion >= VERSION_15)
                {
                    rStream >> bVal; bHTMLInsWithCaption = bVal;
                    rStream >> bVal; bHTMLInsTblFormatNum = bVal;
                    rStream >> bVal; bHTMLInsTblAlignNum = bVal;
                    rStream >> nVal; nHTMLInsTblFlags = nVal;
                    rStream >> nVal;
                    InsCaptionOpt aOpt;

                    for (USHORT i = 0; i < nVal; i++)
                    {
                        rStream >> aOpt;
                        aHTMLCapOptions.Insert(new InsCaptionOpt(aOpt));
                    }
                }
                else
                {
                    bHTMLInsWithCaption = bHTMLInsTblAlignNum = FALSE;
                    bHTMLInsTblFormatNum = TRUE;
                    nHTMLInsTblFlags = ALL_TBL_INS_ATTR;
                }

                if( nVersion >= VERSION_16)
                {
                    rStream >> bVal;    bInsTblChangeNumFormat = bVal;
                    rStream >> bVal;    bHTMLInsTblChangeNumFormat = bVal;
                    if(nVersion >= VERSION_17)
                    {
                        rStream >> bVal;      bShowAutoTextPreview = bVal;
                        rStream >> bVal;      bShowIndexPreview = bVal;
                        if(nVersion >= VERSION_18)
                        {
                            rStream >> bVal;      bDefaultFontsInCurrDocOnly = bVal;
                        }
                    }
                }
            }
        }

        nRet = SfxConfigItem::ERR_OK;
    }
    else
        nRet = SfxConfigItem::WARNING_VERSION;

    return nRet;
}

/*-----------------13.01.97 12.44-------------------

--------------------------------------------------*/

BOOL SwModuleOptions::Store(SvStream& rStream)
{
    rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
    rStream << (UINT16) MODCFG_VERSION;
    rStream << (UINT16) nDefTab;
    rStream << (UINT16) nTblHMove;
    rStream << (UINT16) nTblVMove;
    rStream << (UINT16) nTblHInsert;
    rStream << (UINT16) nTblVInsert;
    rStream << (BYTE)   eUserMetric;
    rStream << (BYTE)   eWebUserMetric;
    rStream << (BYTE)   eTblChgMode;
    rStream << (BYTE)   bGrfToGalleryAsLnk;
    rStream << (BYTE)   nMailingFormats;
    rStream << (BYTE)   bSinglePrintJob;
    rStream << (BYTE)   bNumAlignSize;
    rStream << (BYTE)   bNameFromColumn;
    rStream.WriteByteString(sMailingPath, eEncoding);
    rStream.WriteByteString(sMailName, eEncoding);
    rStream << (UINT16) aInsertAttr.nItemId;
    rStream << (UINT16) aInsertAttr.nAttr;
    rStream << (UINT32) aInsertAttr.nColor;
    rStream << (UINT16) aDeletedAttr.nItemId;
    rStream << (UINT16) aDeletedAttr.nAttr;
    rStream << (UINT32) aDeletedAttr.nColor;
    rStream << (UINT16) nMarkAlign;
    rStream << (UINT32) aMarkColor.GetColor();
    rStream << (UINT16) aFormatAttr.nItemId;
    rStream << (UINT16) aFormatAttr.nAttr;
    rStream << (UINT32) aFormatAttr.nColor;
    rStream.WriteByteString(sWordDelimiter, eEncoding);
    rStream << (BYTE)   bInsWithCaption;
    rStream << (UINT16) aCapOptions.Count();
    for (USHORT i = 0; i < aCapOptions.Count(); i++)
        rStream << *aCapOptions[i];
    rStream << (BYTE)   bInsTblFormatNum;
    rStream << (BYTE)   bInsTblAlignNum;
    rStream << (UINT16) nLinkMode;
    rStream << (UINT16) nInsTblFlags;
    rStream << (UINT16) nFldUpdateFlags;

    rStream << (BYTE)   bHTMLInsWithCaption;
    rStream << (BYTE)   bHTMLInsTblFormatNum;
    rStream << (BYTE)   bHTMLInsTblAlignNum;
    rStream << (UINT16) nHTMLInsTblFlags;
    rStream << (UINT16) aHTMLCapOptions.Count();
    for( i = 0; i < aHTMLCapOptions.Count(); i++)
        rStream << *aHTMLCapOptions[i];
    rStream << (BYTE)   bHTMLInsTblChangeNumFormat;
    rStream << (BYTE)   bInsTblChangeNumFormat;

    rStream << (BYTE)   bShowAutoTextPreview;
    rStream << (BYTE)   bShowIndexPreview;

    rStream << (BYTE)    bDefaultFontsInCurrDocOnly;
    return SfxConfigItem::ERR_OK;
}
/*-----------------13.01.97 12.44-------------------

--------------------------------------------------*/

void SwModuleOptions::UseDefault()
{
    MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
    if(MEASURE_METRIC != eSys)
    {
        eUserMetric = eWebUserMetric = FUNIT_INCH;
        nDefTab = 720;                  // 1/2"
        nTblHMove = nTblVMove = 360;    // 1/4"
        nTblHInsert = 360;              // 1/4"
        nTblVInsert = 1440;             // 1"
    }
    else
    {
        eUserMetric = eWebUserMetric = FUNIT_CM;
        nDefTab = 709;                  // 1,25 cm
        nTblHMove = nTblVMove = MM50;   // 0,5  cm
        nTblHInsert = MM50;             // 0,5  cm
        nTblVInsert = 1415;             // 2,5  cm
    }

    eTblChgMode = TBLVAR_CHGABS;

    aInsertAttr.nItemId = SID_ATTR_CHAR_UNDERLINE;
    aInsertAttr.nAttr = UNDERLINE_SINGLE;
    aInsertAttr.nColor = COL_TRANSPARENT;
    aDeletedAttr.nItemId = SID_ATTR_CHAR_STRIKEOUT;
    aDeletedAttr.nAttr = STRIKEOUT_SINGLE;
    aDeletedAttr.nColor = COL_TRANSPARENT;
    aFormatAttr.nItemId = SID_ATTR_CHAR_WEIGHT;
    aFormatAttr.nAttr = WEIGHT_BOLD;
    aFormatAttr.nColor = COL_BLACK;
    nMarkAlign = HORI_OUTSIDE;
    aMarkColor.SetColor(COL_BLACK);

    bGrfToGalleryAsLnk = TRUE;

    nMailingFormats = TXTFORMAT_OFFICE;
    bNumAlignSize = TRUE;
    bSinglePrintJob = FALSE;
    bNameFromColumn = FALSE;

    sWordDelimiter = String::CreateFromAscii("() \t\xa\x1");

    bHTMLInsWithCaption = bInsWithCaption = FALSE;

    nHTMLInsTblFlags = nInsTblFlags = ALL_TBL_INS_ATTR;
    bHTMLInsTblFormatNum = bInsTblFormatNum = TRUE;
    bHTMLInsTblChangeNumFormat = bInsTblChangeNumFormat = TRUE;
    bHTMLInsTblAlignNum = bInsTblAlignNum = TRUE;

    nLinkMode = MANUAL;
    nFldUpdateFlags = AUTOUPD_FIELD_ONLY;

    bShowAutoTextPreview = TRUE;
    bShowIndexPreview = TRUE;

    bDefaultFontsInCurrDocOnly = FALSE;

    aCapOptions.DeleteAndDestroy( 0, aCapOptions.Count() );
    aHTMLCapOptions.DeleteAndDestroy( 0, aCapOptions.Count() );

    SfxConfigItem::UseDefault();
}

/*-----------------13.01.97 12.44-------------------

--------------------------------------------------*/

SwModuleOptions::SwModuleOptions() :
    SfxConfigItem( CFG_SW_MODULE )
{
    UseDefault();
}

/*-----------------13.01.97 13.11-------------------

--------------------------------------------------*/

String SwModuleOptions::GetName() const
{
    return String(SW_RES(STR_CFG_MODULE));
}

/*--------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.34  2000/09/18 16:05:15  willem.vandorp
    OpenOffice header added.

    Revision 1.33  2000/07/20 13:16:58  jp
    change old txtatr-character to the two new characters

    Revision 1.32  2000/06/13 13:22:43  os
    #75770# Doc only flag in standard font TabPage now persistent

    Revision 1.31  2000/05/18 14:57:43  os
    append CH_TXTATR instead of 0xff in CreateFromAscii

    Revision 1.30  2000/04/11 08:02:23  os
    UNICODE

    Revision 1.29  2000/02/11 14:43:50  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.28  1999/12/29 07:54:07  os
    #71320# flags for AutoText and index preview

    Revision 1.27  1999/04/20 16:58:38  JP
    Task #65061#: neu: ZahlenFormaterkennung abschaltbar


      Rev 1.26   20 Apr 1999 18:58:38   JP
   Task #65061#: neu: ZahlenFormaterkennung abschaltbar

      Rev 1.25   15 Mar 1999 09:46:20   MA
   #63047# neue Defaults

      Rev 1.24   11 Mar 1999 23:56:40   JP
   Task #63171#: Optionen fuer Feld-/LinkUpdate Doc oder Modul lokal

      Rev 1.23   17 Feb 1999 08:37:56   OS
   #58158# Einfuegen TabPage auch in HTML-Docs

*************************************************************************/



