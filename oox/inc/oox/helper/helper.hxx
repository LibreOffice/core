/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:35:46 $
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

#ifndef OOX_HELPER_HELPER_HXX
#define OOX_HELPER_HELPER_HXX

#include <algorithm>
#include <osl/endian.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <string.h>

namespace oox {

// Helper macros ==============================================================

/** Expands to the number of elements in a STATIC data array. */
#define STATIC_ARRAY_SIZE( array ) \
    (sizeof(array)/sizeof(*(array)))

/** Expands to a pointer behind the last element of a STATIC data array (like
    STL end()). */
#define STATIC_ARRAY_END( array ) \
    ((array)+STATIC_ARRAY_SIZE(array))

/** Expands to the 'index'-th element of a STATIC data array, or to 'def', if
    'index' is out of the array limits. */
#define STATIC_ARRAY_SELECT( array, index, def ) \
    ((static_cast<size_t>(index) < STATIC_ARRAY_SIZE(array)) ? ((array)[static_cast<size_t>(index)]) : (def))

/** Expands to a temporary ::rtl::OString, created from a literal(!) character
    array. */
#define CREATE_OSTRING( ascii ) \
    ::rtl::OString( RTL_CONSTASCII_STRINGPARAM( ascii ) )

/** Expands to a temporary ::rtl::OUString, created from a literal(!) ASCII(!)
    character array. */
#define CREATE_OUSTRING( ascii ) \
    ::rtl::OUString::intern( RTL_CONSTASCII_USTRINGPARAM( ascii ) )

/** Convert an OUString to an ASCII C string. Use for debug purposes only. */
#define OUSTRING_TO_CSTR( str ) \
    ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US ).getStr()

// ============================================================================

// Limitate values ------------------------------------------------------------

template< typename ReturnType, typename Type >
inline ReturnType getLimitedValue( Type nValue, Type nMin, Type nMax )
{
    return static_cast< ReturnType >( ::std::min( ::std::max( nValue, nMin ), nMax ) );
}

// Read from bitfields --------------------------------------------------------

/** Returns true, if at least one of the bits set in nMask is set in nBitField. */
template< typename Type >
inline bool getFlag( Type nBitField, Type nMask )
{
    return (nBitField & nMask) != 0;
}

/** Returns nSet, if at least one bit of nMask is set in nBitField, otherwise nUnset. */
template< typename ReturnType, typename Type >
inline ReturnType getFlagValue( Type nBitField, Type nMask, ReturnType nSet, ReturnType nUnset )
{
    return getFlag( nBitField, nMask ) ? nSet : nUnset;
}

/** Extracts a value from a bit field.

    Returns the data fragment from nBitField, that starts at bit nStartBit
    (0-based, bit 0 is rightmost) with the width of nBitCount. The returned
    value will be right-aligned (normalized).
    For instance: extractValue<T>(0x4321,8,4) returns 3 (value in bits 8-11).
 */
template< typename ReturnType, typename Type >
inline ReturnType extractValue( Type nBitField, sal_uInt8 nStartBit, sal_uInt8 nBitCount )
{
    sal_uInt64 nMask = 1; nMask <<= nBitCount; --nMask;
    return static_cast< ReturnType >( nMask & (nBitField >> nStartBit) );
}

// Write to bitfields ---------------------------------------------------------

/** Sets or clears (according to bSet) all set bits of nMask in ornBitField. */
template< typename Type >
inline void setFlag( Type& ornBitField, Type nMask, bool bSet = true )
{
    if( bSet ) ornBitField |= nMask; else ornBitField &= ~nMask;
}

/** Inserts a value into a bitfield.

    Inserts the lower nBitCount bits of nValue into ornBitField, starting
    there at bit nStartBit. Other contents of ornBitField keep unchanged.
 */
template< typename Type, typename InsertType >
void insertValue( Type& ornBitField, InsertType nValue, sal_uInt8 nStartBit, sal_uInt8 nBitCount )
{
    sal_uInt64 nMask = 1; nMask <<= nBitCount; --nMask;
    Type nNewValue = static_cast< Type >( nValue & nMask );
    (ornBitField &= ~(nMask << nStartBit)) |= (nNewValue << nStartBit);
}

// ============================================================================

/** Provides platform independent functions to convert from or to little-endian
    byte order, e.g. for reading data from or writing data to memory or a
    binary stream.

    On big-endian platforms, the byte order in the passed values is swapped,
    this can be used for converting big-endian to and from little-endian data.

    On little-endian platforms, the conversion functions are implemented empty,
    thus compilers should completely optimize away the function call.
 */
