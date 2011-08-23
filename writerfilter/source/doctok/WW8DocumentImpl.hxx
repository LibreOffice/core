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

#ifndef INCLUDED_WW8_DOCUMENT_IMPL_HXX
#define INCLUDED_WW8_DOCUMENT_IMPL_HXX
    
#include <set>
#include <WW8PieceTable.hxx>
#include <WW8BinTable.hxx>
#include <resources.hxx>
#ifndef INCLUDED_WW8_RESOURCE_MODEL_HXX
#include <resourcemodel/WW8ResourceModel.hxx>
#endif
#include <PLCF.hxx>
#include <WW8Sttbf.hxx>
#include <XNoteHelper.hxx>

namespace writerfilter {
namespace doctok 
{

class WW8DocumentImpl;

/**
   A bookmark.
*/
class Bookmark : public writerfilter::Reference<Properties>
{
    /// bookmark first descriptor
    writerfilter::Reference<Properties>::Pointer_t mpBKF;

    /// name of bookmark
    rtl::OUString mName;

public:
    Bookmark(writerfilter::Reference<Properties>::Pointer_t pBKF,
             rtl::OUString & rName);
    
    /**
       Resolve bookmark to handler.

       @param rHandler    handler to send events to
     */
    virtual void resolve(Properties & rHandler);

    virtual string getType() const;
};

/**
   A helper for bookmarks.
 */
class BookmarkHelper
{
    /// PLCF containing Cps and BKFs for the bookmarks
    PLCF<WW8BKF>::Pointer_t mpStartCps;

    /// Cps of the ends of bookmarks
    WW8StructBase::Pointer_t mpEndCps;

    /// STTBF containing the names of bookmarks
    WW8Sttbf::Pointer_t mpNames;

    /// piece table for converting CPs to FCs
    WW8PieceTable::Pointer_t mpPieceTable;

    /// document to insert CpAndFcs to
    WW8DocumentImpl * mpDoc;
    
    /**
       Return start CpAndFc for a bookmark.

       @param nPos       index of the bookmark
    */
    CpAndFc getStartCpAndFc(sal_uInt32 nPos);

    /**
       Return end CpAndFc for a bookmark.

       @param nPos       index of the bookmark
    */
    CpAndFc getEndCpAndFc(sal_uInt32 nPos);

    /**
       Return the name of a bookmark.

       @param nPos       index of the bookmark
    */
    rtl::OUString getName(sal_uInt32 nPos);

    /**
       Return index of a bookmark.

       @param rCpAndFc    CpAndFc of the bookmark
     */
    sal_uInt32 getIndex(const CpAndFc & rCpAndFc);
    
public:
    typedef boost::shared_ptr<BookmarkHelper> Pointer_t;

    BookmarkHelper(PLCF<WW8BKF>::Pointer_t pStartCps, 
                   WW8StructBase::Pointer_t pEndCps,
                   WW8Sttbf::Pointer_t pNames,
                   WW8PieceTable::Pointer_t pPieceTable,
                   WW8DocumentImpl * pDoc)
    : mpStartCps(pStartCps), mpEndCps(pEndCps), mpNames(pNames),
      mpPieceTable(pPieceTable), mpDoc(pDoc)
    {
    }

    /**
       Initialize.

       Inserts CpAndFcs for the bookmark starts and ends into the
    document.
    */
    void init();

    /**
       Return BKF of a bookmark.

       @param rCpAndFc       CpAndFc of the bookmark
     */
    writerfilter::Reference<Properties>::Pointer_t getBKF
    (const CpAndFc & rCpAndFc);

    /**
       Return the name of a bookmark.

       @param rCpAndFc       CpAndFc of the bookmark
     */
    rtl::OUString getName(const CpAndFc & rCp);

    /**
       Return bookmark.
       @param rCpAndFc       CpAndFc of the bookmark
     */
    writerfilter::Reference<Properties>::Pointer_t 
    getBookmark(const CpAndFc & rCpAndFc);
};

/**
   Helper for shapes.
 */
class ShapeHelper
{
public:
    typedef hash_map<CpAndFc, WW8FSPA::Pointer_t, CpAndFcHash> Map_t;

private:
    WW8DocumentImpl * mpDoc;
    Map_t mMap;

public:
    typedef boost::shared_ptr<ShapeHelper> Pointer_t;
    ShapeHelper(PLCF<WW8FSPA>::Pointer_t pPlcspaMom,
                PLCF<WW8FSPA>::Pointer_t pPlcspaHdr,
                WW8DocumentImpl * pDoc);

    void init();

