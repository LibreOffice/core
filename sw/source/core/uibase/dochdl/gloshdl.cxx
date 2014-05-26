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

#include <hintids.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/adjustitem.hxx>
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#include <vcl/msgbox.hxx>
#include <svl/macitem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <poolfmt.hxx>
#include <fmtcol.hxx>
#include <docary.hxx>
#include <wrtsh.hxx>
#include <uitool.hxx>
#include <view.hxx>
#include <swevent.hxx>
#include <gloshdl.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <swundo.hxx>
#include <expfld.hxx>
#include <initui.hxx>
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

struct TextBlockInfo_Impl
{
    OUString sTitle;
    OUString sLongName;
    OUString sGroupName;
};
typedef boost::ptr_vector<TextBlockInfo_Impl> TextBlockInfoArr;

// Dialog for edit templates
void SwGlossaryHdl::GlossaryDlg()
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AbstractGlossaryDlg* pDlg = pFact->CreateGlossaryDlg(pViewFrame, this, pWrtShell);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    OUString sName;
    OUString sShortName;

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

    if( !sName.isEmpty() || !sShortName.isEmpty() )
        rStatGlossaries.EditGroupDoc( sName, sShortName );
}

// set the default group; if called from the dialog
// the group is created temporarily for faster access
void SwGlossaryHdl::SetCurGroup(const OUString &rGrp, bool bApi, bool bAlwaysCreateNew )
{
    OUString sGroup(rGrp);
    if (sGroup.indexOf(GLOS_DELIM)<0 && !FindGroupName(sGroup))
    {
        sGroup += OUString(GLOS_DELIM) + "0";
    }
    if(pCurGrp)
    {
        bool bPathEqual = false;
        if(!bAlwaysCreateNew)
        {
            INetURLObject aTemp( pCurGrp->GetFileName() );
            const OUString sCurBase = aTemp.getBase();
            aTemp.removeSegment();
            const OUString sCurEntryPath = aTemp.GetMainURL(INetURLObject::NO_DECODE);
            const std::vector<OUString> & rPathArr = rStatGlossaries.GetPathArray();
            sal_uInt16 nCurrentPath = USHRT_MAX;
            for (size_t nPath = 0; nPath < rPathArr.size(); ++nPath)
            {
                if (sCurEntryPath == rPathArr[nPath])
                {
                    nCurrentPath = static_cast<sal_uInt16>(nPath);
                    break;
                }
            }
            const OUString sPath = sGroup.getToken(1, GLOS_DELIM);
            sal_uInt16 nComparePath = (sal_uInt16)sPath.toInt32();
            if(nCurrentPath == nComparePath &&
                sGroup.getToken(0, GLOS_DELIM) == sCurBase)
                bPathEqual = true;
        }

        // When path changed, the name is not reliable
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
        pCurGrp = rStatGlossaries.GetGroupDoc(aCurGrp, true);
    }
}

sal_uInt16 SwGlossaryHdl::GetGroupCnt() const
{
    return rStatGlossaries.GetGroupCnt();
}

OUString SwGlossaryHdl::GetGroupName( sal_uInt16 nId, OUString* pTitle )
{
    OUString sRet = rStatGlossaries.GetGroupName(nId);
    if(pTitle)
    {
        SwTextBlocks* pGroup = rStatGlossaries.GetGroupDoc(sRet, false);
        if(pGroup && !pGroup->GetError())
        {
            *pTitle = pGroup->GetName();
            if (pTitle->isEmpty())
            {
                *pTitle = sRet.getToken(0, GLOS_DELIM);
                pGroup->SetName(*pTitle);
            }
            rStatGlossaries.PutGroupDoc( pGroup );
        }
        else
            sRet = OUString();
    }
    return sRet;
}

bool SwGlossaryHdl::NewGroup(OUString &rGrpName, const OUString& rTitle)
{
    if (rGrpName.indexOf(GLOS_DELIM)<0)
        FindGroupName(rGrpName);
    return rStatGlossaries.NewGroupDoc(rGrpName, rTitle);
}

