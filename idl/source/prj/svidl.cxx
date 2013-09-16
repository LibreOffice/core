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


#include <stdlib.h>
#include <stdio.h>
#include <database.hxx>
#include <globals.hxx>
#include <command.hxx>
#include <tools/string.hxx>
#include <osl/file.hxx>

#define BR 0x8000
sal_Bool FileMove_Impl( const OUString & rFile1, const OUString & rFile2, sal_Bool bImmerVerschieben )
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
    OUString fileURL2;
    osl::FileBase::getFileURLFromSystemPath( rFile2, fileURL2 );
    if( nC1 != nC2 )
    {// something has changed
        OUString fileURL1;
        osl::FileBase::getFileURLFromSystemPath( rFile1, fileURL1 );
        // move file
        if( osl::FileBase::E_None != osl::File::move( fileURL2, fileURL1 ) )
        {
            // delete both files
            osl::File::remove( fileURL1 );
            osl::File::remove( fileURL2 );
            return sal_False;
        }
        return sal_True;
    }
    return osl::FileBase::E_None == osl::File::remove( fileURL2 );
}

//This function gets a system path to a file [fname], creates a temp file in
//the same folder as [fname] and returns the system path of the temp file.
inline OUString tempFileHelper(OUString const & fname)
{
    OUString aTmpFile;

    sal_Int32 delimIndex = fname.lastIndexOf( '/' );
    if( delimIndex > 0 )
    {
        OUString aTmpDir( fname.copy( 0,  delimIndex ) );
        osl::FileBase::getFileURLFromSystemPath( aTmpDir, aTmpDir );
        osl::FileBase::createTempFile( &aTmpDir, 0, &aTmpFile );
        osl::FileBase::getSystemPathFromFileURL( aTmpFile, aTmpFile );
    }
    else
    {
        OStringBuffer aStr("invalid filename: ");
        aStr.append(OUStringToOString(fname, RTL_TEXTENCODING_UTF8));
        fprintf(stderr, "%s\n", aStr.getStr());
    }
    return aTmpFile;
}

