/*************************************************************************
 *
 *  $RCSfile: XMLChangeTrackingImportHelper.hxx,v $
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
#define _SC_XMLCHANGETRACKINGIMPORTHELPER_HXX

#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif

#ifndef __SGI_STL_LIST
#include <stl/list>
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

struct ScMyCellDeleted
{
    ScBaseCell*     pCell;
    rtl::OUString   sFormulaAddress;

    ScMyCellDeleted();
    ~ScMyCellDeleted();
};

struct ScMyDeleted
{
    sal_uInt32 nID;
    ScMyCellDeleted* pCellDeleted;

    ScMyDeleted();
    ~ScMyDeleted();
};

typedef std::list<ScMyDeleted> ScMyDeletedList;

struct ScMyGenerated
{
    ScBigRange      aBigRange;
    rtl::OUString   sFormulaAddress;
    ScBaseCell*     pCell;
    sal_uInt32      nID;

    ScMyGenerated(ScBaseCell* pCell, const rtl::OUString& sFormulaAddress, const ScBigRange& aBigRange);
};

typedef std::list<ScMyGenerated> ScMyGeneratedList;

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

typedef std::list<sal_uInt32> ScMyDependences;

struct ScMyAction
{
    ScMyActionInfo aInfo;
    rtl::OUString sFormulaAddress;
    ScBigRange aBigRange;
    ScMyDependences aDependences;
    ScMyDeletedList aDeletedList;
    ScMyGeneratedList aGeneratedList;
    ScMyMoveCutOffs aMoveCutOffs;
    ScBaseCell* pOldCell;
    ScMyInsertionCutOff* pInsCutOff;
    ScMyMoveRanges* pMoveRanges;
    sal_uInt32 nActionNumber;
    sal_uInt32 nRejectingNumber;
    sal_uInt32 nPreviousAction;
    sal_Int16 nD;
    ScChangeActionType nActionType;
    ScChangeActionState nActionState;

    ScMyAction();
    ~ScMyAction();
};

typedef std::list<ScMyAction*> ScMyActions;

class ScXMLChangeTrackingImportHelper
{
    StrCollection       aUsers;
    ScMyActions         aActions;
    ScDocument*         pDoc;
    ScChangeTrack*      pTrack;
    ScMyAction*         pCurrentAction;
    rtl::OUString       sIDPrefix;
    sal_uInt32          nPrefixLength;
    sal_Int16           nMultiSpanned;
    sal_Int16           nMultiSpannedSlaveCount;

public:
    ScXMLChangeTrackingImportHelper();
    ~ScXMLChangeTrackingImportHelper();

    void StartChangeAction(const ScChangeActionType nActionType);

    sal_uInt32 GetIDFromString(const rtl::OUString& sID);

    void SetActionNumber(const sal_uInt32 nActionNumber) { pCurrentAction->nActionNumber = nActionNumber; }
    void SetActionState(const ScChangeActionState nActionState) { pCurrentAction->nActionState = nActionState; }
    void SetRejectingNumber(const sal_uInt32 nRejectingNumber) { pCurrentAction->nRejectingNumber = nRejectingNumber; }
    void SetActionInfo(const ScMyActionInfo& aInfo);
    void SetBigRange(const ScBigRange& aBigRange) { pCurrentAction->aBigRange = aBigRange; }
    void SetPreviousChange(const sal_uInt32 nPreviousAction, ScBaseCell* pOldCell, const rtl::OUString& sFormulaAddress)
                                { pCurrentAction->nPreviousAction = nPreviousAction; pCurrentAction->pOldCell = pOldCell; pCurrentAction->sFormulaAddress = sFormulaAddress; }
    void SetPosition(const sal_Int32 nPosition, const sal_Int32 nCount, const sal_Int32 nTable);
    void AddDependence(const sal_uInt32 nID) { pCurrentAction->aDependences.push_front(nID); }
    void AddDeleted(const sal_uInt32 nID);
    void AddDeleted(const sal_uInt32 nID, ScBaseCell* pCell, const rtl::OUString& sFormulaAddress);
    void SetMultiSpanned(const sal_Int16 nMultiSpanned);
    void SetInsertionCutOff(const sal_uInt32 nID, const sal_Int32 nPosition);
    void AddMoveCutOff(const sal_uInt32 nID, const sal_Int32 nStartPosition, const sal_Int32 nEndPosition);
    void SetMoveRanges(const ScBigRange& aSourceRange, const ScBigRange& aTargetRange);
    void GetMultiSpannedRange();
    void AddGenerated(ScBaseCell* pCell, const rtl::OUString& sFormulaAddress, const ScBigRange& aBigRange);

    void EndChangeAction();

    void ConvertInfo(const ScMyActionInfo& aInfo, sal_uInt16& nUserPos, DateTime& aDateTime);
    ScChangeAction* CreateInsertAction(ScMyAction* pAction);
    ScChangeAction* CreateDeleteAction(ScMyAction* pAction);
    ScChangeAction* CreateMoveAction(ScMyAction* pAction);
    ScChangeAction* CreateRejectionAction(ScMyAction* pAction);
    ScChangeAction* CreateContentAction(ScMyAction* pAction);

    void CreateGeneratedActions(ScMyAction* pAction);

    void SetDependences(ScMyAction* pAction);

    void SetNewCell(ScMyAction* pAction);

    void CreateChangeTrack(ScDocument* pDoc);
};

#endif
