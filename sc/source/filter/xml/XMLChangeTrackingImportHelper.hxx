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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLCHANGETRACKINGIMPORTHELPER_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLCHANGETRACKINGIMPORTHELPER_HXX

#include "chgtrack.hxx"
#include <list>
#include <com/sun/star/util/DateTime.hpp>

class ScDocument;
class DateTime;

struct ScMyActionInfo
{
    OUString sUser;
    OUString sComment;
    css::util::DateTime aDateTime;
};

struct ScMyCellInfo
{
    ScCellValue maCell;
    OUString      sFormulaAddress;
    OUString      sFormula;
    OUString          sInputString;
    double             fValue;
    sal_Int32          nMatrixCols;
    sal_Int32          nMatrixRows;
    formula::FormulaGrammar::Grammar eGrammar;
    sal_uInt16         nType;
    sal_uInt8          nMatrixFlag;

    ScMyCellInfo(
        const ScCellValue& rCell, const OUString& sFormulaAddress, const OUString& sFormula,
        const formula::FormulaGrammar::Grammar eGrammar, const OUString& sInputString,
        const double& fValue, const sal_uInt16 nType, const sal_uInt8 nMatrixFlag,
        const sal_Int32 nMatrixCols, const sal_Int32 nMatrixRows );
    ~ScMyCellInfo();

    const ScCellValue& CreateCell( ScDocument* pDoc );
};

struct ScMyDeleted
{
    sal_uInt32 nID;
    ScMyCellInfo* pCellInfo;

    ScMyDeleted();
    ~ScMyDeleted();
};

typedef std::list<ScMyDeleted*> ScMyDeletedList;

struct ScMyGenerated
{
    ScBigRange      aBigRange;
    sal_uInt32      nID;
    ScMyCellInfo*   pCellInfo;

    ScMyGenerated(ScMyCellInfo* pCellInfo, const ScBigRange& aBigRange);
    ~ScMyGenerated();
};

typedef std::list<ScMyGenerated*> ScMyGeneratedList;

struct ScMyInsertionCutOff
{
    sal_uInt32 nID;
    sal_Int32 nPosition;

    ScMyInsertionCutOff(const sal_uInt32 nTempID, const sal_Int32 nTempPosition) :
            nID(nTempID), nPosition(nTempPosition) {}
};

struct ScMyMoveCutOff
{
    sal_uInt32 nID;
    sal_Int32 nStartPosition;
    sal_Int32 nEndPosition;

    ScMyMoveCutOff(const sal_uInt32 nTempID, const sal_Int32 nStartPos, const sal_Int32 nEndPos) :
            nID(nTempID), nStartPosition(nStartPos), nEndPosition(nEndPos) {}
};

typedef std::list<ScMyMoveCutOff> ScMyMoveCutOffs;

struct ScMyMoveRanges
{
    ScBigRange aSourceRange;
    ScBigRange aTargetRange;

    ScMyMoveRanges(const ScBigRange& rSource, const ScBigRange& rTarget) :
            aSourceRange(rSource), aTargetRange(rTarget) {}
};

typedef std::list<sal_uInt32> ScMyDependencies;

struct ScMyBaseAction
{
    ScMyActionInfo aInfo;
    ScBigRange aBigRange;
    ScMyDependencies aDependencies;
    ScMyDeletedList aDeletedList;
    sal_uInt32 nActionNumber;
    sal_uInt32 nRejectingNumber;
    sal_uInt32 nPreviousAction;
    ScChangeActionType nActionType;
    ScChangeActionState nActionState;

    ScMyBaseAction(const ScChangeActionType nActionType);
    virtual ~ScMyBaseAction();
};

struct ScMyInsAction : public ScMyBaseAction
{
    ScMyInsAction(const ScChangeActionType nActionType);
    virtual ~ScMyInsAction();
};

struct ScMyDelAction : public ScMyBaseAction
{
    ScMyGeneratedList aGeneratedList;
    ScMyInsertionCutOff* pInsCutOff;
    ScMyMoveCutOffs aMoveCutOffs;
    sal_Int32 nD;

    ScMyDelAction(const ScChangeActionType nActionType);
    virtual ~ScMyDelAction();
};

struct ScMyMoveAction : public ScMyBaseAction
{
    ScMyGeneratedList aGeneratedList;
    ScMyMoveRanges* pMoveRanges;

