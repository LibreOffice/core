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

#include "XMLChangeTrackingImportHelper.hxx"
#include <formulacell.hxx>
#include <document.hxx>
#include <rangeutl.hxx>
#include <tools/datetime.hxx>
#include <osl/diagnose.h>
#include <svl/zforlist.hxx>
#include <sax/tools/converter.hxx>

#define SC_CHANGE_ID_PREFIX "ct"

ScMyCellInfo::ScMyCellInfo(
    const ScCellValue& rCell, const OUString& rFormulaAddress, const OUString& rFormula,
    const formula::FormulaGrammar::Grammar eTempGrammar, const OUString& rInputString,
    const double& rValue, const sal_uInt16 nTempType, const ScMatrixMode nTempMatrixFlag, const sal_Int32 nTempMatrixCols,
    const sal_Int32 nTempMatrixRows ) :
    maCell(rCell),
    sFormulaAddress(rFormulaAddress),
    sFormula(rFormula),
    sInputString(rInputString),
    fValue(rValue),
    nMatrixCols(nTempMatrixCols),
    nMatrixRows(nTempMatrixRows),
    eGrammar( eTempGrammar),
    nType(nTempType),
    nMatrixFlag(nTempMatrixFlag)
{
}

ScMyCellInfo::~ScMyCellInfo() {}

const ScCellValue& ScMyCellInfo::CreateCell( ScDocument* pDoc )
{
    if (!maCell.isEmpty())
        return maCell;

    if (!sFormula.isEmpty() && !sFormulaAddress.isEmpty())
    {
        ScAddress aPos;
        sal_Int32 nOffset(0);
        ScRangeStringConverter::GetAddressFromString(aPos, sFormulaAddress, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset);
        maCell.meType = CELLTYPE_FORMULA;
        maCell.mpFormula = new ScFormulaCell(pDoc, aPos, sFormula, eGrammar, nMatrixFlag);
        maCell.mpFormula->SetMatColsRows(static_cast<SCCOL>(nMatrixCols), static_cast<SCROW>(nMatrixRows));
    }

    if ((nType == css::util::NumberFormat::DATE || nType == css::util::NumberFormat::TIME) && sInputString.isEmpty())
    {
        sal_uInt32 nFormat(0);
        if (nType == css::util::NumberFormat::DATE)
            nFormat = pDoc->GetFormatTable()->GetStandardFormat( SvNumFormatType::DATE, ScGlobal::eLnge );
        else if (nType == css::util::NumberFormat::TIME)
            nFormat = pDoc->GetFormatTable()->GetStandardFormat( SvNumFormatType::TIME, ScGlobal::eLnge );
        pDoc->GetFormatTable()->GetInputLineString(fValue, nFormat, sInputString);
    }

    return maCell;
}

ScMyBaseAction::ScMyBaseAction(const ScChangeActionType nTempActionType)
    : aDependencies(),
    aDeletedList(),
    nActionNumber(0),
    nRejectingNumber(0),
    nPreviousAction(0),
    nActionType(nTempActionType),
    nActionState(SC_CAS_VIRGIN)
{
}

ScMyBaseAction::~ScMyBaseAction()
{
}

ScMyInsAction::ScMyInsAction(const ScChangeActionType nActionTypeP)
    : ScMyBaseAction(nActionTypeP)
{
}

ScMyInsAction::~ScMyInsAction()
{
}

ScMyDelAction::ScMyDelAction(const ScChangeActionType nActionTypeP)
    : ScMyBaseAction(nActionTypeP),
    aGeneratedList(),
    aMoveCutOffs(),
    nD(0)
{
}

ScMyDelAction::~ScMyDelAction()
{
}

ScMyMoveAction::ScMyMoveAction()
    : ScMyBaseAction(SC_CAT_MOVE),
    aGeneratedList()
{
}

ScMyMoveAction::~ScMyMoveAction()
{
}

ScMyContentAction::ScMyContentAction()
    : ScMyBaseAction(SC_CAT_CONTENT)
{
}

ScMyContentAction::~ScMyContentAction()
{
}

ScMyRejAction::ScMyRejAction()
    : ScMyBaseAction(SC_CAT_REJECT)
{
}

ScMyRejAction::~ScMyRejAction()
{
}