    writerfilter::Reference<Properties>::Pointer_t
    getShape(const CpAndFc & rCpAndFc);

};

/**
   Helper for fields.
 */
class FieldHelper 
{
public:
    typedef hash_map<CpAndFc, WW8FLD::Pointer_t, 
                     CpAndFcHash> Map_t;

private:
    WW8DocumentImpl * mpDoc;
    Map_t mMap;
public:

    typedef boost::shared_ptr<FieldHelper> Pointer_t;
    FieldHelper(PLCF<WW8FLD>::Pointer_t pPlcffldMom,
                WW8DocumentImpl * pDoc);

    void init();

    writerfilter::Reference<Properties>::Pointer_t
    getField(const CpAndFc & rCpAndFc);

    WW8FLD::Pointer_t getWW8FLD(const CpAndFc & rCpAndFc);
};

/** 
    Helper for Breaks.
*/
class BreakHelper
{
public:
    typedef hash_map<CpAndFc, WW8BKD::Pointer_t,
                     CpAndFcHash> Map_t;
private:
    WW8DocumentImpl * mpDoc;
    Map_t mMap;
public:
    typedef boost::shared_ptr<BreakHelper> Pointer_t;
    BreakHelper(PLCF<WW8BKD>::Pointer_t pPlcfbkdMom,
                WW8DocumentImpl * pDoc);

    void init();

    writerfilter::Reference<Properties>::Pointer_t
    getBreak(const CpAndFc & rCpAndFc);
};

/**
   Implementation class for document.
 */
class WW8DocumentImpl : public WW8Document
{
    /// true if the document is contained in another document
    bool bSubDocument; 

    /// picture location
    sal_uInt32 mfcPicLoc;

    /// true if FFDATA structure is found at offset mfcPicLoc in data stream
    bool mbPicIsData;

    /// current field descriptor
    WW8FLD::Pointer_t mpFLD;

    /// CpAndFcs in the document where properties change
    CpAndFcs mCpAndFcs; 
 
    /// CpAndFc pointing to the start of the document
    CpAndFc mCpAndFcStart; 

    /// CpAndFc pointing to the end of the document
    CpAndFc mCpAndFcEnd; 

    /// pointer to the cache of FKPs containing character properties
    WW8FKPCache::Pointer_t mpCHPFKPCache;

    /// pointer to the cache of FPKs containing paragraph properties
    WW8FKPCache::Pointer_t mpPAPFKPCache;

    /// pointer to the stream containing the whole document
    WW8Stream::Pointer_t mpStream;

    /// pointer to the substream of the document containing table like
    /// structures
    WW8Stream::Pointer_t mpTableStream;

    /// pointer to the substream of the document containing the
    /// document contents and formatting information
    WW8Stream::Pointer_t mpDocStream;

    /// pointer to the data stream of the document
    WW8Stream::Pointer_t mpDataStream;

    /// pointer to the compound object stream of the document
    WW8Stream::Pointer_t mpCompObjStream;

    /// pointer to the summayry information stream of the document
    WW8Stream::Pointer_t mpSummaryInformationStream;

    /// pointer to the piece table
    WW8PieceTable::Pointer_t mpPieceTable;

    /// pointer to the bin table for paragraph properties
    WW8BinTable::Pointer_t mpBinTablePAPX;

    /// pointer to the bin table for character properties
    WW8BinTable::Pointer_t mpBinTableCHPX;

    /// PLCF containing the section descriptions
    PLCF<WW8SED>::Pointer_t mpSEDs;

    /// pointer to the file information block
    WW8Fib::Pointer_t mpFib;
    
    /// pointer to the file information block for post 2000 documents
    WW8FibRgFcLcb2000::Pointer_t mpFibRgFcLcb2000;

    /// pointer to the offsets of headers/footers
    WW8StructBase::Pointer_t mpHeaderOffsets;

    /// pointer to the helper for footnotes
    XNoteHelper<WW8FRD>::Pointer_t mpFootnoteHelper;

    /// pointer to the helper for endnotes
    XNoteHelper<WW8FRD>::Pointer_t mpEndnoteHelper;

    /// pointer to the helper for annotations
    XNoteHelper<WW8ATRD>::Pointer_t mpAnnotationHelper;

    /// pointer to the helper for bookmarks
    BookmarkHelper::Pointer_t mpBookmarkHelper;

    /// pointer to the helper for fields
    FieldHelper::Pointer_t mpFieldHelper;

    /// pointer to the helper for shapes
    ShapeHelper::Pointer_t mpShapeHelper;

    /// pointer to the helper for breaks
    BreakHelper::Pointer_t mpBreakHelper;


    /// cache for the Cp where main text flow end
    CpAndFc mDocumentEndCpAndFc;

