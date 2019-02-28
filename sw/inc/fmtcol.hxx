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
#ifndef INCLUDED_SW_INC_FMTCOL_HXX
#define INCLUDED_SW_INC_FMTCOL_HXX

#include "swdllapi.h"
#include "format.hxx"
#include "hintids.hxx"
#include <rtl/ustring.hxx>
#include <tools/solar.h>

#include <vector>
#include <memory>

class SwAttrPool;
namespace sw{ class DocumentStylePoolManager; }

class SAL_DLLPUBLIC_RTTI SwFormatColl : public SwFormat
{
protected:
    SwFormatColl( SwAttrPool& rPool, const sal_Char* pFormatName,
                const sal_uInt16* pWhichRanges, SwFormatColl* pDerFrom,
                sal_uInt16 nFormatWhich )
          : SwFormat( rPool, pFormatName, pWhichRanges, pDerFrom, nFormatWhich )
    { SetAuto(false); }

    SwFormatColl( SwAttrPool& rPool, const OUString &rFormatName,
                const sal_uInt16* pWhichRanges, SwFormatColl* pDerFrom,
                sal_uInt16 nFormatWhich )
          : SwFormat( rPool, rFormatName, pWhichRanges, pDerFrom, nFormatWhich )
    { SetAuto(false); }

private:
    SwFormatColl(const SwFormatColl & ) = delete;
    const SwFormatColl &operator=(const SwFormatColl &) = delete;
};

/// Represents the style of a paragraph.
class SW_DLLPUBLIC SwTextFormatColl: public SwFormatColl
{
    friend class SwDoc;
    friend class ::sw::DocumentStylePoolManager;

    SwTextFormatColl(const SwTextFormatColl & rRef) = delete;

    bool mbStayAssignedToListLevelOfOutlineStyle;

    bool mbAssignedToOutlineStyle;

    SwTextFormatColl *mpNextTextFormatColl;

protected:

    SwTextFormatColl( SwAttrPool& rPool, const sal_Char* pFormatCollName,
                    SwTextFormatColl* pDerFrom = nullptr,
                    sal_uInt16 nFormatWh = RES_TXTFMTCOLL )
        : SwFormatColl(rPool, pFormatCollName, aTextFormatCollSetRange, pDerFrom, nFormatWh)
        , mbStayAssignedToListLevelOfOutlineStyle(false)
        , mbAssignedToOutlineStyle(false)
    {
        mpNextTextFormatColl = this;
    }

    SwTextFormatColl( SwAttrPool& rPool, const OUString &rFormatCollName,
                    SwTextFormatColl* pDerFrom,
                    sal_uInt16 nFormatWh = RES_TXTFMTCOLL )
        : SwFormatColl(rPool, rFormatCollName, aTextFormatCollSetRange, pDerFrom, nFormatWh)
        , mbStayAssignedToListLevelOfOutlineStyle(false)
        , mbAssignedToOutlineStyle(false)
    {
        mpNextTextFormatColl = this;
    }

    /// To get UL- / LR- / FontHeight-changes.
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:


    inline void SetNextTextFormatColl(SwTextFormatColl& rNext);
    SwTextFormatColl& GetNextTextFormatColl() const { return *mpNextTextFormatColl; }

    bool IsAtDocNodeSet() const;

    void SetAttrOutlineLevel( int );
    int  GetAttrOutlineLevel() const;

    // Return the list level of the Outline Style - the List Style for the
    // outline numbering -
    // to which the Paragraph Style is assigned.
    int  GetAssignedOutlineStyleLevel() const;

    bool IsAssignedToListLevelOfOutlineStyle() const
    {
        return mbAssignedToOutlineStyle;
    }

    // If a Paragraph Style is assigned to list level N of the Outline Style,
    // then its outline level - AttrOutlineLevel - is set to N+1
    void AssignToListLevelOfOutlineStyle(const int nAssignedListLevel);
    void DeleteAssignmentToListLevelOfOutlineStyle();

