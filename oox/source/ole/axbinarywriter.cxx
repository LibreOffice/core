/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "oox/ole/axbinarywriter.hxx"

#include "oox/ole/olehelper.hxx"

namespace oox {
namespace ole {

namespace {

const sal_uInt32 AX_STRING_COMPRESSED       = 0x80000000;

} // namespace

AxAlignedOutputStream::AxAlignedOutputStream( BinaryOutputStream& rOutStrm ) :
    BinaryStreamBase( false ),
    mpOutStrm( &rOutStrm ),
    mnStrmPos( 0 ),
    mnStrmSize( rOutStrm.getRemaining() ),
    mnWrappedBeginPos( rOutStrm.tell() )
{
    mbEof = mbEof || rOutStrm.isEof();
}

sal_Int64 AxAlignedOutputStream::size() const
{
    return mpOutStrm ? mnStrmSize : -1;
}

sal_Int64 AxAlignedOutputStream::tell() const
{
    return mpOutStrm ? mnStrmPos : -1;
}

void AxAlignedOutputStream::seek( sal_Int64 nPos )
{
    mbEof = (nPos < 0);
    if( !mbEof )
    {
        mpOutStrm->seek( static_cast< sal_Int32 >( mnWrappedBeginPos + nPos  ) );
        mnStrmPos = mpOutStrm->tell() - mnWrappedBeginPos;
    }
}

void AxAlignedOutputStream::close()
{
    mpOutStrm = 0;
    mbEof = true;
}

void AxAlignedOutputStream::writeData( const StreamDataSequence& orData, size_t nAtomSize )
{
    mpOutStrm->writeData( orData, nAtomSize );
    mnStrmPos = mpOutStrm->tell() - mnWrappedBeginPos;
}

void AxAlignedOutputStream::writeMemory( const void* opMem, sal_Int32 nBytes, size_t nAtomSize )
{
    mpOutStrm->writeMemory( opMem, nBytes, nAtomSize );
    mnStrmPos = mpOutStrm->tell() - mnWrappedBeginPos;
}

void AxAlignedOutputStream::pad( sal_Int32 nBytes, size_t nAtomSize )
{
   //PRESUMABLY we need to pad with 0's here as appropriate
   css::uno::Sequence< sal_Int8 > aData( nBytes );
   // ok we could be padding with rubbish here, but really that shouldn't matter
   // set to 0(s), easier to not get fooled by 0's when looking at
   // binary content......
   memset( static_cast<void*>( aData.getArray() ), 0, nBytes );
   mpOutStrm->writeData( aData, nAtomSize );
   mnStrmPos = mpOutStrm->tell() - mnWrappedBeginPos;
}

void AxAlignedOutputStream::align( size_t nSize )
{
    pad( static_cast< sal_Int32 >( (nSize - (mnStrmPos % nSize)) % nSize ) );
}

namespace {

void lclWriteString( AxAlignedOutputStream& rOutStrm, OUString& rValue, sal_uInt32 nSize, bool bArrayString )
{
    bool bCompressed = getFlag( nSize, AX_STRING_COMPRESSED );
    rOutStrm.writeCompressedUnicodeArray( rValue, bCompressed || bArrayString );
}

} // namespace

AxBinaryPropertyWriter::ComplexProperty::~ComplexProperty()
{
}

bool AxBinaryPropertyWriter::PairProperty::writeProperty( AxAlignedOutputStream& rOutStrm )
{
    rOutStrm.WriteInt32(mrPairData.first).WriteInt32(mrPairData.second);
    return true;
}

bool AxBinaryPropertyWriter::StringProperty::writeProperty( AxAlignedOutputStream& rOutStrm )
{
    lclWriteString( rOutStrm, mrValue, mnSize, false );
    return true;
}

AxBinaryPropertyWriter::AxBinaryPropertyWriter( BinaryOutputStream& rOutStrm, bool b64BitPropFlags ) :
    maOutStrm( rOutStrm ),
    mnPropFlags( 0x0 ),
    mbValid( true ),
    mb64BitPropFlags( b64BitPropFlags )
{
    sal_uInt16 nId( 0x0200 );
    maOutStrm.WriteUInt16(nId);
    mnBlockSize = 0; // will be filled in the finalize method

    maOutStrm.WriteUInt16(nId);
    mnPropFlagsStart = maOutStrm.tell();

    if( mb64BitPropFlags )
        maOutStrm.WriteInt64( mnPropFlags );
    else
        maOutStrm.WriteUInt32( mnPropFlags );
    mnNextProp = 1;
}

void AxBinaryPropertyWriter::writeBoolProperty( bool orbValue, bool bReverse )
{
    // orbValue == bReverse false then we want to set the bit, e.g. don't skip
    startNextProperty( orbValue == bReverse );
}

void AxBinaryPropertyWriter::writePairProperty( AxPairData& orPairData )
{
    if( startNextProperty() )
        maLargeProps.push_back( ComplexPropVector::value_type( new PairProperty( orPairData ) ) );
}

void AxBinaryPropertyWriter::writeStringProperty( OUString& orValue, bool bCompressed )
{
    sal_uInt32 nSize = orValue.getLength();
    if ( bCompressed )
        setFlag(  nSize, AX_STRING_COMPRESSED );
    else
        nSize *= 2;
    maOutStrm.writeAligned< sal_uInt32 >( nSize );
    maLargeProps.push_back( ComplexPropVector::value_type( new StringProperty( orValue, nSize ) ) );
    startNextProperty();
}

bool AxBinaryPropertyWriter::finalizeExport()
{
    // write large properties
    maOutStrm.align( 4 );
    if( !maLargeProps.empty() )
    {
        for( ComplexPropVector::iterator aIt = maLargeProps.begin(), aEnd = maLargeProps.end(); ensureValid() && (aIt != aEnd); ++aIt )
        {
            (*aIt)->writeProperty( maOutStrm );
            maOutStrm.align( 4 );
        }
    }

    mnBlockSize = maOutStrm.tell() - mnPropFlagsStart;

    // write stream properties (no stream alignment between properties!)
    if( !maStreamProps.empty() )
        for( ComplexPropVector::iterator aIt = maStreamProps.begin(), aEnd = maStreamProps.end(); ensureValid() && (aIt != aEnd); ++aIt )
           (*aIt)->writeProperty( maOutStrm );

    sal_Int64 nPos = maOutStrm.tell();
    maOutStrm.seek( mnPropFlagsStart - sizeof( mnBlockSize ) );

    maOutStrm.WriteInt16( mnBlockSize );

    if( mb64BitPropFlags )
        maOutStrm.WriteInt64( mnPropFlags );
    else
        maOutStrm.WriteUInt32( mnPropFlags );

    maOutStrm.seek( nPos );
    return true;
}

bool AxBinaryPropertyWriter::ensureValid( bool bCondition )
{
    mbValid = mbValid && bCondition && !maOutStrm.isEof();
    return mbValid;
}

bool AxBinaryPropertyWriter::startNextProperty( bool bSkip )
{
    // if we are skipping then we clear the flag
    setFlag( mnPropFlags, mnNextProp, !bSkip );
    mnNextProp <<= 1;
    return true;
}

} // namespace exp
} // namespace ole

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
