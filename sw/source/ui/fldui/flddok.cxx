/*************************************************************************
 *
 *  $RCSfile: flddok.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:36 $
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _FLDDAT_HXX //autogen
#include <flddat.hxx>
#endif
#ifndef _DOCUFLD_HXX //autogen
#include <docufld.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif

#ifndef _FLDTDLG_HRC
#include <fldtdlg.hrc>
#endif
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _FLDDOK_HXX
#include <flddok.hxx>
#endif

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldDokPage::SwFldDokPage(Window* pWindow, const SfxItemSet& rCoreSet ) :
    SwFldPage( pWindow, SW_RES( TP_FLD_DOK ), rCoreSet ),

    aTypeFT         (this, SW_RES(FT_DOKTYPE)),
    aTypeLB         (this, SW_RES(LB_DOKTYPE)),
    aSelectionFT    (this, SW_RES(FT_DOKSELECTION)),
    aSelectionLB    (this, SW_RES(LB_DOKSELECTION)),
    aValueFT        (this, SW_RES(FT_DOKVALUE)),
    aValueED        (this, SW_RES(ED_DOKVALUE)),
    aLevelED        (this, SW_RES(ED_DOKLEVEL)),
    aDateOffsetED   (this, SW_RES(ED_DOKDATEOFF)),

    aFormatFT       (this, SW_RES(FT_DOKFORMAT)),
    aFormatLB       (this, SW_RES(LB_DOKFORMAT)),
    aNumFormatLB    (this, SW_RES(LB_DOKNUMFORMAT)),
    aFixedCB        (this, SW_RES(CB_DOKFIXEDCONTENT)),

    sDateOffset     (SW_RES(STR_DOKDATEOFF)),
    sTimeOffset     (SW_RES(STR_DOKTIMEOFF)),
    aRootOpened     (SW_RES(BMP_DOKROOT_OPENED)),
    aRootClosed     (SW_RES(BMP_DOKROOT_CLOSED))
{
    FreeResource();

    aSelectionLB.SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    aFormatLB.SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    aNumFormatLB.SetDoubleClickHdl(LINK(this, SwFldDokPage, NumFormatHdl));

    aLevelED.SetMax(MAXLEVEL);
    aDateOffsetED.SetMin(LONG_MIN);
    aDateOffsetED.SetMax(LONG_MAX);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

__EXPORT SwFldDokPage::~SwFldDokPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void __EXPORT SwFldDokPage::Reset(const SfxItemSet& rSet)
{
    SavePos(&aTypeLB);
    Init(); // Allgemeine initialisierung

    // TypeListBox initialisieren
    const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

    aTypeLB.SetUpdateMode(FALSE);
    aTypeLB.Clear();

    USHORT nPos, nTypeId;
    BOOL bPage = FALSE;

    if (!IsFldEdit())
    {
        // Typ-Listbox fuellen
        for(short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);

            switch (nTypeId)
            {
                case TYP_PREVPAGEFLD:
                case TYP_NEXTPAGEFLD:
                case TYP_PAGENUMBERFLD:
                    if (!bPage)
                    {
                        nPos = aTypeLB.InsertEntry(SW_RESSTR(FMT_REF_PAGE));
                        aTypeLB.SetEntryData(nPos, (void*)USHRT_MAX);
                        bPage = TRUE;
                    }
                    break;

                default:
                    nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i));
                    aTypeLB.SetEntryData(nPos, (void*)nTypeId);
                    break;
            }
        }
    }
    else
    {
        nTypeId = GetCurField()->GetTypeId();
        if (nTypeId == TYP_FIXDATEFLD)
            nTypeId = TYP_DATEFLD;
        if (nTypeId == TYP_FIXTIMEFLD)
            nTypeId = TYP_TIMEFLD;
        nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        aTypeLB.SetEntryData(nPos, (void*)nTypeId);
    }

    // alte Pos selektieren
    RestorePos(&aTypeLB);

    aTypeLB.SetUpdateMode(TRUE);
    aTypeLB.SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    aTypeLB.SetSelectHdl(LINK(this, SwFldDokPage, TypeHdl));
    aFormatLB.SetSelectHdl(LINK(this, SwFldDokPage, FormatHdl));

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if( sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1 ))
        {
            String sVal = sUserData.GetToken(1, ';');
            USHORT nVal = sVal.ToInt32();
            if(nVal != USHRT_MAX)
            {
                for(USHORT i = 0; i < aTypeLB.GetEntryCount(); i++)
                    if(nVal == (USHORT)(ULONG)aTypeLB.GetEntryData(i))
                    {
                        aTypeLB.SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    if (IsFldEdit())
    {
        nOldSel = aSelectionLB.GetSelectEntryPos();
        nOldFormat = GetCurField()->GetFormat();
        aFixedCB.SaveValue();
        aValueED.SaveValue();
        aLevelED.SaveValue();
        aDateOffsetED.SaveValue();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDokPage, TypeHdl, ListBox *, EMPTYARG )
{
    // Alte ListBoxPos sichern
    const USHORT nOld = GetTypeSel();

    // Aktuelle ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        aTypeLB.SelectEntryPos(0);
    }

    USHORT nCount;

    if (nOld != GetTypeSel())
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

        // Auswahl-Listbox fuellen
        aSelectionLB.Clear();

        if (nTypeId != USHRT_MAX)
        {
            SvStringsDtor& rLst = GetFldMgr().GetSubTypes(nTypeId);

            if (nTypeId != TYP_AUTHORFLD)
                nCount = rLst.Count();
            else
                nCount = GetFldMgr().GetFormatCount(nTypeId, FALSE, IsFldDlgHtmlMode());

            USHORT nPos;

            for (USHORT i = 0; i < nCount; ++i)
            {
                if (!IsFldEdit())
                {
                    if (nTypeId != TYP_AUTHORFLD)
                        nPos = aSelectionLB.InsertEntry(*rLst[i]);
                    else
                        nPos = aSelectionLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));

                    aSelectionLB.SetEntryData(nPos, (void*)i);
                }
                else
                {
                    BOOL bInsert = FALSE;

                    switch (nTypeId)
                    {
                        case TYP_DATEFLD:
                        case TYP_TIMEFLD:
                            nPos = aSelectionLB.InsertEntry(*rLst[i]);
                            aSelectionLB.SetEntryData(nPos, (void*)i);
                            if (((SwDateTimeField*)GetCurField())->IsFixed() && !i)
                                aSelectionLB.SelectEntryPos(nPos);
                            if (!((SwDateTimeField*)GetCurField())->IsFixed() && i)
                                aSelectionLB.SelectEntryPos(nPos);
                            break;

                        case TYP_EXTUSERFLD:
                        case TYP_DOCSTATFLD:
                            nPos = aSelectionLB.InsertEntry(*rLst[i]);
                            aSelectionLB.SetEntryData(nPos, (void*)i);
                            if (GetCurField()->GetSubType() == i)
                                aSelectionLB.SelectEntryPos(nPos);
                            break;

                        case TYP_AUTHORFLD:
                        {
                            String sFmt(GetFldMgr().GetFormatStr(nTypeId, i));
                            nPos = aSelectionLB.InsertEntry(sFmt);
                            aSelectionLB.SetEntryData(nPos, (void*)i);
                            aSelectionLB.SelectEntry(GetFldMgr().GetFormatStr(nTypeId, GetCurField()->GetFormat()));
                            break;
                        }

                        default:
                            if (*rLst[i] == GetCurField()->GetPar1())
                                bInsert = TRUE;
                            break;
                    }
                    if (bInsert)
                    {
                        nPos = aSelectionLB.InsertEntry(*rLst[i]);
                        aSelectionLB.SetEntryData(nPos, (void*)i);
                        break;
                    }
                }
            }
            aSelectionLB.SetSelectHdl(Link());
        }
        else
        {
            AddSubType(TYP_PAGENUMBERFLD);
            AddSubType(TYP_PREVPAGEFLD);
            AddSubType(TYP_NEXTPAGEFLD);
            nTypeId = (USHORT)(ULONG)aSelectionLB.GetEntryData(0);
            nCount = 3;
            aSelectionLB.SetSelectHdl(LINK(this, SwFldDokPage, SubTypeHdl));
        }

        BOOL bEnable = nCount != 0;

        if (bEnable && !aSelectionLB.GetSelectEntryCount())
            aSelectionLB.SelectEntryPos(0);

        aSelectionLB.Enable( bEnable );
        aSelectionFT.Enable( bEnable );

        // Format-Listbox fuellen
        USHORT nSize = FillFormatLB(nTypeId);

        BOOL bValue = FALSE, bLevel = FALSE, bNumFmt = FALSE, bOffset = FALSE;
        BOOL bFormat = nSize != 0;
        BOOL bOneArea = FALSE;
        BOOL bFixed = FALSE;
        USHORT nFmtType = 0;

        switch (nTypeId)
        {
            case TYP_DATEFLD:
                bFormat = bNumFmt = bOneArea = bOffset = TRUE;

                nFmtType = NUMBERFORMAT_DATE;
                aValueFT.SetText(sDateOffset);
                aDateOffsetED.SetFirst(-31);    // Ein Monat
                aDateOffsetED.SetLast(31);

                if (IsFldEdit())
                    aDateOffsetED.SetValue( ((SwDateTimeField*)GetCurField())->GetOffset() / 24 / 60);
                break;

            case TYP_TIMEFLD:
                bFormat = bNumFmt = bOneArea = bOffset = TRUE;

                nFmtType = NUMBERFORMAT_TIME;
                aValueFT.SetText(sTimeOffset);
                aDateOffsetED.SetFirst(-1440);  // Ein Tag
                aDateOffsetED.SetLast(1440);

                if (IsFldEdit())
                    aDateOffsetED.SetValue( ((SwDateTimeField*)GetCurField())->GetOffset() );
                break;

            case TYP_PREVPAGEFLD:
            case TYP_NEXTPAGEFLD:
                if (IsFldEdit())
                {
                    USHORT nTmp = (USHORT)(ULONG)aFormatLB.GetEntryData(
                                            aFormatLB.GetSelectEntryPos() );
                    String sOldTxt(aValueFT.GetText());

                    if (FMT_NUM_PAGESPECIAL - FMT_NUM_BEGIN != nTmp)
                    {
                        short nOff = GetCurField()->GetPar2().ToInt32();
                        if( TYP_NEXTPAGEFLD == nTypeId && 1 != nOff )
                            aValueED.SetText( nOff - 1 );
                        else if( TYP_PREVPAGEFLD == nTypeId && -1 != nOff )
                            aValueED.SetText( nOff + 1 );
                        else
                            aValueED.SetText(aEmptyStr);
                    }
                    else
                        aValueED.SetText(((SwPageNumberField*)GetCurField())->GetUserString());
                }
                bValue = TRUE;
                break;

            case TYP_CHAPTERFLD:
                aValueFT.SetText(SW_RESSTR(STR_LEVEL));
                if (IsFldEdit())
                    aLevelED.SetText(String::CreateFromInt32(((SwChapterField*)GetCurField())->GetLevel() + 1));
                bLevel = TRUE;
                break;

            case TYP_PAGENUMBERFLD:
                aValueFT.SetText( SW_RESSTR( STR_OFFSET ));
                if (IsFldEdit())
                    aValueED.SetText(GetCurField()->GetPar2());
                bValue = TRUE;
                break;

            case TYP_EXTUSERFLD:
            case TYP_AUTHORFLD:
            case TYP_FILENAMEFLD:
                bFixed = TRUE;
                break;

            default:
                break;
        }

        if (bNumFmt)
        {
            if (IsFldEdit())
            {
                aNumFormatLB.SetDefFormat(GetCurField()->GetFormat());

                if (aNumFormatLB.GetFormatType() == (NUMBERFORMAT_DATE|NUMBERFORMAT_TIME))
                {
                    // Format-Typ immer einstellen, da sonst bei kombinierten Datum/Zeitformaten
                    // beide Formate gleichzeitig angezeigt werden wuerden
                    aNumFormatLB.SetFormatType(0);
                    aNumFormatLB.SetFormatType(nFmtType);
                    // Nochmal richtiges Format einstellen
                    aNumFormatLB.SetDefFormat(GetCurField()->GetFormat());
                }
            }
            else
                aNumFormatLB.SetFormatType(nFmtType);

            aNumFormatLB.SetOneArea(bOneArea);
        }

        aFormatLB.Show(!bNumFmt);
        aNumFormatLB.Show(bNumFmt);

        Size aSz(aFormatLB.GetSizePixel());

        if (bValue | bLevel | bOffset)
            aSz.Height() = aFormatLB.LogicToPixel(Size(1, 82), MAP_APPFONT).Height();
        else
            aSz.Height() = aFormatLB.LogicToPixel(Size(1, 98), MAP_APPFONT).Height();

        aValueFT.Show(bValue | bLevel | bOffset);
        aValueED.Show(bValue);
        aLevelED.Show(bLevel);
        aDateOffsetED.Show(bOffset);
        aFixedCB.Show(!bValue && !bLevel && !bOffset);

        aFormatLB.SetSizePixel(aSz);
        aNumFormatLB.SetSizePixel(aSz);

        aFormatLB.Enable(bFormat);
        aFormatFT.Enable(bFormat);
        aFixedCB.Enable(bFixed);

        if (IsFldEdit())
            aFixedCB.Check(((GetCurField()->GetFormat() & AF_FIXED) != 0) & bFixed);

        if (aNumFormatLB.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
            aNumFormatLB.SelectEntryPos(0);
        aValueFT.Enable(bValue | bLevel | bOffset);
        aValueED.Enable(bValue);
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDokPage::AddSubType(USHORT nTypeId)
{
    USHORT nPos = aSelectionLB.InsertEntry(*SwFieldType::GetFldNames()->GetObject(nTypeId));
    aSelectionLB.SetEntryData(nPos, (void*)nTypeId);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDokPage, SubTypeHdl, ListBox *, EMPTYARG )
{
    USHORT nPos = aSelectionLB.GetSelectEntryPos();
    if(nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    USHORT nTypeId = (USHORT)(ULONG)aSelectionLB.GetEntryData(nPos);
    FillFormatLB(nTypeId);

    USHORT nTextRes = 0;
    switch (nTypeId)
    {
    case TYP_CHAPTERFLD:
        nTextRes = STR_LEVEL;
        break;

    case TYP_PREVPAGEFLD:
    case TYP_NEXTPAGEFLD:
        nTextRes = SVX_NUM_CHAR_SPECIAL == (USHORT)(ULONG)aFormatLB.GetEntryData(
                                        aFormatLB.GetSelectEntryPos() )
                        ? STR_VALUE : STR_OFFSET;
        break;

    case TYP_PAGENUMBERFLD:
        nTextRes = STR_OFFSET;
        break;
    }

    if( nTextRes )
        aValueFT.SetText( SW_RESSTR( nTextRes ));

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldDokPage::FillFormatLB(USHORT nTypeId)
{
    // Format-Listbox fuellen
    aFormatLB.Clear();

    if (nTypeId == TYP_AUTHORFLD)
        return aFormatLB.GetEntryCount();

    USHORT nSize = GetFldMgr().GetFormatCount(nTypeId, FALSE, IsFldDlgHtmlMode());

    for( USHORT i = 0; i < nSize; ++i )
    {
        USHORT nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
        USHORT nFmtId = GetFldMgr().GetFormatId( nTypeId, i );
        aFormatLB.SetEntryData( nPos, (void*)nFmtId );
        if (IsFldEdit() && nFmtId == (GetCurField()->GetFormat() & ~AF_FIXED))
            aFormatLB.SelectEntryPos( nPos );
    }

    if( nSize && !aFormatLB.GetSelectEntryCount() )
    {
        aFormatLB.SelectEntry( SW_RESSTR(FMT_NUM_PAGEDESC) );
        if( !aFormatLB.GetSelectEntryCount() )
        {
            aFormatLB.SelectEntry( SW_RESSTR(FMT_NUM_ARABIC) );
            if( !aFormatLB.GetSelectEntryCount() )
                aFormatLB.SelectEntryPos( 0 );
        }
    }

    FormatHdl();

    return nSize;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDokPage, FormatHdl, ListBox *, EMPTYARG )
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    if (nTypeId == USHRT_MAX)
    {
        USHORT nPos = aSelectionLB.GetSelectEntryPos();
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
            nPos = 0;

        nTypeId = (USHORT)(ULONG)aSelectionLB.GetEntryData(nPos);
    }

    if (nTypeId == TYP_NEXTPAGEFLD || nTypeId == TYP_PREVPAGEFLD)
    {
        // Prev/Next - PageNumFelder Sonderbehandlung:
        USHORT nTmp = (USHORT)(ULONG)aFormatLB.GetEntryData(
                                        aFormatLB.GetSelectEntryPos() );
        String sOldTxt( aValueFT.GetText() );
        String sNewTxt( SW_RES( SVX_NUM_CHAR_SPECIAL == nTmp  ? STR_VALUE
                                                         : STR_OFFSET ));

        if( sOldTxt != sNewTxt )
            aValueFT.SetText( sNewTxt );

        if (sOldTxt != aValueFT.GetText())
            aValueED.SetText( aEmptyStr );
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL __EXPORT SwFldDokPage::FillItemSet(SfxItemSet& rSet)
{
    BOOL bPage = FALSE;
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    if (nTypeId == USHRT_MAX)
    {
        USHORT nPos = aSelectionLB.GetSelectEntryPos();
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
            nPos = 0;
        nTypeId = (USHORT)(ULONG)aSelectionLB.GetEntryData(nPos);
        bPage = TRUE;
    }

    String aVal(aValueED.GetText());
    ULONG nFormat = 0;
    USHORT nSubType = 0;

    if (aFormatLB.IsEnabled())
    {
        USHORT nPos = aFormatLB.GetSelectEntryPos();
        if(nPos != LISTBOX_ENTRY_NOTFOUND)
            nFormat = (USHORT)(ULONG)aFormatLB.GetEntryData(nPos);
    }

    if (aSelectionLB.IsEnabled())
    {
        USHORT nPos = aSelectionLB.GetSelectEntryPos();
        if(nPos != LISTBOX_ENTRY_NOTFOUND)
            nSubType = (USHORT)(ULONG)aSelectionLB.GetEntryData(nPos);
    }

    switch (nTypeId)
    {
        case TYP_AUTHORFLD:
            nFormat = nSubType;
            nSubType = 0;
            // kein break!
        case TYP_EXTUSERFLD:
            nFormat |= aFixedCB.IsChecked() ? AF_FIXED : 0;
            break;

        case TYP_FILENAMEFLD:
            nFormat |= aFixedCB.IsChecked() ? FF_FIXED : 0;
            break;

        case TYP_DATEFLD:
        case TYP_TIMEFLD:
        {
            nFormat = aNumFormatLB.GetFormat();
            long nVal = aDateOffsetED.GetValue();
            if (nTypeId == TYP_DATEFLD)
                aVal = String::CreateFromInt32(nVal * 60 * 24);
            else
                aVal = String::CreateFromInt32(nVal);
            break;
        }

        case TYP_NEXTPAGEFLD:
        case TYP_PREVPAGEFLD:
        case TYP_PAGENUMBERFLD:
        case TYP_GETREFPAGEFLD:
        {
            if( SVX_NUM_CHAR_SPECIAL != nFormat &&
                (TYP_PREVPAGEFLD == nTypeId || TYP_NEXTPAGEFLD == nTypeId))
            {
                short nVal = aValueED.GetText().ToInt32();
                aVal = String::CreateFromInt32(nVal);
            }
            break;
        }

        case TYP_CHAPTERFLD:
            aVal = aLevelED.GetText();
            break;

        default:
            break;
    }

    if (!IsFldEdit() ||
        nOldSel != aSelectionLB.GetSelectEntryPos() ||
        nOldFormat != nFormat ||
        aFixedCB.GetState() != aFixedCB.GetSavedValue() ||
        aValueED.GetText() != aValueED.GetSavedValue() ||
        aLevelED.GetText() != aLevelED.GetSavedValue() ||
        aDateOffsetED.GetText() != aDateOffsetED.GetSavedValue())
    {
        InsertFld( nTypeId, nSubType, aEmptyStr, aVal, nFormat );
    }

    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* __EXPORT SwFldDokPage::Create(  Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldDokPage( pParent, rAttrSet ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldDokPage::GetGroup()
{
    return GRP_DOC;
}

/* -----------------12.01.99 10:09-------------------
 *
 * --------------------------------------------------*/
