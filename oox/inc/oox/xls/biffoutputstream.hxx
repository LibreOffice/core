/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biffoutputstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_XLS_BIFFOUTPUTSTREAM_HXX
#define OOX_XLS_BIFFOUTPUTSTREAM_HXX

#include <vector>
#include "oox/xls/biffhelper.hxx"

namespace oox { class BinaryOutputStream; }

namespace oox {
namespace xls {

// ============================================================================

namespace prv {

/** Buffers the contents of a raw record. */
class BiffOutputRecordBuffer
{
public:
    explicit            BiffOutputRecordBuffer(
                            BinaryOutputStream& rOutStrm,
                            sal_uInt16 nMaxRecSize );

    /** Returns the core stream object. */
    inline const BinaryOutputStream&
                        getCoreStream() const { return mrOutStrm; }

    /** Starts a new record. */
    void                startRecord( sal_uInt16 nRecId );
    /** Finishes the current record. Must be called for every started record. */
    void                endRecord();

    /** Returns the number of remaining bytes in the current record body. */
    inline sal_uInt16   getRecLeft() const { return static_cast< sal_uInt16 >( mnMaxRecSize - maData.size() ); }

    /** Writes nBytes bytes from the existing buffer pData. Must NOT overwrite the destination buffer. */
    void                write( const void* pData, sal_uInt16 nBytes );
    /** Writes a sequence of nBytes bytes with the passed value. */
    void                fill( sal_uInt8 nValue, sal_uInt16 nBytes );
    /** Writes a value. Must NOT overread the buffer. */
    template< typename Type >
    inline void         writeValue( Type nValue );

private:
    typedef ::std::vector< sal_uInt8 > DataBuffer;

    BinaryOutputStream& mrOutStrm;              /// Core ouput stream.
    DataBuffer          maData;                 /// Record data buffer.
    sal_uInt16          mnMaxRecSize;           /// Maximum size of record contents.
    sal_uInt16          mnRecId;                /// Current record identifier.
    bool                mbInRec;                /// True = currently writing inside of a record.
};

// ----------------------------------------------------------------------------

template< typename Type >
inline void BiffOutputRecordBuffer::writeValue( Type nValue )
{
    ByteOrderConverter::convertLittleEndian( nValue );
    write( &nValue, static_cast< sal_uInt16 >( sizeof( Type ) ) );
}

} // namespace prv

// ============================================================================

/** This class is used to export BIFF record streams.

    An instance is constructed with an BinaryOutputStream object and the
    maximum size of BIFF record contents (e.g. 2080 bytes in BIFF2-BIFF5, or
    8224 bytes in BIFF8).

    To start writing a record, call startRecord() with the record identifier.
    Each record must be closed by calling endRecord().

    If some data exceeds the record size limit, a CONTINUE record will be
    started automatically and the new data will be written to this record. If
    specific data pieces must not be split into the current and a following
    CONTINUE record, use setPortionSize(). Example: To write a sequence of
    16-bit values where 4 values form a unit and cannot be split, call
    setPortionSize(8) first (4*2 bytes == 8).

    To write unicode character arrays, call writeUnicodeBuffer(). It creates
    CONTINUE records and repeats the unicode string flag byte automatically.
*/
class BiffOutputStream
{
public:
    explicit            BiffOutputStream(
                            BinaryOutputStream& rOutStream,
                            sal_uInt16 nMaxRecSize );

                        ~BiffOutputStream();

    // record control ---------------------------------------------------------

    /** Starts a new record. */
    void                startRecord( sal_uInt16 nRecId );

    /** Finishes the current record. Must be called for every started record. */
    void                endRecord();

    /** Sets size of data portion in bytes. 0 means no portions are used. */
    void                setPortionSize( sal_uInt16 nSize );

    // stream/record state and info -------------------------------------------

    // stream write access ----------------------------------------------------

    /** Writes nBytes bytes from the passed buffer pData. */
    void                write( const void* pData, sal_uInt32 nBytes );
    /** Writes a sequence of nBytes bytes with the passed value. */
    void                fill( sal_uInt8 nValue, sal_uInt32 nBytes );

    /** Writes a value to the stream and converts it to little-endian byte order. */
    template< typename Type >
    void                writeValue( Type nValue );

private:
    /** Checks the remaining size in the current record, creates CONTINUE record if needed. */
    void                ensureRawBlock( sal_uInt16 nSize );
    /** Checks the remaining size in the current record and creates CONTINUE record if needed.
        @return  Maximum size left for writing to current record. */
    sal_uInt16          prepareRawBlock( sal_uInt32 nTotalSize );

private:
    prv::BiffOutputRecordBuffer maRecBuffer;    /// Raw record data buffer.
    sal_uInt16          mnPortionSize;          /// Size of data portions.
    sal_uInt16          mnPortionPos;           /// Position in current portion.
};

// ----------------------------------------------------------------------------

template< typename Type >
inline void BiffOutputStream::writeValue( Type nValue )
{
    ensureRawBlock( static_cast< sal_uInt16 >( sizeof( Type ) ) );
    maRecBuffer.writeValue( nValue );
}

template< typename Type >
inline BiffOutputStream& operator<<( BiffOutputStream& rStrm, Type nValue )
{
    rStrm.writeValue( nValue );
    return rStrm;
}

// ============================================================================

} // namespace xls
} // namespace oox

#endif

