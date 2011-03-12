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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idl.hxx"

#include <stdlib.h>
#include <stdio.h>
#include <database.hxx>
#include <globals.hxx>
#include <command.hxx>
#include <tools/fsys.hxx>
#include <tools/string.hxx>

#define BR 0x8000
sal_Bool FileMove_Impl( const String & rFile1, const String & rFile2, sal_Bool bImmerVerschieben )
{
    //printf( "Move from %s to %s\n", rFile2.GetStr(), rFile1.GetStr() );
    sal_uLong nC1 = 0;
    sal_uLong nC2 = 1;
    if( !bImmerVerschieben )
    {
        SvFileStream aOutStm1( rFile1, STREAM_STD_READ );
        SvFileStream aOutStm2( rFile2, STREAM_STD_READ );
        if( aOutStm1.GetError() == SVSTREAM_OK )
        {
            sal_uInt8 * pBuf1 = new sal_uInt8[ BR ];
            sal_uInt8 * pBuf2 = new sal_uInt8[ BR ];
            nC1 = aOutStm1.Read( pBuf1, BR );
            nC2 = aOutStm2.Read( pBuf2, BR );
            while( nC1 == nC2 )
            {
                if( memcmp( pBuf1, pBuf2, nC1 ) )
                {
                    nC1++;
                    break;
                }
                else
                {
                    if( 0x8000 != nC1 )
                        break;
                    nC1 = aOutStm1.Read( pBuf1, BR );
                    nC2 = aOutStm2.Read( pBuf2, BR );
                }
            }
            delete[] pBuf1;
            delete[] pBuf2;
        }
    }
    DirEntry aF2( rFile2 );
    if( nC1 != nC2 )
    {// something has changed
        DirEntry aF1( rFile1 );
        aF1.Kill();
        // move file
        if( aF2.MoveTo( aF1 ) )
        {
            // delete both files
            aF1.Kill();
            aF2.Kill();
            return sal_False;
        }
        return sal_True;
    }
    return 0 == aF2.Kill();
}

