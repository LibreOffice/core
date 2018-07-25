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

#include "fldbas.hxx"

class SfxPoolItem;
class SwDoc;
class SwTextNode;
class SwTextField;

bool IsFrameBehind( const SwTextNode& rMyNd, sal_Int32 nMySttPos,
                    const SwTextNode& rBehindNd, sal_Int32 nSttPos );

enum REFERENCESUBTYPE
{
    REF_SETREFATTR = 0,
    REF_SEQUENCEFLD,
    REF_BOOKMARK,
    REF_OUTLINE,
    REF_FOOTNOTE,
    REF_ENDNOTE
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

class SwGetRefFieldType : public SwFieldType
{
    SwDoc* m_pDoc;
protected:
    /// Overlay in order to update all ref-fields.
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
public:
    SwGetRefFieldType(SwDoc* pDoc );
    virtual SwFieldType*    Copy() const override;

    SwDoc*                  GetDoc() const { return m_pDoc; }

    void MergeWithOtherDoc( SwDoc& rDestDoc );

    static SwTextNode* FindAnchor( SwDoc* pDoc, const OUString& rRefMark,
                                        sal_uInt16 nSubType, sal_uInt16 nSeqNo,
                                        sal_Int32* pStt, sal_Int32* pEnd = nullptr );
};

class SW_DLLPUBLIC SwGetRefField : public SwField
{
private:
    OUString m_sSetRefName;
    OUString m_sSetReferenceLanguage;
    OUString m_sText;
    sal_uInt16 m_nSubType;
    sal_uInt16 m_nSeqNo;

    virtual OUString    Expand() const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    // #i81002#
    static OUString MakeRefNumStr( const SwTextNode& rTextNodeOfField,
                          const SwTextNode& rTextNodeOfReferencedItem,
                          const sal_uInt32 nRefNumFormat );

public:
    SwGetRefField( SwGetRefFieldType*, const OUString& rSetRef, const OUString& rReferenceLanguage,
                    sal_uInt16 nSubType, sal_uInt16 nSeqNo, sal_uLong nFormat );

    virtual ~SwGetRefField() override;

    virtual OUString GetFieldName() const override;

    const OUString& GetSetRefName() const { return m_sSetRefName; }

    // #i81002#
    /** The <SwTextField> instance, which represents the text attribute for the
       <SwGetRefField> instance, has to be passed to the method.
       This <SwTextField> instance is needed for the reference format type REF_UPDOWN
       and REF_NUMBER.
       Note: This instance may be NULL (field in Undo/Redo). This will cause
       no update for these reference format types. */
    void                UpdateField( const SwTextField* pFieldTextAttr );

    void                SetExpand( const OUString& rStr ) { m_sText = rStr; }

    /// Get/set sub type.
    virtual sal_uInt16      GetSubType() const override;
    virtual void        SetSubType( sal_uInt16 n ) override;

    // --> #i81002#
    bool IsRefToHeadingCrossRefBookmark() const;
    bool IsRefToNumItemCrossRefBookmark() const;
    const SwTextNode* GetReferencedTextNode() const;
    // #i85090#
    OUString GetExpandedTextOfReferencedTextNode() const;

    /// Get/set SequenceNo (of interest only for REF_SEQUENCEFLD).
    sal_uInt16              GetSeqNo() const        { return m_nSeqNo; }
    void                SetSeqNo( sal_uInt16 n )    { m_nSeqNo = n; }

    // Name of reference.
    virtual OUString    GetPar1() const override;
    virtual void        SetPar1(const OUString& rStr) override;

    virtual OUString    GetPar2() const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId ) override;

    void                ConvertProgrammaticToUIName();

    virtual OUString    GetDescription() const override;
};

#endif /// INCLUDED_SW_INC_REFFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
