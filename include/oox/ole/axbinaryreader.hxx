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

#ifndef INCLUDED_OOX_OLE_AXBINARYREADER_HXX
#define INCLUDED_OOX_OLE_AXBINARYREADER_HXX

#include <utility>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/refvector.hxx>
#include <oox/ole/axfontdata.hxx>

namespace oox {
namespace ole {



/** A wrapper for a binary input stream that supports aligned read operations.

    The implementation does not support seeking back the wrapped stream. All
    seeking operations (tell, seekTo, align) are performed relative to the
    position of the wrapped stream at construction time of this wrapper. It is
    possible to construct this wrapper with an unseekable input stream without
    losing any functionality.
 */
class AxAlignedInputStream : public BinaryInputStream
{
public:
    explicit            AxAlignedInputStream( BinaryInputStream& rInStrm );

    /** Returns the size of the data this stream represents, if the wrapped
        stream supports the size() operation. */
    virtual sal_Int64   size() const override;
    /** Return the current relative stream position (relative to position of
        the wrapped stream at construction time). */
    virtual sal_Int64   tell() const override;
    /** Seeks the stream to the passed relative position, if it is behind the
        current position. */
    virtual void        seek( sal_Int64 nPos ) override;
    /** Closes the input stream but not the wrapped stream. */
    virtual void        close() override;

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize = 1 ) override;
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize = 1 ) override;
    /** Seeks the stream forward by the passed number of bytes. */
    virtual void        skip( sal_Int32 nBytes, size_t nAtomSize = 1 ) override;

    /** Aligns the stream to a multiple of the passed size (relative to the
        position of the wrapped stream at construction time). */
    void                align( size_t nSize );

    /** Aligns the stream according to the passed type and reads a value. */
    template< typename Type >
    SAL_WARN_UNUSED_RESULT
    Type                readAligned() { align( sizeof( Type ) ); return readValue< Type >(); }
    /** Aligns the stream according to the passed type and skips the size of the type. */
    template< typename Type >
    void                skipAligned() { align( sizeof( Type ) ); skip( sizeof( Type ) ); }

private:
    BinaryInputStream*  mpInStrm;           ///< The wrapped input stream.
    sal_Int64           mnStrmPos;          ///< Tracks relative position in the stream.
    sal_Int64           mnStrmSize;         ///< Size of the wrapped stream data.
};



/** A pair of integer values as a property. */
typedef ::std::pair< sal_Int32, sal_Int32 > AxPairData;

/** An array of string values as a property. */
typedef ::std::vector< OUString > AxArrayString;



/** Import helper to read simple and complex ActiveX form control properties
    from a binary input stream. */
class AxBinaryPropertyReader
{
public:
    explicit            AxBinaryPropertyReader( BinaryInputStream& rInStrm, bool b64BitPropFlags = false );

    /** Reads the next integer property value from the stream, if the
        respective flag in the property mask is set. */
    template< typename StreamType, typename DataType >
    void                readIntProperty( DataType& ornValue )
                            { if( startNextProperty() ) ornValue = maInStrm.readAligned< StreamType >(); }
    /** Reads the next boolean property value from the stream, if the
        respective flag in the property mask is set. */
    void                readBoolProperty( bool& orbValue, bool bReverse = false );
    /** Reads the next pair property from the stream, if the respective flag in
        the property mask is set. */
    void                readPairProperty( AxPairData& orPairData );
    /** Reads the next string property from the stream, if the respective flag
        in the property mask is set. */
    void                readStringProperty( OUString& orValue );
    /** Reads ArrayString, an array of fmString ( compressed or uncompressed )
        is read from the stream and inserted into rStrings */
    void                readArrayStringProperty( std::vector< OUString >& rStrings );
    /** Reads the next GUID property from the stream, if the respective flag
        in the property mask is set. The GUID will be enclosed in braces. */
    void                readGuidProperty( OUString& orGuid );
    /** Reads the next font property from the stream, if the respective flag in
        the property mask is set. */
    void                readFontProperty( AxFontData& orFontData );
    /** Reads the next picture property from the stream, if the respective flag
        in the property mask is set. */
    void                readPictureProperty( StreamDataSequence& orPicData );

