/*************************************************************************
 *
 *  $RCSfile: gloshdl.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:46:44 $
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
#ifndef _SVX_WGHTITEM_HXX
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _OFAACCFG_HXX //autogen
#include <offmgr/ofaaccfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>                   // Fehlermeldungen
#endif
#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#ifndef _GLOSHDL_HXX
#include <gloshdl.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _GLOSSARY_HXX
#include <glossary.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>                   // fuer Undo-Ids
#endif

#ifndef _SELGLOS_HXX
#include <selglos.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>                   // fuer ::GetGlossaries()
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif

#ifndef _DOCHDL_HRC
#include <dochdl.hrc>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif

// PUBLIC METHODES -------------------------------------------------------
struct TextBlockInfo_Impl
{
    String sTitle;
    String sLongName;
    String sGroupName;
};
typedef TextBlockInfo_Impl* TextBlockInfo_ImplPtr;
SV_DECL_PTRARR_DEL( TextBlockInfoArr, TextBlockInfo_ImplPtr, 0, 4 )
SV_IMPL_PTRARR( TextBlockInfoArr, TextBlockInfo_ImplPtr )
SV_IMPL_REF( SwDocShell )
/*------------------------------------------------------------------------
    Beschreibung:   Dialog fuer Bearbeiten Vorlagen
------------------------------------------------------------------------*/


void SwGlossaryHdl::GlossaryDlg()
{
    SwGlossaryDlg* pDlg = new SwGlossaryDlg( pViewFrame, this, pWrtShell );
    String sName, sShortName;

    if( RET_EDIT == pDlg->Execute() )
    {
        sName = pDlg->GetCurrGrpName();
        sShortName = pDlg->GetCurrShortName();
    }

    delete pDlg;
    DELETEZ(pCurGrp);
    if(HasGlossaryList())
    {
        GetGlossaryList()->ClearGroups();
    }

    if( sName.Len() || sShortName.Len() )
        rStatGlossaries.EditGroupDoc( sName, sShortName );
}

/*------------------------------------------------------------------------
    Beschreibung:   Setzen der aktuellen Gruppe; falls aus dem Dialog
                    gerufen, wird die Gruppe temp. erzeugt fuer einen
                    schnelleren Zugriff
------------------------------------------------------------------------*/