ScXMLChangeTrackingImportHelper::ScXMLChangeTrackingImportHelper() :
    aActions(),
    pDoc(nullptr),
    pTrack(nullptr),
    nMultiSpanned(0),
    nMultiSpannedSlaveCount(0)
{
}

ScXMLChangeTrackingImportHelper::~ScXMLChangeTrackingImportHelper()
{
}

void ScXMLChangeTrackingImportHelper::StartChangeAction(const ScChangeActionType nActionType)
{
    OSL_ENSURE(!pCurrentAction, "a not inserted action");
    switch (nActionType)
    {
        case SC_CAT_INSERT_COLS:
        case SC_CAT_INSERT_ROWS:
        case SC_CAT_INSERT_TABS:
        {
            pCurrentAction = std::make_unique<ScMyInsAction>(nActionType);
        }
        break;
        case SC_CAT_DELETE_COLS:
        case SC_CAT_DELETE_ROWS:
        case SC_CAT_DELETE_TABS:
        {
            pCurrentAction = std::make_unique<ScMyDelAction>(nActionType);
        }
        break;
        case SC_CAT_MOVE:
        {
            pCurrentAction = std::make_unique<ScMyMoveAction>();
        }
        break;
        case SC_CAT_CONTENT:
        {
            pCurrentAction = std::make_unique<ScMyContentAction>();
        }
        break;
        case SC_CAT_REJECT:
        {
            pCurrentAction = std::make_unique<ScMyRejAction>();
        }
        break;
        default:
        {
            // added to avoid warnings
        }
    }
}

sal_uInt32 ScXMLChangeTrackingImportHelper::GetIDFromString(const OUString& sID)
{
    sal_uInt32 nResult(0);
    if (!sID.isEmpty())
    {
        if (sID.startsWith(SC_CHANGE_ID_PREFIX))
        {
            OUString sValue(sID.copy(strlen(SC_CHANGE_ID_PREFIX)));
            sal_Int32 nValue;
            ::sax::Converter::convertNumber(nValue, sValue);
            OSL_ENSURE(nValue > 0, "wrong change action ID");
            nResult = nValue;
        }
        else
        {
            OSL_FAIL("wrong change action ID");
        }
    }
    return nResult;
}

void ScXMLChangeTrackingImportHelper::SetActionInfo(const ScMyActionInfo& aInfo)
{
    pCurrentAction->aInfo = aInfo;
    aUsers.insert(aInfo.sUser);
}

void ScXMLChangeTrackingImportHelper::SetPreviousChange(const sal_uInt32 nPreviousAction,
                            ScMyCellInfo* pCellInfo)
{
    OSL_ENSURE(pCurrentAction->nActionType == SC_CAT_CONTENT, "wrong action type");
    ScMyContentAction* pAction = static_cast<ScMyContentAction*>(pCurrentAction.get());
    pAction->nPreviousAction = nPreviousAction;
    pAction->pCellInfo.reset( pCellInfo );
}

void ScXMLChangeTrackingImportHelper::SetPosition(const sal_Int32 nPosition, const sal_Int32 nCount, const sal_Int32 nTable)
{
    OSL_ENSURE(((pCurrentAction->nActionType != SC_CAT_MOVE) &&
                (pCurrentAction->nActionType != SC_CAT_CONTENT) &&
                (pCurrentAction->nActionType != SC_CAT_REJECT)), "wrong action type");
    OSL_ENSURE(nCount > 0, "wrong count");
    switch(pCurrentAction->nActionType)
    {
        case SC_CAT_INSERT_COLS:
        case SC_CAT_DELETE_COLS:
        {
            pCurrentAction->aBigRange.Set(nPosition, nInt32Min, nTable,
                                        nPosition + nCount - 1, nInt32Max, nTable);
        }
        break;
        case SC_CAT_INSERT_ROWS:
        case SC_CAT_DELETE_ROWS:
        {
            pCurrentAction->aBigRange.Set(nInt32Min, nPosition, nTable,
                                        nInt32Max, nPosition + nCount - 1, nTable);
        }
        break;
        case SC_CAT_INSERT_TABS:
        case SC_CAT_DELETE_TABS:
        {
            pCurrentAction->aBigRange.Set(nInt32Min, nInt32Min, nPosition,
                                        nInt32Max, nInt32Max, nPosition + nCount - 1);
        }
        break;
        default:
        {
            // added to avoid warnings
        }
    }
}

