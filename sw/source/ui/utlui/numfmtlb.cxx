/*************************************************************************
 *
 *  $RCSfile: numfmtlb.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 12:01:50 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#define _ZFORLIST_DECLARE_TABLE
#include <svtools/zformat.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
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
#ifndef _SVX_NUMFMT_HXX //autogen
#include <svx/numfmt.hxx>
#endif

#ifndef _TBLNUMFM_HXX
#include <tblnumfm.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _NUMFMTLB_HXX
#include <numfmtlb.hxx>
#endif

#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif


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
    pOwnFormatter       (0),
    bUseAutomaticLanguage(TRUE),
    bShowLanguageControl(FALSE)
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
    pOwnFormatter       (0),
    bUseAutomaticLanguage(TRUE),
    bShowLanguageControl(FALSE)
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
        eCurLanguage = pView->GetWrtShell().GetCurLang();
    else
        eCurLanguage = SvxLocaleToLanguage( GetAppLocaleData().getLocale() );

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
            DBG_ASSERT(pView, "no view found")
            if(!pView)
                return;
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
        double fVal = GetDefValue( nFormatType );
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

    SvNumberFormatter* pFormatter;
    if (pOwnFormatter)
        pFormatter = pOwnFormatter;
    else
    {
        SwView *pView = GetView();
        DBG_ASSERT(pView, "no view found")
        if(!pView)
            return;
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
    double fValue = GetDefValue(nType);
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
    if( eCurLanguage == GetAppLanguage() )
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

    if( pView && nPos == pBox->GetEntryCount() - 1 &&
        pBox->GetEntry( nPos ) == sDefine )
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

        SfxItemSet aCoreSet( rSh.GetAttrPool(),
            SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,
            SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO,
            SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
            SID_ATTR_NUMBERFORMAT_NOLANGUAGE, SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
            SID_ATTR_NUMBERFORMAT_ADD_AUTO, SID_ATTR_NUMBERFORMAT_ADD_AUTO,
            0 );

        double fValue = GetDefValue( nCurrFormatType);

        ULONG nFormat = pFormatter->GetStandardFormat( nCurrFormatType, eCurLanguage);
        aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, nFormat ));

        aCoreSet.Put( SvxNumberInfoItem( pFormatter, fValue,
                                            SID_ATTR_NUMBERFORMAT_INFO ) );

        if( (NUMBERFORMAT_DATE | NUMBERFORMAT_TIME) & nCurrFormatType )
            aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, bOneArea));

        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_NOLANGUAGE, !bShowLanguageControl));
        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO, bUseAutomaticLanguage));

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

            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
            if( SFX_ITEM_SET == pOutSet->GetItemState(
                SID_ATTR_NUMBERFORMAT_VALUE, FALSE, &pItem ))
            {
                UINT32 nFormat = ((SfxUInt32Item*)pItem)->GetValue();
                // oj #105473# change order of calls
                eCurLanguage = pFormatter->GetEntry(nFormat)->GetLanguage();
                // SetDefFormat uses eCurLanguage to look for if this format already in the list
                SetDefFormat(nFormat);
            }
            if( bShowLanguageControl && SFX_ITEM_SET == pOutSet->GetItemState(
                SID_ATTR_NUMBERFORMAT_ADD_AUTO, FALSE, &pItem ))
            {
                bUseAutomaticLanguage = ((const SfxBoolItem*)pItem)->GetValue();
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

double NumFormatListBox::GetDefValue(const short nFormatType) const
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

