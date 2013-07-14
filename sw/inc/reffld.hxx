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
#ifndef SW_REFFLD_HXX
#define SW_REFFLD_HXX

#include <fldbas.hxx>

class SfxPoolItem;
class SwDoc;
class SwTxtNode;
class SwTxtFld;

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
    REF_END
};


/// Get reference.

class SwGetRefFieldType : public SwFieldType
{
    SwDoc* pDoc;
protected:
    /// Overlay in order to update all ref-fields.
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );
public:
    SwGetRefFieldType(SwDoc* pDoc );
    virtual SwFieldType*    Copy() const;

    SwDoc*                  GetDoc() const { return pDoc; }

    void MergeWithOtherDoc( SwDoc& rDestDoc );

    static SwTxtNode* FindAnchor( SwDoc* pDoc, const OUString& rRefMark,
                                        sal_uInt16 nSubType, sal_uInt16 nSeqNo,
                                        sal_uInt16* pStt, sal_uInt16* pEnd = 0 );
};


class SW_DLLPUBLIC SwGetRefField : public SwField
{
private:
    OUString sSetRefName;
    OUString sTxt;
    sal_uInt16 nSubType;
    sal_uInt16 nSeqNo;

    virtual OUString    Expand() const;
    virtual SwField*    Copy() const;

    // #i81002#
    OUString MakeRefNumStr( const SwTxtNode& rTxtNodeOfField,
                          const SwTxtNode& rTxtNodeOfReferencedItem,
                          const sal_uInt32 nRefNumFormat ) const;

public:
    SwGetRefField( SwGetRefFieldType*, const OUString& rSetRef,
                    sal_uInt16 nSubType, sal_uInt16 nSeqNo, sal_uLong nFmt );

    virtual ~SwGetRefField();

    virtual OUString GetFieldName() const;

    OUString GetSetRefName() const { return sSetRefName; }

    // #i81002#
    /** The <SwTxtFld> instance, which represents the text attribute for the
       <SwGetRefField> instance, has to be passed to the method.
       This <SwTxtFld> instance is needed for the reference format type REF_UPDOWN
       and REF_NUMBER.
       Note: This instance may be NULL (field in Undo/Redo). This will cause
       no update for these reference format types. */
    void                UpdateField( const SwTxtFld* pFldTxtAttr );

    void                SetExpand( const OUString& rStr ) { sTxt = rStr; }

    /// Get/set sub type.
    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType( sal_uInt16 n );

    // --> #i81002#
    bool IsRefToHeadingCrossRefBookmark() const;
    bool IsRefToNumItemCrossRefBookmark() const;
    const SwTxtNode* GetReferencedTxtNode() const;
    // #i85090#
    OUString GetExpandedTxtOfReferencedTxtNode() const;


    /// Get/set SequenceNo (of interest only for REF_SEQUENCEFLD).
    sal_uInt16              GetSeqNo() const        { return nSeqNo; }
    void                SetSeqNo( sal_uInt16 n )    { nSeqNo = n; }

    // Name of reference.
    virtual OUString    GetPar1() const;
    virtual void        SetPar1(const OUString& rStr);

    virtual OUString GetPar2() const;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    void                ConvertProgrammaticToUIName();

    virtual OUString GetDescription() const;
};


#endif /// SW_REFFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
