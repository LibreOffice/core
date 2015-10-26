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
#include <frmatr.hxx>
#include <swtypes.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <memory>

class SwDoc;
namespace sw{ class DocumentStylePoolManager; }

class SwFormatColl : public SwFormat
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

protected:

    bool mbAssignedToOutlineStyle;

    SwTextFormatColl *mpNextTextFormatColl;

    SwTextFormatColl( SwAttrPool& rPool, const sal_Char* pFormatCollName,
                    SwTextFormatColl* pDerFrom = 0,
                    sal_uInt16 nFormatWh = RES_TXTFMTCOLL )
        : SwFormatColl(rPool, pFormatCollName, aTextFormatCollSetRange, pDerFrom, nFormatWh)
        , mbStayAssignedToListLevelOfOutlineStyle(false)
        , mbAssignedToOutlineStyle(false)
    {
        mpNextTextFormatColl = this;
    }

    SwTextFormatColl( SwAttrPool& rPool, const OUString &rFormatCollName,
                    SwTextFormatColl* pDerFrom = 0,
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

    TYPEINFO_OVERRIDE(); ///< Already in base class Client.

    inline void SetNextTextFormatColl(SwTextFormatColl& rNext);
    SwTextFormatColl& GetNextTextFormatColl() const { return *mpNextTextFormatColl; }

    bool IsAtDocNodeSet() const;

    void SetAttrOutlineLevel( int );
    int  GetAttrOutlineLevel() const;

    // Return the list level of the Outline Style - the List Style for the
    // outline numbering -
    // to which the Paragraph Style is assigned.
    int  GetAssignedOutlineStyleLevel() const;

    inline bool IsAssignedToListLevelOfOutlineStyle() const
    {
        return mbAssignedToOutlineStyle;
    }

    // If a Paragraph Style is assigned to list level N of the Outline Style,
    // then its outline level - AttrOutlineLevel - is set to N+1
    void AssignToListLevelOfOutlineStyle(const int nAssignedListLevel);
    void DeleteAssignmentToListLevelOfOutlineStyle(const bool bResetOutlineLevel
                                                   = true);

    /** Override to recognize changes on the <SwNumRuleItem> and register/unregister
     the paragragh style at the corresponding <SwNumRule> instance. */
    virtual bool SetFormatAttr( const SfxPoolItem& rAttr ) override;
    virtual bool SetFormatAttr( const SfxItemSet& rSet ) override;
    virtual bool ResetFormatAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 ) override;

    /// Override <ResetAllFormatAttr()> to stay assigned to list level of outline style.
    virtual sal_uInt16 ResetAllFormatAttr() override;

    inline bool StayAssignedToListLevelOfOutlineStyle() const
    {
        return mbStayAssignedToListLevelOfOutlineStyle;
    }

    bool AreListLevelIndentsApplicable() const;

    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

class SwGrfFormatColl: public SwFormatColl
{
    friend class SwDoc;
protected:
    SwGrfFormatColl( SwAttrPool& rPool, const sal_Char* pFormatCollName,
                    SwGrfFormatColl* pDerFrom = 0 )
        : SwFormatColl( rPool, pFormatCollName, aGrfFormatCollSetRange,
                    pDerFrom, RES_GRFFMTCOLL )
    {}

    SwGrfFormatColl( SwAttrPool& rPool, const OUString &rFormatCollName,
                    SwGrfFormatColl* pDerFrom = 0 )
        : SwFormatColl( rPool, rFormatCollName, aGrfFormatCollSetRange,
                    pDerFrom, RES_GRFFMTCOLL )
    {}

public:
    TYPEINFO_OVERRIDE(); ///< Already in base class Client.
};

// FEATURE::CONDCOLL
/// Conditional styles.
enum Master_CollConditions
{
    PARA_IN_LIST        = 0x0001,
    PARA_IN_OUTLINE     = 0x0002,
    PARA_IN_FRAME       = 0x0004,
    PARA_IN_TABLEHEAD   = 0x0008,
    PARA_IN_TABLEBODY   = 0x0010,
    PARA_IN_SECTION     = 0x0020,
    PARA_IN_FOOTENOTE   = 0x0040,
    PARA_IN_FOOTER      = 0x0080,
    PARA_IN_HEADER      = 0x0100,
    PARA_IN_ENDNOTE     = 0x0200,

    USRFLD_EXPRESSION   = (int)0x8000
};

class SW_DLLPUBLIC SwCollCondition : public SwClient
{
    sal_uLong nCondition;
    union
    {
        sal_uLong nSubCondition;
        OUString* pFieldExpression;
    } aSubCondition;

public:
    TYPEINFO_OVERRIDE(); ///< Already in base class Client.

    SwCollCondition( SwTextFormatColl* pColl, sal_uLong nMasterCond,
                    sal_uLong nSubCond = 0 );
    SwCollCondition( SwTextFormatColl* pColl, sal_uLong nMasterCond,
                    const OUString& rSubExp );
    virtual ~SwCollCondition();

    /// @@@ public copy ctor, but no copy assignment?
    SwCollCondition( const SwCollCondition& rCpy );
private:
    /// @@@ public copy ctor, but no copy assignment?
    SwCollCondition & operator= (const SwCollCondition &) = delete;
public:

    bool operator==( const SwCollCondition& rCmp ) const;
    bool operator!=( const SwCollCondition& rCmp ) const
                            { return ! (*this == rCmp); }

    sal_uLong GetCondition() const      { return nCondition; }
    sal_uLong GetSubCondition() const   { return aSubCondition.nSubCondition; }
    const OUString* GetFieldExpression() const
                                    { return aSubCondition.pFieldExpression; }

    void SetCondition( sal_uLong nCond, sal_uLong nSubCond );
    SwTextFormatColl* GetTextFormatColl() const     { return const_cast<SwTextFormatColl*>(static_cast<const SwTextFormatColl*>(GetRegisteredIn())); }
    void RegisterToFormat( SwFormat& );
};

class SwFormatCollConditions : public std::vector<std::unique_ptr<SwCollCondition>> {};

class SW_DLLPUBLIC SwConditionTextFormatColl : public SwTextFormatColl
{
    friend class SwDoc;
    friend class ::sw::DocumentStylePoolManager;
protected:
    SwFormatCollConditions m_CondColls;

    SwConditionTextFormatColl( SwAttrPool& rPool, const sal_Char* pFormatCollName,
                            SwTextFormatColl* pDerFrom = 0 )
        : SwTextFormatColl( rPool, pFormatCollName, pDerFrom, RES_CONDTXTFMTCOLL )
    {}
    SwConditionTextFormatColl( SwAttrPool& rPool, const OUString &rFormatCollName,
                            SwTextFormatColl* pDerFrom = 0 )
        : SwTextFormatColl( rPool, rFormatCollName, pDerFrom, RES_CONDTXTFMTCOLL )
    {}

public:
    TYPEINFO_OVERRIDE(); ///< Already in base class Client.

    virtual ~SwConditionTextFormatColl();

    const SwCollCondition* HasCondition( const SwCollCondition& rCond ) const;
    const SwFormatCollConditions& GetCondColls() const { return m_CondColls; }
    void InsertCondition( const SwCollCondition& rCond );
    bool RemoveCondition( const SwCollCondition& rCond );

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