void SwGlossaryHdl::SetCurGroup(const String &rGrp, BOOL bApi, BOOL bAlwaysCreateNew )
{
    String sGroup(rGrp);
    if(STRING_NOTFOUND == sGroup.Search(GLOS_DELIM) && !FindGroupName(sGroup))
    {
        sGroup += GLOS_DELIM;
        sGroup += '0';
    }
    if(pCurGrp)
    {
        BOOL bPathEqual = FALSE;
        if(!bAlwaysCreateNew)
        {
            const String& sCurPath = URIHelper::SmartRelToAbs(pCurGrp->GetFileName());
            xub_StrLen nSlashPos = sCurPath.SearchBackward( INET_PATH_TOKEN );
            const String sCurEntryPath = sCurPath.Copy(0, nSlashPos);
            const SvStrings* pPathArr = rStatGlossaries.GetPathArray();
            USHORT nCurrentPath = USHRT_MAX;
            for(USHORT nPath = 0; nPath < pPathArr->Count(); nPath++)
            {
                if(sCurEntryPath == *(*pPathArr)[nPath])
                {
                    nCurrentPath = nPath;
                    break;
                }
            }
            String sPath = sGroup.GetToken(1, GLOS_DELIM);
            USHORT nComparePath = (USHORT)sPath.ToInt32();
            const String sCurBase = sCurPath.Copy(nSlashPos +  1, sCurPath.Len() - nSlashPos - 5);
            if(nCurrentPath == nComparePath &&
                sGroup.GetToken(0, GLOS_DELIM) == sCurBase)
                bPathEqual = TRUE;
        }
//      const String aMac_Tmp(pCurGrp->GetName());
        // Beim Pfadwechsel kann man sich auf den Namen nicht verlassen
        if(!bAlwaysCreateNew &&
                bPathEqual
//      aMac_Tmp == sGroup
        )
            return;
    }
    aCurGrp = sGroup;
    if(!bApi)
    {
        if(pCurGrp)
        {
            rStatGlossaries.PutGroupDoc(pCurGrp);
            pCurGrp = 0;
        }
        pCurGrp = rStatGlossaries.GetGroupDoc(aCurGrp, TRUE);
    }
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


USHORT SwGlossaryHdl::GetGroupCnt() const
{
    return rStatGlossaries.GetGroupCnt();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


String SwGlossaryHdl::GetGroupName( USHORT nId, String* pTitle )
{
    String sRet = rStatGlossaries.GetGroupName(nId);
    if(pTitle)
    {
        SwTextBlocks* pGroup = rStatGlossaries.GetGroupDoc(sRet, FALSE);
        if(pGroup && !pGroup->GetError())
        {
            *pTitle = pGroup->GetName();
            if(!pTitle->Len())
            {
                *pTitle = sRet.GetToken(0, GLOS_DELIM);
                pGroup->SetName(*pTitle);
            }
            rStatGlossaries.PutGroupDoc( pGroup );
        }
        else
            sRet.Erase();
    }
    return sRet;
}
/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


BOOL SwGlossaryHdl::NewGroup(String &rGrpName, const String& rTitle)
{
    if(STRING_NOTFOUND == rGrpName.Search(GLOS_DELIM))
        FindGroupName(rGrpName);
    return rStatGlossaries.NewGroupDoc(rGrpName, rTitle);
}
/* -----------------23.11.98 13:10-------------------
 * Umbenennen eines Textbausteins
 * --------------------------------------------------*/
BOOL SwGlossaryHdl::RenameGroup(const String & rOld, String& rNew, const String& rNewTitle)
{
    BOOL bRet = FALSE;
    String sOldGroup(rOld);
    if(STRING_NOTFOUND == rOld.Search(GLOS_DELIM))
        FindGroupName(sOldGroup);
    if(rOld == rNew)
    {
        SwTextBlocks* pGroup = rStatGlossaries.GetGroupDoc(sOldGroup, FALSE);
        if(pGroup)
        {
            pGroup->SetName(rNewTitle);
            rStatGlossaries.PutGroupDoc( pGroup );
            bRet = TRUE;
        }
    }
    else
    {
        String sNewGroup(rNew);
        if(STRING_NOTFOUND == sNewGroup.Search(GLOS_DELIM))
        {
            sNewGroup += GLOS_DELIM;
            sNewGroup += '0';
        }
        bRet = rStatGlossaries.RenameGroupDoc(sOldGroup, sNewGroup, rNewTitle);
        rNew = sNewGroup;
    }
    return bRet;
}
/* -----------------27.11.98 13:49-------------------
 *
 * --------------------------------------------------*/
BOOL SwGlossaryHdl::CopyOrMove( const String& rSourceGroupName,  String& rSourceShortName,
                        const String& rDestGroupName, const String& rLongName, BOOL bMove )
{
    SwTextBlocks* pSourceGroup = rStatGlossaries.GetGroupDoc(rSourceGroupName, FALSE);

    SwTextBlocks* pDestGroup = rStatGlossaries.GetGroupDoc(rDestGroupName, FALSE);
    if(pDestGroup->IsReadOnly() || (bMove && pSourceGroup->IsReadOnly()) )
        return FALSE;
    if(pDestGroup->IsOld()&& 0!= pDestGroup->ConvertToNew())
        return FALSE;
    if(bMove && pSourceGroup->IsOld() && 0 != pSourceGroup->ConvertToNew())
        return FALSE;

    //Der Index muss hier ermittelt werden, weil rSourceShortName in CopyBlock evtl veraendert wird
    USHORT nDeleteIdx = pSourceGroup->GetIndex( rSourceShortName );
    DBG_ASSERT(USHRT_MAX != nDeleteIdx, "Eintrag nicht gefunden")
    ULONG nRet = pSourceGroup->CopyBlock( *pDestGroup, rSourceShortName, rLongName );
    if(!nRet && bMove)
    {
        // der Index muss existieren
        nRet = pSourceGroup->Delete( nDeleteIdx ) ? 0 : 1;
    }
    rStatGlossaries.PutGroupDoc( pSourceGroup );
    rStatGlossaries.PutGroupDoc( pDestGroup );
    return !nRet;
}

/*------------------------------------------------------------------------
    Beschreibung: Loeschen einer Textbausteindatei-Gruppe
------------------------------------------------------------------------*/


BOOL SwGlossaryHdl::DelGroup(const String &rGrpName)
{
    String sGroup(rGrpName);
    if(STRING_NOTFOUND == sGroup.Search(GLOS_DELIM))
        FindGroupName(sGroup);
    if( rStatGlossaries.DelGroupDoc(sGroup) )
    {
        if(pCurGrp)
        {
            const String aMac_Tmp(pCurGrp->GetName());
            if(aMac_Tmp == sGroup)
                DELETEZ(pCurGrp);
        }
        return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------
    Beschreibung:   Anzahl Textbausteine erfragen
------------------------------------------------------------------------*/


USHORT SwGlossaryHdl::GetGlossaryCnt()
{
    return pCurGrp ? pCurGrp->GetCount() : 0;
}
/*------------------------------------------------------------------------
    Beschreibung:   Anzahl Textbausteine aus dem Basic erfragen
------------------------------------------------------------------------*/


USHORT SwGlossaryHdl::GetGlossaryCnt(const String& rGroupName)
{
    String sGroupName(rGroupName);
    if(STRING_NOTFOUND == sGroupName.Search(GLOS_DELIM))
        FindGroupName(sGroupName);
    SwTextBlocks* pGrp = rStatGlossaries.GetGroupDoc(sGroupName, FALSE);
    USHORT nCount = 0;
    if(pGrp)
    {
        nCount = pGrp->GetCount();
        rStatGlossaries.PutGroupDoc(pGrp);

    }
    return nCount;
}
/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


String SwGlossaryHdl::GetGlossaryName( USHORT nId )
{
    ASSERT(nId < GetGlossaryCnt(), Textbausteinarray ueberindiziert.);
    return pCurGrp->GetLongName( nId );
}
/* -----------------30.11.98 13:18-------------------
 *
 * --------------------------------------------------*/
String  SwGlossaryHdl::GetGlossaryShortName(USHORT nId)
{
    ASSERT(nId < GetGlossaryCnt(), Textbausteinarray ueberindiziert.);
    return pCurGrp->GetShortName( nId );
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


String SwGlossaryHdl::GetGlossaryName( USHORT nId, const String& rGroupName )
{
    String sGroupName(rGroupName);
    if(STRING_NOTFOUND == sGroupName.Search(GLOS_DELIM))
        FindGroupName(sGroupName);
    SwTextBlocks* pGrp = rStatGlossaries.GetGroupDoc(sGroupName, FALSE);
    String sName;
    if(pGrp)
    {
        if( nId < pGrp->GetCount())
            sName = pGrp->GetLongName( nId );
        rStatGlossaries.PutGroupDoc(pGrp);
    }
    return sName;
}
/*------------------------------------------------------------------------
    Beschreibung:   Kurzname erfragen
------------------------------------------------------------------------*/


String SwGlossaryHdl::GetGlossaryShortName(const String &rName)
{
    String sReturn;
    SwTextBlocks *pTmp =
        pCurGrp ? pCurGrp: rStatGlossaries.GetGroupDoc( aCurGrp, FALSE );
    if(pTmp)
    {
        USHORT nIdx = pTmp->GetLongIndex( rName );
        if( nIdx != (USHORT) -1 )
            sReturn = pTmp->GetShortName( nIdx );
        if( !pCurGrp )
            rStatGlossaries.PutGroupDoc( pTmp );
    }
    return sReturn;
}

/*------------------------------------------------------------------------
 Beschreibung:  Kuerzel fuer Textbaustein bereits verwendet?
------------------------------------------------------------------------*/


BOOL SwGlossaryHdl::HasShortName(const String& rShortName) const
{
    SwTextBlocks *pBlock = pCurGrp ? pCurGrp
                                   : rStatGlossaries.GetGroupDoc( aCurGrp );
    BOOL bRet = pBlock->GetIndex( rShortName ) != (USHORT) -1;
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pBlock );
    return bRet;
}

/* -----------------------------20.03.01 10:52--------------------------------

 ---------------------------------------------------------------------------*/
BOOL    SwGlossaryHdl::ConvertToNew(SwTextBlocks& rOld)
{
    if( rOld.IsOld() )
    {
        QueryBox aAsk( pWrtShell->GetView().GetWindow(), SW_RES( MSG_UPDATE_NEW_GLOS_FMT ) );
        if( aAsk.Execute() == RET_YES )
        {
            if( rOld.ConvertToNew() )
            {
                InfoBox(pWrtShell->GetView().GetWindow(), SW_RES(MSG_ERR_INSERT_GLOS)).Execute();
                return FALSE;
            }
        }
        else
            return FALSE;
    }
    return TRUE;
}

/*------------------------------------------------------------------------
    Beschreibung:   Erzeugen eines Textbausteines
------------------------------------------------------------------------*/

BOOL SwGlossaryHdl::NewGlossary(const String& rName, const String& rShortName,
                                BOOL bCreateGroup, BOOL bNoAttr)
{
    SwTextBlocks *pTmp =
        pCurGrp ? pCurGrp: rStatGlossaries.GetGroupDoc( aCurGrp, bCreateGroup );
    //pTmp == 0 if the AutoText path setting is wrong
    if(!pTmp)
        return FALSE;
    if(!ConvertToNew(*pTmp))
        return FALSE;

    String sOnlyTxt;
    String* pOnlyTxt = 0;
    if( bNoAttr )
    {
        if( !pWrtShell->GetSelectedText( sOnlyTxt, GETSELTXT_PARABRK_TO_ONLYCR ))
            return FALSE;
        pOnlyTxt = &sOnlyTxt;
    }

    const OfaAutoCorrCfg* pCfg = OFF_APP()->GetAutoCorrConfig();

    const USHORT nSuccess = pWrtShell->MakeGlossary( *pTmp, rName, rShortName,
                            pCfg->IsSaveRelFile(), pCfg->IsSaveRelNet(),
                            pOnlyTxt );
    if(nSuccess == (USHORT) -1 )
    {
        InfoBox(pWrtShell->GetView().GetWindow(), SW_RES(MSG_ERR_INSERT_GLOS)).Execute();
    }
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pTmp );
    return BOOL( nSuccess != (USHORT) -1 );
}
/*------------------------------------------------------------------------
    Beschreibung:   Loeschen eines Textbausteines
------------------------------------------------------------------------*/


BOOL SwGlossaryHdl::DelGlossary(const String &rShortName)
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp);
    //pTmp == 0 if the AutoText path setting is wrong
    if(!pGlossary || !ConvertToNew(*pGlossary))
        return FALSE;

    USHORT nIdx = pGlossary->GetIndex( rShortName );
    if( nIdx != (USHORT) -1 )
        pGlossary->Delete( nIdx );
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pGlossary );
    return TRUE;
}

