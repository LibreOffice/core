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
    String      aAuthFields[AUTH_FIELD_END];
    sal_uInt16      nRefCount;
public:
    SwAuthEntry() : nRefCount(0){}
    SwAuthEntry( const SwAuthEntry& rCopy );
    sal_Bool            operator==(const SwAuthEntry& rComp);

    inline const String&    GetAuthorField(ToxAuthorityField ePos)const;
    inline void             SetAuthorField(ToxAuthorityField ePos,
                                            const String& rField);

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
    SwDoc*          m_pDoc;
    SwAuthDataArr   m_DataArr;
    std::vector<long> m_SequArr;
    SortKeyArr      m_SortKeyArr;
    sal_Unicode     m_cPrefix;
    sal_Unicode     m_cSuffix;
    sal_Bool            m_bIsSequence :1;
    sal_Bool            m_bSortByDocument :1;
    LanguageType    m_eLanguage;
    String          m_sSortAlgorithm;

protected:
virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );

public:
    SwAuthorityFieldType(SwDoc* pDoc);
    ~SwAuthorityFieldType();

    virtual SwFieldType* Copy()    const;

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    inline void     SetDoc(SwDoc* pNewDoc)              { m_pDoc = pNewDoc; }
    SwDoc*          GetDoc(){ return m_pDoc; }
    void                RemoveField(long nHandle);
    long                AddField(const String& rFieldContents);
    sal_Bool                AddField(long nHandle);
    void                DelSequenceArray()
                        {
                            m_SequArr.clear();
                        }

    const SwAuthEntry*  GetEntryByHandle(long nHandle) const;

    void                GetAllEntryIdentifiers( std::vector<String>& rToFill )const;
    const SwAuthEntry*  GetEntryByIdentifier(const String& rIdentifier)const;

    bool                ChangeEntryContent(const SwAuthEntry* pNewEntry);
    // import interface
    sal_uInt16              AppendField(const SwAuthEntry& rInsert);
    long                GetHandle(sal_uInt16 nPos);

    sal_uInt16              GetSequencePos(long nHandle);

    sal_Bool                IsSequence() const      {return m_bIsSequence;}
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

    sal_Bool                IsSortByDocument() const {return m_bSortByDocument;}
    void                SetSortByDocument(sal_Bool bSet)
                            {
                                DelSequenceArray();
                                m_bSortByDocument = bSet;
                            }

    sal_uInt16              GetSortKeyCount() const ;
    const SwTOXSortKey* GetSortKey(sal_uInt16 nIdx) const ;
    void                SetSortKeys(sal_uInt16 nKeyCount, SwTOXSortKey nKeys[]);

    //initui.cxx
    static const String&    GetAuthFieldName(ToxAuthorityField eType);
    static const String&    GetAuthTypeName(ToxAuthorityType eType);

    LanguageType    GetLanguage() const {return m_eLanguage;}
    void            SetLanguage(LanguageType nLang)  {m_eLanguage = nLang;}

    const String&   GetSortAlgorithm()const {return m_sSortAlgorithm;}
    void            SetSortAlgorithm(const String& rSet) {m_sSortAlgorithm = rSet;}

};

class SwAuthorityField : public SwField
{
    long            m_nHandle;
    mutable long    m_nTempSequencePos;

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

public:
    SwAuthorityField(SwAuthorityFieldType* pType, const String& rFieldContents);
    SwAuthorityField(SwAuthorityFieldType* pType, long nHandle);
    ~SwAuthorityField();

    const String&       GetFieldText(ToxAuthorityField eField) const;

    virtual void        SetPar1(const OUString& rStr);
    virtual SwFieldType* ChgTyp( SwFieldType* );

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    long                GetHandle() const       { return m_nHandle; }

    virtual String GetDescription() const;
};

// --- inlines -----------------------------------------------------------
inline const String&    SwAuthEntry::GetAuthorField(ToxAuthorityField ePos)const
{
    OSL_ENSURE(AUTH_FIELD_END > ePos, "wrong index");
    return aAuthFields[ePos];
}
inline void SwAuthEntry::SetAuthorField(ToxAuthorityField ePos, const String& rField)
{
    OSL_ENSURE(AUTH_FIELD_END > ePos, "wrong index");
    if(AUTH_FIELD_END > ePos)
        aAuthFields[ePos] = rField;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
