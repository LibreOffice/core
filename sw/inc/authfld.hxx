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
#ifndef INCLUDED_SW_INC_AUTHFLD_HXX
#define INCLUDED_SW_INC_AUTHFLD_HXX

#include "swdllapi.h"
#include <fldbas.hxx>
#include <toxe.hxx>
#include <sal/log.hxx>

#include <memory>
#include <vector>

class SwAuthEntry
{
    OUString        aAuthFields[AUTH_FIELD_END];
    sal_uInt16      nRefCount;
public:
    SwAuthEntry() : nRefCount(0){}
    SwAuthEntry( const SwAuthEntry& rCopy );
    bool            operator==(const SwAuthEntry& rComp);

    inline OUString const & GetAuthorField(ToxAuthorityField ePos) const;
    inline void             SetAuthorField(ToxAuthorityField ePos,
                                            const OUString& rField);

    void            AddRef()                { ++nRefCount; }
    void            RemoveRef()             { --nRefCount; }
    sal_uInt16          GetRefCount()           { return nRefCount; }
};

struct SwTOXSortKey
{
    ToxAuthorityField   eField;
    bool                bSortAscending;
    SwTOXSortKey() :
        eField(AUTH_FIELD_END),
        bSortAscending(true){}
};

class SwAuthorityField;
typedef std::vector<SwTOXSortKey> SortKeyArr;
typedef std::vector<std::unique_ptr<SwAuthEntry>> SwAuthDataArr;

class SW_DLLPUBLIC SwAuthorityFieldType : public SwFieldType
{
    SwDoc*                  m_pDoc;
    SwAuthDataArr           m_DataArr;
    std::vector<sal_IntPtr> m_SequArr;
    SortKeyArr              m_SortKeyArr;
    sal_Unicode             m_cPrefix;
    sal_Unicode             m_cSuffix;
    bool                m_bIsSequence :1;
    bool                m_bSortByDocument :1;
    LanguageType            m_eLanguage;
    OUString                m_sSortAlgorithm;

protected:
virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

public:
    SwAuthorityFieldType(SwDoc* pDoc);
    virtual ~SwAuthorityFieldType();

    virtual SwFieldType* Copy()    const override;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId ) override;

    inline void         SetDoc(SwDoc* pNewDoc)              { m_pDoc = pNewDoc; }
    SwDoc*              GetDoc(){ return m_pDoc; }
    void                RemoveField(sal_IntPtr nHandle);
    sal_IntPtr          AddField(const OUString& rFieldContents);
    bool            AddField(sal_IntPtr nHandle);
    void                DelSequenceArray()
                        {
                            m_SequArr.clear();
                        }

    const SwAuthEntry*  GetEntryByHandle(sal_IntPtr nHandle) const;

    void                GetAllEntryIdentifiers( std::vector<OUString>& rToFill ) const;
    const SwAuthEntry*  GetEntryByIdentifier(const OUString& rIdentifier) const;

    bool                ChangeEntryContent(const SwAuthEntry* pNewEntry);
    // import interface
    sal_uInt16          AppendField(const SwAuthEntry& rInsert);
    sal_IntPtr          GetHandle(sal_uInt16 nPos);

    sal_uInt16          GetSequencePos(sal_IntPtr nHandle);

    bool            IsSequence() const      {return m_bIsSequence;}
    void                SetSequence(bool bSet)
                            {
                                DelSequenceArray();
                                m_bIsSequence = bSet;
                            }

    void                SetPreSuffix( sal_Unicode cPre, sal_Unicode cSuf)
                            {
                                m_cPrefix = cPre;
                                m_cSuffix = cSuf;
                            }
    sal_Unicode         GetPrefix() const { return m_cPrefix;}
    sal_Unicode         GetSuffix() const { return m_cSuffix;}

    bool            IsSortByDocument() const {return m_bSortByDocument;}
    void                SetSortByDocument(bool bSet)
                            {
                                DelSequenceArray();
                                m_bSortByDocument = bSet;
                            }

    sal_uInt16          GetSortKeyCount() const ;
    const SwTOXSortKey* GetSortKey(sal_uInt16 nIdx) const ;
    void                SetSortKeys(sal_uInt16 nKeyCount, SwTOXSortKey nKeys[]);

    //initui.cxx
    static OUString     GetAuthFieldName(ToxAuthorityField eType);
    static OUString     GetAuthTypeName(ToxAuthorityType eType);

    LanguageType    GetLanguage() const {return m_eLanguage;}
    void            SetLanguage(LanguageType nLang)  {m_eLanguage = nLang;}

    const OUString& GetSortAlgorithm() const {return m_sSortAlgorithm;}
    void            SetSortAlgorithm(const OUString& rSet) {m_sSortAlgorithm = rSet;}

};

class SwAuthorityField : public SwField
{
    sal_IntPtr          m_nHandle;
    mutable sal_IntPtr  m_nTempSequencePos;

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

public:
    /// For internal use only, in general continue using ExpandField() instead.
    OUString ConditionalExpandAuthIdentifier() const;

    //To handle Citation
    SW_DLLPUBLIC OUString ExpandCitation(ToxAuthorityField eField) const;

    SwAuthorityField(SwAuthorityFieldType* pType, const OUString& rFieldContents);
    SwAuthorityField(SwAuthorityFieldType* pType, sal_IntPtr nHandle);
    virtual ~SwAuthorityField();

    OUString            GetFieldText(ToxAuthorityField eField) const;

    virtual void        SetPar1(const OUString& rStr) override;
    virtual SwFieldType* ChgTyp( SwFieldType* ) override;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId ) override;

    sal_IntPtr          GetHandle() const       { return m_nHandle; }

    virtual OUString GetDescription() const override;
};

inline OUString const & SwAuthEntry::GetAuthorField(ToxAuthorityField ePos) const
{
    SAL_WARN_IF(AUTH_FIELD_END <= ePos, "sw", "wrong index");
    return aAuthFields[ePos];
}
inline void SwAuthEntry::SetAuthorField(ToxAuthorityField ePos, const OUString& rField)
{
    SAL_WARN_IF(AUTH_FIELD_END <= ePos, "sw", "wrong index");
    if(AUTH_FIELD_END > ePos)
        aAuthFields[ePos] = rField;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