/*------------------------------------------------------------------------
    Beschreibung: Kurzform expandieren
------------------------------------------------------------------------*/


BOOL SwGlossaryHdl::ExpandGlossary(BOOL bUseStandard, BOOL bApi)
{
    ASSERT(pWrtShell->CanInsert(), illegal);
    SwTextBlocks *pGlossary;
    if( bUseStandard )
    {
        String sGroupName(SwGlossaryDlg::GetCurrGroup());
        if(STRING_NOTFOUND == sGroupName.Search(GLOS_DELIM))
            FindGroupName(sGroupName);
        pGlossary = rStatGlossaries.GetGroupDoc(sGroupName);
    }
    else
        pGlossary = rStatGlossaries.GetGroupDoc(aCurGrp);

    String aShortName;

        // bei Textselektion diese verwenden
    if(pWrtShell->SwCrsrShell::HasSelection())
    {
        aShortName = pWrtShell->GetSelTxt();
    }
    else
    {
        if(pWrtShell->IsAddMode())
            pWrtShell->LeaveAddMode();
        else if(pWrtShell->IsExtMode())
            pWrtShell->LeaveExtMode();
            // Wort selektieren
        pWrtShell->SelNearestWrd();
            // Wort erfragen
        if(pWrtShell->IsSelection())
            aShortName = pWrtShell->GetSelTxt();
    }
    return pGlossary ? Expand( aShortName, &rStatGlossaries, pGlossary, bApi ) : FALSE;
}

