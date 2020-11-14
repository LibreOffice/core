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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XESTREAM_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XESTREAM_HXX

#include <map>
#include <stack>
#include <rtl/strbuf.hxx>

#include <oox/core/xmlfilterbase.hxx>
#include <sax/fshelper.hxx>
#include <tools/stream.hxx>
#include <formula/errorcodes.hxx>
#include "ftools.hxx"
#include <types.hxx>

#include <filter/msfilter/mscodec.hxx>
#include <vector>

namespace com::sun::star::beans { struct NamedValue; }

/* ============================================================================
Output stream class for Excel export
- CONTINUE record handling
============================================================================ */

class XclExpString;
class XclExpRoot;
class XclExpBiff8Encrypter;
typedef std::shared_ptr< XclExpBiff8Encrypter > XclExpEncrypterRef;

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

    If specific data pieces must not be split, use SetSliceSize(). For instance:
    To write a sequence of 16-bit values, where 4 values form a unit and cannot be
    split, call SetSliceSize( 8 ) first (4*2 bytes == 8).

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
    const XclExpRoot& GetRoot() const { return mrRoot; }

    /** Starts a new record: writes header data, stores calculated record size. */
    void                StartRecord( sal_uInt16 nRecId, std::size_t nRecSize );
    /** Checks and corrects real record length. Must be called every time a record is finished. */
    void                EndRecord();

    /** Returns the position inside of current record (starts by 0 in every CONTINUE). */
    sal_uInt16   GetRawRecPos() const { return mnCurrSize; }

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
    std::size_t         Write( const void* pData, std::size_t nBytes );
    /** Writes a sequence of nBytes zero bytes (respects slice setting). */
    void                WriteZeroBytes( std::size_t nBytes );

    void                WriteZeroBytesToRecord( std::size_t nBytes );

    /** Copies nBytes bytes from current position of the stream rInStrm.
        @descr  Omitting the second parameter means: read to end of stream. */
    void                CopyFromStream( SvStream& rInStrm, sal_uInt64 nBytes = STREAM_SEEK_TO_END );

    // *** unicode string export is realized with helper class XclExpString ***
    // (slice length setting has no effect here -> disabled automatically)

    /** Writes Unicode buffer as 8/16 bit, repeats nFlags at start of a CONTINUE record. */
    void                WriteUnicodeBuffer( const ScfUInt16Vec& rBuffer, sal_uInt8 nFlags );

    // *** write 8-bit-strings ***
    // (slice length setting has no effect here -> disabled automatically)

    /** Writes string length field and OString buffer. */
    void                WriteByteString( const OString& rString );

    /** Writes 8-bit character buffer. */
    void                WriteCharBuffer( const ScfUInt8Vec& rBuffer );

    // *** SvStream access ***

    /** Sets position of system stream (only allowed outside of records). */
    void                SetSvStreamPos(sal_uInt64 nPos);
    /** Returns the absolute position of the system stream. */
    sal_uInt64   GetSvStreamPos() const { return mrStrm.Tell(); }

    void                SetEncrypter( XclExpEncrypterRef const & xEncrypter );

    bool                HasValidEncrypter() const;

    void                EnableEncryption( bool bEnable = true );

    void                DisableEncryption();

private:
    /** Writes header data, internal setup. */
    void                InitRecord( sal_uInt16 nRecId );
    /** Rewrites correct record length, if different from calculated. */
    void                UpdateRecSize();
    /** Recalculates mnCurrSize and mnSliceSize. */
    void                UpdateSizeVars( std::size_t nSize );
    /** Writes CONTINUE header, internal setup. */
    void                StartContinue();
    /** Refreshes counter vars, creates CONTINUE records. */
    void                PrepareWrite( sal_uInt16 nSize );
    /** Creates CONTINUE record at end of record.
        @return  Maximum data block size remaining. */
    sal_uInt16          PrepareWrite();

    /** Writes a raw sequence of zero bytes. */
    void                WriteRawZeroBytes( std::size_t nBytes );

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
    std::size_t         mnPredictSize;   /// Predicted size received from calling function.

                        // stream position data
    std::size_t         mnLastSizePos;  /// Stream position of size field in current header.
    bool                mbInRec;        /// true = currently writing inside of a record.
};

class XclExpBiff8Encrypter
{
public:
    explicit XclExpBiff8Encrypter( const XclExpRoot& rRoot );
    ~XclExpBiff8Encrypter();

    bool IsValid() const { return mbValid; }

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
    void Init( const css::uno::Sequence< css::beans::NamedValue >& aEncryptionData );

    static sal_uInt32 GetBlockPos( std::size_t nStrmPos );
    static sal_uInt16 GetOffsetInBlock( std::size_t nStrmPos );

private:
    ::msfilter::MSCodec_Std97 maCodec;      /// Crypto algorithm implementation.
    sal_uInt8           mpnDocId[16];
    sal_uInt8           mpnSalt[16];
    sal_uInt8           mpnSaltDigest[16];

    sal_uInt64          mnOldPos;      /// Last known stream position
    bool                mbValid;
};

// `s.GetChar(0) != 0` needed because some strings on export only contain NULL.
#define XESTRING_TO_PSZ(s) \
    (s.Len() && s.GetChar( 0 ) != 0 ? XclXmlUtils::ToOString( s ).getStr() : nullptr)

