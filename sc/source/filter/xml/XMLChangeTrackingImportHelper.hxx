/*************************************************************************
 *
 *  $RCSfile: XMLChangeTrackingImportHelper.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:45:57 $
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
#define _SC_XMLCHANGETRACKINGIMPORTHELPER_HXX

#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif

#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

class ScBaseCell;
class ScDocument;
class DateTime;

struct ScMyActionInfo
{
    rtl::OUString sUser;
    rtl::OUString sComment;
    com::sun::star::util::DateTime aDateTime;
};

struct ScMyCellInfo
{
    ScBaseCell*     pCell;
    rtl::OUString   sFormulaAddress;
    rtl::OUString   sFormula;
    String          sInputString;
    double          fValue;
    sal_Int32       nMatrixCols;
    sal_Int32       nMatrixRows;
    sal_uInt16      nType;
    sal_uInt8       nMatrixFlag;

    ScMyCellInfo();
    ScMyCellInfo(ScBaseCell* pCell, const rtl::OUString& sFormulaAddress, const rtl::OUString& sFormula, const rtl::OUString& sInputString,
                const double& fValue, const sal_uInt16 nType, const sal_uInt8 nMatrixFlag, const sal_Int32 nMatrixCols,
                const sal_Int32 nMatrixRows);
    ~ScMyCellInfo();

    ScBaseCell* CreateCell(ScDocument* pDoc);
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

    ScMyMoveRanges(const ScBigRange& aSource, const ScBigRange aTarget) :
            aSourceRange(aSource), aTargetRange(aTarget) {}
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
    ~ScMyBaseAction();
};

struct ScMyInsAction : public ScMyBaseAction
{
    ScMyInsAction(const ScChangeActionType nActionType);
    ~ScMyInsAction();
};

struct ScMyDelAction : public ScMyBaseAction
{
    ScMyGeneratedList aGeneratedList;
    ScMyInsertionCutOff* pInsCutOff;
    ScMyMoveCutOffs aMoveCutOffs;
    sal_Int32 nD;

    ScMyDelAction(const ScChangeActionType nActionType);
    ~ScMyDelAction();
};

struct ScMyMoveAction : public ScMyBaseAction
{
    ScMyGeneratedList aGeneratedList;
    ScMyMoveRanges* pMoveRanges;

    ScMyMoveAction();
    ~ScMyMoveAction();
};

struct ScMyContentAction : public ScMyBaseAction
{
    ScMyCellInfo*   pCellInfo;

    ScMyContentAction();
    ~ScMyContentAction();
};

struct ScMyRejAction : public ScMyBaseAction
{
    ScMyRejAction();
    ~ScMyRejAction();
};

typedef std::list<ScMyBaseAction*> ScMyActions;

class ScChangeViewSettings;

class ScXMLChangeTrackingImportHelper
{
    StrCollection       aUsers;
    ScMyActions         aActions;
    com::sun::star::uno::Sequence<sal_Int8> aProtect;
    ScDocument*         pDoc;
    ScChangeTrack*      pTrack;
    ScMyBaseAction*     pCurrentAction;
    rtl::OUString       sIDPrefix;
    sal_uInt32          nPrefixLength;
    sal_Int16           nMultiSpanned;
    sal_Int16           nMultiSpannedSlaveCount;
    sal_Bool            bChangeTrack : 1;

private:
    void ConvertInfo(const ScMyActionInfo& aInfo, String& rUser, DateTime& aDateTime);
    ScChangeAction* CreateInsertAction(ScMyInsAction* pAction);
    ScChangeAction* CreateDeleteAction(ScMyDelAction* pAction);
    ScChangeAction* CreateMoveAction(ScMyMoveAction* pAction);
    ScChangeAction* CreateRejectionAction(ScMyRejAction* pAction);
    ScChangeAction* CreateContentAction(ScMyContentAction* pAction);

    void CreateGeneratedActions(ScMyGeneratedList& rList);

public:
    ScXMLChangeTrackingImportHelper();
    ~ScXMLChangeTrackingImportHelper();

    void SetChangeTrack(sal_Bool bValue) { bChangeTrack = bValue; }
    void SetProtection(const com::sun::star::uno::Sequence<sal_Int8>& rProtect) { aProtect = rProtect; }
    void StartChangeAction(const ScChangeActionType nActionType);

    sal_uInt32 GetIDFromString(const rtl::OUString& sID);

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
