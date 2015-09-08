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
#include <vector>

class SwDoc;
class SwDocStyleSheetPool;
class SwPageDesc;
class SwCharFormat;
class SwTextFormatColl;
class SwFrameFormat;
class SwNumRule;

// Temporary StyleSheet.
class SW_DLLPUBLIC SwDocStyleSheet : public SfxStyleSheetBase
{
    friend class SwDocStyleSheetPool;
    friend class SwStyleSheetIterator;

    SwCharFormat*          pCharFormat;
    SwTextFormatColl*       pColl;
    SwFrameFormat*           pFrameFormat;
    const SwPageDesc*   pDesc;
    const SwNumRule*    pNumRule;

    SwDoc&              rDoc;
    SfxItemSet          aCoreSet;

    bool                bPhysical;

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
            std::unique_ptr<SfxItemSet> * o_ppFlatSet = nullptr);

protected:
    virtual ~SwDocStyleSheet();

public:
    SwDocStyleSheet( SwDoc&                 rDoc,
                     const OUString&        rName,
                     SwDocStyleSheetPool*   pPool,
                     SfxStyleFamily         eFam,
                     sal_uInt16             nMask);

    SwDocStyleSheet( const SwDocStyleSheet& );

    void                    Reset();

    void                    SetMask(sal_uInt16 nMsk)            { nMask = nMsk;     }
    void                    SetFamily(SfxStyleFamily eFam)  { nFamily = eFam;   }

    bool                    IsPhysical() const              { return bPhysical; }
    void                    SetPhysical(bool bPhys);

    virtual void            SetHidden( bool bHidden ) SAL_OVERRIDE;
    virtual bool            IsHidden( ) const SAL_OVERRIDE;
    void SetGrabBagItem(const com::sun::star::uno::Any& rVal);
    void GetGrabBagItem(com::sun::star::uno::Any& rVal) const;

    /** add optional parameter <bResetIndentAttrsAtParagraphStyle>, default value false,
     which indicates that the indent attributes at a paragraph style should
     be reset in case that a list style is applied to the paragraph style and
     no indent attributes are applied. */
    void                    SetItemSet( const SfxItemSet& rSet,
                                        const bool bResetIndentAttrsAtParagraphStyle = false );

    virtual SfxItemSet&     GetItemSet() SAL_OVERRIDE;
    virtual std::unique_ptr<SfxItemSet> GetItemSetForPreview() SAL_OVERRIDE;
    /** new method for paragraph styles to merge indent attributes of applied list
     style into the given item set, if the list style indent attributes are applicable. */
    void MergeIndentAttrsOfListStyle( SfxItemSet& rSet );
    virtual const OUString& GetParent() const SAL_OVERRIDE;
    virtual const OUString& GetFollow() const SAL_OVERRIDE;

    virtual sal_uLong GetHelpId( OUString& rFile ) SAL_OVERRIDE;
    virtual void SetHelpId( const OUString& r, sal_uLong nId ) SAL_OVERRIDE;

    /** Preset the members without physical access.
     Used by StyleSheetPool. */
    void                    PresetName(const OUString& rName)  { aName   = rName; }
    void                    PresetNameAndFamily(const OUString& rName);
    void                    PresetParent(const OUString& rName){ aParent = rName; }
    void                    PresetFollow(const OUString& rName){ aFollow = rName; }

    virtual bool            SetName(const OUString& rNewName, bool bReindexNow = true) SAL_OVERRIDE;
    virtual bool            SetParent( const OUString& rStr) SAL_OVERRIDE;
    virtual bool            SetFollow( const OUString& rStr) SAL_OVERRIDE;

    virtual bool            HasFollowSupport() const SAL_OVERRIDE;
    virtual bool            HasParentSupport() const SAL_OVERRIDE;
    virtual bool            HasClearParentSupport() const SAL_OVERRIDE;
    virtual OUString        GetDescription() SAL_OVERRIDE;
    virtual OUString        GetDescription(SfxMapUnit eUnit) SAL_OVERRIDE;

    SwCharFormat*              GetCharFormat();
    SwTextFormatColl*           GetCollection();
    SwFrameFormat*               GetFrameFormat();
    const SwPageDesc*       GetPageDesc();
    const SwNumRule*        GetNumRule();
    void                    SetNumRule(const SwNumRule& rRule);

    virtual bool            IsUsed() const SAL_OVERRIDE;
};

