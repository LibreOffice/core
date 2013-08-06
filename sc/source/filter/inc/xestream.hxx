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

// ============================================================================

#ifndef SC_XESTREAM_HXX
#define SC_XESTREAM_HXX

#include <com/sun/star/beans/NamedValue.hpp>

#include <map>
#include <stack>
#include <string>

#include <oox/core/xmlfilterbase.hxx>
#include <oox/token/tokens.hxx>
#include <sax/fshelper.hxx>

#include "xlstream.hxx"
#include "xestring.hxx"
#include "compiler.hxx"

#include <filter/msfilter/mscodec.hxx>
#include <vector>

/* ============================================================================
Output stream class for Excel export
- CONTINUE record handling
============================================================================ */

class XclExpRoot;
class XclExpBiff8Encrypter;
typedef boost::shared_ptr< XclExpBiff8Encrypter > XclExpEncrypterRef;

/** This class is used to export Excel record streams.
    @descr  An instance is constructed with an SvStream and the maximum size of Excel
    record contents (in BIFF5: 2080 bytes, in BIFF8: 8224 bytes).

    To start writing a record call StartRecord(). Parameters are the record identifier
    and any calculated record size. This is for optimizing the write process: if the real
    written data has the same size as the calculated, the stream will not seek back and
    update the record size field. But it is not mandatory to calculate a size. Each
    record must be closed by calling EndRecord(). This will check (and update) the record
    size field.

    If some data exceeds the record size limit, a CONTINUE record is started automatically
    and the new data will be written to this record.

    If specific data pieces must not be splitted, use SetSliceLen(). For instance:
    To write a sequence of 16-bit values, where 4 values form a unit and cannot be
    split, call SetSliceLen( 8 ) first (4*2 bytes == 8).

    To write unicode character arrays, call WriteUnicodeBuffer(). It creates CONTINUE
    records and repeats the unicode string flag byte automatically. This function is used
    for instance from the class XclExpString which can write complete unicode strings.
*/
class XclExpStream
{
public:
    /** Constructs the Excel record export stream.
        @param rOutStrm  The system output stream to write to.
        @param nMaxRecSize  The maximum allowed size of record content (depending on BIFF type).
        If 0 is passed, the record size will be set automatically, depending on the current BIFF type. */
                        XclExpStream(
                            SvStream& rOutStrm,
                            const XclExpRoot& rRoot,
                            sal_uInt16 nMaxRecSize = 0 );

                        ~XclExpStream();

    /** Returns the filter root data. */
    inline const XclExpRoot& GetRoot() const { return mrRoot; }

    /** Starts a new record: writes header data, stores calculated record size. */
    void                StartRecord( sal_uInt16 nRecId, sal_Size nRecSize );
    /** Checks and corrects real record length. Must be called everytime a record is finished. */
    void                EndRecord();

    /** Returns the position inside of current record (starts by 0 in every CONTINUE). */
    inline sal_uInt16   GetRawRecPos() const { return mnCurrSize; }

    /** Returns the maximum size of a record. */
    inline sal_uInt16   GetMaxRecSize() const { return mnMaxRecSize; }
    /** Sets maximum record size (valid only for current record). */
    inline void         SetMaxRecSize( sal_uInt16 nMax ) { mnCurrMaxSize = nMax; }
    /** Sets maximum size of CONTINUE records (valid only for current record). */
    inline void         SetMaxContSize( sal_uInt16 nMax ) { mnMaxContSize = nMax; }

    /** Sets data slice length. 0 = no slices. */
    void                SetSliceSize( sal_uInt16 nSize );

    XclExpStream& operator<<( sal_Int8 nValue );
    XclExpStream& operator<<( sal_uInt8 nValue );
    XclExpStream& operator<<( sal_Int16 nValue );
    XclExpStream& operator<<( sal_uInt16 nValue );
    XclExpStream& operator<<( sal_Int32 nValue );
    XclExpStream& operator<<( sal_uInt32 nValue );
    XclExpStream& operator<<( float fValue );
    XclExpStream& operator<<( double fValue );

    /** Writes nBytes bytes from memory. */
    sal_Size            Write( const void* pData, sal_Size nBytes );
    /** Writes a sequence of nBytes zero bytes (respects slice setting). */
    void                WriteZeroBytes( sal_Size nBytes );

