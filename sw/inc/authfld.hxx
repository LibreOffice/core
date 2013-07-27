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
#ifndef SW_AUTHFLD_HXX
#define SW_AUTHFLD_HXX

#include "swdllapi.h"
#include <fldbas.hxx>
#include <toxe.hxx>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

class SwAuthEntry
{
    OUString        aAuthFields[AUTH_FIELD_END];
    sal_uInt16      nRefCount;
public:
    SwAuthEntry() : nRefCount(0){}
    SwAuthEntry( const SwAuthEntry& rCopy );
    sal_Bool            operator==(const SwAuthEntry& rComp);

    inline OUString         GetAuthorField(ToxAuthorityField ePos) const;
    inline void             SetAuthorField(ToxAuthorityField ePos,
                                            const OUString& rField);

    void            AddRef()                { ++nRefCount; }
    void            RemoveRef()             { --nRefCount; }
    sal_uInt16          GetRefCount()           { return nRefCount; }
};

struct SwTOXSortKey
{
    ToxAuthorityField   eField;
    sal_Bool                bSortAscending;
    SwTOXSortKey() :
        eField(AUTH_FIELD_END),
        bSortAscending(sal_True){}
};

class SwAuthorityField;
typedef boost::ptr_vector<SwTOXSortKey> SortKeyArr;
typedef boost::ptr_vector<SwAuthEntry> SwAuthDataArr;

class SW_DLLPUBLIC SwAuthorityFieldType : public SwFieldType
{
    SwDoc*                  m_pDoc;
    SwAuthDataArr           m_DataArr;
    std::vector<sal_IntPtr> m_SequArr;
    SortKeyArr              m_SortKeyArr;
    sal_Unicode             m_cPrefix;
    sal_Unicode             m_cSuffix;
    sal_Bool                m_bIsSequence :1;
    sal_Bool                m_bSortByDocument :1;
    LanguageType            m_eLanguage;
    OUString                m_sSortAlgorithm;

protected:
virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );

public:
    SwAuthorityFieldType(SwDoc* pDoc);
    ~SwAuthorityFieldType();

    virtual SwFieldType* Copy()    const;

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    inline void         SetDoc(SwDoc* pNewDoc)              { m_pDoc = pNewDoc; }
    SwDoc*              GetDoc(){ return m_pDoc; }
    void                RemoveField(sal_IntPtr nHandle);
    sal_IntPtr          AddField(const OUString& rFieldContents);
    sal_Bool            AddField(sal_IntPtr nHandle);
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

    sal_Bool            IsSequence() const      {return m_bIsSequence;}
    void                SetSequence(sal_Bool bSet)
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

    sal_Bool            IsSortByDocument() const {return m_bSortByDocument;}
    void                SetSortByDocument(sal_Bool bSet)
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

    OUString        GetSortAlgorithm() const {return m_sSortAlgorithm;}
    void            SetSortAlgorithm(const OUString& rSet) {m_sSortAlgorithm = rSet;}

};

class SwAuthorityField : public SwField
{
    sal_IntPtr          m_nHandle;
    mutable sal_IntPtr  m_nTempSequencePos;

    virtual OUString    Expand() const;
    virtual SwField*    Copy() const;

public:
    SwAuthorityField(SwAuthorityFieldType* pType, const OUString& rFieldContents);
    SwAuthorityField(SwAuthorityFieldType* pType, sal_IntPtr nHandle);
    ~SwAuthorityField();

    OUString            GetFieldText(ToxAuthorityField eField) const;

    virtual void        SetPar1(const OUString& rStr);
    virtual SwFieldType* ChgTyp( SwFieldType* );

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    sal_IntPtr          GetHandle() const       { return m_nHandle; }

    virtual OUString GetDescription() const;
};

// --- inlines -----------------------------------------------------------
inline OUString SwAuthEntry::GetAuthorField(ToxAuthorityField ePos) const
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
