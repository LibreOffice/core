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


#include <hintids.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/adjitem.hxx>
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#include <vcl/msgbox.hxx>
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
#include <docsh.hxx>
#include <crsskip.hxx>

#include <dochdl.hrc>
#include <swerror.h>
#include <frmmgr.hxx>
#include <vcl/lstbox.hxx>

#include <editeng/acorrcfg.hxx>
#include "swabstdlg.hxx"
#include <misc.hrc>

#include <IDocumentFieldsAccess.hxx>

using namespace ::com::sun::star;


const short RET_EDIT = 100;

// PUBLIC METHODS -------------------------------------------------------
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
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AbstractGlossaryDlg* pDlg = pFact->CreateGlossaryDlg( DLG_RENAME_GLOS,
                                                        pViewFrame, this, pWrtShell);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
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
            INetURLObject aTemp( pCurGrp->GetFileName() );
            String sCurBase = aTemp.getBase();
            aTemp.removeSegment();
            const String sCurEntryPath = aTemp.GetMainURL(INetURLObject::NO_DECODE);
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
            if(nCurrentPath == nComparePath &&
                sGroup.GetToken(0, GLOS_DELIM) == sCurBase)
                bPathEqual = TRUE;
        }

        // Beim Pfadwechsel kann man sich auf den Namen nicht verlassen
        if(!bAlwaysCreateNew && bPathEqual)
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

USHORT SwGlossaryHdl::GetGroupCnt() const
{
    return rStatGlossaries.GetGroupCnt();
}

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

BOOL SwGlossaryHdl::NewGroup(String &rGrpName, const String& rTitle)
{
    if(STRING_NOTFOUND == rGrpName.Search(GLOS_DELIM))
        FindGroupName(rGrpName);
    return rStatGlossaries.NewGroupDoc(rGrpName, rTitle);
}

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

BOOL SwGlossaryHdl::CopyOrMove( const String& rSourceGroupName,  String& rSourceShortName,
                        const String& rDestGroupName, const String& rLongName, BOOL bMove )
{
    SwTextBlocks* pSourceGroup = rStatGlossaries.GetGroupDoc(rSourceGroupName, FALSE);

    SwTextBlocks* pDestGroup = rStatGlossaries.GetGroupDoc(rDestGroupName, FALSE);
    if(pDestGroup->IsReadOnly() || (bMove && pSourceGroup->IsReadOnly()) )
        return FALSE;

    //Der Index muss hier ermittelt werden, weil rSourceShortName in CopyBlock evtl veraendert wird
    USHORT nDeleteIdx = pSourceGroup->GetIndex( rSourceShortName );
    OSL_ENSURE(USHRT_MAX != nDeleteIdx, "entry not found");
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

String SwGlossaryHdl::GetGlossaryName( USHORT nId )
{
    OSL_ENSURE(nId < GetGlossaryCnt(), "Textbausteinarray ueberindiziert.");
    return pCurGrp->GetLongName( nId );
}

String  SwGlossaryHdl::GetGlossaryShortName(USHORT nId)
{
    OSL_ENSURE(nId < GetGlossaryCnt(), "Textbausteinarray ueberindiziert.");
    return pCurGrp->GetShortName( nId );
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

BOOL    SwGlossaryHdl::ConvertToNew(SwTextBlocks& /*rOld*/)
{
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

    const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();

    const USHORT nSuccess = pWrtShell->MakeGlossary( *pTmp, rName, rShortName,
                            pCfg->IsSaveRelFile(), pOnlyTxt );
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
BOOL SwGlossaryHdl::ExpandGlossary()
{
    OSL_ENSURE(pWrtShell->CanInsert(), "illegal");
    SwTextBlocks *pGlossary;
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    ::GlossaryGetCurrGroup fnGetCurrGroup = pFact->GetGlossaryCurrGroupFunc( DLG_RENAME_GLOS );
    OSL_ENSURE(fnGetCurrGroup, "Dialogdiet fail!");
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
        // select word
        pWrtShell->SelNearestWrd();
            // Wort erfragen
        if(pWrtShell->IsSelection())
            aShortName = pWrtShell->GetSelTxt();
    }
    return pGlossary ? Expand( aShortName, &rStatGlossaries, pGlossary ) : FALSE;
}

BOOL SwGlossaryHdl::Expand( const String& rShortName,
                            SwGlossaries *pGlossaries,
                            SwTextBlocks *pGlossary  )
{
    TextBlockInfoArr aFoundArr;
    String aShortName( rShortName );
    BOOL bCancel = FALSE;
    // search for text block
    //#b6633427# - don't prefer current group depending on configuration setting
    const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();
    USHORT nFound = !pCfg->IsSearchInAllCategories() ? pGlossary->GetIndex( aShortName ) : -1;
    // if not found then search in all groups
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
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                AbstractSwSelGlossaryDlg* pDlg = pFact->CreateSwSelGlossaryDlg( 0, aShortName, DLG_SEL_GLOS );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
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

    // not found
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
            String aTmp( SW_RES(STR_NOGLOS));
            aTmp.SearchAndReplaceAscii("%1", aShortName);
            InfoBox( pWrtShell->GetView().GetWindow(), aTmp ).Execute();
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
    OSL_ENSURE(pWrtShell->CanInsert(), "illegal");

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
    USHORT nIdx = pGlos->GetIndex( rShortName );
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

/*--------------------------------------------------
    Gruppe ohne Pfadindex finden
--------------------------------------------------*/
BOOL SwGlossaryHdl::FindGroupName(String & rGroup)
{
    return rStatGlossaries.FindGroupName(rGroup);
}

BOOL SwGlossaryHdl::CopyToClipboard(SwWrtShell& rSh, const String& rShortName)
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

BOOL SwGlossaryHdl::ImportGlossaries( const String& rName )
{
    BOOL bRet = FALSE;
    if( rName.Len() )
    {
        const SfxFilter* pFilter = 0;
        SfxMedium* pMed = new SfxMedium( rName, STREAM_READ, TRUE, 0, 0 );
        SfxFilterMatcher aMatcher( String::CreateFromAscii("swriter") );
        pMed->UseInteractionHandler( TRUE );
        if( !aMatcher.GuessFilter( *pMed, &pFilter, FALSE ) )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