class ByteOrderConverter
{
public:
    inline static void  convertLittleEndian( sal_Int8& ) {}
    inline static void  convertLittleEndian( sal_uInt8& ) {}
#ifdef OSL_BIGENDIAN
    inline static void  convertLittleEndian( sal_Int16& rnValue )  { swap2( reinterpret_cast< sal_uInt8* >( &rnValue ) ); }
    inline static void  convertLittleEndian( sal_uInt16& rnValue ) { swap2( reinterpret_cast< sal_uInt8* >( &rnValue ) ); }
    inline static void  convertLittleEndian( sal_Int32& rnValue )  { swap4( reinterpret_cast< sal_uInt8* >( &rnValue ) ); }
    inline static void  convertLittleEndian( sal_uInt32& rnValue ) { swap4( reinterpret_cast< sal_uInt8* >( &rnValue ) ); }
    inline static void  convertLittleEndian( sal_Int64& rnValue )  { swap8( reinterpret_cast< sal_uInt8* >( &rnValue ) ); }
    inline static void  convertLittleEndian( sal_uInt64& rnValue ) { swap8( reinterpret_cast< sal_uInt8* >( &rnValue ) ); }
    inline static void  convertLittleEndian( float& rfValue )      { swap4( reinterpret_cast< sal_uInt8* >( &rfValue ) ); }
    inline static void  convertLittleEndian( double& rfValue )     { swap8( reinterpret_cast< sal_uInt8* >( &rfValue ) ); }
#else
    inline static void  convertLittleEndian( sal_Int16& ) {}
    inline static void  convertLittleEndian( sal_uInt16& ) {}
    inline static void  convertLittleEndian( sal_Int32& ) {}
    inline static void  convertLittleEndian( sal_uInt32& ) {}
    inline static void  convertLittleEndian( sal_Int64& ) {}
    inline static void  convertLittleEndian( sal_uInt64& ) {}
    inline static void  convertLittleEndian( float& ) {}
    inline static void  convertLittleEndian( double& ) {}
#endif

    /** Reads a value from memory, assuming memory buffer in little-endian.
        @param ornValue  (out-parameter) Contains the value read from memory.
        @param pSrcBuffer  The memory buffer to read the value from.
     */
    template< typename Type >
    inline static void  readLittleEndian( Type& ornValue, const void* pSrcBuffer );

    /** Writes a value to memory, while converting it to little-endian.
        @param pDstBuffer  The memory buffer to write the value to.
        @param nValue  The value to be written to memory in little-endian.
     */
    template< typename Type >
    inline static void  writeLittleEndian( void* pDstBuffer, Type nValue );

#ifdef OSL_BIGENDIAN
private:
    inline static void  swap( sal_uInt8& rnData1, sal_uInt8& rnData2 );
    inline static void  swap2( sal_uInt8* pnData );
    inline static void  swap4( sal_uInt8* pnData );
    inline static void  swap8( sal_uInt8* pnData );
#endif
};

// ----------------------------------------------------------------------------

template< typename Type >
inline void ByteOrderConverter::readLittleEndian( Type& ornValue, const void* pSrcBuffer )
{
    memcpy( &ornValue, pSrcBuffer, sizeof( Type ) );
    convertLittleEndian( ornValue );
}

template< typename Type >
inline void ByteOrderConverter::writeLittleEndian( void* pDstBuffer, Type nValue )
{
    convertLittleEndian( nValue );
    memcpy( pDstBuffer, &nValue, sizeof( Type ) );
}

#ifdef OSL_BIGENDIAN
inline void ByteOrderConverter::swap( sal_uInt8& rnData1, sal_uInt8& rnData2 )
{
    rnData1 ^= rnData2 ^= rnData1 ^= rnData2;
}

inline void ByteOrderConverter::swap2( sal_uInt8* pnData )
{
    swap( pnData[ 0 ], pnData[ 1 ] );
}

inline void ByteOrderConverter::swap4( sal_uInt8* pnData )
{
    swap( pnData[ 0 ], pnData[ 3 ] );
    swap( pnData[ 1 ], pnData[ 2 ] );
}

inline void ByteOrderConverter::swap8( sal_uInt8* pnData )
{
    swap( pnData[ 0 ], pnData[ 7 ] );
    swap( pnData[ 1 ], pnData[ 6 ] );
    swap( pnData[ 2 ], pnData[ 5 ] );
    swap( pnData[ 3 ], pnData[ 4 ] );
}
#endif

// ============================================================================

} // namespace oox

#endif