    /// cache for the Cp where footnotes section ends
    CpAndFc mFootnoteEndCpAndFc;

    /// cache for the Cp where header section ends
    CpAndFc mHeaderEndCpAndFc;

    /// cache for the Cp where annotation section ends
    CpAndFc mAnnotationEndCpAndFc;

    /// cache for the Cp where endnote section ends
    CpAndFc mEndnoteEndCpAndFc;

    /// cache for the Cp where textbox section ends
    CpAndFc mTextboxEndCpAndFc;

    /// cache for the Cp where textbox in header section ends
    CpAndFc mTextboxHeaderEndCpAndFc;

    /// DffBlock of document
    DffBlock::Pointer_t mpDffBlock;

    /// Textbox stories
    PLCF<WW8FTXBXS>::Pointer_t mpTextBoxStories;
    
    bool mbInSection;
    bool mbInParagraphGroup;
    bool mbInCharacterGroup;

    bool isSpecial(sal_uInt32 nChar);
    
    WW8Stream::Pointer_t getSubStream(const ::rtl::OUString & sId) const;

    /**
       Parse bin table and create CpAndFcs for all points in the
       document where properties of the given type change.

       @param rTable   bin table to parse
       @param eType_   type of CpAndFcs to create
     */
    void parseBinTableCpAndFcs(WW8BinTable & rTable, PropertyType eType_);

    void startCharacterGroup(Stream & rStream);
    void endCharacterGroup(Stream & rStream);
    void startParagraphGroup(Stream & rStream);
    void endParagraphGroup(Stream & rStream);
    void startSectionGroup(Stream & rStream);
    void endSectionGroup(Stream & rStream);
    void text(Stream & rStream, const sal_uInt8 * data, size_t len);
    void utext(Stream & rStream, const sal_uInt8 * data, size_t len);
    
public:
    WW8DocumentImpl(WW8Stream::Pointer_t rpStream);
    WW8DocumentImpl(const WW8DocumentImpl & rSrc, 
                    const CpAndFc & rStart, const CpAndFc & rEnd);
    virtual ~WW8DocumentImpl();

    virtual WW8DocumentImpl & Assign(const WW8DocumentImpl & rSrc);

    virtual string getType() const;

    virtual WW8Document::Pointer_t getSubDocument(SubDocumentId nId);
    virtual WW8DocumentIterator::Pointer_t getIterator
    (const CpAndFc & rCpAndFc);
    virtual WW8DocumentIterator::Pointer_t begin();
    virtual WW8DocumentIterator::Pointer_t end();

    virtual WW8Stream::Sequence getText(const CpAndFc & rStart);

    /**
       Returns the document stream.
    */

    WW8Stream::Pointer_t getDocStream() const;

    /**
       Returns the data stream.
     */
    WW8Stream::Pointer_t getDataStream() const;

    /**
       Returns distance in bytes to next CpAndFc.

       @param rCpAndFc  CpAndFc to start at
     */
    sal_uInt32 getByteLength(const CpAndFc & rCpAndFc) const;

    /**
       Returns first character position in document.
     */
    const CpAndFc & getFirstCp() const;

    /**
       Returns last character position in document.
    */
    const CpAndFc & getLastCp() const;

    /**
       Returns next character position with change in properties.

       @param rCpAndFc   position to start at
     */
    CpAndFc getNextCp(const CpAndFc & rCpAndFc) const;

    /**
       Returns previous character position with change in properties.

       @param rCpAndFc   position to start at
    */
    CpAndFc getPrevCp(const CpAndFc & rCpAndFc) const;

    /**
       Returns character position where main text flow ends.
     */
    CpAndFc getDocumentEndCp() const;

    /**
       Returns character position where footnotes end.
    */
    CpAndFc getFootnodeEndCp() const;

    /**
       Returns character position where headers end.
    */
    CpAndFc getHeaderEndCp() const;

    /**
       Returns character position where annatations end.
    */
    CpAndFc getAnnotationEndCp() const;
    
    /**
       Returns character position where endnotes end.
    */
    CpAndFc getEndnoteEndCp() const;

    /**
       Returns character position where textboxes end.
    */
    CpAndFc getTextboxEndCp() const;

    /**
       Returns character positoion where textboxes in headers end.
    */
    CpAndFc getTextboxHeaderEndCp() const;

    /**
       Insert CpAndFc to set of CpAndFcs.

       @param rCpAndFc    CpAndFc to insert
    */       
    void insertCpAndFc(const CpAndFc & rCpAndFc);

