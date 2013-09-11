/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OOX_OLE_AXBINARYWRITER_HXX
#define OOX_OLE_AXBINARYWRITER_HXX

#include <utility>
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/helper/refvector.hxx"

namespace oox {
namespace ole {
// ============================================================================

/** A wrapper for a binary output stream that supports aligned write operations.

    The implementation does support seeking back the wrapped stream. All
    seeking operations (tell, seekTo, align) are performed relative to the
    position of the wrapped stream at construction time of this wrapper.
    Unlike it's reader class counterpart it is NOT possible to construct this
    wrapper with an unseekable output stream.
 */
class AxAlignedOutputStream : public BinaryOutputStream
{
public:
    explicit            AxAlignedOutputStream( BinaryOutputStream& rOutStrm );

    /** Returns the size of the data this stream represents, if the wrapped
        stream supports the size() operation. */
    virtual sal_Int64   size() const;
    /** Return the current relative stream position (relative to position of
        the wrapped stream at construction time). */
    virtual sal_Int64   tell() const;
    /** Seeks the stream to the passed relative position, if it is behind the
        current position. */
    virtual void        seek( sal_Int64 nPos );
    /** Closes the input stream but not the wrapped stream. */
    virtual void        close();

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual void writeData( const StreamDataSequence& orData, size_t nAtomSize = 1 );
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual void   writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Aligns the stream to a multiple of the passed size (relative to the
        position of the wrapped stream at construction time). */
    void                align( size_t nSize );

    void         pad( sal_Int32 nBytes, size_t nAtomSize = 1);
    /** Aligns the stream according to the passed type and reads a value. */
    template< typename Type >
    void         writeAligned( Type nVal ) { align( sizeof( Type ) ); writeValue( nVal ); }
    /** Aligns the stream according to the passed type and skips the size of the type. */
    template< typename Type >
    void         padAligned() { align( sizeof( Type ) ); pad( sizeof( Type ) ); }

private:
    BinaryOutputStream*  mpOutStrm;           ///< The wrapped input stream.
    sal_Int64           mnStrmPos;          ///< Tracks relative position in the stream.
    sal_Int64           mnStrmSize;         ///< Size of the wrapped stream data.
    sal_Int64           mnWrappedBeginPos;     ///< starting pos or wrapped stream
};

/** A pair of integer values as a property. */
typedef ::std::pair< sal_Int32, sal_Int32 > AxPairData;

/** An array of string values as a property. */
typedef ::std::vector< OUString > AxStringArray;

// ============================================================================

/** Export helper to write simple and complex ActiveX form control properties
    to a binary input stream. */
class AxBinaryPropertyWriter
{
public:
    explicit            AxBinaryPropertyWriter( BinaryOutputStream& rOutStrm, bool b64BitPropFlags = false );

    /** Write an integer property value to the stream, the
        respective flag in the property mask is set. */
    template< typename StreamType, typename DataType >
    void                writeIntProperty( DataType& ornValue )
                            { if( startNextProperty() ) maOutStrm.writeAligned< StreamType >( ornValue ); }
    /** Write a boolean property value to the stream, the
        respective flag in the property mask is set. */
    void                writeBoolProperty( bool orbValue, bool bReverse = false );
    /** Write a pair property the stream, the respective flag in
        the property mask is set. */
    void                writePairProperty( AxPairData& orPairData );
    /** Write a string property to the stream, the respective flag
        in the property mask is set. */
    void                writeStringProperty( OUString& orValue, bool bCompressed = true );

    /** Skips the next property clears the respective
        flag in the property mask. */
    void                skipProperty() { startNextProperty( true ); }

    /** Final processing, write contents of all complex properties, writes record size */
    bool                finalizeExport();

private:
    bool                ensureValid( bool bCondition = true );
    bool                startNextProperty( bool bSkip = false );

private:
    /** Base class for complex properties such as string, point, size, GUID, picture. */
    struct ComplexProperty
    {
        virtual             ~ComplexProperty();
        virtual bool        writeProperty( AxAlignedOutputStream& rOutStrm ) = 0;
    };

    /** Complex property for a 32-bit value pair, e.g. point or size. */
    struct PairProperty : public ComplexProperty
    {
        AxPairData&         mrPairData;

        explicit            PairProperty( AxPairData& rPairData ) :
                                mrPairData( rPairData ) {}
        virtual bool        writeProperty( AxAlignedOutputStream& rOutStrm );
    };

    /** Complex property for a string value. */
    struct StringProperty : public ComplexProperty
    {
        OUString&    mrValue;
        sal_uInt32          mnSize;

        explicit            StringProperty( OUString& rValue, sal_uInt32 nSize ) :
                                mrValue( rValue ), mnSize( nSize ) {}
        virtual bool        writeProperty( AxAlignedOutputStream& rOutStrm );
    };

    /** Stream property for a picture or mouse icon. */
    struct PictureProperty : public ComplexProperty
    {
        StreamDataSequence& mrPicData;

        explicit            PictureProperty( StreamDataSequence& rPicData ) :
                                mrPicData( rPicData ) {}
        virtual bool        writeProperty( AxAlignedOutputStream& rOutStrm );
    };

    typedef RefVector< ComplexProperty > ComplexPropVector;

private:
    AxAlignedOutputStream maOutStrm;        ///< The input stream to read from.
    ComplexPropVector   maLargeProps;       ///< Stores info for all used large properties.
    ComplexPropVector   maStreamProps;      ///< Stores info for all used stream data properties.
    AxPairData          maDummyPairData;    ///< Dummy pair for unsupported properties.
    StreamDataSequence  maDummyPicData;     ///< Dummy picture for unsupported properties.
    OUString     maDummyString;      ///< Dummy string for unsupported properties.
    AxStringArray       maDummyStringArray; ///< Dummy string array for unsupported properties.
    sal_Int16           mnBlockSize;
    sal_Int64           mnPropFlagsStart;     ///< pos of Prop flags
    sal_Int64           mnPropFlags;        ///< Flags specifying existing properties.
    sal_Int64           mnNextProp;         ///< Next property to read.
    bool                mbValid;            ///< True = stream still valid.
    bool                mb64BitPropFlags;
};

// ============================================================================
} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