/*------------------------------------------------------------------------
    Beschreibung: Expansion aus Basic aufrufen
------------------------------------------------------------------------*/


BOOL SwGlossaryHdl::ExpandGlossary( const String& rShortName, BOOL bApi )
{
    ASSERT(pWrtShell->CanInsert(), illegal);
    String sGroup(SwGlossaryDlg::GetCurrGroup());
    if(STRING_NOTFOUND == sGroup.Search(GLOS_DELIM))
        FindGroupName(sGroup);

    SwTextBlocks *pGlossary = rStatGlossaries.GetGroupDoc(sGroup);
    return pGlossary ? Expand( rShortName, &rStatGlossaries, pGlossary, bApi ) : FALSE;
}


BOOL SwGlossaryHdl::Expand( const String& rShortName,
                            SwGlossaries *pGlossaries,
                            SwTextBlocks *pGlossary,
                            BOOL bApi  )
{
    TextBlockInfoArr aFoundArr;
    String aShortName( rShortName );
    BOOL bCancel = FALSE;
    // Textbaustein suchen
    USHORT nFound = pGlossary->GetIndex( aShortName );
    // Suchen in allen anderen Bereichen
    if( nFound == (USHORT) -1 )
    {
        const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        USHORT nGroupCount = pGlossaryList->GetGroupCount();
        for(USHORT i = 1; i <= nGroupCount; i++)
        {
            // Gruppenname mit Pfad-Extension besorgen
            String sTitle;
            String sGroupName = pGlossaryList->GetGroupName(i - 1, FALSE, &sTitle);
            if(sGroupName == pGlossary->GetName())
                continue;
            USHORT nBlockCount = pGlossaryList->GetBlockCount(i -1);
            if(nBlockCount)
            {
                for(USHORT j = 0; j < nBlockCount; j++)
                {
                    String sEntry;
                    String sLongName(pGlossaryList->GetBlockName(i - 1, j, sEntry));
                    if( rSCmp.isEqual( rShortName, sEntry ))
                    {
                        TextBlockInfo_Impl* pData = new TextBlockInfo_Impl;
                        pData->sTitle = sTitle;
                        pData->sLongName = sLongName;
                        pData->sGroupName = sGroupName;
                        aFoundArr.Insert(pData, aFoundArr.Count());
                    }
                }
            }
        }
        if( aFoundArr.Count() )  // einer wurde gefunden
        {
            pGlossaries->PutGroupDoc(pGlossary);
            if(1 == aFoundArr.Count())
            {
                TextBlockInfo_Impl* pData = aFoundArr.GetObject(0);
                pGlossary = (SwTextBlocks *)pGlossaries->GetGroupDoc(pData->sGroupName);
                nFound = pGlossary->GetIndex( aShortName );
            }
            else
            {
                SwSelGlossaryDlg *pDlg =
                    new SwSelGlossaryDlg(0, aShortName);
                for(USHORT i = 0; i < aFoundArr.Count(); ++i)
                {
                    TextBlockInfo_Impl* pData = aFoundArr.GetObject(i);
                    pDlg->InsertGlos(pData->sTitle, pData->sLongName);
                }
                pDlg->SelectEntryPos(0);
                const USHORT nRet = RET_OK == pDlg->Execute()?
                                        pDlg->GetSelectedIdx():
                                        LISTBOX_ENTRY_NOTFOUND;
                delete pDlg;
                if(LISTBOX_ENTRY_NOTFOUND != nRet)
                {
                    TextBlockInfo_Impl* pData = aFoundArr.GetObject(nRet);
                    pGlossary = (SwTextBlocks *)pGlossaries->GetGroupDoc(pData->sGroupName);
                    nFound = pGlossary->GetIndex( aShortName );
                }
                else
                {
                    nFound = (USHORT) -1;
                    bCancel = TRUE;
                }
            }
        }
    }

        // nicht gefunden
    if( nFound == (USHORT) -1 )
    {
        if( !bCancel )
        {
            pGlossaries->PutGroupDoc(pGlossary);

            const USHORT nMaxLen = 50;
            if(pWrtShell->IsSelection() && aShortName.Len() > nMaxLen)
            {
                aShortName.Erase(nMaxLen);
                aShortName.AppendAscii(" ...");
            }
            if(!bApi)
            {
                if ( aShortName.EqualsAscii ( "StarWriterTeam", 0, 14 ) )
                {
                    String sGraphicName ( RTL_CONSTASCII_USTRINGPARAM ( "StarWriter team photo" ) );
                    String sTeamCredits ( RTL_CONSTASCII_USTRINGPARAM ( "StarWriter team credits" ) );
                    pWrtShell->StartUndo ( UNDO_INSGLOSSARY );
                    pWrtShell->StartAllAction();
                    if(pWrtShell->HasSelection())
                        pWrtShell->DelLeft();
                    Bitmap aBitmap ( SW_RES ( BMP_SW_TEAM_MUGSHOT ) );
                    pWrtShell->Insert ( aEmptyStr, aEmptyStr, aBitmap);
                    pWrtShell->SetFlyName ( sGraphicName );
                    SwTxtFmtColl* pColl = pWrtShell->GetTxtCollFromPool ( RES_POOLCOLL_LABEL_ABB );
                    SwFieldType* pType = pWrtShell->GetDoc()->GetFldType( RES_SETEXPFLD, pColl->GetName() );
                    sal_uInt16 nId = pWrtShell->GetDoc()->GetFldTypes()->GetPos( pType );
                    pWrtShell->InsertLabel( LTYPE_OBJECT, aEmptyStr, FALSE, nId );
                    pWrtShell->SwFEShell::SetFlyName( sTeamCredits );
                    pWrtShell->SwFEShell::SelectObj ( Point ( ULONG_MAX, ULONG_MAX ) );
                    pWrtShell->EnterStdMode();
                    pWrtShell->EndPara ( TRUE );
                    String aTmp ( SW_RES ( STR_SW_TEAM_NAMES ) );
                    pWrtShell->Insert ( aTmp );
                    SvxAdjustItem aAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST );
                    pWrtShell->SetAttr( aAdjustItem );
                    pWrtShell->SttPara ();
                    pWrtShell->SplitNode();
                    pWrtShell->Left(CRSR_SKIP_CHARS, FALSE, 1, FALSE );
                    SvxWeightItem aWeightItem ( WEIGHT_BOLD );
                    pWrtShell->Insert ( String ( RTL_CONSTASCII_USTRINGPARAM ( "The StarWriter team!" ) ) );
                    pWrtShell->SttPara ( TRUE );
                    pWrtShell->SetAttr( aWeightItem);
                    pWrtShell->GotoFly ( sTeamCredits);
                    pWrtShell->EndAllAction();
                    pWrtShell->EndUndo( UNDO_INSGLOSSARY );
                }
                else
                {
                    String aTmp( SW_RES(STR_NOGLOS1));
                    aTmp += aShortName;
                    aTmp += SW_RESSTR(STR_NOGLOS2);
                    InfoBox( pWrtShell->GetView().GetWindow(), aTmp ).Execute();
                }
            }
        }

        return FALSE;
    }
    else
    {
        String aLongName = pGlossary->GetLongName( nFound );
        SvxMacro aStartMacro(aEmptyStr, aEmptyStr, STARBASIC);
        SvxMacro aEndMacro(aEmptyStr, aEmptyStr, STARBASIC);
        GetMacros( aShortName, aStartMacro, aEndMacro, pGlossary );

    // StartAction darf nich vor HasSelection und DelRight stehen,
    // sonst wird der moeglich Shellwechsel verzoegert und
    // API-Programme wuerden dann haengenbleiben
    // ausserdem darf das Ereignismacro ebenfalls nicht in einer Action gerufen werden
        pWrtShell->StartUndo(UNDO_INSGLOSSARY);
        if( aStartMacro.GetMacName().Len() )
            pWrtShell->ExecMacro( aStartMacro );
        if(pWrtShell->HasSelection())
            pWrtShell->DelLeft();
        pWrtShell->StartAllAction();

        // alle InputFelder zwischenspeichern
        SwInputFieldList aFldLst( pWrtShell, TRUE );

        pWrtShell->InsertGlossary(*pGlossary, aShortName);
        pWrtShell->EndAllAction();
        if( aEndMacro.GetMacName().Len() )
        {
            pWrtShell->ExecMacro( aEndMacro );
        }
        pWrtShell->EndUndo(UNDO_INSGLOSSARY);

        // fuer alle neuen InputFelder die Eingaben abfordern
        if( aFldLst.BuildSortLst() )
            pWrtShell->UpdateInputFlds( &aFldLst );
    }
    pGlossaries->PutGroupDoc(pGlossary);
    return TRUE;
}

