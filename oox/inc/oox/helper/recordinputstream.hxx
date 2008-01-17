/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recordinputstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:47 $
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

#ifndef OOX_HELPER_RECORDINPUTSTREAM_HXX
#define OOX_HELPER_RECORDINPUTSTREAM_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include "oox/helper/helper.hxx"

namespace oox {

// ============================================================================

typedef ::com::sun::star::uno::Sequence< sal_Int8 > RecordDataSequence;

// ----------------------------------------------------------------------------

/** Buffers the contents of a record from a binary OOBIN stream. */
class RecordInputStream
{
public:
    explicit            RecordInputStream( const RecordDataSequence& rData );

    /** Returns record reading state: false = record overread. */
    inline bool         isValid() const { return mbValid; }
    /** Returns the record size. */
    inline sal_Int32    getRecSize() const { return mnRecSize; }
    /** Returns the current read position in the record body. */
    inline sal_Int32    getRecPos() const { return mnRecPos; }
    /** Returns the number of remaining bytes in the record body. */
    inline sal_Int32    getRecLeft() const { return mnRecSize - mnRecPos; }

    /** Reads nBytes bytes, writes them to the existing buffer pData. */
    sal_Int32           read( void* opData, sal_Int32 nBytes );

    /** Reads a value from the stream and converts it to platform byte order. */
    template< typename Type >
    void                readValue( Type& ornValue );
    /** Reads a value from the stream and converts it to platform byte order. */
    template< typename Type >
    inline Type         readValue() { Type nValue; readValue( nValue ); return nValue; }

    inline sal_Int8     readInt8() { return readValue< sal_Int8 >(); }
    inline sal_uInt8    readuInt8() { return readValue< sal_uInt8 >(); }
    inline sal_Int16    readInt16() { return readValue< sal_Int16 >(); }
    inline sal_uInt16   readuInt16() { return readValue< sal_uInt16 >(); }
    inline sal_Int32    readInt32() { return readValue< sal_Int32 >(); }
    inline sal_uInt32   readuInt32() { return readValue< sal_uInt32 >(); }
    inline sal_Int64    readInt64() { return readValue< sal_Int64 >(); }
    inline sal_uInt64   readuInt64() { return readValue< sal_uInt64 >(); }
    inline float        readFloat() { return readValue< float >(); }
    inline double       readDouble() { return readValue< double >(); }

    /** Reads a string with leading 16-bit or 32-bit length field. */
    ::rtl::OUString     readString( bool b32BitLen = true );

    /** Seeks to the passed record position. */
    void                seek( sal_Int32 nRecPos );
    /** Ignores nBytes bytes. */
    void                skip( sal_Int32 nBytes );

private:
    RecordDataSequence  maData;             /// Reference to record data.
    sal_Int32           mnRecSize;          /// Current record size.
    sal_Int32           mnRecPos;           /// Current position in record body.
    bool                mbValid;            /// False = record overread.
};

// ----------------------------------------------------------------------------

template< typename Type >
inline void RecordInputStream::readValue( Type& ornValue )
{
    read( &ornValue, static_cast< sal_Int32 >( sizeof( Type ) ) );
    ByteOrderConverter::convertLittleEndian( ornValue );
}

template< typename Type >
inline RecordInputStream& operator>>( RecordInputStream& rStrm, Type& ornValue )
{
    rStrm.readValue( ornValue );
    return rStrm;
}

inline RecordInputStream& operator>>( RecordInputStream& rStrm, ::rtl::OUString& orString )
{
    orString = rStrm.readString();
    return rStrm;
}

// ============================================================================

} // namespace oox

#endif

