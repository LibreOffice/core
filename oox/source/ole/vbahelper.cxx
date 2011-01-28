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

#include "oox/ole/vbahelper.hxx"
#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryinputstream.hxx"

namespace oox {
namespace ole {

// ============================================================================

using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

/*static*/ OUString VbaHelper::getBasicScriptUrl(
        const OUString& rLibraryName, const OUString& rModuleName, const OUString& rMacroName )
{
    OSL_ENSURE( rLibraryName.getLength() > 0, "VbaHelper::getBasicScriptUrl - library name is empty" );
    OSL_ENSURE( rModuleName.getLength() > 0, "VbaHelper::getBasicScriptUrl - module name is empty" );
    OSL_ENSURE( rMacroName.getLength() > 0, "VbaHelper::getBasicScriptUrl - macro name is empty" );
    const sal_Unicode cDot = '.';
    return OUStringBuffer().
        appendAscii( RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.script:" ) ).
        append( rLibraryName ).append( cDot ).append( rModuleName ).append( cDot ).append( rMacroName ).
        appendAscii( RTL_CONSTASCII_STRINGPARAM( "?language=Basic&location=document" ) ).
        makeStringAndClear();
}

/*static*/ bool VbaHelper::readDirRecord( sal_uInt16& rnRecId, StreamDataSequence& rRecData, BinaryInputStream& rInStrm )
{
    // read the record header
    sal_Int32 nRecSize;
    rInStrm >> rnRecId >> nRecSize;
    // for no obvious reason, PROJECTVERSION record contains size field of 4, but is 6 bytes long
    if( rnRecId == VBA_ID_PROJECTVERSION )
    {
        OSL_ENSURE( nRecSize == 4, "VbaHelper::readDirRecord - unexpected record size for PROJECTVERSION" );
        nRecSize = 6;
    }
    // read the record contents into the passed sequence
    return !rInStrm.isEof() && (rInStrm.readData( rRecData, nRecSize ) == nRecSize);
}

/*static*/ bool VbaHelper::extractKeyValue( OUString& rKey, OUString& rValue, const OUString& rKeyValue )
{
    sal_Int32 nEqSignPos = rKeyValue.indexOf( '=' );
    if( nEqSignPos > 0 )
    {
        rKey = rKeyValue.copy( 0, nEqSignPos ).trim();
        rValue = rKeyValue.copy( nEqSignPos + 1 ).trim();
        return (rKey.getLength() > 0) && (rValue.getLength() > 0);
    }
    return false;
}

// ============================================================================

} // namespace ole
} // namespace oox
