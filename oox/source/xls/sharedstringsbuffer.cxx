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

#include "oox/xls/sharedstringsbuffer.hxx"
#include "oox/xls/biffinputstream.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XText;

namespace oox {
namespace xls {

// ============================================================================

SharedStringsBuffer::SharedStringsBuffer( const WorkbookHelper& rHelper ) :
     WorkbookHelper( rHelper )
{
}

RichStringRef SharedStringsBuffer::createRichString()
{
    RichStringRef xString( new RichString( *this ) );
    maStrings.push_back( xString );
    return xString;
}

void SharedStringsBuffer::importSst( BiffInputStream& rStrm )
{
    rStrm.skip( 4 );
    sal_Int32 nStringCount = rStrm.readInt32();
    if( nStringCount > 0 )
    {
        maStrings.clear();
        maStrings.reserve( static_cast< size_t >( nStringCount ) );
        for( ; !rStrm.isEof() && (nStringCount > 0); --nStringCount )
        {
            RichStringRef xString( new RichString( *this ) );
            maStrings.push_back( xString );
            xString->importUniString( rStrm );
        }
    }
}

void SharedStringsBuffer::finalizeImport()
{
    maStrings.forEachMem( &RichString::finalizeImport );
}

void SharedStringsBuffer::convertString( const Reference< XText >& rxText, sal_Int32 nStringId, sal_Int32 nXfId ) const
{
    if( rxText.is() )
        if( const RichString* pString = maStrings.get( nStringId ).get() )
            pString->convert( rxText, nXfId );
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
