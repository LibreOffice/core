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
#include "XMLConverter.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "chgviset.hxx"
#include "rangeutl.hxx"
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
            nFormat = pDoc->GetFormatTable()->GetStandardFormat( css::util::NumberFormat::DATE, ScGlobal::eLnge );
        else if (nType == css::util::NumberFormat::TIME)
            nFormat = pDoc->GetFormatTable()->GetStandardFormat( css::util::NumberFormat::TIME, ScGlobal::eLnge );
        pDoc->GetFormatTable()->GetInputLineString(fValue, nFormat, sInputString);
    }

    return maCell;
}

ScMyDeleted::ScMyDeleted()
    : nID(0)
    , pCellInfo(nullptr)
{
}

ScMyDeleted::~ScMyDeleted()
{
}

ScMyGenerated::ScMyGenerated(ScMyCellInfo* pTempCellInfo, const ScBigRange& aTempBigRange)
    : aBigRange(aTempBigRange)
    , nID(0)
    , pCellInfo(pTempCellInfo)
{
}

ScMyGenerated::~ScMyGenerated()
{
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
    pInsCutOff(nullptr),
    aMoveCutOffs(),
    nD(0)
{
}

ScMyDelAction::~ScMyDelAction()
{
}

ScMyMoveAction::ScMyMoveAction()
    : ScMyBaseAction(SC_CAT_MOVE),
    aGeneratedList(),
    pMoveRanges(nullptr)
{
}

ScMyMoveAction::~ScMyMoveAction()
{
}

ScMyContentAction::ScMyContentAction()
    : ScMyBaseAction(SC_CAT_CONTENT),
    pCellInfo(nullptr)
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
    pCurrentAction(nullptr),
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
            pCurrentAction = new ScMyInsAction(nActionType);
        }
        break;
        case SC_CAT_DELETE_COLS:
        case SC_CAT_DELETE_ROWS:
        case SC_CAT_DELETE_TABS:
        {
            pCurrentAction = new ScMyDelAction(nActionType);
        }
        break;
        case SC_CAT_MOVE:
        {
            pCurrentAction = new ScMyMoveAction();
        }
        break;
        case SC_CAT_CONTENT:
        {
            pCurrentAction = new ScMyContentAction();
        }
        break;
        case SC_CAT_REJECT:
        {
            pCurrentAction = new ScMyRejAction();
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
    ScMyContentAction* pAction = static_cast<ScMyContentAction*>(pCurrentAction);
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
    ScMyDeleted* pDeleted = new ScMyDeleted();
    pDeleted->nID = nID;
    pCurrentAction->aDeletedList.push_front(pDeleted);
}

