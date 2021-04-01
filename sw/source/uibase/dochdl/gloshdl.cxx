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

#include <vcl/errinf.hxx>
#include <vcl/weld.hxx>
#include <svl/macitem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <gloshdl.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <swundo.hxx>
#include <expfld.hxx>
#include <initui.hxx>
#include <gloslst.hxx>
#include <swdtflvr.hxx>

#include <strings.hrc>
#include <vcl/svapp.hxx>
#include <osl/diagnose.h>

#include <editeng/acorrcfg.hxx>
#include <sfx2/event.hxx>
#include <swabstdlg.hxx>

#include <memory>

using namespace ::com::sun::star;

const short RET_EDIT = 100;

namespace {

struct TextBlockInfo_Impl
{
    OUString sTitle;
    OUString sLongName;
    OUString sGroupName;
    TextBlockInfo_Impl(OUString const& rTitle, OUString const& rLongName, OUString const& rGroupName)
        : sTitle(rTitle), sLongName(rLongName), sGroupName(rGroupName) {}
};

}

// Dialog for edit templates
void SwGlossaryHdl::GlossaryDlg()
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractGlossaryDlg> pDlg(pFact->CreateGlossaryDlg(pViewFrame, this, pWrtShell));
    OUString sName;
    OUString sShortName;

    if( RET_EDIT == pDlg->Execute() )
    {
        sName = pDlg->GetCurrGrpName();
        sShortName = pDlg->GetCurrShortName();
    }

    pDlg.disposeAndClear();
    pCurGrp.reset();
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
        sGroup += OUStringChar(GLOS_DELIM) + "0";
    }
    if(pCurGrp)
    {
        bool bPathEqual = false;
        if(!bAlwaysCreateNew)
        {
            INetURLObject aTemp( pCurGrp->GetFileName() );
            const OUString sCurBase = aTemp.getBase();
            aTemp.removeSegment();
            const OUString sCurEntryPath = aTemp.GetMainURL(INetURLObject::DecodeMechanism::NONE);
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
            sal_uInt16 nComparePath = static_cast<sal_uInt16>(sPath.toInt32());
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
        pCurGrp = rStatGlossaries.GetGroupDoc(aCurGrp, true);
    }
}

size_t SwGlossaryHdl::GetGroupCnt() const
{
    return rStatGlossaries.GetGroupCnt();
}

OUString SwGlossaryHdl::GetGroupName( size_t nId, OUString* pTitle )
{
    OUString sRet = rStatGlossaries.GetGroupName(nId);
    if(pTitle)
    {
        std::unique_ptr<SwTextBlocks> pGroup = rStatGlossaries.GetGroupDoc(sRet);
        if (pGroup && !pGroup->GetError())
        {
            *pTitle = pGroup->GetName();
            if (pTitle->isEmpty())
            {
                *pTitle = sRet.getToken(0, GLOS_DELIM);
                pGroup->SetName(*pTitle);
            }
        }
        else
        {
            sRet.clear();
        }
    }
    return sRet;
}

void SwGlossaryHdl::NewGroup(OUString &rGrpName, const OUString& rTitle)
{
    if (rGrpName.indexOf(GLOS_DELIM)<0)
        FindGroupName(rGrpName);
    rStatGlossaries.NewGroupDoc(rGrpName, rTitle);
}

void SwGlossaryHdl::RenameGroup(const OUString& rOld, OUString& rNew, const OUString& rNewTitle)
{
    OUString sOldGroup(rOld);
    if (rOld.indexOf(GLOS_DELIM)<0)
        FindGroupName(sOldGroup);
    if(rOld == rNew)
    {
        std::unique_ptr<SwTextBlocks> pGroup = rStatGlossaries.GetGroupDoc(sOldGroup);
        if(pGroup)
        {
            pGroup->SetName(rNewTitle);
        }
    }
    else
    {
        OUString sNewGroup(rNew);
        if (sNewGroup.indexOf(GLOS_DELIM)<0)
        {
            sNewGroup += OUStringChar(GLOS_DELIM) + "0";
        }
        rStatGlossaries.RenameGroupDoc(sOldGroup, sNewGroup, rNewTitle);
        rNew = sNewGroup;
    }
}

// delete an autotext-file-group
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
                pCurGrp.reset();
        }
        return true;
    }
    return false;
}

