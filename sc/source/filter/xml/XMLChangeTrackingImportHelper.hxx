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

#pragma once

#include <memory>
#include <chgtrack.hxx>
#include <com/sun/star/util/DateTime.hpp>

class ScDocument;
class DateTime;
enum class ScMatrixMode : sal_uInt8;

struct ScMyActionInfo
{
    OUString sUser;
    OUString sComment;
    css::util::DateTime aDateTime;
};

struct ScMyCellInfo
{
    ScCellValue        maCell;
    OUString           sFormulaAddress;
    OUString           sFormula;
    OUString           sInputString;
    double             fValue;
    sal_Int32          nMatrixCols;
    sal_Int32          nMatrixRows;
    formula::FormulaGrammar::Grammar eGrammar;
    sal_uInt16         nType;
    ScMatrixMode       nMatrixFlag;

    ScMyCellInfo(
        const ScCellValue& rCell, const OUString& sFormulaAddress, const OUString& sFormula,
        const formula::FormulaGrammar::Grammar eGrammar, const OUString& sInputString,
        const double& fValue, const sal_uInt16 nType, const ScMatrixMode nMatrixFlag,
        const sal_Int32 nMatrixCols, const sal_Int32 nMatrixRows );
    ~ScMyCellInfo();

    const ScCellValue& CreateCell( ScDocument& rDoc );
};

struct ScMyDeleted
{
    sal_uInt32 nID = 0;
    std::unique_ptr<ScMyCellInfo> pCellInfo;

    ScMyDeleted(sal_uInt32 id, std::unique_ptr<ScMyCellInfo> p) : nID(id), pCellInfo(std::move(p)) {}
};

struct ScMyGenerated
{
    ScBigRange      aBigRange;
    sal_uInt32      nID = 0;
    std::unique_ptr<ScMyCellInfo> pCellInfo;

    ScMyGenerated(ScBigRange range, sal_uInt32 id, std::unique_ptr<ScMyCellInfo> p)
      : aBigRange(range), nID(id), pCellInfo(std::move(p)) {}
};

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

struct ScMyMoveRanges
{
    ScBigRange aSourceRange;
    ScBigRange aTargetRange;

    ScMyMoveRanges(const ScBigRange& rSource, const ScBigRange& rTarget) :
            aSourceRange(rSource), aTargetRange(rTarget) {}
};

struct ScMyBaseAction
{
    ScMyActionInfo aInfo;
    ScBigRange aBigRange;
    std::vector<sal_uInt32> aDependencies;
    std::vector<ScMyDeleted> aDeletedList;
    sal_uInt32 nActionNumber;
    sal_uInt32 nRejectingNumber;
    sal_uInt32 nPreviousAction;
    ScChangeActionType nActionType;
    ScChangeActionState nActionState;

    explicit ScMyBaseAction(const ScChangeActionType nActionType);
    virtual ~ScMyBaseAction();
};

struct ScMyInsAction : public ScMyBaseAction
{
    explicit ScMyInsAction(const ScChangeActionType nActionType);
    virtual ~ScMyInsAction() override;
};

struct ScMyDelAction : public ScMyBaseAction
{
    std::vector<ScMyGenerated> aGeneratedList;
    std::unique_ptr<ScMyInsertionCutOff> pInsCutOff;
    std::vector<ScMyMoveCutOff> aMoveCutOffs;
    sal_Int32 nD;

    explicit ScMyDelAction(const ScChangeActionType nActionType);
    virtual ~ScMyDelAction() override;
};

struct ScMyMoveAction : public ScMyBaseAction
{
    std::vector<ScMyGenerated> aGeneratedList;
    std::unique_ptr<ScMyMoveRanges> pMoveRanges;

    ScMyMoveAction();
    virtual ~ScMyMoveAction() override;
};

struct ScMyContentAction : public ScMyBaseAction
{
    std::unique_ptr<ScMyCellInfo>  pCellInfo;

    ScMyContentAction();
    virtual ~ScMyContentAction() override;
};

