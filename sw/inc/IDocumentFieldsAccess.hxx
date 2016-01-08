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

#ifndef INCLUDED_SW_INC_IDOCUMENTFIELDSACCESS_HXX
#define INCLUDED_SW_INC_IDOCUMENTFIELDSACCESS_HXX

#include <sal/types.h>
#include <tools/solar.h>

class SwFieldTypes;
class SwFieldType;
class SfxPoolItem;
struct SwPosition;
class SwDocUpdateField;
class SwCalc;
class SwTextField;
class SwField;
class SwMsgPoolItem;
class DateTime;
class _SetGetExpField;
struct SwHash;
class SwNode;

namespace rtl { class OUString; }
using rtl::OUString;
namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }

 /** Document fields related interfaces
 */
 class IDocumentFieldsAccess
 {
 public:
    virtual const SwFieldTypes *GetFieldTypes() const = 0;

    virtual SwFieldType *InsertFieldType(const SwFieldType &) = 0;

    virtual SwFieldType *GetSysFieldType( const sal_uInt16 eWhich ) const = 0;

    virtual SwFieldType* GetFieldType(sal_uInt16 nResId, const OUString& rName, bool bDbFieldMatching) const = 0;

    virtual void RemoveFieldType(size_t nField) = 0;

    virtual void UpdateFields( SfxPoolItem* pNewHt, bool bCloseDB) = 0;

    virtual void InsDeletedFieldType(SwFieldType &) = 0;

    /**
       Puts a value into a field at a certain position.

       A missing field at the given position leads to a failure.

       @param rPosition        position of the field
       @param rVal             the value
       @param nMId

       @retval true            putting of value was successful
       @retval false           else
    */
    virtual bool PutValueToField(const SwPosition & rPos, const css::uno::Any& rVal, sal_uInt16 nWhich) = 0;

    // Call update of expression fields. All expressions are re-evaluated.

    /** Updates a field.

        @param rDstFormatField field to update
        @param rSrcField field containing the new values
        @param pMsgHint
        @param bUpdateTableFields TRUE: update table fields, too.

        @retval true             update was successful
        @retval false            else
    */
    virtual bool UpdateField(SwTextField * rDstFormatField, SwField & rSrcField, SwMsgPoolItem * pMsgHint, bool bUpdateTableFields) = 0;

    virtual void UpdateRefFields(SfxPoolItem* pHt) = 0;

    virtual void UpdateTableFields(SfxPoolItem* pHt) = 0;

    virtual void UpdateExpFields(SwTextField* pField, bool bUpdateRefFields) = 0;

    virtual void UpdateUsrFields() = 0;

    virtual void UpdatePageFields(SfxPoolItem*) = 0;

    virtual void LockExpFields() = 0;

    virtual void UnlockExpFields() = 0;

    virtual bool IsExpFieldsLocked() const = 0;

    virtual SwDocUpdateField& GetUpdateFields() const = 0;

    /* How many DB records are needed to fill the document

       Counts the fields, which advance the DB cursor, + 1.
       Result is always > 0, as even a document without fields will
       advance the cursor in mail merge.

       @return amount of needed DB records
     */
    virtual sal_uInt16 WantedDBrecords() const = 0;

    /*  @@@MAINTAINABILITY-HORROR@@@
        SwNode (see parameter pChk) is (?) part of the private
        data structure of SwDoc and should not be exposed
    */
    virtual bool SetFieldsDirty(bool b, const SwNode* pChk, sal_uLong nLen) = 0;

    virtual void SetFixFields(bool bOnlyTimeDate, const DateTime* pNewDateTime) = 0;

    // In Calculator set all SetExpression fields that are valid up to the indicated position
    // (Node [ + css::ucb::Content]).
    // A generated list of all fields may be passed along too
    // (if the address != 0 and the pointer == 0 a new list will be returned).
    virtual void FieldsToCalc(SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt) = 0;

    virtual void FieldsToCalc(SwCalc& rCalc, const _SetGetExpField& rToThisField) = 0;

    virtual void FieldsToExpand(SwHash**& ppTable, sal_uInt16& rTableSize, const _SetGetExpField& rToThisField) = 0;

    virtual bool IsNewFieldLst() const = 0;

    virtual void SetNewFieldLst( bool bFlag) = 0;

    virtual void InsDelFieldInFieldLst(bool bIns, const SwTextField& rField) = 0;

protected:
    virtual ~IDocumentFieldsAccess() {};
 };

#endif // INCLUDED_SW_INC_IDOCUMENTFIELDSACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