bool SwGlossaryHdl::RenameGroup(const OUString& rOld, OUString& rNew, const OUString& rNewTitle)
{
    bool bRet = false;
    OUString sOldGroup(rOld);
    if (rOld.indexOf(GLOS_DELIM)<0)
        FindGroupName(sOldGroup);
    if(rOld == rNew)
    {
        SwTextBlocks* pGroup = rStatGlossaries.GetGroupDoc(sOldGroup, false);
        if(pGroup)
        {
            pGroup->SetName(rNewTitle);
            rStatGlossaries.PutGroupDoc( pGroup );
            bRet = true;
        }
    }
    else
    {
        OUString sNewGroup(rNew);
        if (sNewGroup.indexOf(GLOS_DELIM)<0)
        {
            sNewGroup += OUString(GLOS_DELIM) + "0";
        }
        bRet = rStatGlossaries.RenameGroupDoc(sOldGroup, sNewGroup, rNewTitle);
        rNew = sNewGroup;
    }
    return bRet;
}

bool SwGlossaryHdl::CopyOrMove( const OUString& rSourceGroupName, OUString& rSourceShortName,
                                const OUString& rDestGroupName, const OUString& rLongName, bool bMove )
{
    SwTextBlocks* pSourceGroup = rStatGlossaries.GetGroupDoc(rSourceGroupName, false);

    SwTextBlocks* pDestGroup = rStatGlossaries.GetGroupDoc(rDestGroupName, false);
    if(pDestGroup->IsReadOnly() || (bMove && pSourceGroup->IsReadOnly()) )
        return false;

    //The index must be determined here because rSourceShortName maybe changed in CopyBlock
    sal_uInt16 nDeleteIdx = pSourceGroup->GetIndex( rSourceShortName );
    OSL_ENSURE(USHRT_MAX != nDeleteIdx, "entry not found");
    sal_uLong nRet = pSourceGroup->CopyBlock( *pDestGroup, rSourceShortName, rLongName );
    if(!nRet && bMove)
    {
        // the index must be existing
        nRet = pSourceGroup->Delete( nDeleteIdx ) ? 0 : 1;
    }
    rStatGlossaries.PutGroupDoc( pSourceGroup );
    rStatGlossaries.PutGroupDoc( pDestGroup );
    return !nRet;
}

// delete a autotext-file-group
bool SwGlossaryHdl::DelGroup(const OUString &rGrpName)
{
    OUString sGroup(rGrpName);
    if (sGroup.indexOf(GLOS_DELIM)<0)
        FindGroupName(sGroup);
    if( rStatGlossaries.DelGroupDoc(sGroup) )
    {
        if(pCurGrp)
        {
            if (pCurGrp->GetName() == sGroup)
                DELETEZ(pCurGrp);
        }
        return true;
    }
    return false;
}

// ask for number of autotexts
sal_uInt16 SwGlossaryHdl::GetGlossaryCnt()
{
    return pCurGrp ? pCurGrp->GetCount() : 0;
}

OUString SwGlossaryHdl::GetGlossaryName( sal_uInt16 nId )
{
    OSL_ENSURE(nId < GetGlossaryCnt(), "Textbausteinarray ueberindiziert.");
    return pCurGrp->GetLongName( nId );
}

OUString SwGlossaryHdl::GetGlossaryShortName(sal_uInt16 nId)
{
    OSL_ENSURE(nId < GetGlossaryCnt(), "Textbausteinarray ueberindiziert.");
    return pCurGrp->GetShortName( nId );
}

// ask for short name
OUString SwGlossaryHdl::GetGlossaryShortName(const OUString &rName)
{
    OUString sReturn;
    SwTextBlocks *pTmp =
        pCurGrp ? pCurGrp: rStatGlossaries.GetGroupDoc( aCurGrp, false );
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

// short name for autotext already used?
bool SwGlossaryHdl::HasShortName(const OUString& rShortName) const
{
    SwTextBlocks *pBlock = pCurGrp ? pCurGrp
                                   : rStatGlossaries.GetGroupDoc( aCurGrp );
    bool bRet = pBlock->GetIndex( rShortName ) != (sal_uInt16) -1;
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pBlock );
    return bRet;
}

bool SwGlossaryHdl::ConvertToNew(SwTextBlocks& /*rOld*/)
{
    return true;
}

// Create autotext
bool SwGlossaryHdl::NewGlossary(const OUString& rName, const OUString& rShortName,
                                bool bCreateGroup, bool bNoAttr)
{
    SwTextBlocks *pTmp =
        pCurGrp ? pCurGrp: rStatGlossaries.GetGroupDoc( aCurGrp, bCreateGroup );
    //pTmp == 0 if the AutoText path setting is wrong
    if(!pTmp)
        return false;
    if(!ConvertToNew(*pTmp))
        return false;

    OUString sOnlyTxt;
    OUString* pOnlyTxt = 0;
    if( bNoAttr )
    {
        if( !pWrtShell->GetSelectedText( sOnlyTxt, GETSELTXT_PARABRK_TO_ONLYCR ))
            return false;
        pOnlyTxt = &sOnlyTxt;
    }

    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();

    const sal_uInt16 nSuccess = pWrtShell->MakeGlossary( *pTmp, rName, rShortName,
                            rCfg.IsSaveRelFile(), pOnlyTxt );
    if(nSuccess == (sal_uInt16) -1 )
    {
        InfoBox(pWrtShell->GetView().GetWindow(), SW_RES(MSG_ERR_INSERT_GLOS)).Execute();
    }
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pTmp );
    return nSuccess != (sal_uInt16) -1;
}