// ask for number of autotexts
sal_uInt16 SwGlossaryHdl::GetGlossaryCnt() const
{
    return pCurGrp ? pCurGrp->GetCount() : 0;
}

OUString SwGlossaryHdl::GetGlossaryName( sal_uInt16 nId )
{
    OSL_ENSURE(nId < GetGlossaryCnt(), "Text building block array over-indexed.");
    return pCurGrp->GetLongName( nId );
}

OUString SwGlossaryHdl::GetGlossaryShortName(sal_uInt16 nId)
{
    OSL_ENSURE(nId < GetGlossaryCnt(), "Text building block array over-indexed.");
    return pCurGrp->GetShortName( nId );
}

// ask for short name
OUString SwGlossaryHdl::GetGlossaryShortName(const OUString &rName)
{
    OUString sReturn;
    SwTextBlocks *pTmp =
        pCurGrp ? pCurGrp.get() : rStatGlossaries.GetGroupDoc( aCurGrp ).release();
    if(pTmp)
    {
        sal_uInt16 nIdx = pTmp->GetLongIndex( rName );
        if( nIdx != sal_uInt16(-1) )
            sReturn = pTmp->GetShortName( nIdx );
        if( !pCurGrp )
            delete pTmp;
    }
    return sReturn;
}

// short name for autotext already used?
bool SwGlossaryHdl::HasShortName(const OUString& rShortName) const
{
    SwTextBlocks *pBlock = pCurGrp ? pCurGrp.get()
                                   : rStatGlossaries.GetGroupDoc( aCurGrp ).release();
    bool bRet = pBlock->GetIndex( rShortName ) != sal_uInt16(-1);
    if( !pCurGrp )
        delete pBlock;
    return bRet;
}

// Create autotext
bool SwGlossaryHdl::NewGlossary(const OUString& rName, const OUString& rShortName,
                                bool bCreateGroup, bool bNoAttr)
{
    SwTextBlocks *pTmp =
        pCurGrp ? pCurGrp.get() : rStatGlossaries.GetGroupDoc( aCurGrp, bCreateGroup ).release();
    //pTmp == 0 if the AutoText path setting is wrong
    if(!pTmp)
    {
        if (!pCurGrp)
            delete pTmp;
        return false;
    }

    OUString sOnlyText;
    OUString* pOnlyText = nullptr;
    if( bNoAttr )
    {
        pWrtShell->GetSelectedText( sOnlyText, ParaBreakType::ToOnlyCR );
        pOnlyText = &sOnlyText;
    }

    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();

    const sal_uInt16 nSuccess = pWrtShell->MakeGlossary( *pTmp, rName, rShortName,
                            rCfg.IsSaveRelFile(), pOnlyText );
    if(nSuccess == sal_uInt16(-1) )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWrtShell->GetView().GetFrameWeld(),
                                                                                   VclMessageType::Info, VclButtonsType::Ok, SwResId(STR_ERR_INSERT_GLOS)));
        xBox->run();
    }
    if( !pCurGrp )
        delete pTmp;
    return nSuccess != sal_uInt16(-1);
}

// Delete an autotext
bool SwGlossaryHdl::DelGlossary(const OUString &rShortName)
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp.get()
                                    : rStatGlossaries.GetGroupDoc(aCurGrp).release();
    //pTmp == 0 if the AutoText path setting is wrong
    if(!pGlossary)
    {
        if( !pCurGrp )
            delete pGlossary;
        return false;
    }

    sal_uInt16 nIdx = pGlossary->GetIndex( rShortName );
    if( nIdx != sal_uInt16(-1) )
        pGlossary->Delete( nIdx );
    if( !pCurGrp )
        delete pGlossary;
    return true;
}