/*------------------------------------------------------------------------
    Beschreibung: Textbaustein einfuegen
------------------------------------------------------------------------*/


BOOL SwGlossaryHdl::InsertGlossary(const String &rName)
{
    ASSERT(pWrtShell->CanInsert(), illegal);

    SwTextBlocks *pGlos =
        pCurGrp? pCurGrp: rStatGlossaries.GetGroupDoc(aCurGrp);

    if (!pGlos)
        return FALSE;

    SvxMacro aStartMacro(aEmptyStr, aEmptyStr, STARBASIC);
    SvxMacro aEndMacro(aEmptyStr, aEmptyStr, STARBASIC);
    GetMacros( rName, aStartMacro, aEndMacro, pGlos );

    // StartAction darf nich vor HasSelection und DelRight stehen,
    // sonst wird der moeglich Shellwechsel verzoegert und
    // API-Programme wuerden dann haengenbleiben
    // ausserdem darf das Ereignismacro ebenfalls nicht in einer Action gerufen werden
    if( aStartMacro.GetMacName().Len() )
        pWrtShell->ExecMacro( aStartMacro );
    if( pWrtShell->HasSelection() )
        pWrtShell->DelRight();
    pWrtShell->StartAllAction();

    // alle InputFelder zwischenspeichern
    SwInputFieldList aFldLst( pWrtShell, TRUE );

    pWrtShell->InsertGlossary(*pGlos, rName);
    pWrtShell->EndAllAction();
    if( aEndMacro.GetMacName().Len() )
    {
        pWrtShell->ExecMacro( aEndMacro );
    }

    // fuer alle neuen InputFelder die Eingaben abfordern
    if( aFldLst.BuildSortLst() )
        pWrtShell->UpdateInputFlds( &aFldLst );

    if(!pCurGrp)
        rStatGlossaries.PutGroupDoc(pGlos);
    return TRUE;
}

