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

#include "oox/helper/recordinputstream.hxx"
#include <vector>
#include <string.h>

using ::rtl::OUString;

namespace oox {

// ============================================================================

RecordInputStream::RecordInputStream( const StreamDataSequence& rData ) :
    SequenceInputStream( rData )
{
}

OUString RecordInputStream::readString( bool b32BitLen )
{
    OUString aString;
    if( !isEof() )
    {
        sal_Int32 nCharCount = b32BitLen ? readValue< sal_Int32 >() : readValue< sal_Int16 >();
        // string length -1 is often used to indicate a missing string
        OSL_ENSURE( !isEof() && (nCharCount >= -1), "RecordInputStream::readString - invalid string length" );
        if( !isEof() && (nCharCount > 0) )
        {
            ::std::vector< sal_Unicode > aBuffer;
            aBuffer.reserve( getLimitedValue< size_t, sal_Int32 >( nCharCount + 1, 0, 0xFFFF ) );
            for( sal_Int32 nCharIdx = 0; !isEof() && (nCharIdx < nCharCount); ++nCharIdx )
            {
                sal_uInt16 nChar;
                readValue( nChar );
                aBuffer.push_back( static_cast< sal_Unicode >( nChar ) );
            }
            aBuffer.push_back( 0 );
            aString = OUString( &aBuffer.front() );
        }
    }
    return aString;
}

// ============================================================================

} // namespace oox