void ScXMLChangeTrackingImportHelper::AddDeleted(const sal_uInt32 nID)
{
    pCurrentAction->aDeletedList.emplace_front( nID, nullptr );
}

void ScXMLChangeTrackingImportHelper::AddDeleted(const sal_uInt32 nID, std::unique_ptr<ScMyCellInfo> pCellInfo)
{
    pCurrentAction->aDeletedList.emplace_front( nID, std::move(pCellInfo) );
}

void ScXMLChangeTrackingImportHelper::SetMultiSpanned(const sal_Int16 nTempMultiSpanned)
{
    if (nTempMultiSpanned)
    {
        OSL_ENSURE(((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
                    (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS)), "wrong action type");
        nMultiSpanned = nTempMultiSpanned;
        nMultiSpannedSlaveCount = 0;
    }
}

void ScXMLChangeTrackingImportHelper::SetInsertionCutOff(const sal_uInt32 nID, const sal_Int32 nPosition)
{
    if ((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
        (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS))
    {
        static_cast<ScMyDelAction*>(pCurrentAction.get())->pInsCutOff.reset( new ScMyInsertionCutOff(nID, nPosition) );
    }
    else
    {
        OSL_FAIL("wrong action type");
    }
}

void ScXMLChangeTrackingImportHelper::AddMoveCutOff(const sal_uInt32 nID, const sal_Int32 nStartPosition, const sal_Int32 nEndPosition)
{
    if ((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
        (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS))
    {
        static_cast<ScMyDelAction*>(pCurrentAction.get())->aMoveCutOffs.push_front(ScMyMoveCutOff(nID, nStartPosition, nEndPosition));
    }
    else
    {
        OSL_FAIL("wrong action type");
    }
}

void ScXMLChangeTrackingImportHelper::SetMoveRanges(const ScBigRange& aSourceRange, const ScBigRange& aTargetRange)
{
    if (pCurrentAction->nActionType == SC_CAT_MOVE)
    {
         static_cast<ScMyMoveAction*>(pCurrentAction.get())->pMoveRanges.reset( new ScMyMoveRanges(aSourceRange, aTargetRange) );
    }
    else
    {
        OSL_FAIL("wrong action type");
    }
}

void ScXMLChangeTrackingImportHelper::GetMultiSpannedRange()
{
    if ((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
        (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS))
    {
        if (nMultiSpannedSlaveCount)
        {
            static_cast<ScMyDelAction*>(pCurrentAction.get())->nD = nMultiSpannedSlaveCount;
        }
        ++nMultiSpannedSlaveCount;
        if (nMultiSpannedSlaveCount >= nMultiSpanned)
        {
            nMultiSpanned = 0;
            nMultiSpannedSlaveCount = 0;
        }
    }
    else
    {
        OSL_FAIL("wrong action type");
    }
}

void ScXMLChangeTrackingImportHelper::AddGenerated(std::unique_ptr<ScMyCellInfo> pCellInfo, const ScBigRange& aBigRange)
{
    ScMyGenerated aGenerated { aBigRange, 0, std::move(pCellInfo) };
    if (pCurrentAction->nActionType == SC_CAT_MOVE)
    {
        static_cast<ScMyMoveAction*>(pCurrentAction.get())->aGeneratedList.push_back(std::move(aGenerated));
    }
    else if ((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
        (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS))
    {
        static_cast<ScMyDelAction*>(pCurrentAction.get())->aGeneratedList.push_back(std::move(aGenerated));
    }
    else
    {
        OSL_FAIL("try to insert a generated action to a wrong action");
    }
}

void ScXMLChangeTrackingImportHelper::EndChangeAction()
{
    if (!pCurrentAction)
    {
        OSL_FAIL("no current action");
        return;
    }

    if ((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
        (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS))
        GetMultiSpannedRange();

    if  (pCurrentAction->nActionNumber > 0)
        aActions.push_back(std::move(pCurrentAction));
    else
    {
        OSL_FAIL("no current action");
    }

    pCurrentAction = nullptr;
}

void ScXMLChangeTrackingImportHelper::ConvertInfo(const ScMyActionInfo& aInfo, OUString& rUser, DateTime& aDateTime)
{
    aDateTime = DateTime( aInfo.aDateTime);

    // old files didn't store nanoseconds, enable again
    if ( aInfo.aDateTime.NanoSeconds )
        pTrack->SetTimeNanoSeconds( true );

    const std::set<OUString>& rUsers = pTrack->GetUserCollection();
    std::set<OUString>::const_iterator it = rUsers.find(aInfo.sUser);
    if (it != rUsers.end())
    {
        // It's probably pointless to do this.
        rUser = *it;
    }
    else
        rUser = aInfo.sUser; // shouldn't happen
}

std::unique_ptr<ScChangeAction> ScXMLChangeTrackingImportHelper::CreateInsertAction(const ScMyInsAction* pAction)
{
    DateTime aDateTime( Date(0), tools::Time(0) );
    OUString aUser;
    ConvertInfo(pAction->aInfo, aUser, aDateTime);

    OUString sComment (pAction->aInfo.sComment);

    return std::make_unique<ScChangeActionIns>(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, pAction->nActionType);
}

std::unique_ptr<ScChangeAction> ScXMLChangeTrackingImportHelper::CreateDeleteAction(const ScMyDelAction* pAction)
{
    DateTime aDateTime( Date(0), tools::Time(0) );
    OUString aUser;
    ConvertInfo(pAction->aInfo, aUser, aDateTime);

    OUString sComment (pAction->aInfo.sComment);

    return std::make_unique<ScChangeActionDel>(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, pAction->nActionType, pAction->nD, pTrack);
}

std::unique_ptr<ScChangeAction> ScXMLChangeTrackingImportHelper::CreateMoveAction(const ScMyMoveAction* pAction)
{
    OSL_ENSURE(pAction->pMoveRanges, "no move ranges");
    if (pAction->pMoveRanges)
    {
        DateTime aDateTime( Date(0), tools::Time(0) );
        OUString aUser;
        ConvertInfo(pAction->aInfo, aUser, aDateTime);

        OUString sComment (pAction->aInfo.sComment);

        return std::make_unique<ScChangeActionMove>(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
            pAction->pMoveRanges->aTargetRange, aUser, aDateTime, sComment, pAction->pMoveRanges->aSourceRange , pTrack);
    }
    return nullptr;
}

std::unique_ptr<ScChangeAction> ScXMLChangeTrackingImportHelper::CreateRejectionAction(const ScMyRejAction* pAction)
{
    DateTime aDateTime( Date(0), tools::Time(0) );
    OUString aUser;
    ConvertInfo(pAction->aInfo, aUser, aDateTime);

    OUString sComment (pAction->aInfo.sComment);

    return std::make_unique<ScChangeActionReject>(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment);
}

std::unique_ptr<ScChangeAction> ScXMLChangeTrackingImportHelper::CreateContentAction(const ScMyContentAction* pAction)
{
    ScCellValue aCell;
    OUString sInputString;
    if (pAction->pCellInfo)
    {
        aCell = pAction->pCellInfo->CreateCell(pDoc);
        sInputString = pAction->pCellInfo->sInputString;
    }

    DateTime aDateTime( Date(0), tools::Time(0) );
    OUString aUser;
    ConvertInfo(pAction->aInfo, aUser, aDateTime);

    OUString sComment (pAction->aInfo.sComment);

    return std::make_unique<ScChangeActionContent>(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, aCell, pDoc, sInputString);
}

void ScXMLChangeTrackingImportHelper::CreateGeneratedActions(std::deque<ScMyGenerated>& rList)
{
    for (ScMyGenerated & rGenerated : rList)
    {
        if (rGenerated.nID == 0)
        {
            ScCellValue aCell;
            if (rGenerated.pCellInfo)
                aCell = rGenerated.pCellInfo->CreateCell(pDoc);

            if (!aCell.isEmpty())
            {
                rGenerated.nID = pTrack->AddLoadedGenerated(aCell, rGenerated.aBigRange, rGenerated.pCellInfo->sInputString);
                OSL_ENSURE(rGenerated.nID, "could not insert generated action");
            }
        }
    }
}

void ScXMLChangeTrackingImportHelper::SetDeletionDependencies(ScMyDelAction* pAction, ScChangeActionDel* pDelAct)
{
    if (!pAction->aGeneratedList.empty())
    {
        OSL_ENSURE(((pAction->nActionType == SC_CAT_DELETE_COLS) ||
            (pAction->nActionType == SC_CAT_DELETE_ROWS) ||
            (pAction->nActionType == SC_CAT_DELETE_TABS)), "wrong action type");
        if (pDelAct)
        {
            for (const ScMyGenerated & rGenerated : pAction->aGeneratedList)
            {
                OSL_ENSURE(rGenerated.nID, "a not inserted generated action");
                pDelAct->SetDeletedInThis(rGenerated.nID, pTrack);
            }
            pAction->aGeneratedList.clear();
        }
    }
    if (pAction->pInsCutOff)
    {
        OSL_ENSURE(((pAction->nActionType == SC_CAT_DELETE_COLS) ||
            (pAction->nActionType == SC_CAT_DELETE_ROWS) ||
            (pAction->nActionType == SC_CAT_DELETE_TABS)), "wrong action type");
        ScChangeAction* pChangeAction = pTrack->GetAction(pAction->pInsCutOff->nID);
        if (pChangeAction && pChangeAction->IsInsertType())
        {
            ScChangeActionIns* pInsAction = static_cast<ScChangeActionIns*>(pChangeAction);
            if (pDelAct)
                pDelAct->SetCutOffInsert(pInsAction, static_cast<sal_Int16>(pAction->pInsCutOff->nPosition));
        }
        else
        {
            OSL_FAIL("no cut off insert action");
        }
    }
    if (!pAction->aMoveCutOffs.empty())
    {
        OSL_ENSURE(((pAction->nActionType == SC_CAT_DELETE_COLS) ||
            (pAction->nActionType == SC_CAT_DELETE_ROWS) ||
            (pAction->nActionType == SC_CAT_DELETE_TABS)), "wrong action type");
        for (const ScMyMoveCutOff & rCutOff : pAction->aMoveCutOffs)
        {
            ScChangeAction* pChangeAction = pTrack->GetAction(rCutOff.nID);
            if (pChangeAction && (pChangeAction->GetType() == SC_CAT_MOVE))
            {
                ScChangeActionMove* pMoveAction = static_cast<ScChangeActionMove*>(pChangeAction);
                if (pDelAct)
                    pDelAct->AddCutOffMove(pMoveAction, static_cast<sal_Int16>(rCutOff.nStartPosition),
                                        static_cast<sal_Int16>(rCutOff.nEndPosition));
            }
            else
            {
                OSL_FAIL("no cut off move action");
            }
        }
        pAction->aMoveCutOffs.clear();
    }
}

void ScXMLChangeTrackingImportHelper::SetMovementDependencies(ScMyMoveAction* pAction, ScChangeActionMove* pMoveAct)
{
    if (!pAction->aGeneratedList.empty())
    {
        if (pAction->nActionType == SC_CAT_MOVE)
        {
            if (pMoveAct)
            {
                for (const ScMyGenerated & rGenerated : pAction->aGeneratedList)
                {
                    OSL_ENSURE(rGenerated.nID, "a not inserted generated action");
                    pMoveAct->SetDeletedInThis(rGenerated.nID, pTrack);
                }
                pAction->aGeneratedList.clear();
            }
        }
    }
}

void ScXMLChangeTrackingImportHelper::SetContentDependencies(const ScMyContentAction* pAction, ScChangeActionContent* pActContent)
{
    if (!pActContent || !pAction->nPreviousAction)
        return;

    OSL_ENSURE(pAction->nActionType == SC_CAT_CONTENT, "wrong action type");
    ScChangeAction* pPrevAct = pTrack->GetAction(pAction->nPreviousAction);
    if (!pPrevAct || pPrevAct->GetType() != SC_CAT_CONTENT)
        return;

    ScChangeActionContent* pPrevActContent = static_cast<ScChangeActionContent*>(pPrevAct);

    pActContent->SetPrevContent(pPrevActContent);
    pPrevActContent->SetNextContent(pActContent);
    const ScCellValue& rOldCell = pActContent->GetOldCell();
    if (rOldCell.isEmpty())
        return;

    pPrevActContent->SetNewCell(rOldCell, pDoc, EMPTY_OUSTRING);
}

void ScXMLChangeTrackingImportHelper::SetDependencies(ScMyBaseAction* pAction)
{
    ScChangeAction* pAct = pTrack->GetAction(pAction->nActionNumber);
    if (pAct)
    {
        if (!pAction->aDependencies.empty())
        {
            for (const auto & rID : pAction->aDependencies)
            {
                pAct->AddDependent(rID, pTrack);
            }
            pAction->aDependencies.clear();
        }
        if (!pAction->aDeletedList.empty())
        {
            for(const ScMyDeleted & rDeleted : pAction->aDeletedList)
            {
                pAct->SetDeletedInThis(rDeleted.nID, pTrack);
                ScChangeAction* pDeletedAct = pTrack->GetAction(rDeleted.nID);
                if ((pDeletedAct->GetType() == SC_CAT_CONTENT) && rDeleted.pCellInfo)
                {
                    ScChangeActionContent* pContentAct = static_cast<ScChangeActionContent*>(pDeletedAct);
                    if (rDeleted.pCellInfo)
                    {
                        const ScCellValue& rCell = rDeleted.pCellInfo->CreateCell(pDoc);
                        if (!rCell.equalsWithoutFormat(pContentAct->GetNewCell()))
                        {
                            // #i40704# Don't overwrite SetNewCell result by calling SetNewValue,
                            // instead pass the input string to SetNewCell.
                            pContentAct->SetNewCell(rCell, pDoc, rDeleted.pCellInfo->sInputString);
                        }
                    }
                }
            }
            pAction->aDeletedList.clear();
        }
        if ((pAction->nActionType == SC_CAT_DELETE_COLS) ||
            (pAction->nActionType == SC_CAT_DELETE_ROWS))
            SetDeletionDependencies(static_cast<ScMyDelAction*>(pAction), static_cast<ScChangeActionDel*>(pAct));
        else if (pAction->nActionType == SC_CAT_MOVE)
            SetMovementDependencies(static_cast<ScMyMoveAction*>(pAction), static_cast<ScChangeActionMove*>(pAct));
        else if (pAction->nActionType == SC_CAT_CONTENT)
            SetContentDependencies(static_cast<ScMyContentAction*>(pAction), static_cast<ScChangeActionContent*>(pAct));
    }
    else
    {
        OSL_FAIL("could not find the action");
    }
}

void ScXMLChangeTrackingImportHelper::SetNewCell(const ScMyContentAction* pAction)
{
    ScChangeAction* pChangeAction = pTrack->GetAction(pAction->nActionNumber);
    if (pChangeAction)
    {
        assert(dynamic_cast<ScChangeActionContent*>(pChangeAction));
        ScChangeActionContent* pChangeActionContent = static_cast<ScChangeActionContent*>(pChangeAction);
        if (pChangeActionContent->IsTopContent() && !pChangeActionContent->IsDeletedIn())
        {
            sal_Int32 nCol, nRow, nTab, nCol2, nRow2, nTab2;
            pAction->aBigRange.GetVars(nCol, nRow, nTab, nCol2, nRow2, nTab2);
            if ((nCol >= 0) && (nCol <= MAXCOL) &&
                (nRow >= 0) && (nRow <= MAXROW) &&
                (nTab >= 0) && (nTab <= MAXTAB))
            {
                ScAddress aAddress (static_cast<SCCOL>(nCol),
                                    static_cast<SCROW>(nRow),
                                    static_cast<SCTAB>(nTab));
                ScCellValue aCell;
                aCell.assign(*pDoc, aAddress);
                if (!aCell.isEmpty())
                {
                    ScCellValue aNewCell;
                    if (aCell.meType != CELLTYPE_FORMULA)
                    {
                        aNewCell = aCell;
                        pChangeActionContent->SetNewCell(aNewCell, pDoc, EMPTY_OUSTRING);
                        pChangeActionContent->SetNewValue(aCell, pDoc);
                    }
                    else
                    {
                        ScMatrixMode nMatrixFlag = aCell.mpFormula->GetMatrixFlag();
                        OUString sFormula;
                        // With GRAM_ODFF reference detection is faster on compilation.
                        /* FIXME: new cell should be created with a clone
                         * of the token array instead. Any reason why this
                         * wasn't done? */
                        aCell.mpFormula->GetFormula(sFormula, formula::FormulaGrammar::GRAM_ODFF);

                        // #i87826# [Collaboration] Rejected move destroys formulas
                        // FIXME: adjust ScFormulaCell::GetFormula(), so that the right formula string
                        //        is returned and no further string handling is necessary
                        OUString sFormula2;
                        if ( nMatrixFlag != ScMatrixMode::NONE )
                        {
                            sFormula2 = sFormula.copy( 2, sFormula.getLength() - 3 );
                        }
                        else
                        {
                            sFormula2 = sFormula.copy( 1 );
                        }

                        aNewCell.meType = CELLTYPE_FORMULA;
                        aNewCell.mpFormula = new ScFormulaCell(pDoc, aAddress, sFormula2,formula::FormulaGrammar::GRAM_ODFF, nMatrixFlag);
                        if (nMatrixFlag == ScMatrixMode::Formula)
                        {
                            SCCOL nCols;
                            SCROW nRows;
                            aCell.mpFormula->GetMatColsRows(nCols, nRows);
                            aNewCell.mpFormula->SetMatColsRows(nCols, nRows);
                        }
                        aNewCell.mpFormula->SetInChangeTrack(true);
                        pChangeActionContent->SetNewCell(aNewCell, pDoc, EMPTY_OUSTRING);
                        // #i40704# don't overwrite the formula string via SetNewValue()
                    }
                }
            }
            else
            {
                OSL_FAIL("wrong cell position");
            }
        }
    }
}

void ScXMLChangeTrackingImportHelper::CreateChangeTrack(ScDocument* pTempDoc)
{
    pDoc = pTempDoc;
    if (pDoc)
    {
        pTrack = new ScChangeTrack(pDoc, aUsers);
        // old files didn't store nanoseconds, disable until encountered
        pTrack->SetTimeNanoSeconds( false );

        for (const auto & rAction : aActions)
        {
            std::unique_ptr<ScChangeAction> pAction;

            switch (rAction->nActionType)
            {
                case SC_CAT_INSERT_COLS:
                case SC_CAT_INSERT_ROWS:
                case SC_CAT_INSERT_TABS:
                {
                    pAction = CreateInsertAction(static_cast<ScMyInsAction*>(rAction.get()));
                }
                break;
                case SC_CAT_DELETE_COLS:
                case SC_CAT_DELETE_ROWS:
                case SC_CAT_DELETE_TABS:
                {
                    ScMyDelAction* pDelAct = static_cast<ScMyDelAction*>(rAction.get());
                    pAction = CreateDeleteAction(pDelAct);
                    CreateGeneratedActions(pDelAct->aGeneratedList);
                }
                break;
                case SC_CAT_MOVE:
                {
                    ScMyMoveAction* pMovAct = static_cast<ScMyMoveAction*>(rAction.get());
                    pAction = CreateMoveAction(pMovAct);
                    CreateGeneratedActions(pMovAct->aGeneratedList);
                }
                break;
                case SC_CAT_CONTENT:
                {
                    pAction = CreateContentAction(static_cast<ScMyContentAction*>(rAction.get()));
                }
                break;
                case SC_CAT_REJECT:
                {
                    pAction = CreateRejectionAction(static_cast<ScMyRejAction*>(rAction.get()));
                }
                break;
                default:
                {
                    // added to avoid warnings
                }
            }

            if (pAction)
                pTrack->AppendLoaded(std::move(pAction));
            else
            {
                OSL_FAIL("no action");
            }
        }
        if (pTrack->GetLast())
            pTrack->SetActionMax(pTrack->GetLast()->GetActionNumber());

        auto aItr = aActions.begin();
        while (aItr != aActions.end())
        {
            SetDependencies(aItr->get());

            if ((*aItr)->nActionType == SC_CAT_CONTENT)
                ++aItr;
            else
                aItr = aActions.erase(aItr);
        }

        for (const auto& rxAction : aActions)
        {
            OSL_ENSURE(rxAction->nActionType == SC_CAT_CONTENT, "wrong action type");
            SetNewCell(static_cast<ScMyContentAction*>(rxAction.get()));
        }
        aActions.clear();
        if (aProtect.getLength())
            pTrack->SetProtection(aProtect);
        else if (pDoc->GetChangeTrack() && pDoc->GetChangeTrack()->IsProtected())
            pTrack->SetProtection(pDoc->GetChangeTrack()->GetProtection());

        if ( pTrack->GetLast() )
            pTrack->SetLastSavedActionNumber(pTrack->GetLast()->GetActionNumber());

        pDoc->SetChangeTrack(std::unique_ptr<ScChangeTrack>(pTrack));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
