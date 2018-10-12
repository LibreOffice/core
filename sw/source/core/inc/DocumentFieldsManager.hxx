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
#include <sal/types.h>
#include <memory>

class SwDoc;
class SwDBNameInfField;

namespace sw {

class DocumentFieldsManager : public IDocumentFieldsAccess
{

public:

    DocumentFieldsManager( SwDoc& i_rSwdoc );

    virtual const SwFieldTypes *GetFieldTypes() const override;
    virtual SwFieldType *InsertFieldType(const SwFieldType &) override;
    virtual SwFieldType *GetSysFieldType( const SwFieldIds eWhich ) const override;
    virtual SwFieldType* GetFieldType(SwFieldIds nResId, const OUString& rName, bool bDbFieldMatching) const override;
    virtual void RemoveFieldType(size_t nField) override;
    virtual void UpdateFields(bool bCloseDB) override;
    virtual void InsDeletedFieldType(SwFieldType &) override;
    virtual void PutValueToField(const SwPosition & rPos, const css::uno::Any& rVal, sal_uInt16 nWhich) override;
    virtual bool UpdateField(SwTextField * rDstFormatField, SwField & rSrcField, SwMsgPoolItem * pMsgHint, bool bUpdateTableFields) override;
    virtual void UpdateRefFields() override;
    virtual void UpdateTableFields(SfxPoolItem* pHt) override;
    virtual void UpdateExpFields(SwTextField* pField, bool bUpdateRefFields) override;
    virtual void UpdateUsrFields() override;
    virtual void UpdatePageFields(SfxPoolItem*) override;
    virtual void LockExpFields() override;
    virtual void UnlockExpFields() override;
    virtual bool IsExpFieldsLocked() const override;
    virtual SwDocUpdateField& GetUpdateFields() const override;
    virtual bool SetFieldsDirty(bool b, const SwNode* pChk, sal_uLong nLen) override;
    virtual void SetFixFields(const DateTime* pNewDateTime) override;
    virtual void FieldsToCalc(SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt) override;
    virtual void FieldsToCalc(SwCalc& rCalc, const SetGetExpField& rToThisField, SwRootFrame const* pLayout) override;
    virtual void FieldsToExpand(SwHashTable<HashStr>& rTable, const SetGetExpField& rToThisField, SwRootFrame const& rLayout) override;
    virtual bool IsNewFieldLst() const override;
    virtual void SetNewFieldLst( bool bFlag) override;
    virtual void InsDelFieldInFieldLst(bool bIns, const SwTextField& rField) override;
    virtual sal_Int32 GetRecordsPerDocument() const override;

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

    void InitFieldTypes();

    void ClearFieldTypes();

    void UpdateDBNumFields( SwDBNameInfField& rDBField, SwCalc& rCalc );

    virtual ~DocumentFieldsManager() override;

private:

    DocumentFieldsManager(DocumentFieldsManager const&) = delete;
    DocumentFieldsManager& operator=(DocumentFieldsManager const&) = delete;

    void UpdateExpFieldsImpl(SwTextField* pField, SwRootFrame const* pLayout);

    SwDoc& m_rDoc;

    bool mbNewFieldLst; //< TRUE: Rebuild field-list.
    std::unique_ptr<SwDocUpdateField> mpUpdateFields; //< Struct for updating fields
    std::unique_ptr<SwFieldTypes>     mpFieldTypes;
    sal_Int8    mnLockExpField;  //< If != 0 UpdateExpFields() has no effect!
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