#if defined( UNX ) || defined( PM2 ) || defined (__MINGW32__) || defined( OS2 )
int main ( int argc, char ** argv)
{
#else
int cdecl main ( int argc, char ** argv)
{
#endif

    String aTmpListFile;
    String aTmpSlotMapFile;
    String aTmpSfxItemFile;
    String aTmpDataBaseFile;
    String aTmpCallingFile;
    String aTmpSrcFile;
    String aTmpCxxFile;
    String aTmpHxxFile;
    String aTmpHelpIdFile;
    String aTmpCSVFile;
    String aTmpDocuFile;

    SvCommand aCommand( argc, argv );

    if( aCommand.nVerbosity != 0 )
        printf( "StarView Interface Definition Language (IDL) Compiler 3.0\n" );

    Init();
    SvIdlWorkingBase * pDataBase = new SvIdlWorkingBase(aCommand);

    int nExit = 0;
    if( aCommand.aExportFile.Len() )
    {
        DirEntry aDE( aCommand.aExportFile );
        pDataBase->SetExportFile( aDE.GetName() );
    }

    if( ReadIdl( pDataBase, aCommand ) )
    {
        if( nExit == 0 && aCommand.aDocuFile.Len() )
        {
            DirEntry aDE( aCommand.aDocuFile );
            aDE.ToAbs();
            aTmpDocuFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aOutStm( aTmpDocuFile, STREAM_READWRITE | STREAM_TRUNC );
            if( !pDataBase->WriteDocumentation( aOutStm ) )
            {
                nExit = -1;
                ByteString aStr = "cannot write documentation file: ";
                aStr += ByteString( aCommand.aDocuFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
        if( nExit == 0 && aCommand.aListFile.Len() )
        {
            DirEntry aDE( aCommand.aListFile );
            aDE.ToAbs();
            aTmpListFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aOutStm( aTmpListFile, STREAM_READWRITE | STREAM_TRUNC );
            if( !pDataBase->WriteSvIdl( aOutStm ) )
            {
                nExit = -1;
                ByteString aStr = "cannot write list file: ";
                aStr += ByteString( aCommand.aListFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
        if( nExit == 0 && aCommand.aSlotMapFile.Len() )
        {
            DirEntry aDE( aCommand.aSlotMapFile );
            aDE.ToAbs();
            aTmpSlotMapFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aOutStm( aTmpSlotMapFile, STREAM_READWRITE | STREAM_TRUNC );
            if( !pDataBase->WriteSfx( aOutStm ) )
            {
                nExit = -1;
                ByteString aStr = "cannot write slotmap file: ";
                aStr += ByteString( aCommand.aSlotMapFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
        if( nExit == 0 && aCommand.aHelpIdFile.Len() )
        {
            DirEntry aDE( aCommand.aHelpIdFile );
            aDE.ToAbs();
            aTmpHelpIdFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aStm( aTmpHelpIdFile, STREAM_READWRITE | STREAM_TRUNC );
            if (!pDataBase->WriteHelpIds( aStm ) )
            {
                nExit = -1;
                ByteString aStr = "cannot write help ID file: ";
                aStr += ByteString( aCommand.aHelpIdFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
        if( nExit == 0 && aCommand.aCSVFile.Len() )
        {
            DirEntry aDE( aCommand.aCSVFile );
            aDE.ToAbs();
            aTmpCSVFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aStm( aTmpCSVFile, STREAM_READWRITE | STREAM_TRUNC );
            if (!pDataBase->WriteCSV( aStm ) )
            {
                nExit = -1;
                ByteString aStr = "cannot write CSV file: ";
                aStr += ByteString( aCommand.aCSVFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
        if( nExit == 0 && aCommand.aSfxItemFile.Len() )
        {
            DirEntry aDE( aCommand.aSfxItemFile );
            aDE.ToAbs();
            aTmpSfxItemFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aOutStm( aTmpSfxItemFile, STREAM_READWRITE | STREAM_TRUNC );
            if( !pDataBase->WriteSfxItem( aOutStm ) )
            {
                nExit = -1;
                ByteString aStr = "cannot write item file: ";
                aStr += ByteString( aCommand.aSfxItemFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
        if( nExit == 0 && aCommand.aDataBaseFile.Len() )
        {
            DirEntry aDE( aCommand.aDataBaseFile );
            aDE.ToAbs();
            aTmpDataBaseFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aOutStm( aTmpDataBaseFile, STREAM_READWRITE | STREAM_TRUNC );
            pDataBase->Save( aOutStm, aCommand.nFlags );
            if( aOutStm.GetError() != SVSTREAM_OK )
            {
                nExit = -1;
                ByteString aStr = "cannot write database file: ";
                aStr += ByteString( aCommand.aDataBaseFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
    }
    else
        nExit = -1;

    if( nExit == 0 )
    {
        sal_Bool bErr = sal_False;
        sal_Bool bDoMove = aCommand.aTargetFile.Len() == 0;
        String aErrFile, aErrFile2;
        if( !bErr && aCommand.aListFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aListFile, aTmpListFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aListFile;
                aErrFile2 = aTmpListFile;
            }
        }
        if( !bErr && aCommand.aSlotMapFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aSlotMapFile, aTmpSlotMapFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aSlotMapFile;
                aErrFile2 = aTmpSlotMapFile;
            }
        }
        if( !bErr && aCommand.aSfxItemFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aSfxItemFile, aTmpSfxItemFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aSfxItemFile;
                aErrFile2 = aTmpSfxItemFile;
            }
        }
        if( !bErr && aCommand.aDataBaseFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aDataBaseFile, aTmpDataBaseFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aDataBaseFile;
                aErrFile2 = aTmpDataBaseFile;
            }
        }
        if( !bErr && aCommand.aCallingFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aCallingFile, aTmpCallingFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aCallingFile;
                aErrFile2 = aTmpCallingFile;
            }
        }
        if( !bErr && aCommand.aCxxFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aCxxFile, aTmpCxxFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aCxxFile;
                aErrFile2 = aTmpCxxFile;
            }
        }
        if( !bErr && aCommand.aHxxFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aHxxFile, aTmpHxxFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aHxxFile;
                aErrFile2 = aTmpHxxFile;
            }
        }
        if( !bErr && aCommand.aHelpIdFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aHelpIdFile, aTmpHelpIdFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aHelpIdFile;
                aErrFile2 = aTmpHelpIdFile;
            }
        }
        if( !bErr && aCommand.aCSVFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aCSVFile, aTmpCSVFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aCSVFile;
                aErrFile2 = aTmpCSVFile;
            }
        }
        if( !bErr && aCommand.aDocuFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aDocuFile, aTmpDocuFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aDocuFile;
                aErrFile2 = aTmpDocuFile;
            }
        }

        if( bErr )
        {
            nExit = -1;
            ByteString aStr = "cannot move file from: ";
            aStr += ByteString( aErrFile2, RTL_TEXTENCODING_UTF8 );
            aStr += "\n              to file: ";
            aStr += ByteString( aErrFile, RTL_TEXTENCODING_UTF8 );
            fprintf( stderr, "%s\n", aStr.GetBuffer() );
        }
        else
        {
            if( aCommand.aTargetFile.Len() )
            {
#ifdef ICC
                DirEntry aT(aCommand.aTargetFile);
                aT.Kill();
#endif
                // stamp file, because idl passed through correctly
                SvFileStream aOutStm( aCommand.aTargetFile,
                                STREAM_READWRITE | STREAM_TRUNC );
            }
        }
    }

    if( nExit != 0 )
    {
        if( aCommand.aListFile.Len() )
            DirEntry( aTmpListFile ).Kill();
        if( aCommand.aSlotMapFile.Len() )
            DirEntry( aTmpSlotMapFile ).Kill();
        if( aCommand.aSfxItemFile.Len() )
            DirEntry( aTmpSfxItemFile ).Kill();
        if( aCommand.aDataBaseFile.Len() )
            DirEntry( aTmpDataBaseFile ).Kill();
        if( aCommand.aCallingFile.Len() )
            DirEntry( aTmpCallingFile ).Kill();
        if( aCommand.aCxxFile.Len() )
            DirEntry( aTmpCxxFile ).Kill();
        if( aCommand.aHxxFile.Len() )
            DirEntry( aTmpHxxFile ).Kill();
    }

    delete pDataBase;
    DeInit();
    if( nExit != 0 )
        fprintf( stderr, "svidl terminated with errors\n" );
    return nExit;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