    void                WriteZeroBytesToRecord( sal_Size nBytes );

    /** Copies nBytes bytes from current position of the stream rInStrm.
        @descr  Omitting the second parameter means: read to end of stream. */
    sal_Size            CopyFromStream( SvStream& rInStrm, sal_Size nBytes = STREAM_SEEK_TO_END );

    // *** unicode string export is realized with helper class XclExpString ***
    // (slice length setting has no effect here -> disabled automatically)

    /** Writes Unicode buffer as 8/16 bit, repeats nFlags at start of a CONTINUE record. */
    void                WriteUnicodeBuffer( const ScfUInt16Vec& rBuffer, sal_uInt8 nFlags );

    // *** write 8-bit-strings ***
    // (slice length setting has no effect here -> disabled automatically)

    /** Writes string length field and OString buffer. */
    void                WriteByteString(
                            const OString& rString,
                            sal_uInt16 nMaxLen = 0x00FF,
                            bool b16BitCount = false );

    /** Writes 8-bit character buffer. */
    void                WriteCharBuffer( const ScfUInt8Vec& rBuffer );

    // *** SvStream access ***

    /** Sets position of system stream (only allowed outside of records). */
    sal_Size            SetSvStreamPos( sal_Size nPos );
    /** Returns the absolute position of the system stream. */
    inline sal_Size     GetSvStreamPos() const { return mrStrm.Tell(); }

    void                SetEncrypter( XclExpEncrypterRef xEncrypter );

    bool                HasValidEncrypter() const;

    void                EnableEncryption( bool bEnable = true );

    void                DisableEncryption();

private:
    /** Writes header data, internal setup. */
    void                InitRecord( sal_uInt16 nRecId );
    /** Rewrites correct record length, if different from calculated. */
    void                UpdateRecSize();
    /** Recalculates mnCurrSize and mnSliceSize. */
    void                UpdateSizeVars( sal_Size nSize );
    /** Writes CONTINUE header, internal setup. */
    void                StartContinue();
    /** Refreshes counter vars, creates CONTINUE records. */
    void                PrepareWrite( sal_uInt16 nSize );
    /** Creates CONTINUE record at end of record.
        @return  Maximum data block size remaining. */
    sal_uInt16          PrepareWrite();

    /** Writes a raw sequence of zero bytes. */
    void                WriteRawZeroBytes( sal_Size nBytes );

private:
    SvStream&           mrStrm;         /// Reference to the system output stream.
    const XclExpRoot&   mrRoot;         /// Filter root data.

    bool                mbUseEncrypter;
    XclExpEncrypterRef  mxEncrypter;

                        // length data
    sal_uInt16          mnMaxRecSize;   /// Maximum size of record content.
    sal_uInt16          mnMaxContSize;  /// Maximum size of CONTINUE content.
    sal_uInt16          mnCurrMaxSize;  /// Current maximum, either mnMaxRecSize or mnMaxContSize.
    sal_uInt16          mnMaxSliceSize; /// Maximum size of data slices (parts that cannot be split).
    sal_uInt16          mnHeaderSize;   /// Record size written in last record header.
    sal_uInt16          mnCurrSize;     /// Count of bytes already written in current record.
    sal_uInt16          mnSliceSize;    /// Count of bytes already written in current slice.
    sal_Size            mnPredictSize;   /// Predicted size received from calling function.

                        // stream position data
    sal_Size            mnLastSizePos;  /// Stream position of size field in current header.
    bool                mbInRec;        /// true = currently writing inside of a record.
};

// ============================================================================

class XclExpBiff8Encrypter
{
public:
    explicit XclExpBiff8Encrypter( const XclExpRoot& rRoot );
    ~XclExpBiff8Encrypter();

    bool IsValid() const;

    void GetSaltDigest( sal_uInt8 pnSaltDigest[16] ) const;
    void GetSalt( sal_uInt8 pnSalt[16] ) const;
    void GetDocId( sal_uInt8 pnDocId[16] ) const;

    void Encrypt( SvStream& rStrm, sal_uInt8  nData );
    void Encrypt( SvStream& rStrm, sal_uInt16 nData );
    void Encrypt( SvStream& rStrm, sal_uInt32 nData );

