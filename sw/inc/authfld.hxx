/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _AUTHFLD_HXX
#define _AUTHFLD_HXX

#include "swdllapi.h"
#include <fldbas.hxx>
#include <toxe.hxx>
#include <vector>

class SwAuthDataArr;

class SwAuthEntry
{
    String      aAuthFields[AUTH_FIELD_END];
    USHORT      nRefCount;
public:
    SwAuthEntry() : nRefCount(0){}
    SwAuthEntry( const SwAuthEntry& rCopy );
    BOOL            operator==(const SwAuthEntry& rComp);

    inline const String&    GetAuthorField(ToxAuthorityField ePos)const;
    inline void             SetAuthorField(ToxAuthorityField ePos,
                                            const String& rField);

    void            AddRef()                { ++nRefCount; }
    void            RemoveRef()             { --nRefCount; }
    USHORT          GetRefCount()           { return nRefCount; }
};

struct SwTOXSortKey
{
    ToxAuthorityField   eField;
    BOOL                bSortAscending;
    SwTOXSortKey() :
        eField(AUTH_FIELD_END),
        bSortAscending(TRUE){}
};

class SwAuthorityField;
class SortKeyArr;

class SW_DLLPUBLIC SwAuthorityFieldType : public SwFieldType
{
    SwDoc*          m_pDoc;
    SwAuthDataArr*  m_pDataArr;
    std::vector<long> m_SequArr;
    SortKeyArr*     m_pSortKeyArr;
    sal_Unicode     m_cPrefix;
    sal_Unicode     m_cSuffix;
    BOOL            m_bIsSequence :1;
    BOOL            m_bSortByDocument :1;
    LanguageType    m_eLanguage;
    String          m_sSortAlgorithm;

    // @@@ private copy assignment, but public copy ctor? @@@
    const SwAuthorityFieldType& operator=( const SwAuthorityFieldType& );

public:
    SwAuthorityFieldType(SwDoc* pDoc);
    SwAuthorityFieldType( const SwAuthorityFieldType& );
    ~SwAuthorityFieldType();

    virtual SwFieldType* Copy()    const;
    virtual void        Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhichId );

    inline void     SetDoc(SwDoc* pNewDoc)              { m_pDoc = pNewDoc; }
    SwDoc*          GetDoc(){ return m_pDoc; }
    void                RemoveField(long nHandle);
    long                AddField(const String& rFieldContents);
    BOOL                AddField(long nHandle);
    void                DelSequenceArray()
                        {
                            m_SequArr.clear();
                        }

    const SwAuthEntry*  GetEntryByHandle(long nHandle) const;

    void                GetAllEntryIdentifiers( SvStringsDtor& rToFill )const;
    const SwAuthEntry*  GetEntryByIdentifier(const String& rIdentifier)const;

    bool                ChangeEntryContent(const SwAuthEntry* pNewEntry);
    // import interface
    USHORT              AppendField(const SwAuthEntry& rInsert);
    long                GetHandle(USHORT nPos);

    USHORT              GetSequencePos(long nHandle);

    BOOL                IsSequence() const      {return m_bIsSequence;}
    void                SetSequence(BOOL bSet)
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

    BOOL                IsSortByDocument() const {return m_bSortByDocument;}
    void                SetSortByDocument(BOOL bSet)
                            {
                                DelSequenceArray();
                                m_bSortByDocument = bSet;
                            }

    USHORT              GetSortKeyCount() const ;
    const SwTOXSortKey* GetSortKey(USHORT nIdx) const ;
    void                SetSortKeys(USHORT nKeyCount, SwTOXSortKey nKeys[]);

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
public:
    SwAuthorityField(SwAuthorityFieldType* pType, const String& rFieldContents);
    SwAuthorityField(SwAuthorityFieldType* pType, long nHandle);
    ~SwAuthorityField();

    const String&       GetFieldText(ToxAuthorityField eField) const;

    virtual String      Expand() const;
    virtual SwField*    Copy() const;
    virtual void        SetPar1(const String& rStr);
    virtual SwFieldType* ChgTyp( SwFieldType* );

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhichId );

    long                GetHandle() const       { return m_nHandle; }

    virtual String GetDescription() const;
};

// --- inlines -----------------------------------------------------------
inline const String&    SwAuthEntry::GetAuthorField(ToxAuthorityField ePos)const
{
    DBG_ASSERT(AUTH_FIELD_END > ePos, "wrong index");
    return aAuthFields[ePos];
}
inline void SwAuthEntry::SetAuthorField(ToxAuthorityField ePos, const String& rField)
{
    DBG_ASSERT(AUTH_FIELD_END > ePos, "wrong index");
    if(AUTH_FIELD_END > ePos)
        aAuthFields[ePos] = rField;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
