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

    virtual const SwFldTypes *GetFldTypes() const SAL_OVERRIDE;
    virtual SwFieldType *InsertFldType(const SwFieldType &) SAL_OVERRIDE;
    virtual SwFieldType *GetSysFldType( const sal_uInt16 eWhich ) const SAL_OVERRIDE;
    virtual SwFieldType* GetFldType(sal_uInt16 nResId, const OUString& rName, bool bDbFieldMatching) const SAL_OVERRIDE;
    virtual void RemoveFldType(size_t nFld) SAL_OVERRIDE;
    virtual void UpdateFlds( SfxPoolItem* pNewHt, bool bCloseDB) SAL_OVERRIDE;
    virtual void InsDeletedFldType(SwFieldType &) SAL_OVERRIDE;
    virtual bool PutValueToField(const SwPosition & rPos, const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich) SAL_OVERRIDE;
    virtual bool UpdateFld(SwTxtFld * rDstFmtFld, SwField & rSrcFld, SwMsgPoolItem * pMsgHnt, bool bUpdateTblFlds) SAL_OVERRIDE;
    virtual void UpdateRefFlds(SfxPoolItem* pHt) SAL_OVERRIDE;
    virtual void UpdateTblFlds(SfxPoolItem* pHt) SAL_OVERRIDE;
    virtual void UpdateExpFlds(SwTxtFld* pFld, bool bUpdateRefFlds) SAL_OVERRIDE;
    virtual void UpdateUsrFlds() SAL_OVERRIDE;
    virtual void UpdatePageFlds(SfxPoolItem*) SAL_OVERRIDE;
    virtual void LockExpFlds() SAL_OVERRIDE;
    virtual void UnlockExpFlds() SAL_OVERRIDE;
    virtual bool IsExpFldsLocked() const SAL_OVERRIDE;
    virtual SwDocUpdtFld& GetUpdtFlds() const SAL_OVERRIDE;
    virtual bool SetFieldsDirty(bool b, const SwNode* pChk, sal_uLong nLen) SAL_OVERRIDE;
    virtual void SetFixFields(bool bOnlyTimeDate, const DateTime* pNewDateTime) SAL_OVERRIDE;
    virtual void FldsToCalc(SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt) SAL_OVERRIDE;
    virtual void FldsToCalc(SwCalc& rCalc, const _SetGetExpFld& rToThisFld) SAL_OVERRIDE;
    virtual void FldsToExpand(SwHash**& ppTbl, sal_uInt16& rTblSize, const _SetGetExpFld& rToThisFld) SAL_OVERRIDE;
    virtual bool IsNewFldLst() const SAL_OVERRIDE;
    virtual void SetNewFldLst( bool bFlag) SAL_OVERRIDE;
    virtual void InsDelFldInFldLst(bool bIns, const SwTxtFld& rFld) SAL_OVERRIDE;

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
    static SwTxtFld* GetTxtFldAtPos(const SwPosition& rPos);

    bool containsUpdatableFields();

    // Delete all unreferenced field types.
    void GCFieldTypes();

    void _InitFieldTypes();

    void ClearFieldTypes();

    void UpdateDBNumFlds( SwDBNameInfField& rDBFld, SwCalc& rCalc );

    virtual ~DocumentFieldsManager();

private:

    SwDoc& m_rDoc;

    bool mbNewFldLst; //< TRUE: Rebuild field-list.
    SwDocUpdtFld    *mpUpdtFlds; //< Struct for updating fields
    SwFldTypes      *mpFldTypes;
    sal_Int8    mnLockExpFld;  //< If != 0 UpdateExpFlds() has no effect!
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