// Iterator for Pool.
class SwStyleSheetIterator : public SfxStyleSheetIterator, public SfxListener
{
    // Local helper class.
    class SwPoolFormatList
    {
        std::vector<OUString> maImpl;
        typedef std::unordered_map<OUString, sal_uInt32, OUStringHash> UniqueHash;
        UniqueHash maUnique;
        void rehash();
    public:
        SwPoolFormatList() {}
        void Append( char cChar, const OUString& rStr );
        void clear() { maImpl.clear(); maUnique.clear(); }
        size_t size() { return maImpl.size(); }
        bool empty() { return maImpl.empty(); }
        sal_uInt32 FindName(SfxStyleFamily eFam, const OUString &rName);
        void RemoveName(SfxStyleFamily eFam, const OUString &rName);
        const OUString &operator[](sal_uInt32 nIdx) { return maImpl[ nIdx ]; }
    };

    rtl::Reference< SwDocStyleSheet > mxIterSheet;
    rtl::Reference< SwDocStyleSheet > mxStyleSheet;
    SwPoolFormatList       aLst;
    sal_uInt32          nLastPos;
    bool                bFirstCalled;

    void                AppendStyleList(const ::std::vector<OUString>& rLst,
                                        bool        bUsed,
                                        bool        bTestHidden,
                                        bool        bOnlyHidden,
                                        sal_uInt16  nSection,
                                        char        cType);

public:
    SwStyleSheetIterator( SwDocStyleSheetPool* pBase,
                          SfxStyleFamily eFam, sal_uInt16 n=SFXSTYLEBIT_ALL );
    virtual ~SwStyleSheetIterator();

    virtual sal_uInt16 Count() SAL_OVERRIDE;
    virtual SfxStyleSheetBase *operator[](sal_uInt16 nIdx) SAL_OVERRIDE;
    virtual SfxStyleSheetBase* First() SAL_OVERRIDE;
    virtual SfxStyleSheetBase* Next() SAL_OVERRIDE;
    virtual SfxStyleSheetBase* Find(const OUString& rStr) SAL_OVERRIDE;

    virtual void Notify( SfxBroadcaster&, const SfxHint& ) SAL_OVERRIDE;

    void InvalidateIterator();
};

class SwDocStyleSheetPool : public SfxStyleSheetBasePool
{
    rtl::Reference< SwDocStyleSheet > mxStyleSheet;
    SwDoc&              rDoc;
    bool                bOrganizer : 1;     ///< Organizer

    virtual SfxStyleSheetBase* Create( const OUString&, SfxStyleFamily, sal_uInt16 nMask) SAL_OVERRIDE;
    virtual SfxStyleSheetBase* Create( const SfxStyleSheetBase& ) SAL_OVERRIDE;

    using SfxStyleSheetBasePool::Find;

public:
    SwDocStyleSheetPool( SwDoc&, bool bOrganizer = false );

    virtual void Replace( SfxStyleSheetBase& rSource,
                          SfxStyleSheetBase& rTarget ) SAL_OVERRIDE;
    virtual SfxStyleSheetBase& Make(const OUString&, SfxStyleFamily,
            sal_uInt16 nMask) SAL_OVERRIDE;

    virtual SfxStyleSheetBase* Find( const OUString&, SfxStyleFamily eFam,
                                    sal_uInt16 n=SFXSTYLEBIT_ALL ) SAL_OVERRIDE;

    virtual bool SetParent( SfxStyleFamily eFam, const OUString &rStyle,
                            const OUString &rParent ) SAL_OVERRIDE;

    virtual void Remove( SfxStyleSheetBase* pStyle) SAL_OVERRIDE;

    void    SetOrganizerMode( bool bMode )  { bOrganizer = bMode; }
    bool    IsOrganizerMode() const         { return bOrganizer; }

    virtual SfxStyleSheetIteratorPtr CreateIterator( SfxStyleFamily, sal_uInt16 nMask ) SAL_OVERRIDE;

    SwDoc& GetDoc() const { return rDoc; }

    void dispose();

    virtual void SAL_CALL acquire(  ) throw () SAL_OVERRIDE;
    virtual void SAL_CALL release(  ) throw () SAL_OVERRIDE;

    void InvalidateIterator();

protected:
    virtual ~SwDocStyleSheetPool();

private:
    SwDocStyleSheetPool( const SwDocStyleSheetPool& ) SAL_DELETED_FUNCTION;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