// expand short name
bool SwGlossaryHdl::ExpandGlossary(weld::Window* pParent)
{
    OSL_ENSURE(pWrtShell->CanInsert(), "illegal");
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ::GlossaryGetCurrGroup fnGetCurrGroup = pFact->GetGlossaryCurrGroupFunc();
    OUString sGroupName( (*fnGetCurrGroup)() );
    if (sGroupName.indexOf(GLOS_DELIM)<0)
        FindGroupName(sGroupName);
    std::unique_ptr<SwTextBlocks> pGlossary = rStatGlossaries.GetGroupDoc(sGroupName);

    OUString aShortName;

        // use this at text selection
    if(pWrtShell->SwCursorShell::HasSelection() && !pWrtShell->IsBlockMode())
    {
        aShortName = pWrtShell->GetSelText();
    }
    else
    {
        if(pWrtShell->IsAddMode())
            pWrtShell->LeaveAddMode();
        else if(pWrtShell->IsBlockMode())
            pWrtShell->LeaveBlockMode();
        else if(pWrtShell->IsExtMode())
            pWrtShell->LeaveExtMode();
        // select word (tdf#126589: part to the left of cursor)
        if (pWrtShell->IsInWord() || pWrtShell->IsEndWrd())
            pWrtShell->PrvWrd(true);
            // ask for word
        if(pWrtShell->IsSelection())
            aShortName = pWrtShell->GetSelText();
    }
    return pGlossary && Expand(pParent, aShortName, &rStatGlossaries, std::move(pGlossary));
}

bool SwGlossaryHdl::Expand(weld::Window* pParent, const OUString& rShortName,
                            SwGlossaries *pGlossaries,
                            std::unique_ptr<SwTextBlocks> pGlossary)
{
    std::vector<TextBlockInfo_Impl> aFoundArr;
    OUString aShortName( rShortName );
    bool bCancel = false;
    // search for text block
    // - don't prefer current group depending on configuration setting
    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    sal_uInt16 nFound = !rCfg.IsSearchInAllCategories() ? pGlossary->GetIndex( aShortName ) : -1;
    // if not found then search in all groups
    if( nFound == sal_uInt16(-1) )
    {
        const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        const size_t nGroupCount = pGlossaryList->GetGroupCount();
        for(size_t i = 0; i < nGroupCount; ++i)
        {
            // get group name with path-extension
            const OUString sGroupName = pGlossaryList->GetGroupName(i);
            if(sGroupName == pGlossary->GetName())
                continue;
            const sal_uInt16 nBlockCount = pGlossaryList->GetBlockCount(i);
            if(nBlockCount)
            {
                const OUString sTitle = pGlossaryList->GetGroupTitle(i);
                for(sal_uInt16 j = 0; j < nBlockCount; j++)
                {
                    const OUString sLongName(pGlossaryList->GetBlockLongName(i, j));
                    const OUString sShortName(pGlossaryList->GetBlockShortName(i, j));
                    if( rSCmp.isEqual( rShortName, sShortName ))
                    {
                        aFoundArr.emplace_back(sTitle, sLongName, sGroupName);
                    }
                }
            }
        }
        if( !aFoundArr.empty() )  // one was found
        {
            pGlossary.reset();
            if (1 == aFoundArr.size())
            {
                TextBlockInfo_Impl& rData = aFoundArr.front();
                pGlossary = pGlossaries->GetGroupDoc(rData.sGroupName);
                nFound = pGlossary->GetIndex( aShortName );
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractSwSelGlossaryDlg> pDlg(pFact->CreateSwSelGlossaryDlg(pParent, aShortName));
                for(const TextBlockInfo_Impl & i : aFoundArr)
                {
                    pDlg->InsertGlos(i.sTitle, i.sLongName);
                }
                pDlg->SelectEntryPos(0);
                const sal_Int32 nRet = RET_OK == pDlg->Execute() ?
                                        pDlg->GetSelectedIdx() :
                                        -1;
                pDlg.disposeAndClear();
                if (nRet != -1)
                {
                    TextBlockInfo_Impl& rData = aFoundArr[nRet];
                    pGlossary = pGlossaries->GetGroupDoc(rData.sGroupName);
                    nFound = pGlossary->GetIndex( aShortName );
                }
                else
                {
                    nFound = sal_uInt16(-1);
                    bCancel = true;
                }
            }
        }
    }

    // not found
    if( nFound == sal_uInt16(-1) )
    {
        if( !bCancel )
        {
            pGlossary.reset();

            const sal_Int32 nMaxLen = 50;
            if(pWrtShell->IsSelection() && aShortName.getLength() > nMaxLen)
            {
                aShortName = OUString::Concat(aShortName.subView(0, nMaxLen)) + " ...";
            }
            OUString aTmp( SwResId(STR_NOGLOS));
            aTmp = aTmp.replaceFirst("%1", aShortName);
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWrtShell->GetView().GetFrameWeld(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          aTmp));
            xInfoBox->run();
        }

        return false;
    }
    else
    {
        SvxMacro aStartMacro(OUString(), OUString(), STARBASIC);
        SvxMacro aEndMacro(OUString(), OUString(), STARBASIC);
        GetMacros( aShortName, aStartMacro, aEndMacro, pGlossary.get() );

        // StartAction must not be before HasSelection and DelRight,
        // otherwise the possible Shell change gets delayed and
        // API-programs would hang.
        // Moreover the event macro must also not be called in an action
        pWrtShell->StartUndo(SwUndoId::INSGLOSSARY);
        if( aStartMacro.HasMacro() )
            pWrtShell->ExecMacro( aStartMacro );
        if(pWrtShell->HasSelection())
            pWrtShell->DelLeft();
        pWrtShell->StartAllAction();

        // cache all InputFields
        SwInputFieldList aFieldLst( pWrtShell, true );

        pWrtShell->InsertGlossary(*pGlossary, aShortName);
        pWrtShell->EndAllAction();
        if( aEndMacro.HasMacro() )
        {
            pWrtShell->ExecMacro( aEndMacro );
        }
        pWrtShell->EndUndo(SwUndoId::INSGLOSSARY);

        // demand input for all new InputFields
        if( aFieldLst.BuildSortLst() )
            pWrtShell->UpdateInputFields( &aFieldLst );
    }
    return true;
}