void    SwFldDokPage::FillUserData()
{
    String sData( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( USER_DATA_VERSION )));
    sData += ';';
    USHORT nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (ULONG)aTypeLB.GetEntryData( nTypeSel );
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.36  2000/09/18 16:05:28  willem.vandorp
    OpenOffice header added.

    Revision 1.35  2000/08/23 11:33:25  os
    CreateFromInt32

    Revision 1.34  2000/06/30 08:52:52  os
    #76541# string assertions removed

    Revision 1.33  2000/05/23 18:36:41  jp
    Bugfixes for Unicode

    Revision 1.32  2000/04/18 15:17:31  os
    UNICODE

    Revision 1.31  2000/02/24 17:57:26  hr
    #73447#: removed temporary

    Revision 1.30  2000/02/11 14:46:18  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.29  1999/03/23 09:51:34  OS
    #61673# Reihenfolge der Seitenfelder besser


      Rev 1.28   23 Mar 1999 10:51:34   OS
   #61673# Reihenfolge der Seitenfelder besser

      Rev 1.27   25 Feb 1999 17:24:50   JP
   Bug #62438#: UserData nur auswerten, wenn kein Refresh ist

      Rev 1.26   21 Jan 1999 09:50:36   OS
   #59900# Fussnoten im Dialog korrekt sortieren; keine prot. Member

      Rev 1.25   12 Jan 1999 11:42:24   OS
   #60579# ausgewaehlten Typ in den UserData speichern

      Rev 1.24   17 Nov 1998 10:50:38   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.23   04 Nov 1998 12:11:06   OM
   #58851# Fixe Dateinamenfelder bearbeiten

      Rev 1.22   27 Oct 1998 15:53:24   OM
   #58427# Fixes FilenameField

      Rev 1.21   10 Aug 1998 16:39:58   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.20   20 May 1998 15:59:52   OM
   Kombinierte DateTime-Formate verarbeiten

      Rev 1.19   20 May 1998 13:24:22   OM
   Offset fuer DateTimeFields

      Rev 1.18   03 Apr 1998 13:16:48   OM
   #49062 Spinbutton fuer Kapitelebenen

      Rev 1.17   26 Mar 1998 17:00:28   OM
   Feldbefehl: Seitenanzahl im Html-Mode

      Rev 1.16   26 Mar 1998 16:43:10   OM
   Feldbefehl: Seitenanzahl im Html-Mode

      Rev 1.15   06 Mar 1998 15:38:30   OM
   Nur bei Aenderung Feld aktualisieren

      Rev 1.14   25 Feb 1998 16:47:58   OM
   Fixe Author- und ExtUser-Felder

      Rev 1.13   16 Feb 1998 10:10:22   OM
   Fuer Solaris nicht direkt von void* auf ushort casten

      Rev 1.12   12 Feb 1998 13:58:24   OM
   #47262# Offset loeschen

      Rev 1.11   14 Jan 1998 10:30:26   OM
   Kapitelfeldebene bearbeiten

      Rev 1.10   09 Jan 1998 16:56:50   OM
   Bei Dok-Wechsel updaten

      Rev 1.9   08 Jan 1998 14:58:24   OM
   Traveling

      Rev 1.8   07 Jan 1998 17:17:42   OM
   Referenzen editieren

      Rev 1.7   05 Jan 1998 17:44:34   OM
   DB-Feldbefehl bearbeiten

      Rev 1.6   19 Dec 1997 18:24:30   OM
   Feldbefehl-bearbeiten Dlg

      Rev 1.5   12 Dec 1997 16:10:06   OM
   AutoUpdate bei FocusWechsel u.a.

      Rev 1.4   11 Dec 1997 16:58:00   OM
   Feldumstellung

      Rev 1.3   20 Nov 1997 17:01:22   OM
   Neuer Felddialog

      Rev 1.2   07 Nov 1997 16:39:32   OM
   NumberFormat-Listbox

      Rev 1.1   05 Nov 1997 15:35:40   OM
   Neuer Feldbefehldialog

      Rev 1.0   04 Nov 1997 10:07:18   OM
   Initial revision.

------------------------------------------------------------------------*/

