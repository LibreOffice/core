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

#include <oox/ole/vbamodule.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <filter/msfilter/msvbahelper.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/storagebase.hxx>
#include <oox/helper/textinputstream.hxx>
#include <oox/ole/vbahelper.hxx>
#include <oox/ole/vbainputstream.hxx>

namespace oox::ole {

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script::vba;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

using ::com::sun::star::awt::KeyEvent;

VbaModule::VbaModule( const Reference< XComponentContext >& rxContext,
                      const Reference< frame::XModel >& rxDocModel,
                      const OUString& rName, rtl_TextEncoding eTextEnc, bool bExecutable ) :
    mxContext( rxContext ),
    mxDocModel( rxDocModel ),
    maName( rName ),
    meTextEnc( eTextEnc ),
    mnType( script::ModuleType::UNKNOWN ),
    mnOffset( SAL_MAX_UINT32 ),
    mbReadOnly( false ),
    mbPrivate( false ),
    mbExecutable( bExecutable )
{
}

void VbaModule::importDirRecords( BinaryInputStream& rDirStrm )
{
    sal_uInt16 nRecId = 0;
    StreamDataSequence aRecData;
    while( VbaHelper::readDirRecord( nRecId, aRecData, rDirStrm ) && (nRecId != VBA_ID_MODULEEND) )
    {
        SequenceInputStream aRecStrm( aRecData );
        sal_Int32 nRecSize = aRecData.getLength();
        switch( nRecId )
        {
#define OOX_ENSURE_RECORDSIZE( cond ) OSL_ENSURE( cond, "VbaModule::importDirRecords - invalid record size" )
            case VBA_ID_MODULENAME:
                OSL_FAIL( "VbaModule::importDirRecords - unexpected MODULENAME record" );
                maName = aRecStrm.readCharArrayUC( nRecSize, meTextEnc );
            break;
            case VBA_ID_MODULENAMEUNICODE:
            break;
            case VBA_ID_MODULESTREAMNAME:
                maStreamName = aRecStrm.readCharArrayUC( nRecSize, meTextEnc );
                // Actually the stream name seems the best name to use
                // the VBA_ID_MODULENAME name can sometimes be the wrong case
                maName = maStreamName;
            break;
            case VBA_ID_MODULESTREAMNAMEUNICODE:
            break;
            case VBA_ID_MODULEDOCSTRING:
                maDocString = aRecStrm.readCharArrayUC( nRecSize, meTextEnc );
            break;
            case VBA_ID_MODULEDOCSTRINGUNICODE:
            break;
            case VBA_ID_MODULEOFFSET:
                OOX_ENSURE_RECORDSIZE( nRecSize == 4 );
                mnOffset = aRecStrm.readuInt32();
            break;
            case VBA_ID_MODULEHELPCONTEXT:
                OOX_ENSURE_RECORDSIZE( nRecSize == 4 );
            break;
            case VBA_ID_MODULECOOKIE:
                OOX_ENSURE_RECORDSIZE( nRecSize == 2 );
            break;
            case VBA_ID_MODULETYPEPROCEDURAL:
                OOX_ENSURE_RECORDSIZE( nRecSize == 0 );
                OSL_ENSURE( mnType == script::ModuleType::UNKNOWN, "VbaModule::importDirRecords - multiple module type records" );
                mnType = script::ModuleType::NORMAL;
            break;
            case VBA_ID_MODULETYPEDOCUMENT:
                OOX_ENSURE_RECORDSIZE( nRecSize == 0 );
                OSL_ENSURE( mnType == script::ModuleType::UNKNOWN, "VbaModule::importDirRecords - multiple module type records" );
                mnType = script::ModuleType::DOCUMENT;
            break;
            case VBA_ID_MODULEREADONLY:
                OOX_ENSURE_RECORDSIZE( nRecSize == 0 );
                mbReadOnly = true;
            break;
            case VBA_ID_MODULEPRIVATE:
                OOX_ENSURE_RECORDSIZE( nRecSize == 0 );
                mbPrivate = true;
            break;
            default:
                OSL_FAIL( "VbaModule::importDirRecords - unknown module record" );
#undef OOX_ENSURE_RECORDSIZE
        }
    }
    OSL_ENSURE( !maName.isEmpty(), "VbaModule::importDirRecords - missing module name" );
    OSL_ENSURE( !maStreamName.isEmpty(), "VbaModule::importDirRecords - missing module stream name" );
    OSL_ENSURE( mnType != script::ModuleType::UNKNOWN, "VbaModule::importDirRecords - missing module type" );
    OSL_ENSURE( mnOffset < SAL_MAX_UINT32, "VbaModule::importDirRecords - missing module stream offset" );
}

void VbaModule::createAndImportModule( StorageBase& rVbaStrg,
                                       const Reference< container::XNameContainer >& rxBasicLib,
                                       const Reference< container::XNameAccess >& rxDocObjectNA ) const
{
    OUString aVBASourceCode = readSourceCode( rVbaStrg );
    createModule( aVBASourceCode, rxBasicLib, rxDocObjectNA );
}

void VbaModule::createEmptyModule( const Reference< container::XNameContainer >& rxBasicLib,
                                   const Reference< container::XNameAccess >& rxDocObjectNA ) const
{
    createModule( u"", rxBasicLib, rxDocObjectNA );
}

OUString VbaModule::readSourceCode( StorageBase& rVbaStrg ) const
{
    OUStringBuffer aSourceCode(512);
    static const char sUnmatchedRemovedTag[] = "Rem removed unmatched Sub/End: ";
    if( !maStreamName.isEmpty() && (mnOffset != SAL_MAX_UINT32) )
    {
        BinaryXInputStream aInStrm( rVbaStrg.openInputStream( maStreamName ), true );
        OSL_ENSURE( !aInStrm.isEof(), "VbaModule::readSourceCode - cannot open module stream" );
        // skip the 'performance cache' stored before the actual source code
        aInStrm.seek( mnOffset );
        // if stream is still valid, load the source code
        if( !aInStrm.isEof() )
        {
            // decompression starts at current stream position of aInStrm
            VbaInputStream aVbaStrm( aInStrm );
            // load the source code line-by-line, with some more processing
            TextInputStream aVbaTextStrm( mxContext, aVbaStrm, meTextEnc );

            struct ProcedurePair
            {
                bool bInProcedure;
                sal_uInt32 nPos;
                ProcedurePair() : bInProcedure( false ), nPos( 0 ) {};
            } procInfo;

            while( !aVbaTextStrm.isEof() )
            {
                OUString aCodeLine = aVbaTextStrm.readLine();
                if( aCodeLine.match( "Attribute " ) )
                {
                    // attribute
                    int index = aCodeLine.indexOf( ".VB_ProcData.VB_Invoke_Func = " );
                    if ( index != -1 )
                    {
                        // format is
                        //    'Attribute Procedure.VB_ProcData.VB_Invoke_Func = "*\n14"'
                        //    where 'Procedure' is the procedure name and '*' is the shortcut key
                        // note: his is only relevant for Excel, seems that
                        // word doesn't store the shortcut in the module
                        // attributes
                        int nSpaceIndex = aCodeLine.indexOf(' ');
                        OUString sProc = aCodeLine.copy( nSpaceIndex + 1, index - nSpaceIndex - 1);
                        // for Excel short cut key seems limited to cntrl+'a-z, A-Z'
                        OUString sKey = aCodeLine.copy( aCodeLine.lastIndexOf("= ") + 3, 1 );
                        // only alpha key valid for key shortcut, however the api will accept other keys
                        if ( rtl::isAsciiAlpha( sKey[ 0 ] ) )
                        {
                            // cntrl modifier is explicit ( but could be cntrl+shift ), parseKeyEvent
                            // will handle and uppercase letter appropriately
                            OUString sApiKey = "^" + sKey;
                            try
                            {
                                KeyEvent aKeyEvent = ooo::vba::parseKeyEvent( sApiKey );
                                ooo::vba::applyShortCutKeyBinding( mxDocModel, aKeyEvent, sProc );
                            }
                            catch (const Exception&)
                            {
                            }
                        }
                    }
                }
                else
                {
                    // Hack here to weed out any unmatched End Sub / Sub Foo statements.
                    // The behaviour of the vba ide practically guarantees the case and
                    // spacing of Sub statement(s). However, indentation can be arbitrary hence
                    // the trim.
                    OUString trimLine( aCodeLine.trim() );
                    if ( mbExecutable && (
                      trimLine.match("Sub ")         ||
                      trimLine.match("Public Sub ")  ||
                      trimLine.match("Private Sub ") ||
                      trimLine.match("Static Sub ") ) )
                    {
                        // this should never happen, basic doesn't support nested procedures
                        // first Sub Foo must be bogus
                        if ( procInfo.bInProcedure )
                        {
                            // comment out the line
                            aSourceCode.insert( procInfo.nPos, sUnmatchedRemovedTag );
                            // mark location of this Sub
                            procInfo.nPos = aSourceCode.getLength();
                        }
                        else
                        {
                            procInfo.bInProcedure = true;
                            procInfo.nPos = aSourceCode.getLength();
                        }
                    }
                    else if ( mbExecutable && aCodeLine.trim().match("End Sub") )
                    {
                        // un-matched End Sub
                        if ( !procInfo.bInProcedure )
                        {
                            aSourceCode.append( sUnmatchedRemovedTag );
                        }
                        else
                        {
                            procInfo.bInProcedure = false;
                            procInfo.nPos = 0;
                        }
                    }
                    // normal source code line
                    if( !mbExecutable )
                        aSourceCode.append( "Rem " );
                    aSourceCode.append( aCodeLine ).append( '\n' );
                }
            }
        }
    }
    return aSourceCode.makeStringAndClear();
}

void VbaModule::createModule( std::u16string_view rVBASourceCode,
                              const Reference< container::XNameContainer >& rxBasicLib,
                              const Reference< container::XNameAccess >& rxDocObjectNA ) const
{
    if( maName.isEmpty() )
        return;

    // prepare the Basic module
    script::ModuleInfo aModuleInfo;
    aModuleInfo.ModuleType = mnType;
    OUStringBuffer aSourceCode(512);
    aSourceCode.append( "Rem Attribute VBA_ModuleType=" );
    switch( mnType )
    {
        case script::ModuleType::NORMAL:
            aSourceCode.append( "VBAModule" );
        break;
        case script::ModuleType::CLASS:
            aSourceCode.append( "VBAClassModule" );
        break;
        case script::ModuleType::FORM:
            aSourceCode.append( "VBAFormModule" );
            // hack from old filter, document Basic should know the XModel, but it doesn't
            aModuleInfo.ModuleObject.set( mxDocModel, UNO_QUERY );
        break;
        case script::ModuleType::DOCUMENT:
            aSourceCode.append( "VBADocumentModule" );
            // get the VBA implementation object associated to the document module
            if( rxDocObjectNA.is() ) try
            {
                aModuleInfo.ModuleObject.set( rxDocObjectNA->getByName( maName ), UNO_QUERY );
            }
            catch (const Exception&)
            {
            }
        break;
        default:
            aSourceCode.append( "VBAUnknown" );
    }
    aSourceCode.append( '\n' );
    if( mbExecutable )
    {
        aSourceCode.append( "Option VBASupport 1\n" );
        if( mnType == script::ModuleType::CLASS )
            aSourceCode.append( "Option ClassModule\n" );
    }
    else
    {
        // add a subroutine named after the module itself
        aSourceCode.append( "Sub " + maName.replace( ' ', '_' ) + "\n" );
    }

    // append passed VBA source code
    aSourceCode.append( rVBASourceCode );

    // close the subroutine named after the module
    if( !mbExecutable )
        aSourceCode.append( "End Sub\n" );

    // insert extended module info
    try
    {
        Reference< XVBAModuleInfo > xVBAModuleInfo( rxBasicLib, UNO_QUERY_THROW );
        xVBAModuleInfo->insertModuleInfo( maName, aModuleInfo );
    }
    catch (const Exception&)
    {
    }

    // insert the module into the passed Basic library
    try
    {
        rxBasicLib->insertByName( maName, Any( aSourceCode.makeStringAndClear() ) );
    }
    catch (const Exception&)
    {
        OSL_FAIL( "VbaModule::createModule - cannot insert module into library" );
    }
}

} // namespace oox::ole

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
