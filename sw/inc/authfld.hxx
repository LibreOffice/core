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
#include "fldbas.hxx"
#include "toxe.hxx"
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <memory>
#include <vector>

class SwAuthEntry : public salhelper::SimpleReferenceObject
{
    OUString        aAuthFields[AUTH_FIELD_END];
public:
    SwAuthEntry() = default;
    SwAuthEntry( const SwAuthEntry& rCopy );
    bool            operator==(const SwAuthEntry& rComp);

    inline OUString const & GetAuthorField(ToxAuthorityField ePos) const;
    inline void             SetAuthorField(ToxAuthorityField ePos,
                                            const OUString& rField);
};

struct SwTOXSortKey
{
    ToxAuthorityField   eField;
    bool                bSortAscending;
    SwTOXSortKey() :
        eField(AUTH_FIELD_END),
        bSortAscending(true){}
};

typedef std::vector<SwTOXSortKey> SortKeyArr;
typedef std::vector<rtl::Reference<SwAuthEntry>> SwAuthDataArr;

class SW_DLLPUBLIC SwAuthorityFieldType : public SwFieldType
{
    SwDoc*                  m_pDoc;
    SwAuthDataArr           m_DataArr;
    std::vector<SwAuthEntry*> m_SequArr;
    std::vector<SwAuthEntry*> m_SequArrRLHidden; ///< hidden redlines
    SortKeyArr              m_SortKeyArr;
    sal_Unicode             m_cPrefix;
    sal_Unicode             m_cSuffix;
    bool                    m_bIsSequence :1;
    bool                    m_bSortByDocument :1;
    LanguageType            m_eLanguage;
    OUString                m_sSortAlgorithm;

protected:
virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

public:
    SwAuthorityFieldType(SwDoc* pDoc);
    virtual ~SwAuthorityFieldType() override;

    virtual SwFieldType* Copy()    const override;

    virtual void        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const override;
    virtual void        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId ) override;

    void                SetDoc(SwDoc* pNewDoc)              { m_pDoc = pNewDoc; }
    SwDoc*              GetDoc(){ return m_pDoc; }
    void                RemoveField(const SwAuthEntry* nHandle);
    SwAuthEntry*        AddField(const OUString& rFieldContents);
    void                DelSequenceArray()
                        {
                            m_SequArr.clear();
                            m_SequArrRLHidden.clear();
                        }

    void                GetAllEntryIdentifiers( std::vector<OUString>& rToFill ) const;
    SwAuthEntry*        GetEntryByIdentifier(const OUString& rIdentifier) const;

    bool                ChangeEntryContent(const SwAuthEntry* pNewEntry);
    // import interface
    sal_uInt16          AppendField(const SwAuthEntry& rInsert);

    sal_uInt16          GetSequencePos(const SwAuthEntry* pAuthEntry, SwRootFrame const* pLayout);

    bool                IsSequence() const      {return m_bIsSequence;}
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

    bool                IsSortByDocument() const {return m_bSortByDocument;}
    void                SetSortByDocument(bool bSet)
                            {
                                DelSequenceArray();
                                m_bSortByDocument = bSet;
                            }

    sal_uInt16          GetSortKeyCount() const ;
    const SwTOXSortKey* GetSortKey(sal_uInt16 nIdx) const ;
    void                SetSortKeys(sal_uInt16 nKeyCount, SwTOXSortKey const nKeys[]);

    //initui.cxx
    static OUString const & GetAuthFieldName(ToxAuthorityField eType);
    static OUString const & GetAuthTypeName(ToxAuthorityType eType);

    LanguageType    GetLanguage() const {return m_eLanguage;}
    void            SetLanguage(LanguageType nLang)  {m_eLanguage = nLang;}

    const OUString& GetSortAlgorithm() const {return m_sSortAlgorithm;}
    void            SetSortAlgorithm(const OUString& rSet) {m_sSortAlgorithm = rSet;}

};

/** invariant for SwAuthorityField is that it is always registered at its
    SwAuthorityFieldType via AddField()/RemoveField() & therefore has m_nHandle
    set - but it's possible that multiple SwAuthorityField have the same
    m_nHandle & so the number of instances is an upper bound on
    SwAuthorityField::m_DataArr.size() - it's not clear to me if more than one
    one of the instances with the same m_nHandle is actually in the document,
    they're all cloned via CopyField()...
 */
class SwAuthorityField : public SwField
{
    rtl::Reference<SwAuthEntry>  m_xAuthEntry;
    mutable sal_IntPtr  m_nTempSequencePos;
    mutable sal_IntPtr  m_nTempSequencePosRLHidden; ///< hidden redlines

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    /// For internal use only, in general continue using ExpandField() instead.
    OUString ConditionalExpandAuthIdentifier(SwRootFrame const* pLayout) const;

    //To handle Citation
    SW_DLLPUBLIC OUString ExpandCitation(ToxAuthorityField eField, SwRootFrame const* pLayout) const;

    SwAuthorityField(SwAuthorityFieldType* pType, const OUString& rFieldContents);
    SwAuthorityField(SwAuthorityFieldType* pType, SwAuthEntry* pAuthEntry);
    virtual ~SwAuthorityField() override;

    OUString            GetFieldText(ToxAuthorityField eField) const;

    virtual void        SetPar1(const OUString& rStr) override;
    virtual SwFieldType* ChgTyp( SwFieldType* ) override;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId ) override;

    SwAuthEntry*        GetAuthEntry() const       { return m_xAuthEntry.get(); }

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
