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

#include <sal/config.h>

#include "swdllapi.h"
#include "fldbas.hxx"
#include "toxe.hxx"
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <memory>
#include <vector>

class SwTOXInternational;
class SwTextAttr;
class SwForm;

class SwAuthEntry final : public salhelper::SimpleReferenceObject
{
friend class SwAuthorityFieldType;
    OUString        m_aAuthFields[AUTH_FIELD_END];
public:
    SwAuthEntry() = default;
    SwAuthEntry( const SwAuthEntry& rCopy );
    bool            operator==(const SwAuthEntry& rComp) const;

    inline OUString const & GetAuthorField(ToxAuthorityField ePos) const;
    inline void             SetAuthorField(ToxAuthorityField ePos,
                                            const OUString& rField);
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
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

class SAL_DLLPUBLIC_RTTI SwAuthorityFieldType final : public SwFieldType
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

    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:
    SwAuthorityFieldType(SwDoc* pDoc);
    virtual ~SwAuthorityFieldType() override;

    virtual std::unique_ptr<SwFieldType> Copy() const override;

    virtual void        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const override;
    virtual void        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId ) override;

    void                SetDoc(SwDoc* pNewDoc)              { m_pDoc = pNewDoc; }
    SwDoc*              GetDoc(){ return m_pDoc; }
    void                RemoveField(const SwAuthEntry* nHandle);
    SwAuthEntry*        AddField(std::u16string_view rFieldContents);
    void                DelSequenceArray()
                        {
                            m_SequArr.clear();
                            m_SequArrRLHidden.clear();
                        }

    SW_DLLPUBLIC void   GetAllEntryIdentifiers( std::vector<OUString>& rToFill ) const;
    SW_DLLPUBLIC SwAuthEntry* GetEntryByIdentifier(std::u16string_view rIdentifier) const;

    bool                ChangeEntryContent(const SwAuthEntry* pNewEntry);
    // import interface
    SwAuthEntry*        AppendField(const SwAuthEntry& rInsert);

    sal_uInt16          GetSequencePos(const SwAuthEntry* pAuthEntry, SwRootFrame const* pLayout);
    std::unique_ptr<SwTOXInternational> CreateTOXInternational() const;

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

    SW_DLLPUBLIC sal_uInt16 GetSortKeyCount() const ;
    SW_DLLPUBLIC const SwTOXSortKey* GetSortKey(sal_uInt16 nIdx) const ;
    void                SetSortKeys(sal_uInt16 nKeyCount, SwTOXSortKey const nKeys[]);

    //initui.cxx
    SW_DLLPUBLIC static OUString const & GetAuthFieldName(ToxAuthorityField eType);
    SW_DLLPUBLIC static OUString const & GetAuthTypeName(ToxAuthorityType eType);

    LanguageType    GetLanguage() const {return m_eLanguage;}
    void            SetLanguage(LanguageType nLang)  {m_eLanguage = nLang;}

    const OUString& GetSortAlgorithm() const {return m_sSortAlgorithm;}
    void            SetSortAlgorithm(const OUString& rSet) {m_sSortAlgorithm = rSet;}
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

};

/** Represents an inserted bibliography entry, created using Insert -> Table of Contents and Index
    -> Bibliography Entry.

    invariant for SwAuthorityField is that it is always registered at its
    SwAuthorityFieldType via AddField()/RemoveField() & therefore has m_nHandle
    set - but it's possible that multiple SwAuthorityField have the same
    m_nHandle & so the number of instances is an upper bound on
    SwAuthorityField::m_DataArr.size() - it's not clear to me if more than one
    one of the instances with the same m_nHandle is actually in the document,
    they're all cloned via CopyField()...
 */
class SAL_DLLPUBLIC_RTTI SwAuthorityField final : public SwField
{
    rtl::Reference<SwAuthEntry>  m_xAuthEntry;
    mutable sal_IntPtr  m_nTempSequencePos;
    mutable sal_IntPtr  m_nTempSequencePosRLHidden; ///< hidden redlines

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    enum TargetType : sal_uInt16
    {
        UseDisplayURL           = 0,
        UseTargetURL            = 1,
        None                    = 2,
        BibliographyTableRow    = 3,
        // BibliographyTablePage   = 4, // TODO: implement
    };


    /// For internal use only, in general continue using ExpandField() instead.
    OUString ConditionalExpandAuthIdentifier(SwRootFrame const* pLayout) const;

    //To handle Citation
    SW_DLLPUBLIC OUString ExpandCitation(ToxAuthorityField eField, SwRootFrame const* pLayout) const;

    SwAuthorityField(SwAuthorityFieldType* pType, std::u16string_view rFieldContents);
    SwAuthorityField(SwAuthorityFieldType* pType, SwAuthEntry* pAuthEntry);
    virtual ~SwAuthorityField() override;

    const OUString &    GetFieldText(ToxAuthorityField eField) const;

    virtual void        SetPar1(const OUString& rStr) override;
    virtual SwFieldType* ChgTyp( SwFieldType* ) override;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId ) override;

    SwAuthEntry*        GetAuthEntry() const       { return m_xAuthEntry.get(); }

    virtual OUString GetDescription() const override;

    /**
     * Returns the line matching the source's default row in the ToX.
     *
     * \param   pLayout     layout to be used
     * \param   pTOX        bibliography table to take the format of the string from
     * \return              entry formatted as the appropriate authority type in the table
     */
    SW_DLLPUBLIC OUString GetAuthority(const SwRootFrame *pLayout,
                          const SwForm *pTOX = nullptr) const;

    /**
     * Returns which target should be used when the entry
     *   (the standalone field, such as '[ASDF]', not in the table) is clicked.
     */
    TargetType GetTargetType() const;
    /**
     * Returns absolute target URL in case there is one (GetTargetType() should be checked).
     *   If there isn't one, the result is undefined.
     */
    SW_DLLPUBLIC OUString GetAbsoluteURL() const;

    /**
     * Returns relative URI for the URL
     */
    OUString GetRelativeURI() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline OUString const & SwAuthEntry::GetAuthorField(ToxAuthorityField ePos) const
{
    SAL_WARN_IF(AUTH_FIELD_END <= ePos, "sw", "wrong index");
    return m_aAuthFields[ePos];
}
inline void SwAuthEntry::SetAuthorField(ToxAuthorityField ePos, const OUString& rField)
{
    SAL_WARN_IF(AUTH_FIELD_END <= ePos, "sw", "wrong index");
    if(AUTH_FIELD_END > ePos)
        m_aAuthFields[ePos] = rField;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