// add autotext
bool SwGlossaryHdl::InsertGlossary(const OUString &rName)
{
    OSL_ENSURE(pWrtShell->CanInsert(), "illegal");

    SwTextBlocks *pGlos =
        pCurGrp ? pCurGrp.get() : rStatGlossaries.GetGroupDoc(aCurGrp).release();

    if (!pGlos)
    {
        if (!pCurGrp)
            delete pGlos;
        return false;
    }

    SvxMacro aStartMacro(OUString(), OUString(), STARBASIC);
    SvxMacro aEndMacro(OUString(), OUString(), STARBASIC);
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
    SwInputFieldList aFieldLst( pWrtShell, true );

    pWrtShell->InsertGlossary(*pGlos, rName);
    pWrtShell->EndAllAction();
    if( aEndMacro.HasMacro() )
    {
        pWrtShell->ExecMacro( aEndMacro );
    }

    // demand input for all new InputFields
    if( aFieldLst.BuildSortLst() )
        pWrtShell->UpdateInputFields( &aFieldLst );

    if(!pCurGrp)
        delete pGlos;
    return true;
}

// set / ask for macro
void SwGlossaryHdl::SetMacros(const OUString& rShortName,
                              const SvxMacro* pStart,
                              const SvxMacro* pEnd,
                              SwTextBlocks *pGlossary )
{
    SwTextBlocks *pGlos = pGlossary ? pGlossary :
                                pCurGrp ? pCurGrp.get()
                                  : rStatGlossaries.GetGroupDoc( aCurGrp ).release();
    SvxMacroTableDtor aMacroTable;
    if( pStart )
        aMacroTable.Insert( SvMacroItemId::SwStartInsGlossary, *pStart);
    if( pEnd )
        aMacroTable.Insert( SvMacroItemId::SwEndInsGlossary, *pEnd);
    sal_uInt16 nIdx = pGlos->GetIndex( rShortName );
    if( !pGlos->SetMacroTable( nIdx, aMacroTable ) && pGlos->GetError() )
        ErrorHandler::HandleError( pGlos->GetError() );

    if(!pCurGrp && !pGlossary)
        delete pGlos;
}

void SwGlossaryHdl::GetMacros( const OUString &rShortName,
                                SvxMacro& rStart,
                                SvxMacro& rEnd,
                                SwTextBlocks *pGlossary  )
{
    SwTextBlocks *pGlos = pGlossary ? pGlossary
                                    : pCurGrp ? pCurGrp.get()
                                        : rStatGlossaries.GetGroupDoc(aCurGrp).release();
    sal_uInt16 nIndex = pGlos->GetIndex( rShortName );
    if( nIndex != USHRT_MAX )
    {
        SvxMacroTableDtor aMacroTable;
        if( pGlos->GetMacroTable( nIndex, aMacroTable ) )
        {
            SvxMacro *pMacro = aMacroTable.Get( SvMacroItemId::SwStartInsGlossary );
            if( pMacro )
                rStart = *pMacro;

            pMacro = aMacroTable.Get( SvMacroItemId::SwEndInsGlossary );
            if( pMacro )
                rEnd = *pMacro;
        }
    }

    if( !pCurGrp && !pGlossary )
        delete pGlos;
}