void ScXMLChangeTrackingImportHelper::AddDeleted(const sal_uInt32 nID, ScMyCellInfo* pCellInfo)
{
    ScMyDeleted* pDeleted = new ScMyDeleted();
    pDeleted->nID = nID;
    pDeleted->pCellInfo.reset(pCellInfo);
    pCurrentAction->aDeletedList.push_front(pDeleted);
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
        static_cast<ScMyDelAction*>(pCurrentAction)->pInsCutOff.reset( new ScMyInsertionCutOff(nID, nPosition) );
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
        static_cast<ScMyDelAction*>(pCurrentAction)->aMoveCutOffs.push_front(ScMyMoveCutOff(nID, nStartPosition, nEndPosition));
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
         static_cast<ScMyMoveAction*>(pCurrentAction)->pMoveRanges.reset( new ScMyMoveRanges(aSourceRange, aTargetRange) );
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
            static_cast<ScMyDelAction*>(pCurrentAction)->nD = nMultiSpannedSlaveCount;
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

void ScXMLChangeTrackingImportHelper::AddGenerated(ScMyCellInfo* pCellInfo, const ScBigRange& aBigRange)
{
    ScMyGenerated* pGenerated = new ScMyGenerated(pCellInfo, aBigRange);
    if (pCurrentAction->nActionType == SC_CAT_MOVE)
    {
        static_cast<ScMyMoveAction*>(pCurrentAction)->aGeneratedList.push_back(pGenerated);
    }
    else if ((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
        (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS))
    {
        static_cast<ScMyDelAction*>(pCurrentAction)->aGeneratedList.push_back(pGenerated);
    }
    else
    {
        delete pGenerated;
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
        aActions.push_back(pCurrentAction);
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

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateInsertAction(ScMyInsAction* pAction)
{
    DateTime aDateTime( Date(0), tools::Time(0) );
    OUString aUser;
    ConvertInfo(pAction->aInfo, aUser, aDateTime);

    OUString sComment (pAction->aInfo.sComment);

    ScChangeAction* pNewAction = new ScChangeActionIns(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, pAction->nActionType);
    return pNewAction;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateDeleteAction(ScMyDelAction* pAction)
{
    DateTime aDateTime( Date(0), tools::Time(0) );
    OUString aUser;
    ConvertInfo(pAction->aInfo, aUser, aDateTime);

    OUString sComment (pAction->aInfo.sComment);

    ScChangeAction* pNewAction = new ScChangeActionDel(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, pAction->nActionType, pAction->nD, pTrack);
    return pNewAction;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateMoveAction(ScMyMoveAction* pAction)
{
    OSL_ENSURE(pAction->pMoveRanges, "no move ranges");
    if (pAction->pMoveRanges)
    {
        DateTime aDateTime( Date(0), tools::Time(0) );
        OUString aUser;
        ConvertInfo(pAction->aInfo, aUser, aDateTime);

        OUString sComment (pAction->aInfo.sComment);

        ScChangeAction* pNewAction = new ScChangeActionMove(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
            pAction->pMoveRanges->aTargetRange, aUser, aDateTime, sComment, pAction->pMoveRanges->aSourceRange , pTrack);
        return pNewAction;
    }
    return nullptr;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateRejectionAction(ScMyRejAction* pAction)
{
    DateTime aDateTime( Date(0), tools::Time(0) );
    OUString aUser;
    ConvertInfo(pAction->aInfo, aUser, aDateTime);

    OUString sComment (pAction->aInfo.sComment);

    ScChangeAction* pNewAction = new ScChangeActionReject(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment);
    return pNewAction;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateContentAction(ScMyContentAction* pAction)
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

    ScChangeAction* pNewAction = new ScChangeActionContent(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, aCell, pDoc, sInputString);
    return pNewAction;
}

void ScXMLChangeTrackingImportHelper::CreateGeneratedActions(ScMyGeneratedList& rList)
{
    if (!rList.empty())
    {
        ScMyGeneratedList::iterator aItr(rList.begin());
        ScMyGeneratedList::iterator aEndItr(rList.end());
        while (aItr != aEndItr)
        {
            if ((*aItr)->nID == 0)
            {
                ScCellValue aCell;
                if ((*aItr)->pCellInfo)
                    aCell = (*aItr)->pCellInfo->CreateCell(pDoc);

                if (!aCell.isEmpty())
                {
                    (*aItr)->nID = pTrack->AddLoadedGenerated(aCell, (*aItr)->aBigRange, (*aItr)->pCellInfo->sInputString);
                    OSL_ENSURE((*aItr)->nID, "could not insert generated action");
                }
            }
            ++aItr;
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
            ScMyGeneratedList::iterator aItr(pAction->aGeneratedList.begin());
            ScMyGeneratedList::iterator aEndItr(pAction->aGeneratedList.end());
            while (aItr != aEndItr)
            {
                OSL_ENSURE((*aItr)->nID, "a not inserted generated action");
                pDelAct->SetDeletedInThis((*aItr)->nID, pTrack);
                if (*aItr)
                    delete *aItr;
                aItr = pAction->aGeneratedList.erase(aItr);
            }
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
            if (pInsAction && pDelAct)
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
        ScMyMoveCutOffs::iterator aItr(pAction->aMoveCutOffs.begin());
        ScMyMoveCutOffs::iterator aEndItr(pAction->aMoveCutOffs.end());
        while(aItr != aEndItr)
        {
            ScChangeAction* pChangeAction = pTrack->GetAction(aItr->nID);
            if (pChangeAction && (pChangeAction->GetType() == SC_CAT_MOVE))
            {
                ScChangeActionMove* pMoveAction = static_cast<ScChangeActionMove*>(pChangeAction);
                if (pMoveAction && pDelAct)
                    pDelAct->AddCutOffMove(pMoveAction, static_cast<sal_Int16>(aItr->nStartPosition),
                                        static_cast<sal_Int16>(aItr->nEndPosition));
            }
            else
            {
                OSL_FAIL("no cut off move action");
            }
            aItr = pAction->aMoveCutOffs.erase(aItr);
        }
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
                ScMyGeneratedList::iterator aItr(pAction->aGeneratedList.begin());
                ScMyGeneratedList::iterator aEndItr(pAction->aGeneratedList.end());
                while (aItr != aEndItr)
                {
                    OSL_ENSURE((*aItr)->nID, "a not inserted generated action");
                    pMoveAct->SetDeletedInThis((*aItr)->nID, pTrack);
                    if (*aItr)
                        delete *aItr;
                    aItr = pAction->aGeneratedList.erase(aItr);
                }
            }
        }
    }
}

void ScXMLChangeTrackingImportHelper::SetContentDependencies(ScMyContentAction* pAction, ScChangeActionContent* pActContent)
{
    if (!pAction->nPreviousAction)
        return;

    OSL_ENSURE(pAction->nActionType == SC_CAT_CONTENT, "wrong action type");
    ScChangeAction* pPrevAct = pTrack->GetAction(pAction->nPreviousAction);
    if (!pPrevAct)
        return;

    ScChangeActionContent* pPrevActContent = static_cast<ScChangeActionContent*>(pPrevAct);
    if (!pPrevActContent || !pActContent)
        return;

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
            ScMyDependencies::iterator aItr(pAction->aDependencies.begin());
            ScMyDependencies::iterator aEndItr(pAction->aDependencies.end());
            while(aItr != aEndItr)
            {
                pAct->AddDependent(*aItr, pTrack);
                aItr = pAction->aDependencies.erase(aItr);
            }
        }
        if (!pAction->aDeletedList.empty())
        {
            ScMyDeletedList::iterator aItr(pAction->aDeletedList.begin());
            ScMyDeletedList::iterator aEndItr(pAction->aDeletedList.end());
            while(aItr != aEndItr)
            {
                pAct->SetDeletedInThis((*aItr)->nID, pTrack);
                ScChangeAction* pDeletedAct = pTrack->GetAction((*aItr)->nID);
                if ((pDeletedAct->GetType() == SC_CAT_CONTENT) && (*aItr)->pCellInfo)
                {
                    ScChangeActionContent* pContentAct = static_cast<ScChangeActionContent*>(pDeletedAct);
                    if (pContentAct && (*aItr)->pCellInfo)
                    {
                        const ScCellValue& rCell = (*aItr)->pCellInfo->CreateCell(pDoc);
                        if (!rCell.equalsWithoutFormat(pContentAct->GetNewCell()))
                        {
                            // #i40704# Don't overwrite SetNewCell result by calling SetNewValue,
                            // instead pass the input string to SetNewCell.
                            pContentAct->SetNewCell(rCell, pDoc, (*aItr)->pCellInfo->sInputString);
                        }
                    }
                }
                if (*aItr)
                    delete *aItr;
                aItr = pAction->aDeletedList.erase(aItr);
            }
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

void ScXMLChangeTrackingImportHelper::SetNewCell(ScMyContentAction* pAction)
{
    ScChangeAction* pChangeAction = pTrack->GetAction(pAction->nActionNumber);
    if (pChangeAction)
    {
        ScChangeActionContent* pChangeActionContent = static_cast<ScChangeActionContent*>(pChangeAction);
        if (pChangeActionContent)
        {
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
                                sFormula2 = sFormula.copy( 1, sFormula.getLength() - 1 );
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
}

void ScXMLChangeTrackingImportHelper::CreateChangeTrack(ScDocument* pTempDoc)
{
    pDoc = pTempDoc;
    if (pDoc)
    {
        pTrack = new ScChangeTrack(pDoc, aUsers);
        // old files didn't store nanoseconds, disable until encountered
        pTrack->SetTimeNanoSeconds( false );

        ScMyActions::iterator aItr(aActions.begin());
        ScMyActions::iterator aEndItr(aActions.end());
        while (aItr != aEndItr)
        {
            ScChangeAction* pAction = nullptr;

            switch ((*aItr)->nActionType)
            {
                case SC_CAT_INSERT_COLS:
                case SC_CAT_INSERT_ROWS:
                case SC_CAT_INSERT_TABS:
                {
                    pAction = CreateInsertAction(static_cast<ScMyInsAction*>(*aItr));
                }
                break;
                case SC_CAT_DELETE_COLS:
                case SC_CAT_DELETE_ROWS:
                case SC_CAT_DELETE_TABS:
                {
                    ScMyDelAction* pDelAct = static_cast<ScMyDelAction*>(*aItr);
                    pAction = CreateDeleteAction(pDelAct);
                    CreateGeneratedActions(pDelAct->aGeneratedList);
                }
                break;
                case SC_CAT_MOVE:
                {
                    ScMyMoveAction* pMovAct = static_cast<ScMyMoveAction*>(*aItr);
                    pAction = CreateMoveAction(pMovAct);
                    CreateGeneratedActions(pMovAct->aGeneratedList);
                }
                break;
                case SC_CAT_CONTENT:
                {
                    pAction = CreateContentAction(static_cast<ScMyContentAction*>(*aItr));
                }
                break;
                case SC_CAT_REJECT:
                {
                    pAction = CreateRejectionAction(static_cast<ScMyRejAction*>(*aItr));
                }
                break;
                default:
                {
                    // added to avoid warnings
                }
            }

            if (pAction)
                pTrack->AppendLoaded(pAction);
            else
            {
                OSL_FAIL("no action");
            }

            ++aItr;
        }
        if (pTrack->GetLast())
            pTrack->SetActionMax(pTrack->GetLast()->GetActionNumber());

        aItr = aActions.begin();
        aEndItr = aActions.end();
        while (aItr != aEndItr)
        {
            SetDependencies(*aItr);

            if ((*aItr)->nActionType == SC_CAT_CONTENT)
                ++aItr;
            else
            {
                if (*aItr)
                    delete (*aItr);
                aItr = aActions.erase(aItr);
            }
        }

        aItr = aActions.begin();
        aEndItr = aActions.end();
        while (aItr != aEndItr)
        {
            OSL_ENSURE((*aItr)->nActionType == SC_CAT_CONTENT, "wrong action type");
            SetNewCell(static_cast<ScMyContentAction*>(*aItr));
            if (*aItr)
                delete (*aItr);
            aItr = aActions.erase(aItr);
        }
        if (aProtect.getLength())
            pTrack->SetProtection(aProtect);
        else if (pDoc->GetChangeTrack() && pDoc->GetChangeTrack()->IsProtected())
            pTrack->SetProtection(pDoc->GetChangeTrack()->GetProtection());

        if ( pTrack->GetLast() )
            pTrack->SetLastSavedActionNumber(pTrack->GetLast()->GetActionNumber());

        pDoc->SetChangeTrack(pTrack);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
