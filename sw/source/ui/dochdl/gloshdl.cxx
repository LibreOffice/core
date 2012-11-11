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
    String sTitle;
    String sLongName;
    String sGroupName;
};
typedef boost::ptr_vector<TextBlockInfo_Impl> TextBlockInfoArr;
SV_IMPL_REF( SwDocShell )

/*------------------------------------------------------------------------
    Description:    Dialog for edit templates
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
    Description:    set the default group; if called from the dialog
                    the group is created temporarily for faster access
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
            const std::vector<String> & rPathArr =
                rStatGlossaries.GetPathArray();
            sal_uInt16 nCurrentPath = USHRT_MAX;
            for (size_t nPath = 0; nPath < rPathArr.size(); ++nPath)
            {
                if (sCurEntryPath == rPathArr[nPath])
                {
                    nCurrentPath = static_cast<sal_uInt16>(nPath);
                    break;
                }
            }
            String sPath = sGroup.GetToken(1, GLOS_DELIM);
            sal_uInt16 nComparePath = (sal_uInt16)sPath.ToInt32();
            if(nCurrentPath == nComparePath &&
                sGroup.GetToken(0, GLOS_DELIM) == sCurBase)
                bPathEqual = sal_True;
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

sal_Bool SwGlossaryHdl::NewGroup(String &rGrpName, const String& rTitle)
{
    if(STRING_NOTFOUND == rGrpName.Search(GLOS_DELIM))
        FindGroupName(rGrpName);
    return rStatGlossaries.NewGroupDoc(rGrpName, rTitle);
}

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

sal_Bool SwGlossaryHdl::CopyOrMove( const String& rSourceGroupName,  String& rSourceShortName,
                        const String& rDestGroupName, const String& rLongName, sal_Bool bMove )
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

/*------------------------------------------------------------------------
    Description: delete a autotext-file-group
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
    Description:    ask for number of autotexts
------------------------------------------------------------------------*/
sal_uInt16 SwGlossaryHdl::GetGlossaryCnt()
{
    return pCurGrp ? pCurGrp->GetCount() : 0;
}

String SwGlossaryHdl::GetGlossaryName( sal_uInt16 nId )
{
    OSL_ENSURE(nId < GetGlossaryCnt(), "Textbausteinarray ueberindiziert.");
    return pCurGrp->GetLongName( nId );
}

String  SwGlossaryHdl::GetGlossaryShortName(sal_uInt16 nId)
{
    OSL_ENSURE(nId < GetGlossaryCnt(), "Textbausteinarray ueberindiziert.");
    return pCurGrp->GetShortName( nId );
}

/*------------------------------------------------------------------------
    Description:    ask for short name
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
    Description:    short name for autotext already used?
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

sal_Bool    SwGlossaryHdl::ConvertToNew(SwTextBlocks& /*rOld*/)
{
    return sal_True;
}

/*------------------------------------------------------------------------
    Desription:     Create autotext
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
/*------------------------------------------------------------------------
    Description:    Delete a autotext
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
    Description: expand short name
------------------------------------------------------------------------*/
sal_Bool SwGlossaryHdl::ExpandGlossary()
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

sal_Bool SwGlossaryHdl::Expand( const String& rShortName,
                            SwGlossaries *pGlossaries,
                            SwTextBlocks *pGlossary  )
{
    TextBlockInfoArr aFoundArr;
    String aShortName( rShortName );
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
            // get group name with path-extension
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

                AbstractSwSelGlossaryDlg* pDlg = pFact->CreateSwSelGlossaryDlg( 0, aShortName, DLG_SEL_GLOS );
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

/*------------------------------------------------------------------------
    Description: add autotext
------------------------------------------------------------------------*/
bool SwGlossaryHdl::InsertGlossary(const String &rName)
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

/*------------------------------------------------------------------------
 Description:   set / ask for macro
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
        aMacroTbl.Insert( SW_EVENT_START_INS_GLOSSARY, *pStart);
    if( pEnd )
        aMacroTbl.Insert( SW_EVENT_END_INS_GLOSSARY, *pEnd);
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
    Description:    ctor, dtor
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
    Description:    rename an autotext
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

/*--------------------------------------------------
    find group without path index
--------------------------------------------------*/
sal_Bool SwGlossaryHdl::FindGroupName(String & rGroup)
{
    return rStatGlossaries.FindGroupName(rGroup);
}

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
        SfxMedium* pMed = new SfxMedium( rName, STREAM_READ, 0, 0 );
        SfxFilterMatcher aMatcher( rtl::OUString("swriter") );
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