// Delete a autotext
bool SwGlossaryHdl::DelGlossary(const OUString &rShortName)
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp);
    //pTmp == 0 if the AutoText path setting is wrong
    if(!pGlossary || !ConvertToNew(*pGlossary))
        return false;

    sal_uInt16 nIdx = pGlossary->GetIndex( rShortName );
    if( nIdx != (sal_uInt16) -1 )
        pGlossary->Delete( nIdx );
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pGlossary );
    return true;
}

// expand short name
bool SwGlossaryHdl::ExpandGlossary()
{
    OSL_ENSURE(pWrtShell->CanInsert(), "illegal");
    SwTextBlocks *pGlossary;
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    ::GlossaryGetCurrGroup fnGetCurrGroup = pFact->GetGlossaryCurrGroupFunc();
    OSL_ENSURE(fnGetCurrGroup, "Dialogdiet fail!");
    OUString sGroupName( (*fnGetCurrGroup)() );
    if (sGroupName.indexOf(GLOS_DELIM)<0)
        FindGroupName(sGroupName);
    pGlossary = rStatGlossaries.GetGroupDoc(sGroupName);

    OUString aShortName;

        // use this at text selection
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
            // ask for word
        if(pWrtShell->IsSelection())
            aShortName = pWrtShell->GetSelTxt();
    }
    return pGlossary ? Expand( aShortName, &rStatGlossaries, pGlossary ) : sal_False;
}