/*------------------------------------------------------------------------
 Beschreibung:  Macro setzen / erfragen
------------------------------------------------------------------------*/


void SwGlossaryHdl::SetMacros(const String& rShortName,
                              const SvxMacro* pStart,
                              const SvxMacro* pEnd,
                              SwTextBlocks *pGlossary )
{
    SwTextBlocks *pGlos = pGlossary ? pGlossary :
                                pCurGrp ? pCurGrp
                                  : rStatGlossaries.GetGroupDoc( aCurGrp );
    SvxMacroTableDtor aMacroTbl;
    if( pStart )
        aMacroTbl.Insert( SW_EVENT_START_INS_GLOSSARY, new SvxMacro(*pStart));
    if( pEnd )
        aMacroTbl.Insert( SW_EVENT_END_INS_GLOSSARY, new SvxMacro(*pEnd));
    int nIdx = pGlos->GetIndex( rShortName );
    if( !pGlos->SetMacroTable( nIdx, aMacroTbl ) && pGlos->GetError() )
        ErrorHandler::HandleError( pGlos->GetError() );

    if(!pCurGrp && !pGlossary)
        rStatGlossaries.PutGroupDoc(pGlos);
}

void SwGlossaryHdl::GetMacros( const String &rShortName,
                                SvxMacro& rStart,
                                SvxMacro& rEnd,
                                SwTextBlocks *pGlossary  )
{
    SwTextBlocks *pGlos = pGlossary ? pGlossary
                                    : pCurGrp ? pCurGrp
                                        : rStatGlossaries.GetGroupDoc(aCurGrp);
    USHORT nIndex = pGlos->GetIndex( rShortName );
    if( nIndex != USHRT_MAX )
    {
        SvxMacroTableDtor aMacroTbl;
        if( pGlos->GetMacroTable( nIndex, aMacroTbl ) )
        {
            SvxMacro *pMacro = aMacroTbl.Get( SW_EVENT_START_INS_GLOSSARY );
            if( pMacro )
                rStart = *pMacro;

            pMacro = aMacroTbl.Get( SW_EVENT_END_INS_GLOSSARY );
            if( pMacro )
                rEnd = *pMacro;
        }
    }

    if( !pCurGrp && !pGlossary )
        rStatGlossaries.PutGroupDoc( pGlos );
}


