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
    REF_PAGE = REF_BEGIN,
    REF_CHAPTER,
    REF_CONTENT,
    REF_UPDOWN,
    REF_PAGE_PGDESC,
    REF_ONLYNUMBER,
    REF_ONLYCAPTION,
    REF_ONLYSEQNO,
    // --> #i81002#
    // new reference format types for referencing bookmarks and set references
    REF_NUMBER,
    REF_NUMBER_NO_CONTEXT,
    REF_NUMBER_FULL_CONTEXT,
    // <--
    REF_END
};


/*--------------------------------------------------------------------
    Beschreibung: Referenz holen
 --------------------------------------------------------------------*/

class SwGetRefFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwGetRefFieldType(SwDoc* pDoc );
    virtual SwFieldType*    Copy() const;

    SwDoc*                  GetDoc() const { return pDoc; }
    // ueberlagert, um alle Ref-Felder zu updaten
    virtual void Modify( SfxPoolItem *, SfxPoolItem * );

    void MergeWithOtherDoc( SwDoc& rDestDoc );

    static SwTxtNode* FindAnchor( SwDoc* pDoc, const String& rRefMark,
                                        sal_uInt16 nSubType, sal_uInt16 nSeqNo,
                                        sal_uInt16* pStt, sal_uInt16* pEnd = 0 );
};

/*--------------------------------------------------------------------
    Beschreibung: Referenzfeld
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwGetRefField : public SwField
{
private:
    String sSetRefName;
    String sTxt;
    sal_uInt16 nSubType;
    sal_uInt16 nSeqNo;

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    // #i81002#
    String MakeRefNumStr( const SwTxtNode& rTxtNodeOfField,
                          const SwTxtNode& rTxtNodeOfReferencedItem,
                          const sal_uInt32 nRefNumFormat ) const;

public:
    SwGetRefField( SwGetRefFieldType*, const String& rSetRef,
                    sal_uInt16 nSubType, sal_uInt16 nSeqNo, sal_uLong nFmt );

    virtual ~SwGetRefField();

    virtual String      GetFieldName() const;

    const String&       GetSetRefName() const { return sSetRefName; }

    // #i81002#
    // The <SwTxtFld> instance, which represents the text attribute for the
    // <SwGetRefField> instance, has to be passed to the method.
    // This <SwTxtFld> instance is needed for the reference format type REF_UPDOWN
    // and REF_NUMBER.
    // Note: This instance may be NULL (field in Undo/Redo). This will cause
    // no update for these reference format types.
    void                UpdateField( const SwTxtFld* pFldTxtAttr );

    void                SetExpand( const String& rStr ) { sTxt = rStr; }

    // SubType erfragen/setzen
    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType( sal_uInt16 n );

    // --> #i81002#
    bool IsRefToHeadingCrossRefBookmark() const;
    bool IsRefToNumItemCrossRefBookmark() const;
    const SwTxtNode* GetReferencedTxtNode() const;
    // <--
    // #i85090#
    String GetExpandedTxtOfReferencedTxtNode() const;


    // SequenceNo erfragen/setzen (nur fuer REF_SEQUENCEFLD interressant)
    sal_uInt16              GetSeqNo() const        { return nSeqNo; }
    void                SetSeqNo( sal_uInt16 n )    { nSeqNo = n; }

    // Name der Referenz
    virtual const String& GetPar1() const;
    virtual void        SetPar1(const String& rStr);

    virtual String      GetPar2() const;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    void                ConvertProgrammaticToUIName();

    virtual String GetDescription() const;
};


#endif // SW_REFFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