bool SwGlossaryHdl::Expand( const OUString& rShortName,
                            SwGlossaries *pGlossaries,
                            SwTextBlocks *pGlossary  )
{
    TextBlockInfoArr aFoundArr;
    OUString aShortName( rShortName );
    bool bCancel = false;
    // search for text block
    // - don't prefer current group depending on configuration setting
    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    sal_uInt16 nFound = !rCfg.IsSearchInAllCategories() ? pGlossary->GetIndex( aShortName ) : -1;
    // if not found then search in all groups
    if( nFound == (sal_uInt16) -1 )
    {
        const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        sal_uInt16 nGroupCount = pGlossaryList->GetGroupCount();
        for(sal_uInt16 i = 1; i <= nGroupCount; i++)
        {
            OUString sTitle = pGlossaryList->GetGroupTitle(i - 1);
            // get group name with path-extension
            OUString sGroupName = pGlossaryList->GetGroupName(i - 1, false);
            if(sGroupName == pGlossary->GetName())
                continue;
            sal_uInt16 nBlockCount = pGlossaryList->GetBlockCount(i -1);
            if(nBlockCount)
            {
                for(sal_uInt16 j = 0; j < nBlockCount; j++)
                {
                    OUString sLongName(pGlossaryList->GetBlockLongName(i - 1, j));
                    OUString sShortName(pGlossaryList->GetBlockShortName(i - 1, j));
                    if( rSCmp.isEqual( rShortName, sShortName ))
                    {
                        TextBlockInfo_Impl* pData = new TextBlockInfo_Impl;
                        pData->sTitle = sTitle;
                        pData->sLongName = sLongName;
                        pData->sGroupName = sGroupName;
                        aFoundArr.push_back(pData);
                    }
                }
            }
        }
        if( !aFoundArr.empty() )  // one was found
        {
            pGlossaries->PutGroupDoc(pGlossary);
            if(1 == aFoundArr.size())
            {
                TextBlockInfo_Impl* pData = &aFoundArr.front();
                pGlossary = (SwTextBlocks *)pGlossaries->GetGroupDoc(pData->sGroupName);
                nFound = pGlossary->GetIndex( aShortName );
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                AbstractSwSelGlossaryDlg* pDlg = pFact->CreateSwSelGlossaryDlg(0, aShortName);
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                for(sal_uInt16 i = 0; i < aFoundArr.size(); ++i)
                {
                    TextBlockInfo_Impl* pData = &aFoundArr[i];
                    pDlg->InsertGlos(pData->sTitle, pData->sLongName);
                }
                pDlg->SelectEntryPos(0);
                const sal_Int32 nRet = RET_OK == pDlg->Execute()?
                                        pDlg->GetSelectedIdx():
                                        LISTBOX_ENTRY_NOTFOUND;
                delete pDlg;
                if(LISTBOX_ENTRY_NOTFOUND != nRet)
                {
                    TextBlockInfo_Impl* pData = &aFoundArr[nRet];
                    pGlossary = (SwTextBlocks *)pGlossaries->GetGroupDoc(pData->sGroupName);
                    nFound = pGlossary->GetIndex( aShortName );
                }
                else
                {
                    nFound = (sal_uInt16) -1;
                    bCancel = true;
                }
            }
        }
    }

    // not found
    if( nFound == (sal_uInt16) -1 )
    {
        if( !bCancel )
        {
            pGlossaries->PutGroupDoc(pGlossary);

            const sal_Int32 nMaxLen = 50;
            if(pWrtShell->IsSelection() && aShortName.getLength() > nMaxLen)
            {
                aShortName = aShortName.copy(0, nMaxLen) + " ...";
            }
            OUString aTmp( SW_RES(STR_NOGLOS));
            aTmp = aTmp.replaceFirst("%1", aShortName);
            InfoBox( pWrtShell->GetView().GetWindow(), aTmp ).Execute();
        }

        return false;
    }
    else
    {
        SvxMacro aStartMacro(aEmptyOUStr, aEmptyOUStr, STARBASIC);
        SvxMacro aEndMacro(aEmptyOUStr, aEmptyOUStr, STARBASIC);
        GetMacros( aShortName, aStartMacro, aEndMacro, pGlossary );

        // StartAction must not be before HasSelection and DelRight,
        // otherwise the possible Shell change gets delayed and
        // API-programs would hang.
        // Moreover the event macro must also not be called in an action
        pWrtShell->StartUndo(UNDO_INSGLOSSARY);
        if( aStartMacro.HasMacro() )
            pWrtShell->ExecMacro( aStartMacro );
        if(pWrtShell->HasSelection())
            pWrtShell->DelLeft();
        pWrtShell->StartAllAction();

        // cache all InputFields
        SwInputFieldList aFldLst( pWrtShell, true );

        pWrtShell->InsertGlossary(*pGlossary, aShortName);
        pWrtShell->EndAllAction();
        if( aEndMacro.HasMacro() )
        {
            pWrtShell->ExecMacro( aEndMacro );
        }
        pWrtShell->EndUndo(UNDO_INSGLOSSARY);

        // demand input for all new InputFields
        if( aFldLst.BuildSortLst() )
            pWrtShell->UpdateInputFlds( &aFldLst );
    }
    pGlossaries->PutGroupDoc(pGlossary);
    return true;
}

// add autotext
bool SwGlossaryHdl::InsertGlossary(const OUString &rName)
{
    OSL_ENSURE(pWrtShell->CanInsert(), "illegal");

    SwTextBlocks *pGlos =
        pCurGrp? pCurGrp: rStatGlossaries.GetGroupDoc(aCurGrp);

    if (!pGlos)
        return false;

    SvxMacro aStartMacro(aEmptyOUStr, aEmptyOUStr, STARBASIC);
    SvxMacro aEndMacro(aEmptyOUStr, aEmptyOUStr, STARBASIC);
    GetMacros( rName, aStartMacro, aEndMacro, pGlos );

    // StartAction must not be before HasSelection and DelRight,
    // otherwise the possible Shell change gets delayed and
    // API-programs would hang.
    // Moreover the event macro must also not be called in an action
    if( aStartMacro.HasMacro() )
        pWrtShell->ExecMacro( aStartMacro );
    if( pWrtShell->HasSelection() )
        pWrtShell->DelRight();
    pWrtShell->StartAllAction();

    // cache all InputFields
    SwInputFieldList aFldLst( pWrtShell, true );

    pWrtShell->InsertGlossary(*pGlos, rName);
    pWrtShell->EndAllAction();
    if( aEndMacro.HasMacro() )
    {
        pWrtShell->ExecMacro( aEndMacro );
    }

    // demand input for all new InputFields
    if( aFldLst.BuildSortLst() )
        pWrtShell->UpdateInputFlds( &aFldLst );

    if(!pCurGrp)
        rStatGlossaries.PutGroupDoc(pGlos);
    return true;
}