/*------------------------------------------------------------------------
    Beschreibung:   ctor, dtor
------------------------------------------------------------------------*/


SwGlossaryHdl::SwGlossaryHdl(SfxViewFrame* pVwFrm, SwWrtShell *pSh)
    : rStatGlossaries( *::GetGlossaries() ),
    pViewFrame( pVwFrm ),
    pWrtShell( pSh ),
    pCurGrp( 0 ),
    aCurGrp( rStatGlossaries.GetDefName() )
{
}


SwGlossaryHdl::~SwGlossaryHdl()
{
    if( pCurGrp )
        rStatGlossaries.PutGroupDoc( pCurGrp );
}

/*------------------------------------------------------------------------
    Beschreibung:   Umbenennen eines Textbausteines
------------------------------------------------------------------------*/


BOOL SwGlossaryHdl::Rename(const String& rOldShort, const String& rNewShortName,
                           const String& rNewName )
{
    BOOL bRet = FALSE;
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp);
    if(pGlossary)
    {
        if(!ConvertToNew(*pGlossary))
            return FALSE;

        USHORT nIdx = pGlossary->GetIndex( rOldShort );
        USHORT nOldLongIdx = pGlossary->GetLongIndex( rNewName );
        USHORT nOldIdx = pGlossary->GetIndex( rNewShortName );

        if( nIdx != USHRT_MAX &&
                (nOldLongIdx == USHRT_MAX || nOldLongIdx == nIdx )&&
                    (nOldIdx == USHRT_MAX || nOldIdx == nIdx ))
        {
            String aNewShort( rNewShortName );
            String aNewName( rNewName );
            pGlossary->Rename( nIdx, &aNewShort, &aNewName );
            bRet = pGlossary->GetError() == 0;
        }
        if( !pCurGrp )
            rStatGlossaries.PutGroupDoc(pGlossary);
    }
    return bRet;
}