    void Encrypt( SvStream& rStrm, sal_Int8  nData );
    void Encrypt( SvStream& rStrm, sal_Int16 nData );
    void Encrypt( SvStream& rStrm, sal_Int32 nData );

    void Encrypt( SvStream& rStrm, float fValue );
    void Encrypt( SvStream& rStrm, double fValue );

    void EncryptBytes( SvStream& rStrm, ::std::vector<sal_uInt8>& aBytes );

private:
    void Init( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aEncryptionData );

    sal_uInt32 GetBlockPos( sal_Size nStrmPos ) const;
    sal_uInt16 GetOffsetInBlock( sal_Size nStrmPos ) const;

private:
    ::msfilter::MSCodec_Std97 maCodec;      /// Crypto algorithm implementation.
    sal_uInt8           mpnDocId[16];
    sal_uInt8           mpnSalt[16];
    sal_uInt8           mpnSaltDigest[16];

    sal_Size            mnOldPos;      /// Last known stream position
    bool                mbValid;
};

// ----------------------------------------------------------------------------


// ============================================================================

// `s.GetChar(0) != 0` needed because some strings on export only contain NULL.
#define XESTRING_TO_PSZ(s) \
    (s.Len() && s.GetChar( 0 ) != 0 ? XclXmlUtils::ToOString( s ).getStr() : NULL)

class ScAddress;
class ScDocShell;
class ScDocument;
class ScFormulaCell;
class ScRange;
class ScRangeList;
class ScTokenArray;
struct XclAddress;
struct XclFontData;
struct XclRange;
class XclRangeList;

class XclXmlUtils
{
    XclXmlUtils();
    ~XclXmlUtils();
    XclXmlUtils(const XclXmlUtils&);
    XclXmlUtils& operator=(const XclXmlUtils&);
public:
    static void                     GetFormulaTypeAndValue( ScFormulaCell& rCell, const char*& sType, OUString& rValue);
    static OUString          GetStreamName( const char* sStreamDir, const char* sStream, sal_Int32 nId );

    static OString ToOString( const Color& rColor );
    static OString ToOString( const OUString& s );
    static OString ToOString( const ScfUInt16Vec& rBuffer );
    static OString ToOString( const String& s );
    static OString ToOString( const ScAddress& rRange );
    static OString ToOString( const ScRange& rRange );
    static OString ToOString( const ScRangeList& rRangeList );
    static OString ToOString( const XclAddress& rAddress );
    static OString ToOString( const XclExpString& s );
    static OString ToOString( const XclRange& rRange );
    static OString ToOString( const XclRangeList& rRangeList );

    static OUString ToOUString( const char* s );
    static OUString ToOUString( const ScfUInt16Vec& rBuffer, sal_Int32 nStart = 0, sal_Int32 nLength = -1 );
    static OUString ToOUString( const String& s );
    static OUString ToOUString( ScDocument& rDocument, const ScAddress& rAddress,
                                const ScTokenArray* pTokenArray, const ScCompiler::OpCodeMapPtr & xOpCodeMap );
    static OUString ToOUString( const XclExpString& s );
    static const char* ToPsz( bool b );

    static sax_fastparser::FSHelperPtr  WriteElement( sax_fastparser::FSHelperPtr pStream, sal_Int32 nElement, sal_Int32 nValue );
    static sax_fastparser::FSHelperPtr  WriteElement( sax_fastparser::FSHelperPtr pStream, sal_Int32 nElement, sal_Int64 nValue );
    static sax_fastparser::FSHelperPtr  WriteElement( sax_fastparser::FSHelperPtr pStream, sal_Int32 nElement, const char* sValue );
    static sax_fastparser::FSHelperPtr  WriteFontData( sax_fastparser::FSHelperPtr pStream, const XclFontData& rFontData, sal_Int32 nNameId );
};

class XclExpXmlStream : public oox::core::XmlFilterBase
{
public:
    XclExpXmlStream( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rCC );
    virtual ~XclExpXmlStream();

    /** Returns the filter root data. */
    inline const XclExpRoot& GetRoot() const { return *mpRoot; }

    sax_fastparser::FSHelperPtr& GetCurrentStream();
    void PushStream( sax_fastparser::FSHelperPtr aStream );
    void PopStream();

