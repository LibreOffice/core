/*************************************************************************
 *
 *  $RCSfile: svidl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <database.hxx>
#include <globals.hxx>
#include <command.hxx>

#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#pragma hdrstop

#define BR 0x8000
BOOL FileMove_Impl( const String & rFile1, const String & rFile2, BOOL bImmerVerschieben )
{
    //printf( "Move from %s to %s\n", rFile2.GetStr(), rFile1.GetStr() );
    ULONG nC1 = 0;
    ULONG nC2 = 1;
    if( !bImmerVerschieben )
    {
        SvFileStream aOutStm1( rFile1, STREAM_STD_READ );
        SvFileStream aOutStm2( rFile2, STREAM_STD_READ );
        if( aOutStm1.GetError() == SVSTREAM_OK )
        {
            void * pBuf1 = new BYTE[ BR ];
            void * pBuf2 = new BYTE[ BR ];
            nC1 = aOutStm1.Read( pBuf1, BR );
            nC2 = aOutStm2.Read( pBuf2, BR );
            BOOL bOk = TRUE;
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
            delete pBuf1;
            delete pBuf2;
        }
    }
    DirEntry aF2( rFile2 );
    if( nC1 != nC2 )
    {// es hat sich etwas geaendert
        DirEntry aF1( rFile1 );
        aF1.Kill();
        // Datei verschieben
        if( aF2.MoveTo( aF1 ) )
        {
            // Beide Dateien loeschen
            aF1.Kill();
            aF2.Kill();
            return FALSE;
        }
/*
        else
        {
            printf( "%s to %s moved\n",
                     rFile2.GetStr(), rFile1.GetStr() );
        }
*/
        return TRUE;
    }
    return 0 == aF2.Kill();
}

/*************************************************************************
|*    main()
|*
|*    Beschreibung
*************************************************************************/
#if defined( UNX ) || (defined( PM2 ) && defined( CSET )) || defined (WTC) || defined (MAC) || defined (MTW)
int main ( int argc, char ** argv)
{
#else
int cdecl main ( int argc, char ** argv)
{
#endif

    printf( "StarView Interface Definition Language (IDL) Compiler 3.0\n" );

/*
    pStr = ::ResponseFile( &aCmdLine, argv, argc );
    if( pStr )
    {
        printf( "Cannot open response file <%s>\n", pStr );
        return( 1 );
    };
*/

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
    Init( aCommand );
    SvIdlWorkingBase * pDataBase = new SvIdlWorkingBase();

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
/*
        if( nExit == 0 && aCommand.aCallingFile.Len() )
        {
            DirEntry aDE( aCommand.aCallingFile );
            aDE.ToAbs();
            aTmpCallingFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aOutStm( aTmpCallingFile, STREAM_READWRITE | STREAM_TRUNC );
            pDataBase->WriteSbx( aOutStm );
            //pDataBase->Save( aOutStm, aCommand.nFlags | IDL_WRITE_CALLING );
            if( aOutStm.GetError() != SVSTREAM_OK )
            {
                nExit = -1;
                ByteString aStr = "cannot write calling file: ";
                aStr += aCommand.aCallingFile;
                fprintf( stderr, "%s\n", aStr.GetStr() );
            }
        }
        if( nExit == 0 && aCommand.aCxxFile.Len() )
        {
            DirEntry aDE( aCommand.aCxxFile );
            aDE.ToAbs();
            aTmpCxxFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aOutStm( aTmpCxxFile, STREAM_READWRITE | STREAM_TRUNC );

            aOutStm << "#pragma hdrstop" << endl;
            aOutStm << "#include <";
            if( aCommand.aHxxFile.Len() )
                aOutStm << DirEntry(aCommand.aHxxFile).GetName().GetBuffer();
            else
            {
                DirEntry aDE( aCommand.aCxxFile );
                aDE.SetExtension( "hxx" );
                aOutStm << aDE.GetName().GetBuffer);
            }
            aOutStm << '>' << endl;
            if( !pDataBase->WriteCxx( aOutStm ) )
            {
                nExit = -1;
                ByteString aStr = "cannot write cxx file: ";
                aStr += ByteString( aCommand.aCxxFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
        if( nExit == 0 && aCommand.aHxxFile.Len() )
        {
            DirEntry aDE( aCommand.aHxxFile );
            aDE.ToAbs();
            aTmpHxxFile = aDE.GetPath().TempName().GetFull();
            SvFileStream aOutStm( aTmpHxxFile, STREAM_READWRITE | STREAM_TRUNC );

            aOutStm << "#include <somisc.hxx>" << endl;
            if( !pDataBase->WriteHxx( aOutStm ) )
            {
                nExit = -1;
                ByteString aStr = "cannot write cxx file: ";
                aStr += ByteString( aCommand.aHxxFile, RTL_TEXTENCODING_UTF8 );
                fprintf( stderr, "%s\n", aStr.GetBuffer() );
            }
        }
 */
    }
    else
        nExit = -1;

    if( nExit == 0 )
    {
        BOOL bErr = FALSE;
        BOOL bDoMove = aCommand.aTargetFile.Len() == 0;
        String aErrFile;
        if( !bErr && aCommand.aListFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aListFile, aTmpListFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aListFile;
        }
        if( !bErr && aCommand.aSlotMapFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aSlotMapFile, aTmpSlotMapFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aSlotMapFile;
        }
        if( !bErr && aCommand.aSfxItemFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aSfxItemFile, aTmpSfxItemFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aSfxItemFile;
        }
        if( !bErr && aCommand.aDataBaseFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aDataBaseFile, aTmpDataBaseFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aDataBaseFile;
        }
        if( !bErr && aCommand.aCallingFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aCallingFile, aTmpCallingFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aCallingFile;
        }
        if( !bErr && aCommand.aCxxFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aCxxFile, aTmpCxxFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aCxxFile;
        }
        if( !bErr && aCommand.aHxxFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aHxxFile, aTmpHxxFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aHxxFile;
        }
        if( !bErr && aCommand.aHelpIdFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aHelpIdFile, aTmpHelpIdFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aHelpIdFile;
        }
        if( !bErr && aCommand.aCSVFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aCSVFile, aTmpCSVFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aCSVFile;
        }
        if( !bErr && aCommand.aDocuFile.Len() )
        {
            bErr |= !FileMove_Impl( aCommand.aDocuFile, aTmpDocuFile, bDoMove );
            if( bErr )
                aErrFile = aCommand.aDocuFile;
        }

        if( bErr )
        {
            nExit = -1;
            ByteString aStr = "cannot write file: ";
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
                // Datei stempeln, da idl korrekt durchlaufen wurde
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