    /** Override to recognize changes on the <SwNumRuleItem> and register/unregister
     the paragragh style at the corresponding <SwNumRule> instance. */
    virtual bool SetFormatAttr( const SfxPoolItem& rAttr ) override;
    virtual bool SetFormatAttr( const SfxItemSet& rSet ) override;
    virtual bool ResetFormatAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 ) override;

    /// Override <ResetAllFormatAttr()> to stay assigned to list level of outline style.
    virtual sal_uInt16 ResetAllFormatAttr() override;

    bool StayAssignedToListLevelOfOutlineStyle() const
    {
        return mbStayAssignedToListLevelOfOutlineStyle;
    }

    bool AreListLevelIndentsApplicable() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SwGrfFormatColl final : public SwFormatColl
{
    friend class SwDoc;

    SwGrfFormatColl( SwAttrPool& rPool, const sal_Char* pFormatCollName,
                    SwGrfFormatColl* pDerFrom = nullptr )
        : SwFormatColl( rPool, pFormatCollName, aGrfFormatCollSetRange,
                    pDerFrom, RES_GRFFMTCOLL )
    {}

    SwGrfFormatColl( SwAttrPool& rPool, const OUString &rFormatCollName,
                    SwGrfFormatColl* pDerFrom )
        : SwFormatColl( rPool, rFormatCollName, aGrfFormatCollSetRange,
                    pDerFrom, RES_GRFFMTCOLL )
    {}
};

// FEATURE::CONDCOLL
/// Conditional styles.
enum class Master_CollCondition
{
    NONE,
    PARA_IN_LIST,
    PARA_IN_OUTLINE,
    PARA_IN_FRAME,
    PARA_IN_TABLEHEAD,
    PARA_IN_TABLEBODY,
    PARA_IN_SECTION,
    PARA_IN_FOOTNOTE,
    PARA_IN_FOOTER,
    PARA_IN_HEADER,
    PARA_IN_ENDNOTE
};

class SW_DLLPUBLIC SwCollCondition : public SwClient
{
    Master_CollCondition m_nCondition;
    sal_uLong m_nSubCondition;

public:

    SwCollCondition( SwTextFormatColl* pColl, Master_CollCondition nMasterCond,
                    sal_uLong nSubCond );
    virtual ~SwCollCondition() override;

    /// @@@ public copy ctor, but no copy assignment?
    SwCollCondition( const SwCollCondition& rCpy );
private:
    /// @@@ public copy ctor, but no copy assignment?
    SwCollCondition & operator= (const SwCollCondition &) = delete;
public:

    bool operator==( const SwCollCondition& rCmp ) const;

    Master_CollCondition GetCondition() const      { return m_nCondition; }
    sal_uLong GetSubCondition() const   { return m_nSubCondition; }

    void SetCondition( Master_CollCondition nCond, sal_uLong nSubCond );
    SwTextFormatColl* GetTextFormatColl() const     { return const_cast<SwTextFormatColl*>(static_cast<const SwTextFormatColl*>(GetRegisteredIn())); }
    void RegisterToFormat( SwFormat& );
};

using SwFormatCollConditions = std::vector<std::unique_ptr<SwCollCondition>>;

class SW_DLLPUBLIC SwConditionTextFormatColl final : public SwTextFormatColl
{
    friend class SwDoc;
    friend class ::sw::DocumentStylePoolManager;

    SwFormatCollConditions m_CondColls;

    SwConditionTextFormatColl( SwAttrPool& rPool, const OUString &rFormatCollName,
                            SwTextFormatColl* pDerFrom )
        : SwTextFormatColl( rPool, rFormatCollName, pDerFrom, RES_CONDTXTFMTCOLL )
    {}

public:

    virtual ~SwConditionTextFormatColl() override;

    const SwCollCondition* HasCondition( const SwCollCondition& rCond ) const;
    const SwFormatCollConditions& GetCondColls() const { return m_CondColls; }
    void InsertCondition( const SwCollCondition& rCond );
    void RemoveCondition( const SwCollCondition& rCond );

    void SetConditions( const SwFormatCollConditions& );
};

// FEATURE::CONDCOLL
/// Inline implementations.
inline void SwTextFormatColl::SetNextTextFormatColl( SwTextFormatColl& rNext )
{
    mpNextTextFormatColl = &rNext;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