    /** Skips the next integer property value in the stream, if the respective
        flag in the property mask is set. */
    template< typename StreamType >
    void                skipIntProperty() { if( startNextProperty() ) maInStrm.skipAligned< StreamType >(); }
    /** Skips the next boolean property value in the stream, if the respective
        flag in the property mask is set. */
    void                skipBoolProperty() { (void)startNextProperty(); }
    /** Skips the next string property in the stream, if the respective flag in
        the property mask is set. */
    void                skipStringProperty() { readStringProperty( maDummyString ); }
    /** Skips the next ArrayString property in the stream, if the respective flag in
        the property mask is set. */
    void                skipArrayStringProperty() { readArrayStringProperty( maDummyArrayString ); }
    /** Skips the next GUID property in the stream, if the respective flag in
        the property mask is set. */
    void                skipGuidProperty() { readGuidProperty( maDummyString ); }
    /** Skips the next picture property in the stream, if the respective flag
        in the property mask is set. */
    void                skipPictureProperty() { readPictureProperty( maDummyPicData ); }
    /** Has to be called for undefined properties. If the respective flag in
        the mask is set, the property import cannot be finished successfully. */
    void                skipUndefinedProperty() { ensureValid( !startNextProperty() ); }

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

        explicit            PairProperty( AxPairData& rPairData ) :
                                mrPairData( rPairData ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm ) override;
    };

    /** Complex property for a string value. */
    struct StringProperty : public ComplexProperty
    {
        OUString&    mrValue;
        sal_uInt32          mnSize;

        explicit            StringProperty( OUString& rValue, sal_uInt32 nSize ) :
                                mrValue( rValue ), mnSize( nSize ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm ) override;
    };

    /** Complex property for an array of strings. */
    struct ArrayStringProperty : public ComplexProperty
    {
        AxArrayString&      mrArray;
        sal_uInt32          mnSize;
        explicit            ArrayStringProperty( AxArrayString& rArray, sal_uInt32 nSize ) :
                                mrArray( rArray ), mnSize( nSize ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm ) override;
    };

    /** Complex property for a GUID value. */
    struct GuidProperty : public ComplexProperty
    {
        OUString&    mrGuid;

        explicit            GuidProperty( OUString& rGuid ) :
                                mrGuid( rGuid ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm ) override;
    };

    /** Stream property for a font structure. */
    struct FontProperty : public ComplexProperty
    {
        AxFontData&         mrFontData;

        explicit            FontProperty( AxFontData& rFontData ) :
                                mrFontData( rFontData ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm ) override;
    };

    /** Stream property for a picture or mouse icon. */
    struct PictureProperty : public ComplexProperty
    {
        StreamDataSequence& mrPicData;

        explicit            PictureProperty( StreamDataSequence& rPicData ) :
                                mrPicData( rPicData ) {}
        virtual bool        readProperty( AxAlignedInputStream& rInStrm ) override;
    };

    typedef RefVector< ComplexProperty > ComplexPropVector;

private:
    AxAlignedInputStream maInStrm;          ///< The input stream to read from.
    ComplexPropVector   maLargeProps;       ///< Stores info for all used large properties.
    ComplexPropVector   maStreamProps;      ///< Stores info for all used stream data properties.
    StreamDataSequence  maDummyPicData;     ///< Dummy picture for unsupported properties.
    OUString            maDummyString;      ///< Dummy string for unsupported properties.
    AxArrayString       maDummyArrayString; ///< Dummy strings for unsupported ArrayString properties.
    sal_Int64           mnPropFlags;        ///< Flags specifying existing properties.
    sal_Int64           mnNextProp;         ///< Next property to read.
    sal_Int64           mnPropsEnd;         ///< End position of simple/large properties.
    bool                mbValid;            ///< True = stream still valid.
};



} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