    /**
       Return FKP for certain CpAndFc.

       @param rCpAndFc CpAndFc for which the FKP is looked for
     */
    WW8FKP::Pointer_t getFKP(const CpAndFc & rCpAndFc);

    /**
       Return FKP containing character properties.

       @param nIndex      index of FKP to return
       @param bComplex    true if FKP contains complex FCs
     */
    WW8FKP::Pointer_t getFKPCHPX(sal_uInt32 nIndex, bool bComplex);

    /**
       Return FKP containing paragraph properties.

       @param nIndex      index of FKP to return
       @param bComplex    true if FKP contains complex FCs
     */
    WW8FKP::Pointer_t getFKPPAPX(sal_uInt32 nIndex, bool bComplex);

    /**
       Return property set valid at a certain CpAndFc.

       @param rCpAndFc    CpAndFc to look at
     */
    writerfilter::Reference<Properties>::Pointer_t 
    getProperties(const CpAndFc & rCpAndFc);

    /**
       Return subdocument referenced at a certain point in document.

       @param rCpAndFc       CpAndFc where subdocument is referenced
     */
    writerfilter::Reference<Stream>::Pointer_t
    getSubDocument(const CpAndFc & rCpAndFc);

    /**
       Return section description at a certain CpAndFc.

       @param rCpAndFc   CpAndFc to look at
     */
    WW8SED * getSED(const CpAndFc & rCpAndFc) const;

    /**
     Return reference to list plcs.
    */
    writerfilter::Reference<Table>::Pointer_t getListTplcs() const;
    
    /**
       Return reference to list table.
     */
    writerfilter::Reference<Table>::Pointer_t getListTable() const;

    /**
       Return reference to table of list level overrides.
    */
    writerfilter::Reference<Table>::Pointer_t getLFOTable() const;

    /**
       Return reference to font table.
     */
    writerfilter::Reference<Table>::Pointer_t getFontTable() const;

    /**
       Return reference to style sheet.
    */
    writerfilter::Reference<Table>::Pointer_t getStyleSheet() const;

    /**
       Return reference to associated data.
     */
    writerfilter::Reference<Table>::Pointer_t getAssocTable() const;

    /**
       Return count of headers/footers.
    */
    sal_uInt32 getHeaderCount() const;

    /**
       Return CpAndFc for a header or footer.

       @param nPos   index in the list of headers and footers
     */
    CpAndFc getHeaderCpAndFc(sal_uInt32 nPos);
    
    /**
       Return subdocument for header/footer.

       @param nPos   index in the list of headers and footers
     */
    writerfilter::Reference<Stream>::Pointer_t getHeader(sal_uInt32 nPos);

    /**
       Return count of footnotes.
    */
    sal_uInt32 getFootnoteCount() const;

    /**
       Return subdocument for footnote.

       @param nPos     index of the footnote
    */
    writerfilter::Reference<Stream>::Pointer_t getFootnote(sal_uInt32 nPos);

    /**
       Return subdocument for footnote at a certain position in document.

       @param rCpAndFc      position in document
     */
    writerfilter::Reference<Stream>::Pointer_t getFootnote
    (const CpAndFc & rCpAndFc);

    /**
       Return count of endnotes.
    */
    sal_uInt32 getEndnoteCount() const;

    /**
       Return subdocument for an endnote.

       @param nPos       index of the endnote
     */
    writerfilter::Reference<Stream>::Pointer_t getEndnote(sal_uInt32 nPos);    

    /**
       Return subdocument for an endnote.

       @param rCpAndFc    CpAndFc where endnote is referenced
     */
    writerfilter::Reference<Stream>::Pointer_t getEndnote
    (const CpAndFc & rCpAndFc);

    /**
       Return count of annotations.
    */
    sal_uInt32 getAnnotationCount() const;

    /**
       Return subdocument for an annotation.

       @param nPos       index of the annotation
     */
    writerfilter::Reference<Stream>::Pointer_t getAnnotation(sal_uInt32 nPos);

    /**
       Return subdocument for an annotation.

       @param rCpAndFc    CpAndFc where annotation is referenced
     */
    writerfilter::Reference<Stream>::Pointer_t getAnnotation
    (const CpAndFc & rCpAndFc);

    /**
       Return bookmark.

       @param rCpAndFc    CpAndFc where bookmark begins or ends
     */
    writerfilter::Reference<Properties>::Pointer_t 
    getBookmark(const CpAndFc & rCpAndFc) const;

    /**
       Return shape.
       
       @param rCpAndFc    CpAndFc of the shape
     */
    writerfilter::Reference<Properties>::Pointer_t
    getShape(const CpAndFc & rCpAndFc) const;

