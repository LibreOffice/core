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
#include <uitool.hxx>                   // error messages
#include <view.hxx>
#include <swevent.hxx>
#include <gloshdl.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <swundo.hxx>                   // for Undo-Ids
#include <expfld.hxx>
#include <initui.hxx>                   // for ::GetGlossaries()
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
    OUString sTitle;
    OUString sLongName;
    OUString sGroupName;
};
typedef boost::ptr_vector<TextBlockInfo_Impl> TextBlockInfoArr;
SV_IMPL_REF( SwDocShell )


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
void SwGlossaryHdl::SetCurGroup(const OUString &rGrp, sal_Bool bApi, sal_Bool bAlwaysCreateNew )
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
        pCurGrp = rStatGlossaries.GetGroupDoc(aCurGrp, sal_True);
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
        SwTextBlocks* pGroup = rStatGlossaries.GetGroupDoc(sRet, sal_False);
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

sal_Bool SwGlossaryHdl::NewGroup(OUString &rGrpName, const OUString& rTitle)
{
    if (rGrpName.indexOf(GLOS_DELIM)<0)
        FindGroupName(rGrpName);
    return rStatGlossaries.NewGroupDoc(rGrpName, rTitle);
}

sal_Bool SwGlossaryHdl::RenameGroup(const OUString& rOld, OUString& rNew, const OUString& rNewTitle)
{
    sal_Bool bRet = sal_False;
    OUString sOldGroup(rOld);
    if (rOld.indexOf(GLOS_DELIM)<0)
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

sal_Bool SwGlossaryHdl::CopyOrMove( const OUString& rSourceGroupName, OUString& rSourceShortName,
                        const OUString& rDestGroupName, const OUString& rLongName, sal_Bool bMove )
{
    SwTextBlocks* pSourceGroup = rStatGlossaries.GetGroupDoc(rSourceGroupName, sal_False);

    SwTextBlocks* pDestGroup = rStatGlossaries.GetGroupDoc(rDestGroupName, sal_False);
    if(pDestGroup->IsReadOnly() || (bMove && pSourceGroup->IsReadOnly()) )
        return sal_False;

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
sal_Bool SwGlossaryHdl::DelGroup(const OUString &rGrpName)
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
        return sal_True;
    }
    return sal_False;
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

// short name for autotext already used?
sal_Bool SwGlossaryHdl::HasShortName(const OUString& rShortName) const
{
    SwTextBlocks *pBlock = pCurGrp ? pCurGrp
                                   : rStatGlossaries.GetGroupDoc( aCurGrp );
    sal_Bool bRet = pBlock->GetIndex( rShortName ) != (sal_uInt16) -1;
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pBlock );
    return bRet;
}

sal_Bool    SwGlossaryHdl::ConvertToNew(SwTextBlocks& /*rOld*/)
{
    return sal_True;
}

// Create autotext
sal_Bool SwGlossaryHdl::NewGlossary(const OUString& rName, const OUString& rShortName,
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

    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();

    const sal_uInt16 nSuccess = pWrtShell->MakeGlossary( *pTmp, rName, rShortName,
                            rCfg.IsSaveRelFile(), pOnlyTxt );
    if(nSuccess == (sal_uInt16) -1 )
    {
        InfoBox(pWrtShell->GetView().GetWindow(), SW_RES(MSG_ERR_INSERT_GLOS)).Execute();
    }
    if( !pCurGrp )
        rStatGlossaries.PutGroupDoc( pTmp );
    return sal_Bool( nSuccess != (sal_uInt16) -1 );
}

// Delete a autotext
sal_Bool SwGlossaryHdl::DelGlossary(const OUString &rShortName)
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

// expand short name
sal_Bool SwGlossaryHdl::ExpandGlossary()
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

sal_Bool SwGlossaryHdl::Expand( const OUString& rShortName,
                            SwGlossaries *pGlossaries,
                            SwTextBlocks *pGlossary  )
{
    TextBlockInfoArr aFoundArr;
    OUString aShortName( rShortName );
    sal_Bool bCancel = sal_False;
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
                const sal_uInt16 nRet = RET_OK == pDlg->Execute()?
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
                    bCancel = sal_True;
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

        return sal_False;
    }
    else
    {
        SvxMacro aStartMacro(aEmptyStr, aEmptyStr, STARBASIC);
        SvxMacro aEndMacro(aEmptyStr, aEmptyStr, STARBASIC);
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
        SwInputFieldList aFldLst( pWrtShell, sal_True );

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
    return sal_True;
}

// add autotext
bool SwGlossaryHdl::InsertGlossary(const OUString &rName)
{
    OSL_ENSURE(pWrtShell->CanInsert(), "illegal");

    SwTextBlocks *pGlos =
        pCurGrp? pCurGrp: rStatGlossaries.GetGroupDoc(aCurGrp);

    if (!pGlos)
        return false;

    SvxMacro aStartMacro(aEmptyStr, aEmptyStr, STARBASIC);
    SvxMacro aEndMacro(aEmptyStr, aEmptyStr, STARBASIC);
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
    SwInputFieldList aFldLst( pWrtShell, sal_True );

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
sal_Bool SwGlossaryHdl::Rename(const OUString& rOldShort, const OUString& rNewShortName,
                               const OUString& rNewName )
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
            pGlossary->Rename( nIdx, &rNewShortName, &rNewName );
            bRet = pGlossary->GetError() == 0;
        }
        if( !pCurGrp )
            rStatGlossaries.PutGroupDoc(pGlossary);
    }
    return bRet;
}


sal_Bool SwGlossaryHdl::IsReadOnly( const OUString* pGrpNm ) const
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

// find group without path index
sal_Bool SwGlossaryHdl::FindGroupName(OUString& rGroup)
{
    return rStatGlossaries.FindGroupName(rGroup);
}

sal_Bool SwGlossaryHdl::CopyToClipboard(SwWrtShell& rSh, const OUString& rShortName)
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

sal_Bool SwGlossaryHdl::ImportGlossaries( const OUString& rName )
{
    sal_Bool bRet = sal_False;
    if( !rName.isEmpty() )
    {
        const SfxFilter* pFilter = 0;
        SfxMedium* pMed = new SfxMedium( rName, STREAM_READ, 0, 0 );
        SfxFilterMatcher aMatcher( OUString("swriter") );
        pMed->UseInteractionHandler( sal_True );
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
