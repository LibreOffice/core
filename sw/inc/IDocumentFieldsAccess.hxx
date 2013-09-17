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

#ifndef IDOCUMENTFIELDSACCESS_HXX_INCLUDED
#define IDOCUMENTFIELDSACCESS_HXX_INCLUDED

#include <sal/types.h>
#include <tools/solar.h>

class SwFldTypes;
class SwFieldType;
class SfxPoolItem;
struct SwPosition;
class SwDocUpdtFld;
class SwCalc;
class SwTxtFld;
class SwField;
class SwMsgPoolItem;
class DateTime;
class _SetGetExpFld;
struct SwHash;
class SwNode;

namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }

 /** Document fields related interfaces
 */
 class IDocumentFieldsAccess
 {
 public:
    virtual const SwFldTypes *GetFldTypes() const = 0;

    virtual SwFieldType *InsertFldType(const SwFieldType &) = 0;

    virtual SwFieldType *GetSysFldType( const sal_uInt16 eWhich ) const = 0;

    virtual SwFieldType* GetFldType(sal_uInt16 nResId, const String& rName, bool bDbFieldMatching) const = 0;

    virtual void RemoveFldType(sal_uInt16 nFld) = 0;

    virtual void UpdateFlds( SfxPoolItem* pNewHt, bool bCloseDB) = 0;

    virtual void InsDeletedFldType(SwFieldType &) = 0;

    /**
       Puts a value into a field at a certain position.

       A missing field at the given position leads to a failure.

       @param rPosition        position of the field
       @param rVal             the value
       @param nMId

       @retval sal_True            putting of value was successful
       @retval sal_False           else
    */
    virtual bool PutValueToField(const SwPosition & rPos, const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich) = 0;

    // Call update of expression fields. All expressions are re-evaluated.

    /** Updates a field.

        @param rDstFmtFld field to update
        @param rSrcFld field containing the new values
        @param pMsgHnt
        @param bUpdateTblFlds TRUE: update table fields, too.

        @retval sal_True             update was successful
        @retval sal_False            else
    */
    virtual bool UpdateFld(SwTxtFld * rDstFmtFld, SwField & rSrcFld, SwMsgPoolItem * pMsgHnt, bool bUpdateTblFlds) = 0;

    virtual void UpdateRefFlds(SfxPoolItem* pHt) = 0;

    virtual void UpdateTblFlds(SfxPoolItem* pHt) = 0;

    virtual void UpdateExpFlds(SwTxtFld* pFld, bool bUpdateRefFlds) = 0;

    virtual void UpdateUsrFlds() = 0;

    virtual void UpdatePageFlds(SfxPoolItem*) = 0;

    virtual void LockExpFlds() = 0;

    virtual void UnlockExpFlds() = 0;

    virtual bool IsExpFldsLocked() const = 0;

    virtual SwDocUpdtFld& GetUpdtFlds() const = 0;

    /*  @@@MAINTAINABILITY-HORROR@@@
        SwNode (see parameter pChk) is (?) part of the private
        data structure of SwDoc and should not be exposed
    */
    virtual bool SetFieldsDirty(bool b, const SwNode* pChk, sal_uLong nLen) = 0;

    virtual void SetFixFields(bool bOnlyTimeDate, const DateTime* pNewDateTime) = 0;


    // In Calculator set all SetExpression fields that are valid up to the indicated position
    // (Node [ + ::com::sun::star::ucb::Content]).
    // A generated list of all fields may be passed along too
    // (if the addreess != 0 and the pointer == 0 a new list will be returned).
    virtual void FldsToCalc(SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt) = 0;

    virtual void FldsToCalc(SwCalc& rCalc, const _SetGetExpFld& rToThisFld) = 0;

    virtual void FldsToExpand(SwHash**& ppTbl, sal_uInt16& rTblSize, const _SetGetExpFld& rToThisFld) = 0;

    virtual bool IsNewFldLst() const = 0;

    virtual void SetNewFldLst( bool bFlag) = 0;

    virtual void InsDelFldInFldLst(bool bIns, const SwTxtFld& rFld) = 0;

protected:
    virtual ~IDocumentFieldsAccess() {};
 };

 #endif // IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
