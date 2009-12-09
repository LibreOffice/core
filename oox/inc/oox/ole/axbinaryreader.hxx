/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: axbinaryreader.hxx,v $
 * $Revision: 1.1 $
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

#ifndef OOX_OLE_AXBINARYREADER_HXX
#define OOX_OLE_AXBINARYREADER_HXX

#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/containerhelper.hxx"

namespace oox {
namespace ole {

// ============================================================================

/** A wrapper for an unseekable binary input stream. */
class AxAlignedInputStream : public BinaryInputStream
{
public:
    explicit            AxAlignedInputStream( BinaryInputStream& rInStrm );

    /** Return the current stream position (relative to position at construction time). */
    virtual sal_Int64   tell() const;
    /** Seeks the stream to the passed position, if it is behind the current position. */
    virtual void        seek( sal_Int64 nPos );

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes );
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes );
    /** Seeks the stream forward by the passed number of bytes. */
    virtual void        skip( sal_Int32 nBytes );

    /** Aligns the stream to a multiple of the passed size. */
    void                align( size_t nSize );

    /** Aligns the stream according to the passed type and reads an atomar value. */
    template< typename Type >
    inline Type         readAligned() { align( sizeof( Type ) ); return readValue< Type >(); }
    /** Aligns the stream according to the passed type and skips the size of the type. */
    template< typename Type >
    inline void         skipAligned() { align( sizeof( Type ) ); skip( sizeof( Type ) ); }

private:
    BinaryInputStream&  mrInStrm;           /// The wrapped input stream.
    sal_Int64           mnStrmPos;          /// Tracks relative position in the stream.
};

// ============================================================================

/** Import helper to read simple and complex ActiveX form control properties
    from a binary input stream. */
class AxBinaryPropertyReader
{
public:
    explicit            AxBinaryPropertyReader( BinaryInputStream& rInStrm, bool b64BitPropFlags = false );

    /** Reads the next integer property value from the stream, if the
        respective flag in the property mask is set. */
    template< typename StreamType, typename DataType >
    inline void         readIntProperty( DataType& ornValue )
                            { if( startNextProperty() ) ornValue = maInStrm.readAligned< StreamType >(); }
    /** Reads the next boolean property value from the stream, if the
        respective flag in the property mask is set. */
    void                readBoolProperty( bool& orbValue, bool bReverse = false );
    /** Reads the next pair property from the stream, if the respective flag in
        the property mask is set. */
    void                readPairProperty( sal_Int32& ornValue1, sal_Int32& ornValue2 );
    /** Reads the next string property from the stream, if the respective flag
        in the property mask is set. */
    void                readStringProperty( ::rtl::OUString& orValue );
    /** Reads the next picture property from the stream, if the respective flag
        in the property mask is set. */
    void                readPictureProperty( StreamDataSequence& orPicData );

    /** Skips the next integer property value in the stream, if the respective
        flag in the property mask is set. */
    template< typename StreamType >
    inline void         skipIntProperty() { if( startNextProperty() ) maInStrm.skipAligned< StreamType >(); }
    /** Skips the next boolean property value in the stream, if the respective
        flag in the property mask is set. */
    inline void         skipBoolProperty() { startNextProperty(); }
    /** Skips the next string property in the stream, if the respective flag in
        the property mask is set. */
    inline void         skipStringProperty() { readStringProperty( maDummyString ); }
    /** Skips the next picture property in the stream, if the respective flag
        in the property mask is set. */
    inline void         skipPictureProperty() { readPictureProperty( maDummyPicData ); }
    /** Has to be called for undefined properties. If the respective flag in
        the mask is set, the property import cannot be finished successfully. */
    inline void         skipUndefinedProperty() { ensureValid( !startNextProperty() ); }

    /** Final processing, reads contents of all complex properties. */
    bool                finalizeImport();

private:
    bool                ensureValid( bool bCondition = true );
    bool                startNextProperty();

private:
    /** Base class for complex properties such as string, point, size, GUID, picture. */
    struct ComplexProperty
    {
        virtual             ~ComplexProperty();
        virtual bool        readProperty( AxAlignedInputStream& rInStrm ) = 0;
    };

    /** Complex property for a 32-bit value pair, e.g. point or size. */
    struct PairProperty : public ComplexProperty
    {
        sal_Int32&          mrnValue1;
        sal_Int32&          mrnValue2;

        inline explicit     PairProperty( sal_Int32& rnValue1, sal_Int32& rnValue2 ) :
                                mrnValue1( rnValue1 ), mrnValue2( rnValue2 ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm );
    };

    /** Complex property for a string value. */
    struct StringProperty : public ComplexProperty
    {
        ::rtl::OUString&    mrValue;
        sal_uInt32          mnSize;

        inline explicit     StringProperty( ::rtl::OUString& rValue, sal_uInt32 nSize ) :
                                mrValue( rValue ), mnSize( nSize ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm );
    };

    /** Stream property for a picture or mouse icon. */
    struct PictureProperty : public ComplexProperty
    {
        StreamDataSequence& mrPicData;

        inline explicit     PictureProperty( StreamDataSequence& rPicData ) :
                                mrPicData( rPicData ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm );
    };

    typedef RefVector< ComplexProperty > ComplexPropVector;

private:
    AxAlignedInputStream maInStrm;          /// The input stream to read from.
    ComplexPropVector   maLargeProps;       /// Stores info for all used large properties.
    ComplexPropVector   maStreamProps;      /// Stores info for all used stream data properties.
    StreamDataSequence  maDummyPicData;     /// Dummy picture for unsupported properties.
    ::rtl::OUString     maDummyString;      /// Dummy string for unsupported properties.
    sal_Int64           mnPropFlags;        /// Flags specifying existing properties.
    sal_Int64           mnNextProp;         /// Next property to read.
    sal_Int64           mnPropsEnd;         /// End position of simple/large properties.
    bool                mbValid;            /// True = stream still valid.
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

