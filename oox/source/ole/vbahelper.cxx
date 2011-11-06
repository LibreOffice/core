/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