    sax_fastparser::FSHelperPtr     GetStreamForPath( const OUString& rPath );

    // FIXME: if written through this cannot be checked for well-formedness
    sax_fastparser::FSHelperPtr&    WriteAttributes( sal_Int32 nAttribute, const char* value, FSEND_t )
        { return WriteAttributesInternal( nAttribute, value, FSEND_internal ); }
    sax_fastparser::FSHelperPtr&    WriteAttributes( sal_Int32 nAttribute, const OString& value, FSEND_t )
        { return WriteAttributesInternal( nAttribute, value.getStr(), FSEND_internal ); }

    sax_fastparser::FSHelperPtr     CreateOutputStream (
                                        const OUString& sFullStream,
                                        const OUString& sRelativeStream,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xParentRelation,
                                        const char* sContentType,
                                        const char* sRelationshipType,
                                        OUString* pRelationshipId = NULL );

    // ignore
    virtual bool exportDocument() throw();

    // only needed for import; ignore
    virtual bool importDocument() throw();
    virtual oox::vml::Drawing* getVmlDrawing();
    virtual const oox::drawingml::Theme* getCurrentTheme() const;
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles();
    virtual oox::drawingml::chart::ChartConverter* getChartConverter();

    /*
      Now create all the overloads in a typesafe way (i.e. without varargs) by creating a number of overloads
      up to a certain reasonable limit (feel free to raise it). This would be a lot easier with C++11 vararg templates.
    */
    // now overloads for 2 and more pairs
    #define SAX_ARGS_FUNC_DECL( argsdecl, argsuse ) \
        sax_fastparser::FSHelperPtr&    WriteAttributes( argsdecl, FSEND_t ) \
            { return WriteAttributesInternal( argsuse, FSEND_internal ); }
    #define SAX_ARGS_FUNC_NUM( decl1, decl2, use1, use2, convert, num ) \
        SAX_ARGS_FUNC_DECL( SAX_ARGS_ARG##num( decl1, decl2, ), SAX_ARGS_ARG##num( use1, use2, convert ))
    #define SAX_ARGS_FUNC_SUBST( type, convert, num ) \
        SAX_ARGS_FUNC_NUM( sal_Int32 attribute, type value, attribute, value, convert, num )
    #define SAX_ARGS_FUNC( arg, convert ) SAX_ARGS_FUNC_SUBST( arg, convert, 2 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 3 ) SAX_ARGS_FUNC_SUBST( arg, convert, 4 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 5 ) SAX_ARGS_FUNC_SUBST( arg, convert, 6 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 7 ) SAX_ARGS_FUNC_SUBST( arg, convert, 8 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 9 ) SAX_ARGS_FUNC_SUBST( arg, convert, 10 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 11 ) SAX_ARGS_FUNC_SUBST( arg, convert, 12 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 13 ) SAX_ARGS_FUNC_SUBST( arg, convert, 14 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 15 ) SAX_ARGS_FUNC_SUBST( arg, convert, 16 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 17 ) SAX_ARGS_FUNC_SUBST( arg, convert, 18 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 19 ) SAX_ARGS_FUNC_SUBST( arg, convert, 20 ) \
        SAX_ARGS_FUNC_SUBST( arg, convert, 21 ) SAX_ARGS_FUNC_SUBST( arg, convert, 22 )
    SAX_ARGS_FUNC( const char*, )
    SAX_ARGS_FUNC( const OString&, .getStr() )
    #undef SAX_ARGS_FUNC_DECL
    #undef SAX_ARGS_FUNC_NUM
    #undef SAX_ARGS_FUNC_SUBST
    #undef SAX_ARGS_FUNC

private:
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const;
    virtual OUString implGetImplementationName() const;
    ScDocShell *getDocShell();
    sax_fastparser::FSHelperPtr&    WriteAttributesInternal( sal_Int32 nAttribute, ... );

    typedef std::map< OUString,
        std::pair< OUString,
            sax_fastparser::FSHelperPtr > >     XclExpXmlPathToStateMap;

    const XclExpRoot*                           mpRoot;
    std::stack< sax_fastparser::FSHelperPtr >   maStreams;
    XclExpXmlPathToStateMap                     maOpenedStreamMap;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
