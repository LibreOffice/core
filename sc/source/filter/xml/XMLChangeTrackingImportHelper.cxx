/*************************************************************************
 *
 *  $RCSfile: XMLChangeTrackingImportHelper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-22 17:56:54 $
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

#ifndef _SC_XMLCHANGETRACKINGIMPORTHELPER_HXX
#include "XMLChangeTrackingImportHelper.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_CHGVISET_HXX
#include "chgviset.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#define SC_CHANGE_ID_PREFIX "ct"

ScMyCellInfo::ScMyCellInfo()
    : pCell(NULL),
    sFormulaAddress(),
    sFormula(),
    sResult(),
    fValue(0.0),
    nType(NUMBERFORMAT_ALL),
    nMatrixFlag(MM_NONE),
    nMatrixCols(0),
    nMatrixRows(0)
{
}

ScMyCellInfo::ScMyCellInfo(ScBaseCell* pTempCell, const rtl::OUString& rFormulaAddress, const rtl::OUString& rFormula,
            const double& rValue, const sal_uInt16 nTempType, const sal_uInt8 nTempMatrixFlag, const sal_Int32 nTempMatrixCols,
            const sal_Int32 nTempMatrixRows)
    : pCell(pTempCell),
    sFormulaAddress(rFormulaAddress),
    sFormula(rFormula),
    fValue(rValue),
    nType(nTempType),
    nMatrixFlag(nTempMatrixFlag),
    nMatrixCols(nTempMatrixCols),
    nMatrixRows(nTempMatrixRows)
{
}

ScMyCellInfo::~ScMyCellInfo()
{
    if (pCell)
        pCell->Delete();
}

ScBaseCell* ScMyCellInfo::CreateCell(ScDocument* pDoc)
{
    if (!pCell && sFormula.getLength() && sFormulaAddress.getLength())
    {
        ScAddress aPos;
        sal_Int32 nOffset(0);
        ScXMLConverter::GetAddressFromString(aPos, sFormulaAddress, pDoc, nOffset);
        pCell = new ScFormulaCell(pDoc, aPos, sFormula, nMatrixFlag);
        static_cast<ScFormulaCell*>(pCell)->SetMatColsRows(static_cast<sal_uInt16>(nMatrixCols), static_cast<sal_uInt16>(nMatrixRows));
    }

    if (nType != NUMBERFORMAT_ALL)
    {
        if (nType == NUMBERFORMAT_DATE)
            pDoc->GetFormatTable()->GetInputLineString(fValue, NF_DATE_SYS_DDMMYYYY, sResult);
        else if (nType == NUMBERFORMAT_TIME)
            pDoc->GetFormatTable()->GetInputLineString(fValue, NF_TIME_HHMMSS, sResult);
    }

    if (pCell)
        return pCell->Clone(pDoc);
    else
        return NULL;
}

ScMyDeleted::ScMyDeleted()
    : pCellInfo(NULL)
{
}

ScMyDeleted::~ScMyDeleted()
{
    if (pCellInfo)
        delete pCellInfo;
}

ScMyGenerated::ScMyGenerated(ScMyCellInfo* pTempCellInfo, const ScBigRange& aTempBigRange)
    : aBigRange(aTempBigRange),
    nID(0),
    pCellInfo(pTempCellInfo)
{
}

ScMyGenerated::~ScMyGenerated()
{
    if (pCellInfo)
        delete pCellInfo;
}

ScMyBaseAction::ScMyBaseAction(const ScChangeActionType nTempActionType)
    : aDependences(),
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

ScMyInsAction::ScMyInsAction(const ScChangeActionType nActionType)
    : ScMyBaseAction(nActionType)
{
}

ScMyInsAction::~ScMyInsAction()
{
}

ScMyDelAction::ScMyDelAction(const ScChangeActionType nActionType)
    : ScMyBaseAction(nActionType),
    pInsCutOff(NULL),
    aMoveCutOffs(),
    aGeneratedList(),
    nD(0)
{
}

ScMyDelAction::~ScMyDelAction()
{
    if (pInsCutOff)
        delete pInsCutOff;
}

ScMyMoveAction::ScMyMoveAction()
    : ScMyBaseAction(SC_CAT_MOVE),
    pMoveRanges(NULL),
    aGeneratedList()
{
}

ScMyMoveAction::~ScMyMoveAction()
{
    if (pMoveRanges)
        delete pMoveRanges;
}


ScMyContentAction::ScMyContentAction()
    : ScMyBaseAction(SC_CAT_CONTENT),
    pCellInfo(NULL)
{
}

ScMyContentAction::~ScMyContentAction()
{
    if (pCellInfo)
        delete pCellInfo;
}

ScMyRejAction::ScMyRejAction()
    : ScMyBaseAction(SC_CAT_REJECT)
{
}

ScMyRejAction::~ScMyRejAction()
{
}

ScXMLChangeTrackingImportHelper::ScXMLChangeTrackingImportHelper()
    : sIDPrefix(RTL_CONSTASCII_USTRINGPARAM(SC_CHANGE_ID_PREFIX)),
    aActions(),
    aUsers(),
    nMultiSpanned(0),
    nMultiSpannedSlaveCount(0),
    pCurrentAction(NULL),
    pDoc(NULL),
    pTrack(NULL),
    pViewSettings(NULL),
    bChangeTrack(sal_False)
{
    nPrefixLength = sIDPrefix.getLength();
}

ScXMLChangeTrackingImportHelper::~ScXMLChangeTrackingImportHelper()
{
}

void ScXMLChangeTrackingImportHelper::StartChangeAction(const ScChangeActionType nActionType)
{
    DBG_ASSERT(!pCurrentAction, "a not inserted action");
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
    }
}

sal_uInt32 ScXMLChangeTrackingImportHelper::GetIDFromString(const rtl::OUString& sID)
{
    sal_uInt32 nResult(0);
    sal_uInt32 nLength(sID.getLength());
    if (nLength)
    {
        if (sID.compareTo(sIDPrefix, nPrefixLength) == 0)
        {
            rtl::OUString sValue(sID.copy(nPrefixLength, nLength - nPrefixLength));
            sal_Int32 nValue;
            SvXMLUnitConverter::convertNumber(nValue, sValue);
            DBG_ASSERT(nValue > 0, "wrong change action ID");
            nResult = nValue;
        }
        else
            DBG_ERROR("wrong change action ID");
    }
    return nResult;
}

void ScXMLChangeTrackingImportHelper::SetActionInfo(const ScMyActionInfo& aInfo)
{
    pCurrentAction->aInfo = aInfo;
    String aUser(aInfo.sUser);
    StrData* pStrData = new StrData( aUser );
    if ( !aUsers.Insert( pStrData ) )
        delete pStrData;
}

void ScXMLChangeTrackingImportHelper::SetPreviousChange(const sal_uInt32 nPreviousAction,
                            ScMyCellInfo* pCellInfo)
{
    DBG_ASSERT(pCurrentAction->nActionType == SC_CAT_CONTENT, "wrong action type");
    ScMyContentAction* pAction = static_cast<ScMyContentAction*>(pCurrentAction);
    pAction->nPreviousAction = nPreviousAction;
    pAction->pCellInfo = pCellInfo;
}

void ScXMLChangeTrackingImportHelper::SetPosition(const sal_Int32 nPosition, const sal_Int32 nCount, const sal_Int32 nTable)
{
    DBG_ASSERT(((pCurrentAction->nActionType != SC_CAT_MOVE) &&
                (pCurrentAction->nActionType != SC_CAT_CONTENT) &&
                (pCurrentAction->nActionType != SC_CAT_REJECT)), "wrong action type");
    DBG_ASSERT(nCount > 0, "wrong count");
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
    pDeleted->pCellInfo = pCellInfo;
    pCurrentAction->aDeletedList.push_front(pDeleted);
}

void ScXMLChangeTrackingImportHelper::SetMultiSpanned(const sal_Int16 nTempMultiSpanned)
{
    if (nTempMultiSpanned)
    {
        DBG_ASSERT(((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
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
        static_cast<ScMyDelAction*>(pCurrentAction)->pInsCutOff = new ScMyInsertionCutOff(nID, nPosition);
    }
    else
        DBG_ERROR("wrong action type");
}

void ScXMLChangeTrackingImportHelper::AddMoveCutOff(const sal_uInt32 nID, const sal_Int32 nStartPosition, const sal_Int32 nEndPosition)
{
    if ((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
        (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS))
    {
        static_cast<ScMyDelAction*>(pCurrentAction)->aMoveCutOffs.push_front(ScMyMoveCutOff(nID, nStartPosition, nEndPosition));
    }
    else
        DBG_ERROR("wrong action type");
}

void ScXMLChangeTrackingImportHelper::SetMoveRanges(const ScBigRange& aSourceRange, const ScBigRange& aTargetRange)
{
    if (pCurrentAction->nActionType == SC_CAT_MOVE)
    {
         static_cast<ScMyMoveAction*>(pCurrentAction)->pMoveRanges = new ScMyMoveRanges(aSourceRange, aTargetRange);
    }
    else
        DBG_ERROR("wrong action type");
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
        nMultiSpannedSlaveCount++;
        if (nMultiSpannedSlaveCount >= nMultiSpanned)
        {
            nMultiSpanned = 0;
            nMultiSpannedSlaveCount = 0;
        }
    }
    else
        DBG_ERROR("wrong action type");
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
        DBG_ERROR("try to insert a generated action to a wrong action");
}

ScChangeViewSettings* ScXMLChangeTrackingImportHelper::GetViewSettings()
{
    if (!pViewSettings)
        pViewSettings = new ScChangeViewSettings();
    return pViewSettings;
}

void ScXMLChangeTrackingImportHelper::EndChangeAction()
{
    if ((pCurrentAction->nActionType == SC_CAT_DELETE_COLS) ||
        (pCurrentAction->nActionType == SC_CAT_DELETE_ROWS))
        GetMultiSpannedRange();
    if (pCurrentAction && pCurrentAction->nActionNumber > 0)
        aActions.push_back(pCurrentAction);
    else
        DBG_ERROR("no current action");
    pCurrentAction = NULL;
}

void ScXMLChangeTrackingImportHelper::ConvertInfo(const ScMyActionInfo& aInfo, sal_uInt16& nUserPos, DateTime& aDateTime)
{
    Date aDate(aInfo.aDateTime.Day, aInfo.aDateTime.Month, aInfo.aDateTime.Year);
    Time aTime(aInfo.aDateTime.Hours, aInfo.aDateTime.Minutes, aInfo.aDateTime.Seconds, aInfo.aDateTime.HundredthSeconds);
    DateTime aTempDateTime (aDate, aTime);
    aDateTime = aTempDateTime;

    String aTempUser(aInfo.sUser);
    StrData* pStrData = new StrData( aTempUser );
    pTrack->GetUserCollection().Search(pStrData, nUserPos);
    if (pStrData)
        delete pStrData;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateInsertAction(ScMyInsAction* pAction)
{
    DateTime aDateTime;
    sal_uInt16 nPos;
    ConvertInfo(pAction->aInfo, nPos, aDateTime);

    StrData* pUser = (StrData*) aUsers.At( nPos );
    String aUser;
    if ( pUser )
        aUser = pUser->GetString();

    String sComment (pAction->aInfo.sComment);

    ScChangeAction* pNewAction = new ScChangeActionIns(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, pAction->nActionType);
    return pNewAction;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateDeleteAction(ScMyDelAction* pAction)
{
    DateTime aDateTime;
    sal_uInt16 nPos;
    ConvertInfo(pAction->aInfo, nPos, aDateTime);

    StrData* pUser = (StrData*) aUsers.At( nPos );
    String aUser;
    if ( pUser )
        aUser = pUser->GetString();

    String sComment (pAction->aInfo.sComment);

    ScChangeAction* pNewAction = new ScChangeActionDel(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, pAction->nActionType, pAction->nD, pTrack);
    return pNewAction;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateMoveAction(ScMyMoveAction* pAction)
{
    DBG_ASSERT(pAction->pMoveRanges, "no move ranges");
    if (pAction->pMoveRanges)
    {
        DateTime aDateTime;
        sal_uInt16 nPos;
        ConvertInfo(pAction->aInfo, nPos, aDateTime);

        StrData* pUser = (StrData*) aUsers.At( nPos );
        String aUser;
        if ( pUser )
            aUser = pUser->GetString();

        String sComment (pAction->aInfo.sComment);


        ScChangeAction* pNewAction = new ScChangeActionMove(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
            pAction->pMoveRanges->aTargetRange, aUser, aDateTime, sComment, pAction->pMoveRanges->aSourceRange , pTrack);
        return pNewAction;
    }
    return NULL;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateRejectionAction(ScMyRejAction* pAction)
{
    DateTime aDateTime;
    sal_uInt16 nPos;
    ConvertInfo(pAction->aInfo, nPos, aDateTime);

    StrData* pUser = (StrData*) aUsers.At( nPos );
    String aUser;
    if ( pUser )
        aUser = pUser->GetString();

    String sComment (pAction->aInfo.sComment);

    ScChangeAction* pNewAction = new ScChangeActionReject(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment);
    return pNewAction;
}

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateContentAction(ScMyContentAction* pAction)
{
    ScBaseCell* pCell = NULL;
    if (pAction->pCellInfo)
         pCell = pAction->pCellInfo->CreateCell(pDoc);

    DateTime aDateTime;
    sal_uInt16 nPos(0);
    ConvertInfo(pAction->aInfo, nPos, aDateTime);

    StrData* pUser = (StrData*) aUsers.At( nPos );
    String aUser;
    if ( pUser )
        aUser = pUser->GetString();

    String sComment (pAction->aInfo.sComment);

    ScChangeAction* pNewAction = new ScChangeActionContent(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, pCell, pDoc, pAction->pCellInfo->sResult);
    return pNewAction;
}

void ScXMLChangeTrackingImportHelper::CreateGeneratedActions(ScMyGeneratedList& rList)
{
    if (!rList.empty())
    {
        ScMyGeneratedList::iterator aItr = rList.begin();
        while (aItr != rList.end())
        {
            if (((*aItr)->nID == 0))
            {
                ScBaseCell* pCell = NULL;
                if ((*aItr)->pCellInfo)
                    pCell = (*aItr)->pCellInfo->CreateCell(pDoc);

                if (pCell)
                {
                    (*aItr)->nID = pTrack->AddLoadedGenerated(pCell, (*aItr)->aBigRange );
                    DBG_ASSERT((*aItr)->nID, "could not insert generated action");
                }
            }
            aItr++;
        }
    }
}

void ScXMLChangeTrackingImportHelper::SetDeletionDependences(ScMyDelAction* pAction, ScChangeActionDel* pDelAct)
{
    if (!pAction->aGeneratedList.empty())
    {
        DBG_ASSERT(((pAction->nActionType == SC_CAT_DELETE_COLS) ||
            (pAction->nActionType == SC_CAT_DELETE_ROWS) ||
            (pAction->nActionType == SC_CAT_DELETE_TABS)), "wrong action type");
        if (pDelAct)
        {
            ScMyGeneratedList::iterator aItr = pAction->aGeneratedList.begin();
            while (aItr != pAction->aGeneratedList.end())
            {
                DBG_ASSERT((*aItr)->nID, "a not inserted generated action");
                pDelAct->SetDeletedInThis((*aItr)->nID, pTrack);
                if (*aItr)
                    delete *aItr;
                aItr = pAction->aGeneratedList.erase(aItr);
            }
        }
    }
    if (pAction->pInsCutOff)
    {
        DBG_ASSERT(((pAction->nActionType == SC_CAT_DELETE_COLS) ||
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
            DBG_ERROR("no cut off insert action");
    }
    if (!pAction->aMoveCutOffs.empty())
    {
        DBG_ASSERT(((pAction->nActionType == SC_CAT_DELETE_COLS) ||
            (pAction->nActionType == SC_CAT_DELETE_ROWS) ||
            (pAction->nActionType == SC_CAT_DELETE_TABS)), "wrong action type");
        ScMyMoveCutOffs::iterator aItr = pAction->aMoveCutOffs.begin();
        while(aItr != pAction->aMoveCutOffs.end())
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
                DBG_ERROR("no cut off move action");
        }
    }
}

void ScXMLChangeTrackingImportHelper::SetMovementDependences(ScMyMoveAction* pAction, ScChangeActionMove* pMoveAct)
{
    if (!pAction->aGeneratedList.empty())
    {
        if (pAction->nActionType == SC_CAT_MOVE)
        {
            if (pMoveAct)
            {
                ScMyGeneratedList::iterator aItr = pAction->aGeneratedList.begin();
                while (aItr != pAction->aGeneratedList.end())
                {
                    DBG_ASSERT((*aItr)->nID, "a not inserted generated action");
                    pMoveAct->SetDeletedInThis((*aItr)->nID, pTrack);
                    if (*aItr)
                        delete *aItr;
                    aItr = pAction->aGeneratedList.erase(aItr);
                }
            }
        }
    }
}

void ScXMLChangeTrackingImportHelper::SetContentDependences(ScMyContentAction* pAction, ScChangeActionContent* pActContent)
{
    if (pAction->nPreviousAction)
    {
        DBG_ASSERT(pAction->nActionType == SC_CAT_CONTENT, "wrong action type");
        ScChangeAction* pPrevAct = pTrack->GetAction(pAction->nPreviousAction);
        if (pPrevAct)
        {
            ScChangeActionContent* pPrevActContent = static_cast<ScChangeActionContent*>(pPrevAct);
            if (pPrevActContent && pActContent)
            {
                pActContent->SetPrevContent(pPrevActContent);
                pPrevActContent->SetNextContent(pActContent);
                const ScBaseCell* pOldCell = pActContent->GetOldCell();
                if (pOldCell)
                {
                    ScBaseCell* pNewCell = pOldCell->Clone(pDoc);
                    if (pNewCell)
                        pPrevActContent->SetNewCell(pNewCell, pDoc);
                }
            }
        }
    }
}

void ScXMLChangeTrackingImportHelper::SetDependences(ScMyBaseAction* pAction)
{
    ScChangeAction* pAct = pTrack->GetAction(pAction->nActionNumber);
    if (pAct)
    {
        if (!pAction->aDependences.empty())
        {
            ScMyDependences::iterator aItr = pAction->aDependences.begin();
            while(aItr != pAction->aDependences.end())
            {
                pAct->AddDependent(*aItr, pTrack);
                aItr = pAction->aDependences.erase(aItr);
            }
        }
        if (!pAction->aDeletedList.empty())
        {
            ScMyDeletedList::iterator aItr = pAction->aDeletedList.begin();
            while(aItr != pAction->aDeletedList.end())
            {
                pAct->SetDeletedInThis((*aItr)->nID, pTrack);
                ScChangeAction* pDeletedAct = pTrack->GetAction((*aItr)->nID);
                if ((pDeletedAct->GetType() == SC_CAT_CONTENT) && (*aItr)->pCellInfo)
                {
                    ScChangeActionContent* pContentAct = static_cast<ScChangeActionContent*>(pDeletedAct);
                    if (pContentAct && (*aItr)->pCellInfo)
                    {
                        ScBaseCell* pCell = (*aItr)->pCellInfo->CreateCell(pDoc);
                        pContentAct->SetNewCell(pCell, pDoc);
                    }
                }
                if (*aItr)
                    delete *aItr;
                aItr = pAction->aDeletedList.erase(aItr);
            }
        }
        if ((pAction->nActionType == SC_CAT_DELETE_COLS) ||
            (pAction->nActionType == SC_CAT_DELETE_ROWS))
            SetDeletionDependences(static_cast<ScMyDelAction*>(pAction), static_cast<ScChangeActionDel*>(pAct));
        else if (pAction->nActionType == SC_CAT_MOVE)
            SetMovementDependences(static_cast<ScMyMoveAction*>(pAction), static_cast<ScChangeActionMove*>(pAct));
        else if (pAction->nActionType == SC_CAT_CONTENT)
            SetContentDependences(static_cast<ScMyContentAction*>(pAction), static_cast<ScChangeActionContent*>(pAct));
    }
    else
        DBG_ERROR("could not find the action");
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
                    ScAddress aAddress (static_cast<sal_uInt16>(nCol),
                                        static_cast<sal_uInt16>(nRow),
                                        static_cast<sal_uInt16>(nTab));
                    ScBaseCell* pCell = pDoc->GetCell(aAddress);
                    if (pCell)
                    {
                        ScBaseCell* pNewCell = NULL;
                        if (pCell->GetCellType() != CELLTYPE_FORMULA)
                            pNewCell = pCell->Clone(pDoc);
                        else
                        {
                            sal_uInt8 nMatrixFlag = static_cast<ScFormulaCell*>(pCell)->GetMatrixFlag();
                            String sFormula;
                            static_cast<ScFormulaCell*>(pCell)->GetFormula(sFormula);
                            rtl::OUString sOUFormula(sFormula);
                            rtl::OUString sOUFormula2(sOUFormula.copy(2, sOUFormula.getLength() - 3));
                            String sFormula2(sOUFormula2);
                            pNewCell = new ScFormulaCell(pDoc, aAddress, sFormula2, nMatrixFlag);
                            if (pNewCell)
                            {
                                if (nMatrixFlag == MM_FORMULA)
                                {
                                    sal_uInt16 nCols, nRows;
                                    static_cast<ScFormulaCell*>(pCell)->GetMatColsRows(nCols, nRows);
                                    static_cast<ScFormulaCell*>(pNewCell)->SetMatColsRows(nCols, nRows);
                                }
                                static_cast<ScFormulaCell*>(pNewCell)->SetInChangeTrack(sal_True);
                            }
                        }
                        pChangeActionContent->SetNewCell(pNewCell, pDoc);
                    }
                }
                else
                    DBG_ERROR("wrong cell position");
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

        ScMyActions::iterator aItr = aActions.begin();
        while (aItr != aActions.end())
        {
            ScChangeAction* pAction = NULL;

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
            }

            if (pAction)
                pTrack->AppendLoaded(pAction);
            else
                DBG_ERROR("no action");

            aItr++;
        }
        if (pTrack->GetLast())
            pTrack->SetActionMax(pTrack->GetLast()->GetActionNumber());

        aItr = aActions.begin();
        while (aItr != aActions.end())
        {
            SetDependences(*aItr);

            if ((*aItr)->nActionType == SC_CAT_CONTENT)
                aItr++;
            else
            {
                if (*aItr)
                    delete (*aItr);
                aItr = aActions.erase(aItr);
            }
        }

        aItr = aActions.begin();
        while (aItr != aActions.end())
        {
            DBG_ASSERT((*aItr)->nActionType == SC_CAT_CONTENT, "wrong action type");
            SetNewCell(static_cast<ScMyContentAction*>(*aItr));
            if (*aItr)
                delete (*aItr);
            aItr = aActions.erase(aItr);
        }

        pDoc->SetChangeTrack(pTrack);
        if (!pViewSettings)
        {
            pViewSettings = new ScChangeViewSettings();
            pViewSettings->SetShowChanges(sal_True);
        }
        if (sRangeList.getLength())
        {
            ScRangeList aRangeList;
            ScXMLConverter::GetRangeListFromString(aRangeList, sRangeList, pDoc);
            pViewSettings->SetTheRangeList(aRangeList);
        }
        pDoc->SetChangeViewSettings(*pViewSettings);
    }
}
