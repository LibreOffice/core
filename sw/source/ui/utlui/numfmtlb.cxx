/*************************************************************************
 *
 *  $RCSfile: numfmtlb.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 14:18:07 $
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

#include "hintids.hxx"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#define _ZFORLIST_DECLARE_TABLE
#include <svtools/zformat.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_NUMINF_HXX //autogen
#define ITEMID_NUMBERINFO SID_ATTR_NUMBERFORMAT_INFO
#include <svx/numinf.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_NUMFMT_HXX //autogen
#include <svx/numfmt.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif

#include "tblnumfm.hxx"
#include "docsh.hxx"
#include "swtypes.hxx"
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"

#include "utlui.hrc"
#include "numfmtlb.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
#define C2S(cChar) UniString::CreateFromAscii(cChar)

// STATIC DATA -----------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung:
                    nFormatType: Formate dieses Typs anzeigen
                    nDefFmt:     Dieses Format selektieren und ggf vorher
                                 einfuegen
 --------------------------------------------------------------------*/

NumFormatListBox::NumFormatListBox( Window* pWin, const ResId& rResId,
                                    short nFormatType, ULONG nDefFmt,
                                    BOOL bUsrFmts ) :
    ListBox             ( pWin, rResId ),
    nCurrFormatType     (-1),
    nStdEntry           (0),
    bOneArea            (FALSE),
    nDefFormat          (nDefFmt),
    pVw                 (0),
    pOwnFormatter       (0)
{
    Init(nFormatType, bUsrFmts);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

NumFormatListBox::NumFormatListBox( Window* pWin, SwView* pView,
                                    const ResId& rResId, short nFormatType,
                                    ULONG nDefFmt, BOOL bUsrFmts ) :
    ListBox             ( pWin, rResId ),
    nCurrFormatType     (-1),
    nStdEntry           (0),
    bOneArea            (FALSE),
    nDefFormat          (nDefFmt),
    pVw                 (pView),
    pOwnFormatter       (0)
{
    Init(nFormatType, bUsrFmts);
}

/* -----------------15.06.98 11:29-------------------
 *
 * --------------------------------------------------*/

void NumFormatListBox::Init(short nFormatType, BOOL bUsrFmts)
{
    SwView *pView = GetView();

    if (pView)
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE);
        rSh.GetAttr(aSet);
        eCurLanguage = ((const SvxLanguageItem&)aSet.Get( RES_CHRATR_LANGUAGE ) ).GetLanguage();
    }
    else
        eCurLanguage = Application::GetAppInternational().GetLanguage();

    if (bUsrFmts == FALSE)
       {
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        pOwnFormatter = new SvNumberFormatter(xMSF, eCurLanguage);
    }

    SetFormatType(nFormatType);
    SetDefFormat(nDefFormat);

    SetSelectHdl(LINK(this, NumFormatListBox, SelectHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

NumFormatListBox::~NumFormatListBox()
{
    if (pOwnFormatter)
        delete pOwnFormatter;
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwView* NumFormatListBox::GetView()
{
    if( pVw )
        return pVw;
    return ::GetActiveView();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void NumFormatListBox::SetFormatType(const short nFormatType)
{
    if (nCurrFormatType == -1 ||
        (nCurrFormatType & nFormatType) == 0)   // Es gibt Mischformate, wie z.B. DateTime
    {
        SvNumberFormatter* pFormatter;

        if( pOwnFormatter )
            pFormatter = pOwnFormatter;
        else
        {
            SwView *pView = GetView();
            SwWrtShell &rSh = pView->GetWrtShell();
            pFormatter = rSh.GetNumberFormatter();
        }

        Clear();    // Alle Eintraege in der Listbox entfernen

        NfIndexTableOffset eOffsetStart = NF_NUMBER_START;
        NfIndexTableOffset eOffsetEnd = NF_NUMBER_START;

        switch( nFormatType )
        {
        case NUMBERFORMAT_NUMBER:
            eOffsetStart=NF_NUMBER_START;
            eOffsetEnd=NF_NUMBER_END;
            break;

        case NUMBERFORMAT_PERCENT:
            eOffsetStart=NF_PERCENT_START;
            eOffsetEnd=NF_PERCENT_END;
            break;

        case NUMBERFORMAT_CURRENCY:
            eOffsetStart=NF_CURRENCY_START;
            eOffsetEnd=NF_CURRENCY_END;
            break;

        case NUMBERFORMAT_DATETIME:
            eOffsetStart=NF_DATE_START;
            eOffsetEnd=NF_TIME_END;
            break;

        case NUMBERFORMAT_DATE:
            eOffsetStart=NF_DATE_START;
            eOffsetEnd=NF_DATE_END;
            break;

        case NUMBERFORMAT_TIME:
            eOffsetStart=NF_TIME_START;
            eOffsetEnd=NF_TIME_END;
            break;

        case NUMBERFORMAT_SCIENTIFIC:
            eOffsetStart=NF_SCIENTIFIC_START;
            eOffsetEnd=NF_SCIENTIFIC_END;
            break;

        case NUMBERFORMAT_FRACTION:
            eOffsetStart=NF_FRACTION_START;
            eOffsetEnd=NF_FRACTION_END;
            break;

        case NUMBERFORMAT_LOGICAL:
            eOffsetStart=NF_BOOLEAN;
            eOffsetEnd=NF_BOOLEAN;
            break;

        case NUMBERFORMAT_TEXT:
            eOffsetStart=NF_TEXT;
            eOffsetEnd=NF_TEXT;
            break;

        case NUMBERFORMAT_ALL:
            eOffsetStart=NF_NUMERIC_START;
            eOffsetEnd = NfIndexTableOffset( NF_INDEX_TABLE_ENTRIES - 1 );
            break;

        default:
            DBG_ERROR("what a format?");
            break;
        }

        const SvNumberformat* pFmt;
        USHORT nPos, i = 0;
        ULONG  nFormat;
        Color* pCol;
        double fVal = GetDefValue( pFormatter, nFormatType );
        String sValue;

        ULONG nSysNumFmt = pFormatter->GetFormatIndex(
                                        NF_NUMBER_SYSTEM, eCurLanguage );
        ULONG nSysShortDateFmt = pFormatter->GetFormatIndex(
                                        NF_DATE_SYSTEM_SHORT, eCurLanguage );
        ULONG nSysLongDateFmt = pFormatter->GetFormatIndex(
                                        NF_DATE_SYSTEM_LONG, eCurLanguage );

        for( long nIndex = eOffsetStart; nIndex <= eOffsetEnd; ++nIndex )
        {
            nFormat = pFormatter->GetFormatIndex(
                            (NfIndexTableOffset)nIndex, eCurLanguage );
            pFmt = pFormatter->GetEntry( nFormat );

            if( nFormat == pFormatter->GetFormatIndex( NF_NUMBER_STANDARD,
                                                        eCurLanguage )
                || ((SvNumberformat*)pFmt)->GetOutputString( fVal, sValue, &pCol )
                || nFormatType == NUMBERFORMAT_UNDEFINED )
                    sValue = pFmt->GetFormatstring();
            else if( nFormatType == NUMBERFORMAT_TEXT )
            {
                String sTxt(C2S("\"ABC\""));
                pFormatter->GetOutputString( sTxt, nFormat, sValue, &pCol);
            }

            if (nFormat != nSysNumFmt       &&
                nFormat != nSysShortDateFmt &&
                nFormat != nSysLongDateFmt)
            {
                nPos = InsertEntry( sValue );
                SetEntryData( nPos, (void*)nFormat );

                if( nFormat == pFormatter->GetStandardFormat(
                                        nFormatType, eCurLanguage ) )
                    nStdEntry = i;
                ++i;
            }
        }

        if (!pOwnFormatter)
        {
            nPos = InsertEntry(SW_RESSTR( STR_DEFINE_NUMBERFORMAT ));
            SetEntryData( nPos, NULL );
        }

        SelectEntryPos( nStdEntry );

        nCurrFormatType = nFormatType;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void NumFormatListBox::SetDefFormat(const ULONG nDefFmt)
{
    if (nDefFmt == ULONG_MAX)
    {
        nDefFormat = nDefFmt;
        return;
    }

    if (!nDefFmt)
        SelectEntryPos(nStdEntry);
    else
    {
        SvNumberFormatter* pFormatter;

        if (pOwnFormatter)
            pFormatter = pOwnFormatter;
        else
        {
            SwView *pView = GetView();
            SwWrtShell &rSh = pView->GetWrtShell();
            pFormatter = rSh.GetNumberFormatter();
        }

        short nType = pFormatter->GetType(nDefFmt);

        SetFormatType(nType);

        ULONG nFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nDefFmt, eCurLanguage);

        for (USHORT i = 0; i < GetEntryCount(); i++)
        {
            if (nFormat == (ULONG)GetEntryData(i))
            {
                SelectEntryPos(i);
                nStdEntry = i;
                nDefFormat = GetFormat();
                return;
            }
        }

        // Kein Eintrag gefunden:
        double fValue = GetDefValue(pFormatter, nType);
        String sValue;
        Color* pCol = 0;

        if (nType == NUMBERFORMAT_TEXT)
        {
            String sTxt(C2S("\"ABC\""));
            pFormatter->GetOutputString(sTxt, nDefFmt, sValue, &pCol);
        }
        else
            pFormatter->GetOutputString(fValue, nDefFmt, sValue, &pCol);

        USHORT nPos = 0;
        while ((ULONG)GetEntryData(nPos) == ULONG_MAX)
            nPos++;

//
        ULONG nSysNumFmt = pFormatter->GetFormatIndex( NF_NUMBER_SYSTEM, eCurLanguage);
        ULONG nSysShortDateFmt = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eCurLanguage);
        ULONG nSysLongDateFmt = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_LONG, eCurLanguage);
        BOOL bSysLang = FALSE;
        if(::GetSystemLanguage() == eCurLanguage)
            bSysLang = TRUE;
        ULONG nNumFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysNumFmt, LANGUAGE_SYSTEM );
        ULONG nShortDateFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysShortDateFmt, LANGUAGE_SYSTEM );
        ULONG nLongDateFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysLongDateFmt, LANGUAGE_SYSTEM );

        if(nDefFmt == nSysNumFmt||
            nDefFmt == nSysShortDateFmt||
            nDefFmt == nSysLongDateFmt||
            bSysLang && (nDefFmt == nNumFormatForLanguage ||
            nDefFmt == nShortDateFormatForLanguage ||
            nDefFmt == nLongDateFormatForLanguage ))
            sValue += String(SW_RES(RID_STR_SYSTEM));

        nPos = InsertEntry(sValue, nPos);   // Als ersten numerischen Eintrag einfuegen
        SetEntryData(nPos, (void*)nDefFmt);
        SelectEntryPos(nPos);
    }
    nDefFormat = GetFormat();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

ULONG NumFormatListBox::GetFormat() const
{
    USHORT nPos = GetSelectEntryPos();

    return (ULONG)GetEntryData(nPos);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

const String& NumFormatListBox::GetFormatStr() const
{
    return aEmptyStr;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( NumFormatListBox, SelectHdl, ListBox *, pBox )
{
    USHORT nPos = pBox->GetSelectEntryPos();
    String sDefine(SW_RES( STR_DEFINE_NUMBERFORMAT ));
    SwView *pView = GetView();

    if (pView && nPos == pBox->GetEntryCount() - 1 && pBox->GetEntry(nPos) == sDefine)
    {
        SvNumberFormatter* pFormatter;

        SwWrtShell &rSh = pView->GetWrtShell();
        pFormatter = rSh.GetNumberFormatter();

        SfxItemSet aCoreSet( rSh.GetAttrPool(),
                            SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,
                            SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO,
                            SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
                            SID_ATTR_NUMBERFORMAT_NOLANGUAGE, SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
                            0 );


        double fValue = GetDefValue(pFormatter, nCurrFormatType);

        ULONG nFormat = pFormatter->GetStandardFormat(nCurrFormatType, eCurLanguage);
        SfxUInt32Item aVal(SID_ATTR_NUMBERFORMAT_VALUE, nFormat);
        aCoreSet.Put(aVal);

        SvxNumberInfoItem aNumInf(pFormatter, fValue, SID_ATTR_NUMBERFORMAT_INFO);
        aCoreSet.Put(aNumInf);

        if ((nCurrFormatType & NUMBERFORMAT_DATE) || (nCurrFormatType & NUMBERFORMAT_TIME))
            aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, bOneArea));

        // Keine Sprachauswahl im Dialog, da Sprache im Textattribut enthalten ist
        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_NOLANGUAGE, TRUE));

        SwNumFmtDlg* pDlg = new SwNumFmtDlg(this, aCoreSet);

        if (RET_OK == pDlg->Execute())
        {
            const SfxPoolItem* pItem = pView->GetDocShell()->
                            GetItem( SID_ATTR_NUMBERFORMAT_INFO );

            if( pItem && 0 != ((SvxNumberInfoItem*)pItem)->GetDelCount() )
            {
                const ULONG* pDelArr = ((SvxNumberInfoItem*)pItem)->GetDelArray();

                for ( USHORT i = 0; i < ((SvxNumberInfoItem*)pItem)->GetDelCount(); i++ )
                    pFormatter->DeleteEntry( pDelArr[i] );
            }

            if( SFX_ITEM_SET == pDlg->GetOutputItemSet()->GetItemState(
                SID_ATTR_NUMBERFORMAT_VALUE, FALSE, &pItem ))
            {
                SetDefFormat(((SfxUInt32Item*)pItem)->GetValue());
            }
        }
        else
            SetDefFormat(nFormat);

        delete pDlg;
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

double NumFormatListBox::GetDefValue(SvNumberFormatter* pFormatter, const short nFormatType) const
{
    double fDefValue = 0.0;

    switch (nFormatType)
    {
        case NUMBERFORMAT_DATE:
        case NUMBERFORMAT_DATE|NUMBERFORMAT_TIME:
            fDefValue = SVX_NUMVAL_DATE;
            break;

        case NUMBERFORMAT_TIME:
            fDefValue = SVX_NUMVAL_TIME;
            break;
/*      {
            String sValue("31.8.1997 16:57:34");
            ULONG nFormat = pFormatter->GetStandardFormat(nFormatType, LANGUAGE_GERMAN);
            pFormatter->IsNumberFormat( sValue, nFormat, fDefValue );
        }
        break;*/

        case NUMBERFORMAT_TEXT:
        case NUMBERFORMAT_UNDEFINED:
        case NUMBERFORMAT_ENTRY_NOT_FOUND:
            fDefValue = 0;
            break;

        case NUMBERFORMAT_CURRENCY:
            fDefValue = SVX_NUMVAL_CURRENCY;
            break;

        case NUMBERFORMAT_PERCENT:
            fDefValue = SVX_NUMVAL_PERCENT;
            break;

        case NUMBERFORMAT_LOGICAL:
            fDefValue = SVX_NUMVAL_BOOLEAN;
            break;

        default:
            fDefValue = SVX_NUMVAL_STANDARD;
            break;
    }

    return fDefValue;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void NumFormatListBox::Clear()
{
    ListBox::Clear();
    nCurrFormatType = -1;
}

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/20 09:51:30  os
      change: use SvNumberFormatter ctor using XMultiServiceFactory

      Revision 1.1.1.1  2000/09/18 17:14:50  hr
      initial import

      Revision 1.31  2000/09/18 16:06:18  willem.vandorp
      OpenOffice header added.

      Revision 1.30  2000/04/18 15:14:08  os
      UNICODE

      Revision 1.29  2000/03/03 15:17:05  os
      StarView remainders removed

      Revision 1.28  2000/02/25 09:53:39  hr
      #73447#: removed temporary

      Revision 1.27  2000/02/11 15:00:49  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.26  1999/06/28 12:54:58  JP
      Bug #67242#: SetFormatType - use eCurLanguage


      Rev 1.25   28 Jun 1999 14:54:58   JP
   Bug #67242#: SetFormatType - use eCurLanguage

      Rev 1.24   28 Jun 1999 13:57:44   JP
   Bug #67247#: SetFormatType - support of FormatTypeAll

      Rev 1.23   18 Jun 1999 15:28:02   OS
   #60657# Manual sorted number formats

      Rev 1.22   10 Jun 1999 13:14:52   JP
   have to change: no AppWin from SfxApp

      Rev 1.21   10 Feb 1999 14:13:02   OS
   #52055# Systemformate koennen auch unter 0x5000 liegen

      Rev 1.20   21 Jan 1999 11:07:18   OS
   #52055# [System] an Systemformat anhaengen

      Rev 1.19   15 Jun 1998 12:27:56   OM
   #51009# Numberformatter-Listbox ViewPtr reinreichen

      Rev 1.18   20 May 1998 14:24:02   OM
   Kombinierte DateTime-Formate verarbeiten

      Rev 1.17   19 May 1998 14:24:30   OM
   #47310 Definierte Standardwerte und keine Sprachauswahl

      Rev 1.16   11 Feb 1998 16:30:32   OM
   Textdarstellung fuer Felder

      Rev 1.15   13 Jan 1998 15:03:10   OM
   Formula-Field wieder unterstuetzt

      Rev 1.14   12 Jan 1998 12:57:12   OM
   Alle Sprachen anzeigen

      Rev 1.13   12 Jan 1998 12:42:16   OM
   Alle Sprachen anzeigen

      Rev 1.12   09 Jan 1998 10:07:26   OM
   Sprache organisieren

      Rev 1.11   08 Jan 1998 17:49:00   OM
   Neue Datumsfelder einfuegen

      Rev 1.10   06 Jan 1998 18:14:12   OM
   Felbefehl-Dlg

      Rev 1.9   19 Dec 1997 18:23:54   OM
   Feldbefehl-bearbeiten Dlg

      Rev 1.8   16 Dec 1997 17:04:50   OM
   Feldbefehle bearbeiten

      Rev 1.7   11 Dec 1997 17:01:26   OM
   Feldumstellung

      Rev 1.6   29 Nov 1997 15:08:58   MA
   includes

      Rev 1.5   21 Nov 1997 17:19:04   OM
   Feldbefehl-Umstellung: DocInfo

      Rev 1.4   21 Nov 1997 12:10:16   MA
   includes

      Rev 1.3   20 Nov 1997 17:02:34   OM
   Neuer Felddialog

      Rev 1.2   10 Nov 1997 15:29:04   OM
   Zahlenformat-Listbox

      Rev 1.1   07 Nov 1997 16:56:08   OM
   NumberFormat-Listbox

      Rev 1.0   07 Nov 1997 16:37:08   OM
   Initial revision.

*************************************************************************/
