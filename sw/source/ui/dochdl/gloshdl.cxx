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


#include <hintids.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/adjitem.hxx>
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <svl/macitem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>
#include <svl/urihelper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <poolfmt.hxx>
#include <fmtcol.hxx>
#include <docary.hxx>
#include <wrtsh.hxx>
#include <uitool.hxx>                   // Fehlermeldungen
#include <view.hxx>
#include <swevent.hxx>
#include <gloshdl.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <swundo.hxx>                   // fuer Undo-Ids
#include <expfld.hxx>
#include <initui.hxx>                   // fuer ::GetGlossaries()
#include <gloslst.hxx>
#include <swdtflvr.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <crsskip.hxx>

#ifndef _DOCHDL_HRC
#include <dochdl.hrc>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#include <frmmgr.hxx>
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#include <editeng/acorrcfg.hxx>
#include "swabstdlg.hxx"
#include <misc.hrc>

#include <IDocumentFieldsAccess.hxx>

using namespace ::com::sun::star;


const short RET_EDIT = 100;

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
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AbstractGlossaryDlg* pDlg = pFact->CreateGlossaryDlg( DLG_RENAME_GLOS,
                                                        pViewFrame, this, pWrtShell);
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
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


void SwGlossaryHdl::SetCurGroup(const String &rGrp, sal_Bool bApi, sal_Bool bAlwaysCreateNew )
{
    String sGroup(rGrp);
    if(STRING_NOTFOUND == sGroup.Search(GLOS_DELIM) && !FindGroupName(sGroup))
    {
        sGroup += GLOS_DELIM;
        sGroup += '0';
    }
    if(pCurGrp)
    {
        sal_Bool bPathEqual = sal_False;
        if(!bAlwaysCreateNew)
        {
            INetURLObject aTemp( pCurGrp->GetFileName() );
            String sCurBase = aTemp.getBase();
            aTemp.removeSegment();
            const String sCurEntryPath = aTemp.GetMainURL(INetURLObject::NO_DECODE);
            const SvStrings* pPathArr = rStatGlossaries.GetPathArray();
            sal_uInt16 nCurrentPath = USHRT_MAX;
            for(sal_uInt16 nPath = 0; nPath < pPathArr->Count(); nPath++)
            {
                if(sCurEntryPath == *(*pPathArr)[nPath])
                {
                    nCurrentPath = nPath;
                    break;
                }
            }
            String sPath = sGroup.GetToken(1, GLOS_DELIM);
            sal_uInt16 nComparePath = (sal_uInt16)sPath.ToInt32();
            if(nCurrentPath == nComparePath &&
                sGroup.GetToken(0, GLOS_DELIM) == sCurBase)
                bPathEqual = sal_True;
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
        pCurGrp = rStatGlossaries.GetGroupDoc(aCurGrp, sal_True);
    }
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


sal_uInt16 SwGlossaryHdl::GetGroupCnt() const
{
    return rStatGlossaries.GetGroupCnt();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


String SwGlossaryHdl::GetGroupName( sal_uInt16 nId, String* pTitle )
{
    String sRet = rStatGlossaries.GetGroupName(nId);
    if(pTitle)
    {
        SwTextBlocks* pGroup = rStatGlossaries.GetGroupDoc(sRet, sal_False);
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


sal_Bool SwGlossaryHdl::NewGroup(String &rGrpName, const String& rTitle)
{
    if(STRING_NOTFOUND == rGrpName.Search(GLOS_DELIM))
        FindGroupName(rGrpName);
    return rStatGlossaries.NewGroupDoc(rGrpName, rTitle);
}
/* -----------------23.11.98 13:10-------------------
 * Umbenennen eines Textbausteins
 * --------------------------------------------------*/
sal_Bool SwGlossaryHdl::RenameGroup(const String & rOld, String& rNew, const String& rNewTitle)
{
    sal_Bool bRet = sal_False;
    String sOldGroup(rOld);
    if(STRING_NOTFOUND == rOld.Search(GLOS_DELIM))
        FindGroupName(sOldGroup);
    if(rOld == rNew)
    {
        SwTextBlocks* pGroup = rStatGlossaries.GetGroupDoc(sOldGroup, sal_False);
        if(pGroup)
        {
            pGroup->SetName(rNewTitle);
            rStatGlossaries.PutGroupDoc( pGroup );
            bRet = sal_True;
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
sal_Bool SwGlossaryHdl::CopyOrMove( const String& rSourceGroupName,  String& rSourceShortName,
                        const String& rDestGroupName, const String& rLongName, sal_Bool bMove )
{
    SwTextBlocks* pSourceGroup = rStatGlossaries.GetGroupDoc(rSourceGroupName, sal_False);

    SwTextBlocks* pDestGroup = rStatGlossaries.GetGroupDoc(rDestGroupName, sal_False);
    if(pDestGroup->IsReadOnly() || (bMove && pSourceGroup->IsReadOnly()) )
        return sal_False;
    /*if(pDestGroup->IsOld()&& 0!= pDestGroup->ConvertToNew())
        return sal_False;
    if(bMove && pSourceGroup->IsOld() && 0 != pSourceGroup->ConvertToNew())
        return sal_False;*/

    //Der Index muss hier ermittelt werden, weil rSourceShortName in CopyBlock evtl veraendert wird
    sal_uInt16 nDeleteIdx = pSourceGroup->GetIndex( rSourceShortName );
    DBG_ASSERT(USHRT_MAX != nDeleteIdx, "Eintrag nicht gefunden");
    sal_uLong nRet = pSourceGroup->CopyBlock( *pDestGroup, rSourceShortName, rLongName );
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


sal_Bool SwGlossaryHdl::DelGroup(const String &rGrpName)
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
        return sal_True;
    }
    return sal_False;
}

/*------------------------------------------------------------------------
    Beschreibung:   Anzahl Textbausteine erfragen
------------------------------------------------------------------------*/


sal_uInt16 SwGlossaryHdl::GetGlossaryCnt()
{
    return pCurGrp ? pCurGrp->GetCount() : 0;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


String SwGlossaryHdl::GetGlossaryName( sal_uInt16 nId )
{
    ASSERT(nId < GetGlossaryCnt(), Textbausteinarray ueberindiziert.);
    return pCurGrp->GetLongName( nId );
}
/* -----------------30.11.98 13:18-------------------
 *
 * --------------------------------------------------*/
String  SwGlossaryHdl::GetGlossaryShortName(sal_uInt16 nId)
{
    ASSERT(nId < GetGlossaryCnt(), Textbausteinarray ueberindiziert.);
    return pCurGrp->GetShortName( nId );
}


/*------------------------------------------------------------------------
    Beschreibung:   Kurzname erfragen
------------------------------------------------------------------------*/


String SwGlossaryHdl::GetGlossaryShortName(const String &rName)
{
    String sReturn;
    SwTextBlocks *pTmp =
        pCurGrp ? pCurGrp: rStatGlossaries.GetGroupDoc( aCurGrp, sal_False );
    if(pTmp)
    {
        sal_uInt16 nIdx = pTmp->GetLongIndex( rName );
        if( nIdx != (sal_uInt16) -1 )
            sReturn = pTmp->GetShortName( nIdx );
        if( !pCurGrp )
            rStatGlossaries.PutGroupDoc( pTmp );
    }
    return sReturn;
}

/*------------------------------------------------------------------------
 Beschreibung:  Kuerzel fuer Textbaustein bereits verwendet?
------------------------------------------------------------------------*/


sal_Bool SwGlossaryHdl::HasShortName(const String& rShortName) const
{
    SwTextBlocks *pBlock = pCurGrp ? pCurGrp
                                   : rStatGlossaries.GetGroupDoc( aCurGrp );
    sal_Bool bRet = pBlock->GetIndex( rShortName ) != (sal_uInt16) -1;
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pBlock );
    return bRet;
}

/* -----------------------------20.03.01 10:52--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    SwGlossaryHdl::ConvertToNew(SwTextBlocks& /*rOld*/)
{
    /*if( rOld.IsOld() )
    {
        QueryBox aAsk( pWrtShell->GetView().GetWindow(), SW_RES( MSG_UPDATE_NEW_GLOS_FMT ) );
        if( aAsk.Execute() == RET_YES )
        {
            if( rOld.ConvertToNew() )
            {
                InfoBox(pWrtShell->GetView().GetWindow(), SW_RES(MSG_ERR_INSERT_GLOS)).Execute();
                return sal_False;
            }
        }
        else
            return sal_False;
    }*/
    return sal_True;
}

/*------------------------------------------------------------------------
    Beschreibung:   Erzeugen eines Textbausteines
------------------------------------------------------------------------*/

sal_Bool SwGlossaryHdl::NewGlossary(const String& rName, const String& rShortName,
                                sal_Bool bCreateGroup, sal_Bool bNoAttr)
{
    SwTextBlocks *pTmp =
        pCurGrp ? pCurGrp: rStatGlossaries.GetGroupDoc( aCurGrp, bCreateGroup );
    //pTmp == 0 if the AutoText path setting is wrong
    if(!pTmp)
        return sal_False;
    if(!ConvertToNew(*pTmp))
        return sal_False;

    String sOnlyTxt;
    String* pOnlyTxt = 0;
    if( bNoAttr )
    {
        if( !pWrtShell->GetSelectedText( sOnlyTxt, GETSELTXT_PARABRK_TO_ONLYCR ))
            return sal_False;
        pOnlyTxt = &sOnlyTxt;
    }

    const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();

    const sal_uInt16 nSuccess = pWrtShell->MakeGlossary( *pTmp, rName, rShortName,
                            pCfg->IsSaveRelFile(), pOnlyTxt );
    if(nSuccess == (sal_uInt16) -1 )
    {
        InfoBox(pWrtShell->GetView().GetWindow(), SW_RES(MSG_ERR_INSERT_GLOS)).Execute();
    }
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pTmp );
    return sal_Bool( nSuccess != (sal_uInt16) -1 );
}
/*------------------------------------------------------------------------
    Beschreibung:   Loeschen eines Textbausteines
------------------------------------------------------------------------*/


sal_Bool SwGlossaryHdl::DelGlossary(const String &rShortName)
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp);
    //pTmp == 0 if the AutoText path setting is wrong
    if(!pGlossary || !ConvertToNew(*pGlossary))
        return sal_False;

    sal_uInt16 nIdx = pGlossary->GetIndex( rShortName );
    if( nIdx != (sal_uInt16) -1 )
        pGlossary->Delete( nIdx );
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pGlossary );
    return sal_True;
}

/*------------------------------------------------------------------------
    Beschreibung: Kurzform expandieren
------------------------------------------------------------------------*/


sal_Bool SwGlossaryHdl::ExpandGlossary()
{
    ASSERT(pWrtShell->CanInsert(), illegal);
    SwTextBlocks *pGlossary;
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    ::GlossaryGetCurrGroup fnGetCurrGroup = pFact->GetGlossaryCurrGroupFunc( DLG_RENAME_GLOS );
    DBG_ASSERT(fnGetCurrGroup, "Dialogdiet fail!");
    String sGroupName( (*fnGetCurrGroup)() );
    if(STRING_NOTFOUND == sGroupName.Search(GLOS_DELIM))
        FindGroupName(sGroupName);
    pGlossary = rStatGlossaries.GetGroupDoc(sGroupName);

    String aShortName;

        // bei Textselektion diese verwenden
    if(pWrtShell->SwCrsrShell::HasSelection() && !pWrtShell->IsBlockMode())
    {
        aShortName = pWrtShell->GetSelTxt();
    }
    else
    {
        if(pWrtShell->IsAddMode())
            pWrtShell->LeaveAddMode();
        else if(pWrtShell->IsBlockMode())
            pWrtShell->LeaveBlockMode();
        else if(pWrtShell->IsExtMode())
            pWrtShell->LeaveExtMode();
            // Wort selektieren
        pWrtShell->SelNearestWrd();
            // Wort erfragen
        if(pWrtShell->IsSelection())
            aShortName = pWrtShell->GetSelTxt();
    }
    return pGlossary ? Expand( aShortName, &rStatGlossaries, pGlossary ) : sal_False;
}

sal_Bool SwGlossaryHdl::Expand( const String& rShortName,
                            SwGlossaries *pGlossaries,
                            SwTextBlocks *pGlossary  )
{
    TextBlockInfoArr aFoundArr;
    String aShortName( rShortName );
    sal_Bool bCancel = sal_False;
    // search for text block
    //#b6633427# - don't prefer current group depending on configuration setting
    const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
    sal_uInt16 nFound = !pCfg->IsSearchInAllCategories() ? pGlossary->GetIndex( aShortName ) : -1;
    // if not found then search in all groups
    if( nFound == (sal_uInt16) -1 )
    {
        const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        sal_uInt16 nGroupCount = pGlossaryList->GetGroupCount();
        for(sal_uInt16 i = 1; i <= nGroupCount; i++)
        {
            // Gruppenname mit Pfad-Extension besorgen
            String sTitle;
            String sGroupName = pGlossaryList->GetGroupName(i - 1, sal_False, &sTitle);
            if(sGroupName == pGlossary->GetName())
                continue;
            sal_uInt16 nBlockCount = pGlossaryList->GetBlockCount(i -1);
            if(nBlockCount)
            {
                for(sal_uInt16 j = 0; j < nBlockCount; j++)
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
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

                AbstarctSwSelGlossaryDlg* pDlg = pFact->CreateSwSelGlossaryDlg( 0, aShortName, DLG_SEL_GLOS );
                DBG_ASSERT(pDlg, "Dialogdiet fail!");
                for(sal_uInt16 i = 0; i < aFoundArr.Count(); ++i)
                {
                    TextBlockInfo_Impl* pData = aFoundArr.GetObject(i);
                    pDlg->InsertGlos(pData->sTitle, pData->sLongName);
                }
                pDlg->SelectEntryPos(0);
                const sal_uInt16 nRet = RET_OK == pDlg->Execute()?
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
                    nFound = (sal_uInt16) -1;
                    bCancel = sal_True;
                }
            }
        }
    }

        // nicht gefunden
    if( nFound == (sal_uInt16) -1 )
    {
        if( !bCancel )
        {
            pGlossaries->PutGroupDoc(pGlossary);

            const sal_uInt16 nMaxLen = 50;
            if(pWrtShell->IsSelection() && aShortName.Len() > nMaxLen)
            {
                aShortName.Erase(nMaxLen);
                aShortName.AppendAscii(" ...");
            }
            String aTmp( SW_RES(STR_NOGLOS));
            aTmp.SearchAndReplaceAscii("%1", aShortName);
            InfoBox( pWrtShell->GetView().GetWindow(), aTmp ).Execute();
        }

        return sal_False;
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
        SwInputFieldList aFldLst( pWrtShell, sal_True );

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
    return sal_True;
}

/*------------------------------------------------------------------------
    Beschreibung: Textbaustein einfuegen
------------------------------------------------------------------------*/


sal_Bool SwGlossaryHdl::InsertGlossary(const String &rName)
{
    ASSERT(pWrtShell->CanInsert(), illegal);

    SwTextBlocks *pGlos =
        pCurGrp? pCurGrp: rStatGlossaries.GetGroupDoc(aCurGrp);

    if (!pGlos)
        return sal_False;

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
    SwInputFieldList aFldLst( pWrtShell, sal_True );

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
    return sal_True;
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
    sal_uInt16 nIdx = pGlos->GetIndex( rShortName );
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
    sal_uInt16 nIndex = pGlos->GetIndex( rShortName );
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
    aCurGrp( rStatGlossaries.GetDefName() ),
    pViewFrame( pVwFrm ),
    pWrtShell( pSh ),
    pCurGrp( 0 )
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


sal_Bool SwGlossaryHdl::Rename(const String& rOldShort, const String& rNewShortName,
                           const String& rNewName )
{
    sal_Bool bRet = sal_False;
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp);
    if(pGlossary)
    {
        if(!ConvertToNew(*pGlossary))
            return sal_False;

        sal_uInt16 nIdx = pGlossary->GetIndex( rOldShort );
        sal_uInt16 nOldLongIdx = pGlossary->GetLongIndex( rNewName );
        sal_uInt16 nOldIdx = pGlossary->GetIndex( rNewShortName );

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


sal_Bool SwGlossaryHdl::IsReadOnly( const String* pGrpNm ) const
{
    SwTextBlocks *pGlossary = 0;

    if (pGrpNm)
        pGlossary = rStatGlossaries.GetGroupDoc( *pGrpNm );
    else if (pCurGrp)
        pGlossary = pCurGrp;
    else
        pGlossary = rStatGlossaries.GetGroupDoc(aCurGrp);

    sal_Bool bRet = pGlossary ? pGlossary->IsReadOnly() : sal_True;
    if( pGrpNm || !pCurGrp )
        delete pGlossary;
    return bRet;
}


sal_Bool SwGlossaryHdl::IsOld() const
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                      : rStatGlossaries.GetGroupDoc(aCurGrp);
    sal_Bool bRet = pGlossary ? pGlossary->IsOld() : sal_False;
    if( !pCurGrp )
        delete pGlossary;
    return bRet;
}

/*-----------------09.06.97 16:15-------------------
    Gruppe ohne Pfadindex finden
--------------------------------------------------*/
sal_Bool SwGlossaryHdl::FindGroupName(String & rGroup)
{
    return rStatGlossaries.FindGroupName(rGroup);
}

/* -----------------29.07.99 08:34-------------------

 --------------------------------------------------*/
sal_Bool SwGlossaryHdl::CopyToClipboard(SwWrtShell& rSh, const String& rShortName)
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp);

    SwTransferable* pTransfer = new SwTransferable( rSh );
/*??*/uno::Reference<
        datatransfer::XTransferable > xRef( pTransfer );

    int nRet = pTransfer->CopyGlossary( *pGlossary, rShortName );
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pGlossary );
    return 0 != nRet;
}

sal_Bool SwGlossaryHdl::ImportGlossaries( const String& rName )
{
    sal_Bool bRet = sal_False;
    if( rName.Len() )
    {
        const SfxFilter* pFilter = 0;
        SfxMedium* pMed = new SfxMedium( rName, STREAM_READ, sal_True, 0, 0 );
        SfxFilterMatcher aMatcher( String::CreateFromAscii("swriter") );
        pMed->UseInteractionHandler( sal_True );
        if( !aMatcher.GuessFilter( *pMed, &pFilter, sal_False ) )
        {
            SwTextBlocks *pGlossary;
            pMed->SetFilter( pFilter );
            Reader* pR = SwReaderWriter::GetReader( pFilter->GetUserData() );
            if( pR && 0 != ( pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp)) )
            {
                SwReader aReader( *pMed, rName );
                if( aReader.HasGlossaries( *pR ) )
                {
                    const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
                    bRet = aReader.ReadGlossaries( *pR, *pGlossary,
                                pCfg->IsSaveRelFile() );
                }
            }
        }
        DELETEZ(pMed);
    }
    return bRet;
}