struct ScMyRejAction : public ScMyBaseAction
{
    ScMyRejAction();
    virtual ~ScMyRejAction() override;
};

class ScXMLChangeTrackingImportHelper
{
    std::set<OUString>  aUsers;
    std::vector<std::unique_ptr<ScMyBaseAction>> aActions;
    css::uno::Sequence<sal_Int8> aProtect;
    ScChangeTrack*      pTrack;
    std::unique_ptr<ScMyBaseAction> pCurrentAction;
    sal_Int16           nMultiSpanned;
    sal_Int16           nMultiSpannedSlaveCount;

private:
    void ConvertInfo(const ScMyActionInfo& aInfo, OUString& rUser, DateTime& aDateTime);
    std::unique_ptr<ScChangeAction> CreateInsertAction(const ScMyInsAction* pAction);
    std::unique_ptr<ScChangeAction> CreateDeleteAction(const ScMyDelAction* pAction);
    std::unique_ptr<ScChangeAction> CreateMoveAction(const ScMyMoveAction* pAction);
    std::unique_ptr<ScChangeAction> CreateRejectionAction(const ScMyRejAction* pAction);
    std::unique_ptr<ScChangeAction> CreateContentAction(const ScMyContentAction* pAction, ScDocument& rDoc);

    void CreateGeneratedActions(std::vector<ScMyGenerated>& rList, ScDocument& rDoc);

public:
    ScXMLChangeTrackingImportHelper();
    ~ScXMLChangeTrackingImportHelper();

    void SetProtection(const css::uno::Sequence<sal_Int8>& rProtect) { aProtect = rProtect; }
    void StartChangeAction(const ScChangeActionType nActionType);

    static sal_uInt32 GetIDFromString(std::string_view sID);

    void SetActionNumber(const sal_uInt32 nActionNumber) { pCurrentAction->nActionNumber = nActionNumber; }
    void SetActionState(const ScChangeActionState nActionState) { pCurrentAction->nActionState = nActionState; }
    void SetRejectingNumber(const sal_uInt32 nRejectingNumber) { pCurrentAction->nRejectingNumber = nRejectingNumber; }
    void SetActionInfo(const ScMyActionInfo& aInfo);
    void SetBigRange(const ScBigRange& aBigRange) { pCurrentAction->aBigRange = aBigRange; }
    void SetPreviousChange(const sal_uInt32 nPreviousAction, ScMyCellInfo* pCellInfo);
    void SetPosition(const sal_Int32 nPosition, const sal_Int32 nCount, const sal_Int32 nTable);
    void AddDependence(const sal_uInt32 nID) { pCurrentAction->aDependencies.push_back(nID); }
    void AddDeleted(const sal_uInt32 nID);
    void AddDeleted(const sal_uInt32 nID, std::unique_ptr<ScMyCellInfo> pCellInfo);
    void SetMultiSpanned(const sal_Int16 nMultiSpanned);
    void SetInsertionCutOff(const sal_uInt32 nID, const sal_Int32 nPosition);
    void AddMoveCutOff(const sal_uInt32 nID, const sal_Int32 nStartPosition, const sal_Int32 nEndPosition);
    void SetMoveRanges(const ScBigRange& aSourceRange, const ScBigRange& aTargetRange);
    void GetMultiSpannedRange();
    void AddGenerated(std::unique_ptr<ScMyCellInfo> pCellInfo, const ScBigRange& aBigRange);

    void EndChangeAction();

    void SetDeletionDependencies(ScMyDelAction* pAction, ScChangeActionDel* pDelAct);
    void SetMovementDependencies(ScMyMoveAction* pAction, ScChangeActionMove* pMoveAct);
    void SetContentDependencies(const ScMyContentAction* pAction, ScChangeActionContent* pActContent, ScDocument& rDoc);
    void SetDependencies(ScMyBaseAction* pAction, ScDocument& rDoc);

    void SetNewCell(const ScMyContentAction* pAction, ScDocument& rDoc);

    void CreateChangeTrack(ScDocument* pDoc);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
