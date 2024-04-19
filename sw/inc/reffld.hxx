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
#ifndef INCLUDED_SW_INC_REFFLD_HXX
#define INCLUDED_SW_INC_REFFLD_HXX

#include <tools/solar.h>

#include "fldbas.hxx"

class SfxPoolItem;
class SwDoc;
class SwTextNode;
class SwContentFrame;
class SwTextField;
class SwRootFrame;
class SwFrame;

bool IsFrameBehind( const SwTextNode& rMyNd, sal_Int32 nMySttPos,
                    const SwTextNode& rBehindNd, sal_Int32 nSttPos );

#define REFFLDFLAG          0x4000
#define REFFLDFLAG_BOOKMARK 0x4800
#define REFFLDFLAG_FOOTNOTE 0x5000
#define REFFLDFLAG_ENDNOTE  0x6000
// #i83479#
#define REFFLDFLAG_HEADING  0x7100
#define REFFLDFLAG_NUMITEM  0x7200

#define REFFLDFLAG_STYLE    0xc000
/* we skip past 0x8000, 0x9000, 0xa000 and 0xb000 as when we bitwise 'and'
       with REFFLDFLAG they are false */
#define REFFLDFLAG_STYLE_FROM_BOTTOM           0xc100
#define REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL    0xc200

enum REFERENCESUBTYPE
{
    REF_SETREFATTR = 0,
    REF_SEQUENCEFLD,
    REF_BOOKMARK,
    REF_OUTLINE,
    REF_FOOTNOTE,
    REF_ENDNOTE,
    REF_STYLE
};

enum REFERENCEMARK
{
    REF_BEGIN,
    REF_PAGE = REF_BEGIN, ///< "Page"
    REF_CHAPTER,          ///< "Chapter"
    REF_CONTENT,          ///< "Reference"
    REF_UPDOWN,           ///< "Above/Below"
    REF_PAGE_PGDESC,      ///< "As Page Style"
    REF_ONLYNUMBER,       ///< "Category and Number"
    REF_ONLYCAPTION,      ///< "Caption Text"
    REF_ONLYSEQNO,        ///< "Numbering"
    // --> #i81002#
    /// new reference format types for referencing bookmarks and set references
    REF_NUMBER,              ///< "Number"
    REF_NUMBER_NO_CONTEXT,   ///< "Number (no context)"
    REF_NUMBER_FULL_CONTEXT, ///< "Number (full context)"
};

/// Get reference.

class SAL_DLLPUBLIC_RTTI SwGetRefFieldType final : public SwFieldType
{
    SwDoc& m_rDoc;

    /// Overlay in order to update all ref-fields.
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;
public:
    SwGetRefFieldType(SwDoc& rDoc );
    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual void UpdateFields() override {};

    SwDoc&                  GetDoc() const { return m_rDoc; }

    void MergeWithOtherDoc( SwDoc& rDestDoc );

    static SwTextNode* FindAnchor( SwDoc* pDoc, const OUString& rRefMark,
                                        sal_uInt16 nSubType, sal_uInt16 nSeqNo, sal_uInt16 nFlags,
                                        sal_Int32* pStt, sal_Int32* pEnd = nullptr,
                                        SwRootFrame const* pLayout = nullptr,
                                        SwTextNode* pSelf = nullptr, SwFrame* pFrame = nullptr);
    void UpdateGetReferences();
    void UpdateStyleReferences();
};

class SAL_DLLPUBLIC_RTTI SwGetRefField final : public SwField
{
private:
    OUString m_sSetRefName;
    OUString m_sSetReferenceLanguage;
    OUString m_sText;         ///< result
    OUString m_sTextRLHidden; ///< result for layout with redlines hidden
    sal_uInt16 m_nSubType;
    /// reference to either a SwTextFootnote::m_nSeqNo or a SwSetExpField::mnSeqNo
    sal_uInt16 m_nSeqNo;
    sal_uInt16 m_nFlags;

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;
public:
    SW_DLLPUBLIC SwGetRefField( SwGetRefFieldType*, OUString aSetRef, OUString aReferenceLanguage,
                    sal_uInt16 nSubType, sal_uInt16 nSeqNo, sal_uInt16 nFlags, sal_uLong nFormat );

    SW_DLLPUBLIC virtual ~SwGetRefField() override;

    virtual OUString GetFieldName() const override;

    const OUString& GetSetRefName() const { return m_sSetRefName; }

    // #i81002#
    /** The <SwTextField> instance, which represents the text attribute for the
       <SwGetRefField> instance, has to be passed to the method.
       This <SwTextField> instance is needed for the reference format type REF_UPDOWN, REF_NUMBER
       and REF_STYLE.
       Note: This instance may be NULL (field in Undo/Redo). This will cause
       no update for these reference format types. */
    void                UpdateField( const SwTextField* pFieldTextAttr, SwFrame* pFrame );
    void                UpdateField( const SwTextField* pFieldTextAttr, SwFrame* pFrame,
                                     const SwRootFrame* const pLayout, OUString& rText );

    void                SetExpand( const OUString& rStr );

    /// Get/set sub type.
    virtual sal_uInt16      GetSubType() const override;
    SW_DLLPUBLIC virtual void SetSubType( sal_uInt16 n ) override;

    // --> #i81002#
    SW_DLLPUBLIC bool IsRefToHeadingCrossRefBookmark() const;
    SW_DLLPUBLIC bool IsRefToNumItemCrossRefBookmark() const;
    SW_DLLPUBLIC const SwTextNode* GetReferencedTextNode(SwTextNode* pTextNode, SwFrame* pFrame) const;
    // #i85090#
    SW_DLLPUBLIC OUString GetExpandedTextOfReferencedTextNode(SwRootFrame const& rLayout) const;

    /// Get/set SequenceNo (of interest only for REF_SEQUENCEFLD).
    sal_uInt16              GetSeqNo() const        { return m_nSeqNo; }
    void                SetSeqNo( sal_uInt16 n )    { m_nSeqNo = n; }

    /// Get/set flags (currently only used for REF_STYLE)
    sal_uInt16              GetFlags() const        { return m_nFlags; }
    void                SetFlags( sal_uInt16 n )    { m_nFlags = n; }

    // Name of reference.
    SW_DLLPUBLIC virtual OUString GetPar1() const override;
    virtual void        SetPar1(const OUString& rStr) override;

    SW_DLLPUBLIC virtual OUString GetPar2() const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId ) override;

    void                ConvertProgrammaticToUIName();

    virtual OUString    GetDescription() const override;
};

#endif /// INCLUDED_SW_INC_REFFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
