/*************************************************************************
 *
 *  $RCSfile: XMLChangeTrackingImportHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-01 10:15:05 $
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

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#define SC_CHANGE_ID_PREFIX "ct"

ScMyCellDeleted::ScMyCellDeleted()
    : pCell(NULL),
    sFormulaAddress()
{
}

ScMyCellDeleted::~ScMyCellDeleted()
{
    if (pCell)
        pCell->Delete();
}

ScMyDeleted::ScMyDeleted()
    : pCellDeleted(NULL)
{
}

ScMyDeleted::~ScMyDeleted()
{
    if (pCellDeleted)
        delete pCellDeleted;
}

ScMyGenerated::ScMyGenerated(ScBaseCell* pTempCell, const rtl::OUString& sTempFormulaAddress, const ScBigRange& aTempBigRange)
    :
    pCell(pTempCell),
    sFormulaAddress(sTempFormulaAddress),
    aBigRange(aTempBigRange),
    nID(0)
{
}

ScMyAction::ScMyAction()
    : sFormulaAddress(),
    pOldCell(NULL),
    pInsCutOff(NULL),
    pMoveRanges(NULL),
    aDependences(),
    aDeletedList(),
    aMoveCutOffs(),
    nActionNumber(0),
    nRejectingNumber(0),
    nPreviousAction(0),
    nD(0)
{
}

ScMyAction::~ScMyAction()
{
    if (pInsCutOff)
        delete pInsCutOff;
    // pOldCell have not to delete it is given to tracked changes
}

ScXMLChangeTrackingImportHelper::ScXMLChangeTrackingImportHelper()
    : sIDPrefix(RTL_CONSTASCII_USTRINGPARAM(SC_CHANGE_ID_PREFIX)),
    aActions(),
    aUsers(),
    pCurrentAction(NULL),
    pDoc(NULL),
    pTrack(NULL)
{
    nPrefixLength = sIDPrefix.getLength();
}

ScXMLChangeTrackingImportHelper::~ScXMLChangeTrackingImportHelper()
{
}

void ScXMLChangeTrackingImportHelper::StartChangeAction(const ScChangeActionType nActionType)
{
    DBG_ASSERT(!pCurrentAction, "a not inserted action");
    pCurrentAction = new ScMyAction();
    pCurrentAction->nActionType = nActionType;
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
    ScMyDeleted aDeleted;
    aDeleted.nID = nID;
    pCurrentAction->aDeletedList.push_front(aDeleted);
}

void ScXMLChangeTrackingImportHelper::AddDeleted(const sal_uInt32 nID, ScBaseCell* pCell, const rtl::OUString& sFormulaAddress)
{
    ScMyDeleted aDeleted;
    aDeleted.nID = nID;
    aDeleted.pCellDeleted = new ScMyCellDeleted();
    if (aDeleted.pCellDeleted)
    {
        aDeleted.pCellDeleted->pCell = pCell;
        aDeleted.pCellDeleted->sFormulaAddress = sFormulaAddress;
    }
    pCurrentAction->aDeletedList.push_front(aDeleted);
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
    pCurrentAction->pInsCutOff = new ScMyInsertionCutOff(nID, nPosition);
}

void ScXMLChangeTrackingImportHelper::AddMoveCutOff(const sal_uInt32 nID, const sal_Int32 nStartPosition, const sal_Int32 nEndPosition)
{
    pCurrentAction->aMoveCutOffs.push_front(ScMyMoveCutOff(nID, nStartPosition, nEndPosition));
}

void ScXMLChangeTrackingImportHelper::SetMoveRanges(const ScBigRange& aSourceRange, const ScBigRange& aTargetRange)
{
    pCurrentAction->pMoveRanges = new ScMyMoveRanges(aSourceRange, aTargetRange);
}

void ScXMLChangeTrackingImportHelper::GetMultiSpannedRange()
{
    if (nMultiSpannedSlaveCount)
    {
        pCurrentAction->nD = nMultiSpannedSlaveCount;
    }
    nMultiSpannedSlaveCount++;
    if (nMultiSpannedSlaveCount >= nMultiSpanned)
    {
        nMultiSpanned = 0;
        nMultiSpannedSlaveCount = 0;
    }
}

void ScXMLChangeTrackingImportHelper::AddGenerated(ScBaseCell* pCell, const rtl::OUString& sFormulaAddress, const ScBigRange& aBigRange)
{
    ScMyGenerated aGenerated(pCell, sFormulaAddress, aBigRange);
    pCurrentAction->aGeneratedList.push_back(aGenerated);
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

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateInsertAction(ScMyAction* pAction)
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

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateDeleteAction(ScMyAction* pAction)
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

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateMoveAction(ScMyAction* pAction)
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

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateRejectionAction(ScMyAction* pAction)
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

ScChangeAction* ScXMLChangeTrackingImportHelper::CreateContentAction(ScMyAction* pAction)
{
    if (pAction->pOldCell && (pAction->pOldCell->GetCellType() == CELLTYPE_FORMULA) && pAction->sFormulaAddress.getLength())
    {
        ScFormulaCell* pFormulaCell = static_cast<ScFormulaCell*>(pAction->pOldCell);
        sal_Int32 nOffset(0);
        ScXMLConverter::GetAddressFromString(pFormulaCell->aPos, pAction->sFormulaAddress, pDoc, nOffset);
    }

    DateTime aDateTime;
    sal_uInt16 nPos;
    ConvertInfo(pAction->aInfo, nPos, aDateTime);

    StrData* pUser = (StrData*) aUsers.At( nPos );
    String aUser;
    if ( pUser )
        aUser = pUser->GetString();

    String sComment (pAction->aInfo.sComment);

    ScChangeAction* pNewAction = new ScChangeActionContent(pAction->nActionNumber, pAction->nActionState, pAction->nRejectingNumber,
        pAction->aBigRange, aUser, aDateTime, sComment, pAction->pOldCell, pDoc);
    return pNewAction;
}

void ScXMLChangeTrackingImportHelper::CreateGeneratedActions(ScMyAction* pAction)
{
    if (!pAction->aGeneratedList.empty())
    {
        ScMyGeneratedList::iterator aItr = pAction->aGeneratedList.begin();
        while (aItr != pAction->aGeneratedList.end())
        {
            if ((aItr->nID == 0) && aItr->pCell)
            {
                if ((aItr->pCell->GetCellType() == CELLTYPE_FORMULA) &&
                    aItr->sFormulaAddress.getLength())
                {
                    ScFormulaCell* pFormulaCell = static_cast<ScFormulaCell*>(aItr->pCell);
                    sal_Int32 nOffset(0);
                    ScXMLConverter::GetAddressFromString(pFormulaCell->aPos, aItr->sFormulaAddress, pDoc, nOffset);
                }

                aItr->nID = pTrack->AddLoadedGenerated(aItr->pCell, aItr->aBigRange );
                DBG_ASSERT(aItr->nID, "could not insert generated action");
            }
            aItr++;
        }
    }
}

void ScXMLChangeTrackingImportHelper::SetDependences(ScMyAction* pAction)
{
    ScChangeAction* pAct = pTrack->GetAction(pAction->nActionNumber);
    if (pAct)
    {
        if (!pAction->aDependences.empty())
        {
            ScMyDependences::iterator aItr = pAction->aDependences.begin();
            while(aItr != pAction->aDependences.end())
            {
                pAct->LoadDependent(*aItr, pTrack);
                aItr = pAction->aDependences.erase(aItr);
            }
        }
        if (!pAction->aDeletedList.empty())
        {
            ScMyDeletedList::iterator aItr = pAction->aDeletedList.begin();
            while(aItr != pAction->aDeletedList.end())
            {
                pAct->LoadDeleted(aItr->nID, pTrack);
                if ((pAct->GetType() == SC_CAT_CONTENT) && aItr->pCellDeleted)
                {
                    ScChangeActionContent* pContentAct = static_cast<ScChangeActionContent*>(pAct);
                    if (pContentAct)
                    {
                        if (aItr->pCellDeleted->pCell &&
                            (aItr->pCellDeleted->pCell->GetCellType() == CELLTYPE_FORMULA) &&
                            aItr->pCellDeleted->sFormulaAddress.getLength())
                        {
                            ScFormulaCell* pFormulaCell = static_cast<ScFormulaCell*>(aItr->pCellDeleted->pCell);
                            sal_Int32 nOffset(0);
                            ScXMLConverter::GetAddressFromString(pFormulaCell->aPos, aItr->pCellDeleted->sFormulaAddress, pDoc, nOffset);
                        }
                        pContentAct->SetNewCell(aItr->pCellDeleted->pCell, pDoc);
                    }
                }
                aItr = pAction->aDeletedList.erase(aItr);
            }
        }
        if (!pAction->aGeneratedList.empty())
        {
            if (pAction->nActionType == SC_CAT_MOVE)
            {
                ScChangeActionMove* pMoveAct = static_cast<ScChangeActionMove*>(pAct);
                if (pMoveAct)
                {
                    ScMyGeneratedList::iterator aItr = pAction->aGeneratedList.begin();
                    while (aItr != pAction->aGeneratedList.end())
                    {
                        DBG_ASSERT(aItr->nID, "a not inserted generated action");
                        pMoveAct->LoadCellContent(aItr->nID, pTrack);
                        aItr = pAction->aGeneratedList.erase(aItr);
                    }
                }
            }
            else
            {
                DBG_ASSERT(((pAction->nActionType == SC_CAT_DELETE_COLS) ||
                    (pAction->nActionType == SC_CAT_DELETE_ROWS) ||
                    (pAction->nActionType == SC_CAT_DELETE_TABS)), "wrong action type");
                ScChangeActionDel* pDelAct = static_cast<ScChangeActionDel*>(pAct);
                if (pDelAct)
                {
                    ScMyGeneratedList::iterator aItr = pAction->aGeneratedList.begin();
                    while (aItr != pAction->aGeneratedList.end())
                    {
                        DBG_ASSERT(aItr->nID, "a not inserted generated action");
                        pDelAct->LoadCellContent(aItr->nID, pTrack);
                        aItr = pAction->aGeneratedList.erase(aItr);
                    }
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
                if (pInsAction)
                {
                    ScChangeActionDel* pDelAct = static_cast<ScChangeActionDel*>(pAct);
                    if (pDelAct)
                        pDelAct->SetCutOffInsert(pInsAction, static_cast<sal_Int16>(pAction->pInsCutOff->nPosition));
                }
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
                    if (pMoveAction)
                    {
                        ScChangeActionDel* pDelAct = static_cast<ScChangeActionDel*>(pAct);
                        if (pDelAct)
                            pDelAct->AddCutOffMove(pMoveAction, static_cast<sal_Int16>(aItr->nStartPosition),
                                                static_cast<sal_Int16>(aItr->nEndPosition));
                    }
                }
                else
                    DBG_ERROR("no cut off move action");
            }
        }
        if (pAction->nPreviousAction)
        {
            DBG_ASSERT(pAction->nActionType == SC_CAT_CONTENT, "wrong action type");
            ScChangeAction* pPrevAct = pTrack->GetAction(pAction->nPreviousAction);
            if (pPrevAct)
            {
                ScChangeActionContent* pPrevActContent = static_cast<ScChangeActionContent*>(pPrevAct);
                ScChangeActionContent* pActContent = static_cast<ScChangeActionContent*>(pAct);
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
    else
        DBG_ERROR("could not find the action");
}

void ScXMLChangeTrackingImportHelper::SetNewCell(ScMyAction* pAction)
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
                        ScBaseCell* pNewCell = pCell->Clone(pDoc);
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
    if (pDoc && !aActions.empty())
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
                    pAction = CreateInsertAction(*aItr);
                }
                break;
                case SC_CAT_DELETE_COLS:
                case SC_CAT_DELETE_ROWS:
                case SC_CAT_DELETE_TABS:
                {
                    pAction = CreateDeleteAction(*aItr);
                }
                break;
                case SC_CAT_MOVE:
                {
                    pAction = CreateMoveAction(*aItr);
                }
                break;
                case SC_CAT_CONTENT:
                {
                    pAction = CreateContentAction(*aItr);
                }
                break;
                case SC_CAT_REJECT:
                {
                    pAction = CreateRejectionAction(*aItr);
                }
                break;
            }

            if (pAction)
                pTrack->AppendLoaded(pAction);
            else
                DBG_ERROR("no action");

            CreateGeneratedActions(*aItr);

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
            SetNewCell(*aItr);
            if (*aItr)
                delete (*aItr);
            aItr = aActions.erase(aItr);
        }

        pDoc->SetChangeTrack(pTrack);
        ScChangeViewSettings aChgViewSettings;
        aChgViewSettings.SetShowChanges(sal_True);
        pDoc->SetChangeViewSettings(aChgViewSettings);
    }
}