// ctor, dtor
SwGlossaryHdl::SwGlossaryHdl(SfxViewFrame* pVwFrame, SwWrtShell *pSh)
    : rStatGlossaries( *::GetGlossaries() ),
    aCurGrp( SwGlossaries::GetDefName() ),
    pViewFrame( pVwFrame ),
    pWrtShell( pSh )
{
}

SwGlossaryHdl::~SwGlossaryHdl()
{
}

// rename an autotext
bool SwGlossaryHdl::Rename(const OUString& rOldShort, const OUString& rNewShortName,
                           const OUString& rNewName )
{
    bool bRet = false;
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp.get()
                                    : rStatGlossaries.GetGroupDoc(aCurGrp).release();
    if(pGlossary)
    {
        sal_uInt16 nIdx = pGlossary->GetIndex( rOldShort );
        sal_uInt16 nOldLongIdx = pGlossary->GetLongIndex( rNewName );
        sal_uInt16 nOldIdx = pGlossary->GetIndex( rNewShortName );

        if( nIdx != USHRT_MAX &&
                (nOldLongIdx == USHRT_MAX || nOldLongIdx == nIdx )&&
                    (nOldIdx == USHRT_MAX || nOldIdx == nIdx ))
        {
            pGlossary->Rename( nIdx, &rNewShortName, &rNewName );
            bRet = pGlossary->GetError() == ERRCODE_NONE;
        }
        if( !pCurGrp )
            delete pGlossary;
    }
    return bRet;
}

bool SwGlossaryHdl::IsReadOnly( const OUString* pGrpNm ) const
{
    SwTextBlocks *pGlossary = nullptr;

    if (pGrpNm)
        pGlossary = rStatGlossaries.GetGroupDoc( *pGrpNm ).release();
    else if (pCurGrp)
        pGlossary = pCurGrp.get();
    else
        pGlossary = rStatGlossaries.GetGroupDoc(aCurGrp).release();

    const bool bRet = !pGlossary || pGlossary->IsReadOnly();
    if( pGrpNm || !pCurGrp )
        delete pGlossary;
    return bRet;
}

bool SwGlossaryHdl::IsOld() const
{
    if( !pCurGrp )
        rStatGlossaries.GetGroupDoc(aCurGrp).reset();
    return false;
}

// find group without path index
bool SwGlossaryHdl::FindGroupName(OUString& rGroup)
{
    return rStatGlossaries.FindGroupName(rGroup);
}

bool SwGlossaryHdl::CopyToClipboard(SwWrtShell& rSh, const OUString& rShortName)
{
    SwTextBlocks *pGlossary = pCurGrp ? pCurGrp.get()
                                    : rStatGlossaries.GetGroupDoc(aCurGrp).release();

    rtl::Reference<SwTransferable> pTransfer = new SwTransferable( rSh );

    bool bRet = pTransfer->CopyGlossary( *pGlossary, rShortName );
    if( !pCurGrp )
        delete pGlossary;
    return bRet;
}

bool SwGlossaryHdl::ImportGlossaries( const OUString& rName )
{
    bool bRet = false;
    if( !rName.isEmpty() )
    {
        std::shared_ptr<const SfxFilter> pFilter;
        std::unique_ptr<SfxMedium> pMed(new SfxMedium( rName, StreamMode::READ, nullptr, nullptr ));
        SfxFilterMatcher aMatcher( "swriter" );
        pMed->UseInteractionHandler( true );
        if (!aMatcher.GuessFilter(*pMed, pFilter, SfxFilterFlags::NONE))
        {
            SwTextBlocks *pGlossary = nullptr;
            pMed->SetFilter( pFilter );
            Reader* pR = SwReaderWriter::GetReader( pFilter->GetUserData() );
            if( pR && nullptr != ( pGlossary = pCurGrp ? pCurGrp.get()
                                    : rStatGlossaries.GetGroupDoc(aCurGrp).release()) )
            {
                SwReader aReader( *pMed, rName );
                if( aReader.HasGlossaries( *pR ) )
                {
                    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
                    bRet = aReader.ReadGlossaries( *pR, *pGlossary,
                                rCfg.IsSaveRelFile() );
                }

                if (!pCurGrp)
                    delete pGlossary;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
