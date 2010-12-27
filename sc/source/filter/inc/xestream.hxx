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

#include <filter/msfilter/mscodec.hxx>
#include <vector>

/* ============================================================================
Output stream class for Excel export
- CONTINUE record handling
- ByteString and UniString support
============================================================================ */

class XclExpRoot;
class XclExpBiff8Encrypter;
typedef ScfRef< XclExpBiff8Encrypter > XclExpEncrypterRef;

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

//UNUSED2008-05  /** Writes Unicode buffer as 8/16 bit, repeats nFlags at start of a CONTINUE record. */
//UNUSED2008-05  void                WriteUnicodeBuffer( const sal_uInt16* pBuffer, sal_Size nChars, sal_uInt8 nFlags );

    /** Writes Unicode buffer as 8/16 bit, repeats nFlags at start of a CONTINUE record. */
    void                WriteUnicodeBuffer( const ScfUInt16Vec& rBuffer, sal_uInt8 nFlags );

    // *** write 8-bit-strings ***
    // (slice length setting has no effect here -> disabled automatically)

//UNUSED2008-05  /** Writes ByteString buffer (without string length field). */
//UNUSED2008-05  void                WriteByteStringBuffer(
//UNUSED2008-05                          const ByteString& rString,
//UNUSED2008-05                          sal_uInt16 nMaxLen = 0x00FF );

    /** Writes string length field and ByteString buffer. */
    void                WriteByteString(
                            const ByteString& rString,
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

    const XclExpRoot&   mrRoot;
    sal_Size            mnOldPos;      /// Last known stream position
    bool                mbValid;
};

// ----------------------------------------------------------------------------


// ============================================================================

// `s.GetChar(0) != 0` needed because some strings on export only contain NULL.
#define XESTRING_TO_PSZ(s) \
    (s.Len() && s.GetChar( 0 ) != 0 ? XclXmlUtils::ToOString( s ).getStr() : NULL)

class ScAddress;
class ScDocument;
class ScRange;
class ScRangeList;
class ScTokenArray;
struct XclAddress;
struct XclFontData;
class XclRangeList;

class XclXmlUtils
{
    XclXmlUtils();
    ~XclXmlUtils();
    XclXmlUtils(const XclXmlUtils&);
    XclXmlUtils& operator=(const XclXmlUtils&);
public:
    static ::rtl::OUString          GetStreamName( const char* sStreamDir, const char* sStream, sal_Int32 nId );

    static ::rtl::OString ToOString( const Color& rColor );
    static ::rtl::OString ToOString( const ::rtl::OUString& s );
    static ::rtl::OString ToOString( const ScfUInt16Vec& rBuffer );
    static ::rtl::OString ToOString( const String& s );
    static ::rtl::OString ToOString( const ScAddress& rRange );
    static ::rtl::OString ToOString( const ScRange& rRange );
    static ::rtl::OString ToOString( const ScRangeList& rRangeList );
    static ::rtl::OString ToOString( const XclAddress& rAddress );
    static ::rtl::OString ToOString( const XclExpString& s );
    static ::rtl::OString ToOString( const XclRangeList& rRangeList );

    static ::rtl::OUString ToOUString( const char* s );
    static ::rtl::OUString ToOUString( const ScfUInt16Vec& rBuffer, sal_Int32 nStart = 0, sal_Int32 nLength = -1 );
    static ::rtl::OUString ToOUString( const String& s );
    static ::rtl::OUString ToOUString( ScDocument& rDocument, const ScAddress& rAddress, ScTokenArray* pTokenArray );
    static ::rtl::OUString ToOUString( const XclExpString& s );
    static const char* ToPsz( bool b );
};

class XclExpXmlStream : public oox::core::XmlFilterBase
{
public:
    XclExpXmlStream( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext, SvStream& rStrm, const XclExpRoot& rRoot );
    virtual ~XclExpXmlStream();

    /** Returns the filter root data. */
    inline const XclExpRoot& GetRoot() const { return mrRoot; }

    sax_fastparser::FSHelperPtr& GetCurrentStream();
    void PushStream( sax_fastparser::FSHelperPtr aStream );
    void PopStream();

    ::rtl::OUString                 GetIdForPath( const ::rtl::OUString& rPath );
    sax_fastparser::FSHelperPtr     GetStreamForPath( const ::rtl::OUString& rPath );

    sax_fastparser::FSHelperPtr&    WriteAttributes( sal_Int32 nAttribute, ... );
    sax_fastparser::FSHelperPtr&    WriteFontData( const XclFontData& rFontData, sal_Int32 nNameId );

    sax_fastparser::FSHelperPtr     CreateOutputStream (
                                        const ::rtl::OUString& sFullStream,
                                        const ::rtl::OUString& sRelativeStream,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xParentRelation,
                                        const char* sContentType,
                                        const char* sRelationshipType,
                                        ::rtl::OUString* pRelationshipId = NULL );

    // ignore
    virtual bool exportDocument() throw();

    // only needed for import; ignore
    virtual bool importDocument() throw();
    virtual oox::vml::Drawing* getVmlDrawing();
    virtual const oox::drawingml::Theme* getCurrentTheme() const;
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles();
    virtual oox::drawingml::chart::ChartConverter& getChartConverter();

    void Trace( const char* format, ...);
private:
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const;
    virtual ::rtl::OUString implGetImplementationName() const;

    typedef std::map< ::rtl::OUString,
        std::pair< ::rtl::OUString,
            sax_fastparser::FSHelperPtr > >     XclExpXmlPathToStateMap;

    const XclExpRoot&                           mrRoot;         /// Filter root data.
    std::stack< sax_fastparser::FSHelperPtr >   maStreams;
    XclExpXmlPathToStateMap                     maOpenedStreamMap;
};

#endif

