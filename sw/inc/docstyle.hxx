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
#ifndef INCLUDED_SW_INC_DOCSTYLE_HXX
#define INCLUDED_SW_INC_DOCSTYLE_HXX

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <svl/style.hxx>
#include <svl/itemset.hxx>
#include "swdllapi.h"

#include <unordered_map>
#include <memory>
#include <optional>
#include <vector>

class SwDoc;
class SwDocStyleSheetPool;
class SwPageDesc;
class SwCharFormat;
class SwTextFormatColl;
class SwFrameFormat;
class SwNumRule;
class SwTableAutoFormat;
class SwBoxAutoFormat;
enum class SwGetPoolIdFromName : sal_uInt16;

// Temporary StyleSheet.
class SAL_DLLPUBLIC_RTTI SwDocStyleSheet final : public SfxStyleSheetBase
{
    friend class SwDocStyleSheetPool;
    friend class SwStyleSheetIterator;

    SwCharFormat*          m_pCharFormat;
    SwTextFormatColl*       m_pColl;
    SwFrameFormat*           m_pFrameFormat;
    const SwPageDesc*   m_pDesc;
    const SwNumRule*    m_pNumRule;
    SwTableAutoFormat*  m_pTableFormat;
    const SwBoxAutoFormat*   m_pBoxFormat;
    SwDoc&              m_rDoc;
    SfxItemSet          m_aCoreSet;

    bool                m_bPhysical;
    OUString m_aLink;

    /// Make empty shell a real StyleSheet (Core).
    SAL_DLLPRIVATE void              Create();

    /// Fill StyleSheet with data.
    enum FillStyleType {
        FillOnlyName,
        FillAllInfo,
        FillPhysical,
        FillPreview,
    };

    SAL_DLLPRIVATE bool FillStyleSheet(FillStyleType eFType,
            std::optional<SfxItemSet> * o_ppFlatSet = nullptr);

    virtual ~SwDocStyleSheet() override;

public:
    SwDocStyleSheet( SwDoc&                 rDoc,
                     SwDocStyleSheetPool&   rPool);

    SW_DLLPUBLIC SwDocStyleSheet( const SwDocStyleSheet& );

    void                    Reset();

    void                    SetFamily(SfxStyleFamily eFam)  { nFamily = eFam;   }

    bool                    IsPhysical() const              { return m_bPhysical; }
    void                    SetPhysical(bool bPhys);

    virtual void            SetHidden( bool bHidden ) override;
    virtual bool            IsHidden( ) const override;
    void SetGrabBagItem(const css::uno::Any& rVal);
    void GetGrabBagItem(css::uno::Any& rVal) const;

    /** add optional parameter <bResetIndentAttrsAtParagraphStyle>, default value false,
     which indicates that the indent attributes at a paragraph style should
     be reset in case that a list style is applied to the paragraph style and
     no indent attributes are applied. */
    SW_DLLPUBLIC void       SetItemSet( const SfxItemSet& rSet, const bool bBroadcast = true,
                                        const bool bResetIndentAttrsAtParagraphStyle = false );

    SW_DLLPUBLIC virtual SfxItemSet& GetItemSet() override;
    virtual std::optional<SfxItemSet> GetItemSetForPreview() override;
    /** new method for paragraph styles to merge indent attributes of applied list
     style into the given item set, if the list style indent attributes are applicable. */
    SW_DLLPUBLIC void MergeIndentAttrsOfListStyle( SfxItemSet& rSet );
    virtual const OUString& GetParent() const override;
    virtual const OUString& GetFollow() const override;
    const OUString& GetLink() const;

    virtual sal_uInt32 GetHelpId( OUString& rFile ) override;
    virtual void SetHelpId( const OUString& r, sal_uInt32 nId ) override;

    /** Preset the members without physical access.
     Used by StyleSheetPool. */
    void                    PresetName(const OUString& rName)  { aName   = rName; }
    void                    PresetNameAndFamily(SfxStyleFamily eFamily, const OUString& rName);
    void                    PresetParent(const OUString& rName){ aParent = rName; }
    void                    PresetFollow(const OUString& rName){ aFollow = rName; }

    virtual bool            SetName(const OUString& rNewName, bool bReindexNow = true) override;
    virtual bool            SetParent( const OUString& rStr) override;
    virtual bool            SetFollow( const OUString& rStr) override;
    void SetLink(const OUString& rStr);

    virtual bool            HasFollowSupport() const override;
    virtual bool            HasParentSupport() const override;
    virtual bool            HasClearParentSupport() const override;
    virtual OUString        GetDescription(MapUnit eUnit) override;

    virtual OUString GetUsedBy() override;