    writerfilter::Reference<Properties>::Pointer_t
    getShape(sal_uInt32 nSpid);

    /**
       Return blip.

       @param nBlib  number of the blip to return
    */
    writerfilter::Reference<Properties>::Pointer_t
    getBlip(sal_uInt32 nBlib);

    /**
       Return break descriptor.

       @param rCpAndFc    CpAndFc of the break
    */
    writerfilter::Reference<Properties>::Pointer_t
    getBreak(const CpAndFc & rCpAndFc) const;
    

    /**
       Return field.
       
       @param rCpAndFc    CpAndFc of the field
     */
    writerfilter::Reference<Properties>::Pointer_t
    getField(const CpAndFc & rCpAndFc) const;

    /**
       Return document properties.
       
    */
    writerfilter::Reference<Properties>::Pointer_t
    getDocumentProperties() const;

    /**
       Return current field descriptor.
    */
    WW8FLD::Pointer_t getCurrentFLD() const;

    /**
       Return stream of text box.

       @param nShpId    shape id of text box
     */
    writerfilter::Reference<Stream>::Pointer_t
    getTextboxText(sal_uInt32 nShpId) const;

    /**
       Return file character position according to a character
       position.

       @param cp   the character position
     */
    Fc cp2fc(const Cp & cp) const;

    /**
       Return the character position according to file character
       position.

       @param fc  the file character position
     */
    Cp fc2cp(const Fc & fc) const;

    /**
       Return CpAndFc related to character position.

       @param cp    the character position
     */
    CpAndFc getCpAndFc(const Cp & cp, PropertyType type = PROP_DOC) const;

    /**
       Return CpAndFc related to file character position.

       @param fc    the file character position
    */
    CpAndFc getCpAndFc(const Fc & fc, PropertyType type = PROP_DOC) const;

    sal_uInt32 getPicLocation() const;
    void setPicLocation(sal_uInt32 fcPicLoc);

    bool isPicData();
    void setPicIsData(bool bPicIsData);

    /**
       Create events for the document.

       @param rHandler    handler to send the events to
     */
    void resolve(Stream & rHandler);

    /**
       Resolve text.

       The text of the given iterator is split at special
       characters. Each run of non-special characters is send as one
       event. Each run of special characters is send as one event.

       @param pIt        iterator whose text is to be resolved
       @param rStream    handler for the events
     */
    void resolveText(WW8DocumentIterator::Pointer_t pIt, Stream & rStream);

    /**
       Resolve the picture at mfcPicLoc.

       @param rStream
     */
    void resolvePicture(Stream & rStream);

    /**
       Resolve special char.

       @param nChar    the special char
       @param rStream  the stream handler to resolve the special char to
     */
    void resolveSpecialChar(sal_uInt32 nChar, Stream & rStream);
};

/**
   Implentation class for an iterator in a document.
 */
class WW8DocumentIteratorImpl : public WW8DocumentIterator
{
    /// pointer to the document
    WW8DocumentImpl * mpDocument;

    /// CpAndFc the iterator is pointing to
    CpAndFc mCpAndFc;

public:
    WW8DocumentIteratorImpl(WW8DocumentImpl * pDocument,
                            const CpAndFc & rCpAndFc)
    : mpDocument(pDocument), mCpAndFc(rCpAndFc)
    {
    }

    virtual ~WW8DocumentIteratorImpl();

    /**
       Increase the iterator to the next character position.
     */
    WW8DocumentIterator & operator++();

    /**
       Decrease the iterator to the previous character position.
     */
    WW8DocumentIterator & operator--();

    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() 
        const;
    //void setAttributes(const IAttributeSet & aAttributeSet);

    virtual writerfilter::Reference<Stream>::Pointer_t getSubDocument() const;

    /** Return if the text the iterator points to is complex.
        
        @attention The definition of a complex run of text in Word is
        counter-intuitive: Complex runs use 8-bit encoding for characters,
        non-complex ones use 16 bits.
    */    
    bool isComplex() const;
    virtual PropertyType getPropertyType() const;
        
    virtual WW8Stream::Sequence getText();
    virtual bool equal(const WW8DocumentIterator & rIt) const;

    virtual string toString() const;
    virtual void dump(ostream & o) const;

    /**
       Return pointer to the shape at character position the iterator
       is pointing to.
     */
    virtual writerfilter::Reference<Properties>::Pointer_t getShape() const;

    /**
       Return pointer to section description at character position the
       iterator points to.
     */
    WW8SED * getSED() const;
};

/**
   Return string for property type.
 */
string propertyTypeToString(PropertyType nType);

}}
#endif // INCLUDED_WW8_DOCUMENT_IMPL_HXX
