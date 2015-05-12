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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTFIELDSMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTFIELDSMANAGER_HXX

#include <IDocumentFieldsAccess.hxx>
#include <boost/utility.hpp>
#include <sal/types.h>

class SwDoc;
class SwDBNameInfField;

namespace sw {

class DocumentFieldsManager : public IDocumentFieldsAccess,
                              public ::boost::noncopyable
{

public:

    DocumentFieldsManager( SwDoc& i_rSwdoc );

    virtual const SwFieldTypes *GetFieldTypes() const SAL_OVERRIDE;
    virtual SwFieldType *InsertFieldType(const SwFieldType &) SAL_OVERRIDE;
    virtual SwFieldType *GetSysFieldType( const sal_uInt16 eWhich ) const SAL_OVERRIDE;
    virtual SwFieldType* GetFieldType(sal_uInt16 nResId, const OUString& rName, bool bDbFieldMatching) const SAL_OVERRIDE;
    virtual void RemoveFieldType(size_t nField) SAL_OVERRIDE;
    virtual void UpdateFields( SfxPoolItem* pNewHt, bool bCloseDB) SAL_OVERRIDE;
    virtual void InsDeletedFieldType(SwFieldType &) SAL_OVERRIDE;
    virtual bool PutValueToField(const SwPosition & rPos, const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich) SAL_OVERRIDE;
    virtual bool UpdateField(SwTextField * rDstFormatField, SwField & rSrcField, SwMsgPoolItem * pMsgHint, bool bUpdateTableFields) SAL_OVERRIDE;
    virtual void UpdateRefFields(SfxPoolItem* pHt) SAL_OVERRIDE;
    virtual void UpdateTableFields(SfxPoolItem* pHt) SAL_OVERRIDE;
    virtual void UpdateExpFields(SwTextField* pField, bool bUpdateRefFields) SAL_OVERRIDE;
    virtual void UpdateUsrFields() SAL_OVERRIDE;
    virtual void UpdatePageFields(SfxPoolItem*) SAL_OVERRIDE;
    virtual void LockExpFields() SAL_OVERRIDE;
    virtual void UnlockExpFields() SAL_OVERRIDE;
    virtual bool IsExpFieldsLocked() const SAL_OVERRIDE;
    virtual SwDocUpdateField& GetUpdateFields() const SAL_OVERRIDE;
    virtual bool SetFieldsDirty(bool b, const SwNode* pChk, sal_uLong nLen) SAL_OVERRIDE;
    virtual void SetFixFields(bool bOnlyTimeDate, const DateTime* pNewDateTime) SAL_OVERRIDE;
    virtual void FieldsToCalc(SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt) SAL_OVERRIDE;
    virtual void FieldsToCalc(SwCalc& rCalc, const _SetGetExpField& rToThisField) SAL_OVERRIDE;
    virtual void FieldsToExpand(SwHash**& ppTable, sal_uInt16& rTableSize, const _SetGetExpField& rToThisField) SAL_OVERRIDE;
    virtual bool IsNewFieldLst() const SAL_OVERRIDE;
    virtual void SetNewFieldLst( bool bFlag) SAL_OVERRIDE;
    virtual void InsDelFieldInFieldLst(bool bIns, const SwTextField& rField) SAL_OVERRIDE;

    //Non Interface methods

    /** Returns the field at a certain position.
       @param rPos position to search at
       @return pointer to field at the given position or NULL in case no field is found
    */
    static SwField* GetFieldAtPos(const SwPosition& rPos);

    /** Returns the field at a certain position.
       @param rPos position to search at
       @return pointer to field at the given position or NULL in case no field is found
    */
    static SwTextField* GetTextFieldAtPos(const SwPosition& rPos);

    bool containsUpdatableFields();

    // Delete all unreferenced field types.
    void GCFieldTypes();

    void _InitFieldTypes();

    void ClearFieldTypes();

    void UpdateDBNumFields( SwDBNameInfField& rDBField, SwCalc& rCalc );

    virtual ~DocumentFieldsManager();

private:

    SwDoc& m_rDoc;

    bool mbNewFieldLst; //< TRUE: Rebuild field-list.
    SwDocUpdateField    *mpUpdateFields; //< Struct for updating fields
    SwFieldTypes      *mpFieldTypes;
    sal_Int8    mnLockExpField;  //< If != 0 UpdateExpFields() has no effect!
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
