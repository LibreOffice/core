/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    // --> OD 2007-08-24 #i81002#
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
protected:
    // ueberlagert, um alle Ref-Felder zu updaten
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem * );
public:
    SwGetRefFieldType(SwDoc* pDoc );
    virtual SwFieldType*    Copy() const;

    SwDoc*                  GetDoc() const { return pDoc; }

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

    // --> OD 2007-08-24 #i81002#
    String MakeRefNumStr( const SwTxtNode& rTxtNodeOfField,
                          const SwTxtNode& rTxtNodeOfReferencedItem,
                          const sal_uInt32 nRefNumFormat ) const;
    // <--
public:
    SwGetRefField( SwGetRefFieldType*, const String& rSetRef,
                    sal_uInt16 nSubType, sal_uInt16 nSeqNo, sal_uLong nFmt );

    virtual ~SwGetRefField();

    virtual String      GetFieldName() const;

    const String&       GetSetRefName() const { return sSetRefName; }

    // --> OD 2007-09-06 #i81002#
    // The <SwTxtFld> instance, which represents the text attribute for the
    // <SwGetRefField> instance, has to be passed to the method.
    // This <SwTxtFld> instance is needed for the reference format type REF_UPDOWN
    // and REF_NUMBER.
    // Note: This instance may be NULL (field in Undo/Redo). This will cause
    // no update for these reference format types.
    void                UpdateField( const SwTxtFld* pFldTxtAttr );
    // <--
    void                SetExpand( const String& rStr ) { sTxt = rStr; }

    // SubType erfragen/setzen
    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType( sal_uInt16 n );

    // --> OD 2007-11-09 #i81002#
    bool IsRefToHeadingCrossRefBookmark() const;
    bool IsRefToNumItemCrossRefBookmark() const;
    const SwTxtNode* GetReferencedTxtNode() const;
    // <--
    // --> OD 2008-01-09 #i85090#
    String GetExpandedTxtOfReferencedTxtNode() const;
    // <--

    // SequenceNo erfragen/setzen (nur fuer REF_SEQUENCEFLD interressant)
    sal_uInt16              GetSeqNo() const        { return nSeqNo; }
    void                SetSeqNo( sal_uInt16 n )    { nSeqNo = n; }

    // Name der Referenz
    virtual const String& GetPar1() const;
    virtual void        SetPar1(const String& rStr);

    virtual String      GetPar2() const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    void                ConvertProgrammaticToUIName();

    virtual String GetDescription() const;
};


#endif // SW_REFFLD_HXX