    SW_DLLPUBLIC SwCharFormat* GetCharFormat();
    SW_DLLPUBLIC SwTextFormatColl* GetCollection();
    SwFrameFormat*               GetFrameFormat();
    const SwPageDesc*       GetPageDesc();
    const SwNumRule*        GetNumRule();
    void                    SetNumRule(const SwNumRule& rRule);
    SwTableAutoFormat*      GetTableFormat();

    virtual bool            IsUsed() const override;
};

namespace std {
template<>
struct hash<std::pair<SfxStyleFamily,OUString>>
{
    std::size_t operator()(std::pair<SfxStyleFamily,OUString> const & pair) const
    { return static_cast<std::size_t>(pair.first) ^ std::size_t(pair.second.hashCode()); }
};
}


// Iterator for Pool.
class SwStyleSheetIterator final : public SfxStyleSheetIterator, public SfxListener
{
    // Local helper class.
    class SwPoolFormatList
    {
        std::vector<std::pair<SfxStyleFamily, OUString>> maImpl;
        typedef std::unordered_map<std::pair<SfxStyleFamily, OUString>, sal_uInt32> UniqueHash;
        UniqueHash maUnique;
        void rehash();
    public:
        SwPoolFormatList() {}
        void Append( SfxStyleFamily eFam, const OUString& rStr );
        void clear() { maImpl.clear(); maUnique.clear(); }
        size_t size() { return maImpl.size(); }
        bool empty() { return maImpl.empty(); }
        sal_uInt32 FindName(SfxStyleFamily eFam, const OUString& rName);
        void RemoveName(SfxStyleFamily eFam, const OUString& rName);
        const std::pair<SfxStyleFamily,OUString> &operator[](sal_uInt32 nIdx) { return maImpl[ nIdx ]; }
    };

    rtl::Reference< SwDocStyleSheet > mxIterSheet;
    rtl::Reference< SwDocStyleSheet > mxStyleSheet;
    SwPoolFormatList       m_aLst;
    sal_uInt32          m_nLastPos;
    bool                m_bFirstCalled;

    bool IsUsedInComments(const OUString& rName) const;
    void                AppendStyleList(const std::vector<OUString>& rLst,
                                        bool        bUsed,
                                        bool        bTestHidden,
                                        bool        bOnlyHidden,
                                        SwGetPoolIdFromName nSection,
                                        SfxStyleFamily eFamily);

public:
    SwStyleSheetIterator(SwDocStyleSheetPool& rBase,
                         SfxStyleFamily eFam, SfxStyleSearchBits n);
    virtual ~SwStyleSheetIterator() override;

    virtual sal_Int32 Count() override;
    virtual SfxStyleSheetBase *operator[](sal_Int32 nIdx) override;
    virtual SfxStyleSheetBase* First() override;
    virtual SfxStyleSheetBase* Next() override;
    virtual SfxStyleSheetBase* Find(const OUString& rStr) override;

    virtual void Notify( SfxBroadcaster&, const SfxHint& ) override;

    void InvalidateIterator();
};

class SwDocStyleSheetPool final : public SfxStyleSheetBasePool
{
    rtl::Reference< SwDocStyleSheet > mxStyleSheet;
    rtl::Reference< SfxStyleSheetPool > mxEEStyleSheetPool;
    SwDoc&              m_rDoc;
    bool                m_bOrganizer : 1;     ///< Organizer

    virtual rtl::Reference<SfxStyleSheetBase> Create( const OUString&, SfxStyleFamily, SfxStyleSearchBits nMask) override;
    virtual rtl::Reference<SfxStyleSheetBase> Create( const SfxStyleSheetBase& ) override;

    using SfxStyleSheetBasePool::Find;

public:
    SwDocStyleSheetPool( SwDoc&, bool bOrganizer );

    virtual SfxStyleSheetBase& Make(const OUString&, SfxStyleFamily,
            SfxStyleSearchBits nMask = SfxStyleSearchBits::All) override;

    virtual SfxStyleSheetBase* Find( const OUString&, SfxStyleFamily eFam,
                                    SfxStyleSearchBits n=SfxStyleSearchBits::All ) override;

    virtual void Remove( SfxStyleSheetBase* pStyle) override;

    bool    IsOrganizerMode() const         { return m_bOrganizer; }

    virtual std::unique_ptr<SfxStyleSheetIterator> CreateIterator( SfxStyleFamily, SfxStyleSearchBits nMask = SfxStyleSearchBits::All) override;

    SwDoc& GetDoc() const { return m_rDoc; }
    SfxStyleSheetPool* GetEEStyleSheetPool() const { return mxEEStyleSheetPool.get(); }

    void dispose();

    void InvalidateIterator();

private:
    virtual ~SwDocStyleSheetPool() override;

    SwDocStyleSheetPool( const SwDocStyleSheetPool& ) = delete;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
