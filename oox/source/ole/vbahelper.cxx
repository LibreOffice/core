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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <comphelper/string.hxx>
#include "oox/helper/binaryinputstream.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_SET_THROW;

namespace oox {
namespace ole {

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

/*static*/ bool VbaHelper::eatWhitespace( OUString& rCodeLine )
{
    sal_Int32 nIndex = 0;
    while( (nIndex < rCodeLine.getLength()) && ((rCodeLine[ nIndex ] == ' ') || (rCodeLine[ nIndex ] == '\t')) )
        ++nIndex;
    if( nIndex > 0 )
    {
        rCodeLine = rCodeLine.copy( nIndex );
        return true;
    }
    return false;
}

/*static*/ bool VbaHelper::eatKeyword( OUString& rCodeLine, const OUString& rKeyword )
{
    if( rCodeLine.matchIgnoreAsciiCase( rKeyword ) )
    {
        rCodeLine = rCodeLine.copy( rKeyword.getLength() );
        // success, if code line ends after keyword, or if whitespace follows
        return (rCodeLine.getLength() == 0) || eatWhitespace( rCodeLine );
    }
    return false;
}

/*static*/ OUString VbaHelper::getSourceCode( const Reference< XNameContainer >& rxBasicLib, const OUString& rModuleName )
{
    OUString aSourceCode;
    if( rxBasicLib.is() ) try
    {
        rxBasicLib->getByName( rModuleName ) >>= aSourceCode;
    }
    catch( Exception& )
    {
    }
    return aSourceCode;
}

namespace {

bool lclGetLine( OUString& rCodeLine, sal_Int32& rnIndex, const OUString& rSourceCode )
{
    if( rnIndex < rSourceCode.getLength() )
    {
        sal_Int32 nPosLF = rSourceCode.indexOf( '\n', rnIndex );
        if( nPosLF >= rnIndex )
        {
            rCodeLine = rSourceCode.copy( rnIndex, nPosLF - rnIndex ).trim();
            rnIndex = nPosLF + 1;
            return true;
        }
    }
    return false;
}

} // namespace

/*static*/ bool VbaHelper::hasMacro( const OUString& rSourceCode, const OUString& rMacroName )
{
    // scan all text lines for '[Public|Private] [Static] Sub <macroname> (...)'
    const OUString aPublic = CREATE_OUSTRING( "Public" );
    const OUString aPrivate = CREATE_OUSTRING( "Private" );
    const OUString aStatic = CREATE_OUSTRING( "Static" );
    const OUString aSub = CREATE_OUSTRING( "Sub" );

    OUString aCodeLine;
    sal_Int32 nIndex = 0;
    while( lclGetLine( aCodeLine, nIndex, rSourceCode ) )
    {
        // eat optional 'Private' or 'Public', but do not accept both keywords in a row (therefore the ||)
        eatKeyword( aCodeLine, aPublic ) || eatKeyword( aCodeLine, aPrivate );
        // eat optional 'Static'
        eatKeyword( aCodeLine, aStatic );
        // eat 'Sub' keyword, check if macro name follows
        if( eatKeyword( aCodeLine, aSub ) && aCodeLine.matchIgnoreAsciiCase( rMacroName ) )
        {
            // eat macro name and following whitespace
            aCodeLine = aCodeLine.copy( rMacroName.getLength() );
            eatWhitespace( aCodeLine );
            // opening bracket must follow the macro name
            if( (aCodeLine.getLength() >= 2) && (aCodeLine[ 0 ] == '(') )
                return true;
        }
    }
    return false;
}

/*static*/ bool VbaHelper::hasMacro( const Reference< XNameContainer >& rxBasicLib,
    const OUString& rModuleName, const OUString& rMacroName )
{
    return hasMacro( getSourceCode( rxBasicLib, rModuleName ), rMacroName );
}

/*static*/ bool VbaHelper::insertMacro( const Reference< XNameContainer >& rxBasicLib, const OUString& rModuleName,
        const OUString& rMacroName, const OUString& rMacroArgs, const OUString& rMacroType, const OUString& rMacroCode )
{
    if( rxBasicLib.is() ) try
    {
        // receive module source code and check that the specified macro does not exist
        OUString aSourceCode = getSourceCode( rxBasicLib, rModuleName );
        if( !hasMacro( aSourceCode, rMacroName ) )
        {
            bool bFunction = rMacroType.getLength() > 0;
            const sal_Char* pcSubFunc = bFunction ? "Function" : "Sub";
            OUStringBuffer aBuffer( aSourceCode );
            // generate the source code for the new macro
            aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "\nPrivate " ) ).
                appendAscii( pcSubFunc ).append( sal_Unicode( ' ' ) ).
                append( rMacroName ).append( sal_Unicode( '(' ) );
            if( rMacroArgs.getLength() > 0 )
                aBuffer.append( sal_Unicode( ' ' ) ).append( rMacroArgs ).append( sal_Unicode( ' ' ) );
            aBuffer.append( sal_Unicode( ')' ) );
            if( bFunction )
                aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( " As " ) ).append( rMacroType );
            aBuffer.append( sal_Unicode( '\n' ) );
            // replace all $MACRO placeholders with macro name
            if( rMacroCode.getLength() > 0 )
            {
                OUString aMacroCode = ::comphelper::string::searchAndReplaceAsciiL( rMacroCode, RTL_CONSTASCII_STRINGPARAM( "$MACRO" ), rMacroName );
                aBuffer.append( aMacroCode ).append( sal_Unicode( '\n' ) );
            }
            aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "End " ) ).appendAscii( pcSubFunc ).append( sal_Unicode( '\n' ) );
            rxBasicLib->replaceByName( rModuleName, Any( aBuffer.makeStringAndClear() ) );
            return true;
        }
    }
    catch( Exception& )
    {
    }
    return false;
}

/*static*/ bool VbaHelper::attachMacroToEvent( const Reference< XEventsSupplier >& rxEventsSupp,
        const OUString& rEventName, const OUString& rLibraryName, const OUString& rModuleName, const OUString& rMacroName )
{
    if( rxEventsSupp.is() ) try
    {
        Reference< XNameReplace > xEvents( rxEventsSupp->getEvents(), UNO_SET_THROW );
        Sequence< PropertyValue > aEvent( 2 );
        aEvent[ 0 ].Name = CREATE_OUSTRING( "EventType" );
        aEvent[ 0 ].Value <<= CREATE_OUSTRING( "Script" );
        aEvent[ 1 ].Name = CREATE_OUSTRING( "Script" );
        aEvent[ 1 ].Value <<= getBasicScriptUrl( rLibraryName, rModuleName, rMacroName );
        xEvents->replaceByName( rEventName, Any( aEvent ) );
        return true;
    }
    catch( Exception& )
    {
    }
    return false;
}

// ============================================================================

} // namespace ole
} // namespace oox