class ScAddress;
class ScDocShell;
class ScFormulaCell;
class ScRange;
class ScRangeList;
class ScTokenArray;
struct XclAddress;
struct XclFontData;
class XclRangeList;
namespace sc { class CompileFormulaContext; }

class XclXmlUtils
{
public:
    XclXmlUtils() = delete;
    ~XclXmlUtils() = delete;
    XclXmlUtils(const XclXmlUtils&) = delete;
    XclXmlUtils& operator=(const XclXmlUtils&) = delete;

    static void                     GetFormulaTypeAndValue( ScFormulaCell& rCell, const char*& sType, OUString& rValue);
    static OUString          GetStreamName( const char* sStreamDir, const char* sStream, sal_Int32 nId );

    static OString ToOString( const Color& rColor );
    static OString ToOString( const ScfUInt16Vec& rBuffer );
    static OStringBuffer& ToOString( OStringBuffer& s, const ScAddress& rRange );
    static OString ToOString( const ScDocument& rDoc, const ScRange& rRange, bool bFullAddressNotation = false );
    static OString ToOString( const ScDocument& rDoc, const ScRangeList& rRangeList );
    static OStringBuffer& ToOString( OStringBuffer& s, const XclAddress& rAddress );
    static OString ToOString( const XclExpString& s );
    static OString ToOString( const ScDocument& rDoc, const XclRangeList& rRangeList );

    static OUString ToOUString( const char* s );
    static OUString ToOUString( const ScfUInt16Vec& rBuffer, sal_Int32 nStart = 0, sal_Int32 nLength = -1 );
    static OUString ToOUString( sc::CompileFormulaContext& rCtx, const ScAddress& rAddress,
            const ScTokenArray* pTokenArray, FormulaError nErrCode = FormulaError::NONE );
    static OUString ToOUString( const XclExpString& s );

    template <class T>
    static sax_fastparser::FSHelperPtr WriteElement(sax_fastparser::FSHelperPtr pStream, sal_Int32 nElement, const T& value)
    {
        pStream->startElement(nElement);
        pStream->write(value);
        pStream->endElement(nElement);

        return pStream;
    }
    static sax_fastparser::FSHelperPtr  WriteFontData( sax_fastparser::FSHelperPtr pStream, const XclFontData& rFontData, sal_Int32 nNameId );
};

class XclExpXmlStream : public oox::core::XmlFilterBase
{
public:
    XclExpXmlStream( const css::uno::Reference< css::uno::XComponentContext >& rCC, bool bExportVBA, bool bExportTemplate );
    virtual ~XclExpXmlStream() override;

    /** Returns the filter root data. */
    const XclExpRoot& GetRoot() const { return *mpRoot; }

    sax_fastparser::FSHelperPtr& GetCurrentStream();
    void PushStream( sax_fastparser::FSHelperPtr const & aStream );
    void PopStream();

    sax_fastparser::FSHelperPtr     GetStreamForPath( const OUString& rPath );

    template <typename Str, typename... Args>
    void WriteAttributes(sal_Int32 nAttribute, Str&& value, Args&&... rest)
    {
        WriteAttribute(nAttribute, std::forward<Str>(value));
        if constexpr(sizeof...(rest) > 0)
        {
            // coverity[stray_semicolon : FALSE] - coverity parse error
            WriteAttributes(std::forward<Args>(rest)...);
        }
    }

    sax_fastparser::FSHelperPtr     CreateOutputStream (
                                        const OUString& sFullStream,
                                        const OUString& sRelativeStream,
                                        const css::uno::Reference< css::io::XOutputStream >& xParentRelation,
                                        const char* sContentType,
                                        std::u16string_view sRelationshipType,
                                        OUString* pRelationshipId = nullptr );

    // ignore
    virtual bool exportDocument() override;

    // only needed for import; ignore
    virtual bool importDocument() throw() override;
    virtual oox::vml::Drawing* getVmlDrawing() override;
    virtual const oox::drawingml::Theme* getCurrentTheme() const override;
    virtual oox::drawingml::table::TableStyleListPtr getTableStyles() override;
    virtual oox::drawingml::chart::ChartConverter* getChartConverter() override;

private:
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const override;
    virtual OUString SAL_CALL getImplementationName() override;
    ScDocShell *getDocShell();
    void WriteAttribute(sal_Int32 nAttr, const OUString& sVal);
    void WriteAttribute(sal_Int32 nAttr, const OString& sVal)
    {
        WriteAttribute(nAttr, OStringToOUString(sVal, RTL_TEXTENCODING_UTF8));
    }
    void WriteAttribute(sal_Int32 nAttr, const char* sVal)
    {
        if (sVal)
            WriteAttribute(nAttr, OUString(sVal, strlen(sVal), RTL_TEXTENCODING_UTF8));
    }

    void validateTabNames(std::vector<OUString>& aOriginalTabNames);
    void restoreTabNames(const std::vector<OUString>& aOriginalTabNames);
    void renameTab(SCTAB aTab, OUString aNewName);

    typedef std::map< OUString,
        std::pair< OUString,
            sax_fastparser::FSHelperPtr > >     XclExpXmlPathToStateMap;

    const XclExpRoot*                           mpRoot;
    std::stack< sax_fastparser::FSHelperPtr >   maStreams;
    XclExpXmlPathToStateMap                     maOpenedStreamMap;

    bool mbExportVBA;
    bool mbExportTemplate;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
