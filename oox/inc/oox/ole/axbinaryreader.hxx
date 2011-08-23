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

#ifndef OOX_OLE_AXBINARYREADER_HXX
#define OOX_OLE_AXBINARYREADER_HXX

#include <utility>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/containerhelper.hxx"

namespace oox {
namespace ole {

// ============================================================================

/** A wrapper for a binary input stream that supports aligned read operations.

    The implementation does not support seeking back the wrapped stream. All
    seeking operations (tell, seek, align) are performed relative to the
    position of the wrapped stream at construction time of this wrapper. It is
    possible to construct this wrapper with an unseekable input stream without
    loosing any functionality.
 */
class AxAlignedInputStream : public BinaryInputStream
{
public:
    explicit            AxAlignedInputStream( BinaryInputStream& rInStrm );

    /** Return the current relative stream position (relative to position of
        the wrapped stream at construction time). */
    virtual sal_Int64   tell() const;
    /** Seeks the stream to the passed relative position, if it is behind the
        current position. */
    virtual void        seek( sal_Int64 nPos );

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes );
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes );
    /** Seeks the stream forward by the passed number of bytes. */
    virtual void        skip( sal_Int32 nBytes );

    /** Aligns the stream to a multiple of the passed size (relative to the
        position of the wrapped stream at construction time). */
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

/** A pair of integer values as a property. */
typedef ::std::pair< sal_Int32, sal_Int32 > AxPairData;

/** An array of string values as a property. */
typedef ::std::vector< ::rtl::OUString > AxStringArray;

// ============================================================================

const sal_uInt32 AX_FONTDATA_BOLD           = 0x00000001;
const sal_uInt32 AX_FONTDATA_ITALIC         = 0x00000002;
const sal_uInt32 AX_FONTDATA_UNDERLINE      = 0x00000004;
const sal_uInt32 AX_FONTDATA_STRIKEOUT      = 0x00000008;
const sal_uInt32 AX_FONTDATA_DISABLED       = 0x00002000;
const sal_uInt32 AX_FONTDATA_AUTOCOLOR      = 0x40000000;

const sal_Int32 AX_FONTDATA_LEFT            = 1;
const sal_Int32 AX_FONTDATA_RIGHT           = 2;
const sal_Int32 AX_FONTDATA_CENTER          = 3;

/** All entries of a font property. */
struct AxFontData
{
    ::rtl::OUString     maFontName;         /// Name of the used font.
    sal_uInt32          mnFontEffects;      /// Font effect flags.
    sal_Int32           mnFontHeight;       /// Height of the font (not really twips, see code).
    sal_Int32           mnFontCharSet;      /// Windows character set of the font.
    sal_Int32           mnHorAlign;         /// Horizontal text alignment.

    explicit            AxFontData();

    /** Converts the internal representation of the font height to points. */
    sal_Int16           getHeightPoints() const;
    /** Converts the passed font height from points to the internal representation. */
    void                setHeightPoints( sal_Int16 nPoints );
    
    /** Reads the font data settings from the passed input stream. */
    bool                importBinaryModel( BinaryInputStream& rInStrm );
    /** Reads the font data settings from the passed input stream that contains
        an OLE StdFont structure. */
    bool                importStdFont( BinaryInputStream& rInStrm );
    /** Reads the font data settings from the passed input stream depending on
        the GUID preceding the actual font data. */
    bool                importGuidAndFont( BinaryInputStream& rInStrm );
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
    void                readPairProperty( AxPairData& orPairData );
    /** Reads the next string property from the stream, if the respective flag
        in the property mask is set. */
    void                readStringProperty( ::rtl::OUString& orValue );
    /** Reads a string array property from the stream, if the respective flag
        in the property mask is set. */
    void                readStringArrayProperty( AxStringArray& orArray );
    /** Reads the next GUID property from the stream, if the respective flag
        in the property mask is set. The GUID will be enclosed in braces. */
    void                readGuidProperty( ::rtl::OUString& orGuid );
    /** Reads the next font property from the stream, if the respective flag in
        the property mask is set. */
    void                readFontProperty( AxFontData& orFontData );
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
    /** Skips the next pair property in the stream, if the respective flag in
        the property mask is set. */
    void                skipPairProperty() { readPairProperty( maDummyPairData ); }
    /** Skips the next string property in the stream, if the respective flag in
        the property mask is set. */
    inline void         skipStringProperty() { readStringProperty( maDummyString ); }
    /** Skips the next string array property in the stream, if the respective
        flag in the property mask is set. */
    inline void         skipStringArrayProperty() { readStringArrayProperty( maDummyStringArray ); }
    /** Skips the next GUID property in the stream, if the respective flag in
        the property mask is set. */
    inline void         skipGuidProperty() { readGuidProperty( maDummyString ); }
    /** Skips the next font property in the stream, if the respective flag in
        the property mask is set. */
    inline void         skipFontProperty() { readFontProperty( maDummyFontData ); }
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
        AxPairData&         mrPairData;

        inline explicit     PairProperty( AxPairData& rPairData ) :
                                mrPairData( rPairData ) {}
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

    /** Complex property for an array of strings. */
    struct StringArrayProperty : public ComplexProperty
    {
        AxStringArray&      mrArray;
        sal_uInt32          mnSize;
        inline explicit     StringArrayProperty( AxStringArray& rArray, sal_uInt32 nSize ) :
                                mrArray( rArray ), mnSize( nSize ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm );
    };

    /** Complex property for a GUID value. */
    struct GuidProperty : public ComplexProperty
    {
        ::rtl::OUString&    mrGuid;

        inline explicit     GuidProperty( ::rtl::OUString& rGuid ) :
                                mrGuid( rGuid ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm );
    };

    /** Stream property for a font structure. */
    struct FontProperty : public ComplexProperty
    {
        AxFontData&         mrFontData;

        inline explicit     FontProperty( AxFontData& rFontData ) :
                                mrFontData( rFontData ) {}
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
    AxPairData          maDummyPairData;    /// Dummy pair for unsupported properties.
    AxFontData          maDummyFontData;    /// Dummy font for unsupported properties.
    StreamDataSequence  maDummyPicData;     /// Dummy picture for unsupported properties.
    ::rtl::OUString     maDummyString;      /// Dummy string for unsupported properties.
    AxStringArray       maDummyStringArray; /// Dummy string array for unsupported properties.
    sal_Int64           mnPropFlags;        /// Flags specifying existing properties.
    sal_Int64           mnNextProp;         /// Next property to read.
    sal_Int64           mnPropsEnd;         /// End position of simple/large properties.
    bool                mbValid;            /// True = stream still valid.
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