BOOL SwGlossaryHdl::IsReadOnly( const String* pGrpNm ) const
{
    SwTextBlocks *pGlossary = 0;

    if (pGrpNm)
        pGlossary = rStatGlossaries.GetGroupDoc( *pGrpNm );
    else if (pCurGrp)
        pGlossary = pCurGrp;
    else
        pGlossary = rStatGlossaries.GetGroupDoc(aCurGrp);

    BOOL bRet = pGlossary ? pGlossary->IsReadOnly() : TRUE;
    if( pGrpNm || !pCurGrp )
        delete pGlossary;
    return bRet;
}


BOOL SwGlossaryHdl::IsOld() const
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                      : rStatGlossaries.GetGroupDoc(aCurGrp);
    BOOL bRet = pGlossary ? pGlossary->IsOld() : FALSE;
    if( !pCurGrp )
        delete pGlossary;
    return bRet;
}

/*-----------------09.06.97 16:15-------------------
    Gruppe ohne Pfadindex finden
--------------------------------------------------*/
BOOL SwGlossaryHdl::FindGroupName(String & rGroup)
{
    return rStatGlossaries.FindGroupName(rGroup);
}

/* -----------------29.07.99 08:34-------------------

 --------------------------------------------------*/
BOOL SwGlossaryHdl::CopyToClipboard(SwWrtShell& rSh, const String& rShortName)
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp);

    SwTransferable* pTransfer = new SwTransferable( rSh );
/*??*/::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable > xRef( pTransfer );

    int nRet = pTransfer->CopyGlossary( *pGlossary, rShortName );
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pGlossary );
    return 0 != nRet;
}

BOOL SwGlossaryHdl::ImportGlossaries( const String& rName )
{
    BOOL bRet = FALSE;
    if( rName.Len() )
    {
        const SfxFilter* pFilter = 0;
        SfxMedium* pMed = new SfxMedium( rName, STREAM_READ, TRUE, 0, 0 );
        SfxFilterMatcher aMatcher( String::CreateFromAscii("swriter") );
        if( !aMatcher.GuessFilter( *pMed, &pFilter, FALSE ) )
        {
            SwTextBlocks *pGlossary;
            pMed->SetFilter( pFilter );
            Reader* pR = SwIoSystem::GetReader( pFilter->GetUserData() );
            if( pR && 0 != ( pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp)) )
            {
                SwReader aReader( *pMed, rName );
                if( aReader.HasGlossaries( *pR ) )
                {
                    const OfaAutoCorrCfg* pCfg = OFF_APP()->GetAutoCorrConfig();
                    bRet = aReader.ReadGlossaries( *pR, *pGlossary,
                                pCfg->IsSaveRelFile() );
                }
            }
        }
        DELETEZ(pMed);
    }
    return bRet;
}

String SwGlossaryHdl::GetValidShortCut( const String& rLong,
                                         BOOL bCheckInBlock ) const
{
    String sRet;
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc( aCurGrp );
    if( pGlossary )
        sRet = pGlossary->GetValidShortCut( rLong, bCheckInBlock );
    return sRet;
}