    ScMyMoveAction();
    virtual ~ScMyMoveAction();
};

struct ScMyContentAction : public ScMyBaseAction
{
    ScMyCellInfo*   pCellInfo;

    ScMyContentAction();
    virtual ~ScMyContentAction();
};

struct ScMyRejAction : public ScMyBaseAction
{
    ScMyRejAction();
    virtual ~ScMyRejAction();
};

typedef std::list<ScMyBaseAction*> ScMyActions;

class ScXMLChangeTrackingImportHelper
{
    std::set<OUString>  aUsers;
    ScMyActions         aActions;
    css::uno::Sequence<sal_Int8> aProtect;
    ScDocument*         pDoc;
    ScChangeTrack*      pTrack;
    ScMyBaseAction*     pCurrentAction;
    OUString            sIDPrefix;
    sal_uInt32          nPrefixLength;
    sal_Int16           nMultiSpanned;
    sal_Int16           nMultiSpannedSlaveCount;
    bool                bChangeTrack;

private:
    void ConvertInfo(const ScMyActionInfo& aInfo, OUString& rUser, DateTime& aDateTime);
    ScChangeAction* CreateInsertAction(ScMyInsAction* pAction);
    ScChangeAction* CreateDeleteAction(ScMyDelAction* pAction);
    ScChangeAction* CreateMoveAction(ScMyMoveAction* pAction);
    ScChangeAction* CreateRejectionAction(ScMyRejAction* pAction);
    ScChangeAction* CreateContentAction(ScMyContentAction* pAction);

    void CreateGeneratedActions(ScMyGeneratedList& rList);

public:
    ScXMLChangeTrackingImportHelper();
    ~ScXMLChangeTrackingImportHelper();

    void SetChangeTrack(bool bValue) { bChangeTrack = bValue; }
    void SetProtection(const css::uno::Sequence<sal_Int8>& rProtect) { aProtect = rProtect; }
    void StartChangeAction(const ScChangeActionType nActionType);

    sal_uInt32 GetIDFromString(const OUString& sID);

    void SetActionNumber(const sal_uInt32 nActionNumber) { pCurrentAction->nActionNumber = nActionNumber; }
    void SetActionState(const ScChangeActionState nActionState) { pCurrentAction->nActionState = nActionState; }
    void SetRejectingNumber(const sal_uInt32 nRejectingNumber) { pCurrentAction->nRejectingNumber = nRejectingNumber; }
    void SetActionInfo(const ScMyActionInfo& aInfo);
    void SetBigRange(const ScBigRange& aBigRange) { pCurrentAction->aBigRange = aBigRange; }
    void SetPreviousChange(const sal_uInt32 nPreviousAction, ScMyCellInfo* pCellInfo);
    void SetPosition(const sal_Int32 nPosition, const sal_Int32 nCount, const sal_Int32 nTable);
    void AddDependence(const sal_uInt32 nID) { pCurrentAction->aDependencies.push_front(nID); }
    void AddDeleted(const sal_uInt32 nID);
    void AddDeleted(const sal_uInt32 nID, ScMyCellInfo* pCellInfo);
    void SetMultiSpanned(const sal_Int16 nMultiSpanned);
    void SetInsertionCutOff(const sal_uInt32 nID, const sal_Int32 nPosition);
    void AddMoveCutOff(const sal_uInt32 nID, const sal_Int32 nStartPosition, const sal_Int32 nEndPosition);
    void SetMoveRanges(const ScBigRange& aSourceRange, const ScBigRange& aTargetRange);
    void GetMultiSpannedRange();
    void AddGenerated(ScMyCellInfo* pCellInfo, const ScBigRange& aBigRange);

    void EndChangeAction();

    void SetDeletionDependencies(ScMyDelAction* pAction, ScChangeActionDel* pDelAct);
    void SetMovementDependencies(ScMyMoveAction* pAction, ScChangeActionMove* pMoveAct);
    void SetContentDependencies(ScMyContentAction* pAction, ScChangeActionContent* pActContent);
    void SetDependencies(ScMyBaseAction* pAction);

    void SetNewCell(ScMyContentAction* pAction);

    void CreateChangeTrack(ScDocument* pDoc);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