// set / ask for macro
void SwGlossaryHdl::SetMacros(const OUString& rShortName,
                              const SvxMacro* pStart,
                              const SvxMacro* pEnd,
                              SwTextBlocks *pGlossary )
{
    SwTextBlocks *pGlos = pGlossary ? pGlossary :
                                pCurGrp ? pCurGrp
                                  : rStatGlossaries.GetGroupDoc( aCurGrp );
    SvxMacroTableDtor aMacroTbl;
    if( pStart )
        aMacroTbl.Insert( SW_EVENT_START_INS_GLOSSARY, *pStart);
    if( pEnd )
        aMacroTbl.Insert( SW_EVENT_END_INS_GLOSSARY, *pEnd);
    sal_uInt16 nIdx = pGlos->GetIndex( rShortName );
    if( !pGlos->SetMacroTable( nIdx, aMacroTbl ) && pGlos->GetError() )
        ErrorHandler::HandleError( pGlos->GetError() );

    if(!pCurGrp && !pGlossary)
        rStatGlossaries.PutGroupDoc(pGlos);
}

void SwGlossaryHdl::GetMacros( const OUString &rShortName,
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

// ctor, dtor
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

// rename an autotext
bool SwGlossaryHdl::Rename(const OUString& rOldShort, const OUString& rNewShortName,
                           const OUString& rNewName )
{
    bool bRet = false;
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp);
    if(pGlossary)
    {
        if(!ConvertToNew(*pGlossary))
            return false;

        sal_uInt16 nIdx = pGlossary->GetIndex( rOldShort );
        sal_uInt16 nOldLongIdx = pGlossary->GetLongIndex( rNewName );
        sal_uInt16 nOldIdx = pGlossary->GetIndex( rNewShortName );

        if( nIdx != USHRT_MAX &&
                (nOldLongIdx == USHRT_MAX || nOldLongIdx == nIdx )&&
                    (nOldIdx == USHRT_MAX || nOldIdx == nIdx ))
        {
            pGlossary->Rename( nIdx, &rNewShortName, &rNewName );
            bRet = pGlossary->GetError() == 0;
        }
        if( !pCurGrp )
            rStatGlossaries.PutGroupDoc(pGlossary);
    }
    return bRet;
}

bool SwGlossaryHdl::IsReadOnly( const OUString* pGrpNm ) const
{
    SwTextBlocks *pGlossary = 0;

    if (pGrpNm)
        pGlossary = rStatGlossaries.GetGroupDoc( *pGrpNm );
    else if (pCurGrp)
        pGlossary = pCurGrp;
    else
        pGlossary = rStatGlossaries.GetGroupDoc(aCurGrp);

    bool bRet = pGlossary ? pGlossary->IsReadOnly() : sal_True;
    if( pGrpNm || !pCurGrp )
        delete pGlossary;
    return bRet;
}

bool SwGlossaryHdl::IsOld() const
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp
                                      : rStatGlossaries.GetGroupDoc(aCurGrp);
    bool bRet = pGlossary ? pGlossary->IsOld() : sal_False;
    if( !pCurGrp )
        delete pGlossary;
    return bRet;
}

// find group without path index
bool SwGlossaryHdl::FindGroupName(OUString& rGroup)
{
    return rStatGlossaries.FindGroupName(rGroup);
}

bool SwGlossaryHdl::CopyToClipboard(SwWrtShell& rSh, const OUString& rShortName)
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

bool SwGlossaryHdl::ImportGlossaries( const OUString& rName )
{
    bool bRet = false;
    if( !rName.isEmpty() )
    {
        const SfxFilter* pFilter = 0;
        SfxMedium* pMed = new SfxMedium( rName, STREAM_READ, 0, 0 );
        SfxFilterMatcher aMatcher( OUString("swriter") );
        pMed->UseInteractionHandler( true );
        if( !aMatcher.GuessFilter( *pMed, &pFilter, sal_False ) )
        {
            SwTextBlocks *pGlossary = NULL;
            pMed->SetFilter( pFilter );
            Reader* pR = SwReaderWriter::GetReader( pFilter->GetUserData() );
            if( pR && 0 != ( pGlossary = pCurGrp ? pCurGrp
                                    : rStatGlossaries.GetGroupDoc(aCurGrp)) )
            {
                SwReader aReader( *pMed, rName );
                if( aReader.HasGlossaries( *pR ) )
                {
                    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
                    bRet = aReader.ReadGlossaries( *pR, *pGlossary,
                                rCfg.IsSaveRelFile() );
                }
            }
        }
        DELETEZ(pMed);
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