#if defined( UNX ) || defined (__MINGW32__)
int main ( int argc, char ** argv)
{
#else
int cdecl main ( int argc, char ** argv)
{
#endif

    OUString aTmpListFile;
    OUString aTmpSlotMapFile;
    OUString aTmpSfxItemFile;
    OUString aTmpDataBaseFile;
    OUString aTmpHelpIdFile;
    OUString aTmpCSVFile;
    OUString aTmpDocuFile;
    OUString aTmpDepFile;

    SvCommand aCommand( argc, argv );

    if( aCommand.nVerbosity != 0 )
        printf( "StarView Interface Definition Language (IDL) Compiler 3.0\n" );

    Init();
    SvIdlWorkingBase * pDataBase = new SvIdlWorkingBase(aCommand);

    int nExit = 0;
    if( !aCommand.aExportFile.isEmpty() )
    {
        osl::DirectoryItem aDI;
        osl::FileStatus fileStatus( osl_FileStatus_Mask_FileName );
        osl::DirectoryItem::get( aCommand.aExportFile, aDI );
        aDI.getFileStatus(fileStatus);
        pDataBase->SetExportFile( fileStatus.getFileName() );
    }

    if( ReadIdl( pDataBase, aCommand ) )
    {
        if( nExit == 0 && !aCommand.aDocuFile.isEmpty() )
        {
            aTmpDocuFile = tempFileHelper(aCommand.aDocuFile);
            SvFileStream aOutStm( aTmpDocuFile, STREAM_READWRITE | STREAM_TRUNC );
            if( !pDataBase->WriteDocumentation( aOutStm ) )
            {
                nExit = -1;
                OStringBuffer aStr("cannot write documentation file: ");
                aStr.append(OUStringToOString(aCommand.aDocuFile, RTL_TEXTENCODING_UTF8));
                fprintf(stderr, "%s\n", aStr.getStr());
            }
        }
        if( nExit == 0 && !aCommand.aListFile.isEmpty() )
        {
            aTmpListFile = tempFileHelper(aCommand.aListFile);
            SvFileStream aOutStm( aTmpListFile, STREAM_READWRITE | STREAM_TRUNC );
            if( !pDataBase->WriteSvIdl( aOutStm ) )
            {
                nExit = -1;
                OStringBuffer aStr("cannot write list file: ");
                aStr.append(OUStringToOString(aCommand.aListFile, RTL_TEXTENCODING_UTF8));
                fprintf(stderr, "%s\n", aStr.getStr());
            }
        }
        if( nExit == 0 && !aCommand.aSlotMapFile.isEmpty() )
        {
            aTmpSlotMapFile = tempFileHelper(aCommand.aSlotMapFile);
            SvFileStream aOutStm( aTmpSlotMapFile, STREAM_READWRITE | STREAM_TRUNC );
            if( !pDataBase->WriteSfx( aOutStm ) )
            {
                nExit = -1;
                OStringBuffer aStr("cannot write slotmap file: ");
                aStr.append(OUStringToOString(aCommand.aSlotMapFile, RTL_TEXTENCODING_UTF8));
                fprintf(stderr, "%s\n", aStr.getStr());
            }
        }
        if( nExit == 0 && !aCommand.aHelpIdFile.isEmpty() )
        {
            aTmpHelpIdFile = tempFileHelper(aCommand.aHelpIdFile);
            SvFileStream aStm( aTmpHelpIdFile, STREAM_READWRITE | STREAM_TRUNC );
            if (!pDataBase->WriteHelpIds( aStm ) )
            {
                nExit = -1;
                OStringBuffer aStr("cannot write help ID file: ");
                aStr.append(OUStringToOString(aCommand.aHelpIdFile, RTL_TEXTENCODING_UTF8));
                fprintf(stderr, "%s\n", aStr.getStr());
            }
        }
        if( nExit == 0 && !aCommand.aCSVFile.isEmpty() )
        {
            aTmpCSVFile = tempFileHelper(aCommand.aCSVFile);
            SvFileStream aStm( aTmpCSVFile, STREAM_READWRITE | STREAM_TRUNC );
            if (!pDataBase->WriteCSV( aStm ) )
            {
                nExit = -1;
                OStringBuffer aStr("cannot write CSV file: ");
                aStr.append(OUStringToOString(aCommand.aCSVFile, RTL_TEXTENCODING_UTF8));
                fprintf(stderr, "%s\n", aStr.getStr());
            }
        }
        if( nExit == 0 && !aCommand.aSfxItemFile.isEmpty() )
        {
            aTmpSfxItemFile = tempFileHelper(aCommand.aSfxItemFile);
            SvFileStream aOutStm( aTmpSfxItemFile, STREAM_READWRITE | STREAM_TRUNC );
            if( !pDataBase->WriteSfxItem( aOutStm ) )
            {
                nExit = -1;
                OStringBuffer aStr("cannot write item file: ");
                aStr.append(OUStringToOString(aCommand.aSfxItemFile, RTL_TEXTENCODING_UTF8));
                fprintf(stderr, "%s\n", aStr.getStr());
            }
        }
        if( nExit == 0 && !aCommand.aDataBaseFile.isEmpty() )
        {
            aTmpDataBaseFile = tempFileHelper(aCommand.aDataBaseFile);
            SvFileStream aOutStm( aTmpDataBaseFile, STREAM_READWRITE | STREAM_TRUNC );
            pDataBase->Save( aOutStm, aCommand.nFlags );
            if( aOutStm.GetError() != SVSTREAM_OK )
            {
                nExit = -1;
                OStringBuffer aStr("cannot write database file: ");
                aStr.append(OUStringToOString(aCommand.aDataBaseFile, RTL_TEXTENCODING_UTF8));
                fprintf(stderr, "%s\n", aStr.getStr());
            }
        }
        if (nExit == 0 && !aCommand.m_DepFile.isEmpty())
        {
            aTmpDepFile = tempFileHelper(aCommand.m_DepFile);
            SvFileStream aOutStm( aTmpDepFile, STREAM_READWRITE | STREAM_TRUNC );
            pDataBase->WriteDepFile(aOutStm, aCommand.aTargetFile);
            if( aOutStm.GetError() != SVSTREAM_OK )
            {
                nExit = -1;
                fprintf( stderr, "cannot write dependency file: %s\n",
                        OUStringToOString( aCommand.m_DepFile,
                            RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }
    }
    else
        nExit = -1;

    if( nExit == 0 )
    {
        sal_Bool bErr = sal_False;
        sal_Bool bDoMove = aCommand.aTargetFile.isEmpty();
        OUString aErrFile, aErrFile2;
        if( !bErr && !aCommand.aListFile.isEmpty() )
        {
            bErr |= !FileMove_Impl( aCommand.aListFile, aTmpListFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aListFile;
                aErrFile2 = aTmpListFile;
            }
        }
        if( !bErr && !aCommand.aSlotMapFile.isEmpty() )
        {
            bErr |= !FileMove_Impl( aCommand.aSlotMapFile, aTmpSlotMapFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aSlotMapFile;
                aErrFile2 = aTmpSlotMapFile;
            }
        }
        if( !bErr && !aCommand.aSfxItemFile.isEmpty() )
        {
            bErr |= !FileMove_Impl( aCommand.aSfxItemFile, aTmpSfxItemFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aSfxItemFile;
                aErrFile2 = aTmpSfxItemFile;
            }
        }
        if( !bErr && !aCommand.aDataBaseFile.isEmpty() )
        {
            bErr |= !FileMove_Impl( aCommand.aDataBaseFile, aTmpDataBaseFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aDataBaseFile;
                aErrFile2 = aTmpDataBaseFile;
            }
        }
        if( !bErr && !aCommand.aHelpIdFile.isEmpty() )
        {
            bErr |= !FileMove_Impl( aCommand.aHelpIdFile, aTmpHelpIdFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aHelpIdFile;
                aErrFile2 = aTmpHelpIdFile;
            }
        }
        if( !bErr && !aCommand.aCSVFile.isEmpty() )
        {
            bErr |= !FileMove_Impl( aCommand.aCSVFile, aTmpCSVFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aCSVFile;
                aErrFile2 = aTmpCSVFile;
            }
        }
        if( !bErr && !aCommand.aDocuFile.isEmpty() )
        {
            bErr |= !FileMove_Impl( aCommand.aDocuFile, aTmpDocuFile, bDoMove );
            if( bErr ) {
                aErrFile = aCommand.aDocuFile;
                aErrFile2 = aTmpDocuFile;
            }
        }
        if (!bErr && !aCommand.m_DepFile.isEmpty())
        {
            bErr |= !FileMove_Impl( aCommand.m_DepFile, aTmpDepFile, bDoMove );
            if (bErr) {
                aErrFile = aCommand.m_DepFile;
                aErrFile2 = aTmpDepFile;
            }
        }

        if( bErr )
        {
            nExit = -1;
            OStringBuffer aStr("cannot move file from: ");
            aStr.append(OUStringToOString(aErrFile2,
                RTL_TEXTENCODING_UTF8));
            aStr.append("\n              to file: ");
            aStr.append(OUStringToOString(aErrFile,
                RTL_TEXTENCODING_UTF8));
            fprintf( stderr, "%s\n", aStr.getStr() );
        }
        else
        {
            if( !aCommand.aTargetFile.isEmpty() )
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
        if( !aCommand.aListFile.isEmpty() )
        {
            osl::FileBase::getSystemPathFromFileURL( aTmpListFile, aTmpListFile );
            osl::File::remove( aTmpListFile );
        }
        if( !aCommand.aSlotMapFile.isEmpty() )
        {
            osl::FileBase::getSystemPathFromFileURL( aTmpSlotMapFile, aTmpSlotMapFile );
            osl::File::remove( aTmpSlotMapFile );
        }
        if( !aCommand.aSfxItemFile.isEmpty() )
        {
            osl::FileBase::getSystemPathFromFileURL( aTmpSfxItemFile, aTmpSfxItemFile );
            osl::File::remove( aTmpSfxItemFile );
        }
        if( !aCommand.aDataBaseFile.isEmpty() )
        {
            osl::FileBase::getSystemPathFromFileURL( aTmpDataBaseFile, aTmpDataBaseFile );
            osl::File::remove( aTmpDataBaseFile );
        }
    }

    delete pDataBase;
    DeInit();
    if( nExit != 0 )
        fprintf( stderr, "svidl terminated with errors\n" );
    return nExit;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
