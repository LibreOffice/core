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
#include "precompiled_rsc.hxx"
#include <stdlib.h>
#include <stdio.h>

#include <tools/rcid.h>
#include <rschash.hxx>
#include <rscerror.h>
#include <rscall.h>
#include <rscdb.hxx>
#include <rscpar.hxx>

#include "rsclex.hxx"

/*************************************************************************
|*
|*    ERRTYPE::operator = ;
|*
*************************************************************************/
ERRTYPE& ERRTYPE::operator = ( const ERRTYPE & rError )
{
    if( !IsError() ){
        if( rError.IsError() || !IsWarning() )
            nError = rError.nError;
    }
    return *this;
}

/*************************************************************************
|*
|*    RscError::StdOut();
|*
*************************************************************************/
void RscError::StdOut( const char * pStr, const RscVerbosity _verbosityLevel )
{
    if ( m_verbosity >= _verbosityLevel )
    {
        if( pStr ){
            printf( "%s", pStr );
            fflush( stdout );
        }
    }
}

/*************************************************************************
|*
|*    RscError::StdErr();
|*
*************************************************************************/
void RscError::StdErr( const char * pStr )
{
    if( pStr )
        fprintf( stderr, "%s", pStr );
}

/*************************************************************************
|*
|*    RscError::LstOut();
|*
*************************************************************************/
void RscError::LstOut( const char * pStr ){
    if( fListing && pStr )
        fprintf( fListing, "%s", pStr );
}

/*************************************************************************
|*
|*    RscError::StdLstOut();
|*
*************************************************************************/
void RscError::StdLstOut( const char * pStr ){
    StdOut( pStr );
    LstOut( pStr );
}

/*************************************************************************
|*
|*    RscError::StdLstErr();
|*
*************************************************************************/
void RscError::StdLstErr( const char * pStr ){
    StdErr( pStr );
    LstOut( pStr );
}

/*************************************************************************
|*
|*    RscError::WriteError();
|*
*************************************************************************/
void RscError::WriteError( const ERRTYPE& rError, const char * pMessage )
{
    switch( rError )
    {
        case ERR_ERROR: {
            StdLstErr( "!! " );
            if( 1 == nErrors )
                StdLstErr( ByteString::CreateFromInt32( nErrors ).GetBuffer() );
            else
                StdLstErr( ByteString::CreateFromInt32( (nErrors -1) ).GetBuffer() );
            StdLstErr( " Error" );
            StdLstErr( " found!!" );
        }
        break;

        case ERR_UNKNOWN_METHOD:
            StdLstErr( "The used type is not allowed." );
        break;

        case ERR_OPENFILE:
            StdLstErr( "This file <" );
            StdLstErr( pMessage );
            StdLstErr( "> cannot be opened." );
        break;

        case ERR_RENAMEFILE:
            StdLstErr( "rename <" );
            StdLstErr( pMessage );
            StdLstErr( "> s not possible." );
        break;

        case ERR_FILESIZE:
            StdLstErr( "Wrong file <" );
            StdLstErr( pMessage );
            StdLstErr( "> length." );
        break;

        case ERR_FILEFORMAT:
            StdLstErr( "Wrong file type <" );
            StdLstErr( pMessage );
            StdLstErr( ">." );
        break;

        case ERR_NOCHAR:
            StdLstErr( "Character: '\\xxx'; The value xxx is greater than 255.");
            break;

        case ERR_NORSCINST:
            StdLstErr( "Internal error, instance invalid.");
            break;


        case ERR_NOINPUT:
            StdLstErr( "Input file was not specified.\n");
        case ERR_USAGE:
            StdLstOut( "Copyright (C) 2000, 2010 Oracle and/or its affiliates.\n" );
            {
                char    buf[40];

                StdLstOut( "DataVersion: " );
                sprintf( buf, "%d.%d\n\n",
                         RSCVERSION_ID / 100, RSCVERSION_ID % 100 );
                StdLstOut( buf );
            };

            StdLstOut( "Command line: rsc [Switches] <Source File(s)>\n" );
            StdLstOut( "Command line: rsc @<Command File>\n" );
            StdLstOut( "-h  shows this help.\n" );
            StdLstOut( "-p  No Preprocessor.\n" );
            StdLstOut( "-s  Syntax analysis, creates .srs file\n");
            StdLstOut( "-l  Linker, links files created with rsc -s,\n" );
            StdLstOut( "    creates .rc file and .res file.\n" );
            StdLstOut( "-r  Prevents .res file.\n" );
            StdLstOut( "-d  Symbol definitions for the Preprocessor.\n" );
            StdLstOut( "-i  Include directives for the Preprocessor.\n" );
            StdLstOut( "-presponse     Use response file for Preprocessor.\n" );
            StdLstOut( "-lg<language>  Use a different language.\n" );
            StdLstOut( "-pp=<filename> Use a different Preprocessor.\n" );
            StdLstOut( "-rsc2=<filename> Specify the location for rsc2.\n" );
            StdLstOut( "No longer existent: -rc<filename>  Use a different system resource compiler.\n" );
            StdLstOut( "-fs=<filename> Name of the .res file.\n" );
            StdLstOut( "-lip=<path>    additional search path for system dependant files\n" );
            StdLstOut( "-fp=<filename> Renaming of the .srs file.\n" );
            StdLstOut( "-fl=<filename> Listing file.\n" );
            StdLstOut( "-fh=<filename> Header file.\n" );
            StdLstOut( "-fc=<filename> Code file.\n" );
            StdLstOut( "-ft=<filename> Touch a file when done in rsc2 (for dependencies)\n" );
            StdLstOut( "-fr=<filename> Ressource constructor .cxx-file.\n" );
            StdLstOut( "-fx=<filename> Name of .src-file.\n" );
            StdLstOut( "-oil=<dir>     Output directory for image list files\n" );
            StdLstOut( "-r<ENV>=<path> replace <path> by <ENV> in image list files\n" );
            StdLstOut( "-CHARSET_...  Convert to this character set.\n" );
            StdLstOut( "-BIGENDIAN    Format of number values.\n" );
            StdLstOut( "-LITTLEENDIAN Format of number values.\n" );
            StdLstOut( "-SMART        Generate smart names (cxx, hxx).\n" );
            StdLstOut( "-SrsDefault   Only write one language to srs file.\n" );
            StdLstOut( "\nwhen creating multiple .res files in one pass, please give these\n" );
            StdLstOut( "options in consecutive blocks:\n" );
            StdLstOut( "-lg<language> -fs<filename> [-lip<path> [-lip<path>] ]\n" );
            StdLstOut( "a new block begins when either -lg or -fs is used again.\n" );
            break;

        case ERR_UNKNOWNSW:
            StdLstErr( "Unknown switch <" );
            StdLstErr( pMessage );
            StdLstErr( ">." );
            break;

        case ERR_REFTODEEP:
            StdLstErr( "Too many reference levels have been used (see Switch -RefDeep)." );
            break;

        case ERR_CONT_INVALIDPOS:
            StdLstErr( "Internal error, Container class: invalid position." );
            break;

        case ERR_CONT_INVALIDTYPE:
            StdLstErr( "Invalid type <" );
            StdLstErr( pMessage );
            StdLstErr( ">." );
            break;

        case ERR_ARRAY_INVALIDINDEX:
            StdLstErr( "Internal error, Array class: invalid index." );
            break;

        case ERR_RSCINST_NOVARNAME:
            StdLstErr( "Internal error, invalid name of variable." );
            break;

        case ERR_YACC:
            StdLstErr( pMessage );
            break;

        case ERR_DOUBLEID:
            StdLstErr( "Two global resources have the same identifier." );
            break;

        case ERR_FALSETYPE:
            StdLstErr( "Wrong type <" );
            StdLstErr( pMessage );
            StdLstErr( ">." );
            break;

        case ERR_NOVARIABLENAME:
            StdLstErr( "The variable <" );
            StdLstErr( pMessage );
            StdLstErr( "> must not be used here." );
            break;

        case ERR_RSCRANGE_OUTDEFSET:
            StdLstErr( "The used value is not in the expected domain." );
            break;

        case ERR_USHORTRANGE:
            StdLstErr( "Value is <" );
            StdLstErr( pMessage );
            StdLstErr( "> the allowed domain is from 0 up to 65535." );
            break;

        case ERR_IDRANGE:
            StdLstErr( "Value is <" );
            StdLstErr( pMessage );
            StdLstErr( "> the allowed domain is from 1 up to 32767." );
            break;

        case ERR_NOCOPYOBJ:
            StdLstErr( "Default resource <" );
            StdLstErr( pMessage );
            StdLstErr( "> not found." );
            break;

        case ERR_REFNOTALLOWED:
            StdLstErr( "The use of a reference is not allowed." );
            break;

        case ERR_COPYNOTALLOWED:
            StdLstErr( "The use of a default resource is not allowed." );
            break;

        case ERR_IDEXPECTED:
            StdLstErr( "An identifier needs to be specified." );
            break;

        case ERR_DOUBLEDEFINE:
            StdLstErr( "The symbol <" );
            StdLstErr( pMessage );
            StdLstErr( "> is defined twice." );
            break;

        case ERR_RSCINST_RESERVEDNAME:
            StdLstErr( "The symbol <" );
            StdLstErr( pMessage );
            StdLstErr( "> is a reserved name." );
            break;

        case ERR_ZERODIVISION:
            StdLstErr( "Attempt to divide by zero." );
            break;

        case ERR_PRAGMA:
            StdLstErr( "Error in a #pragma statement." );
            break;

        case ERR_DECLAREDEFINE:
            StdLstErr( "Error in the declaration part of the macro." );
            break;

        case ERR_NOTYPE:
            StdLstErr( "type expected." );
            break;

        case ERR_NOIMAGE:
            StdLstErr( "The image(s) <" );
            StdLstErr( pMessage );
            StdLstErr( "> could not be found." );
            break;

/****************** W A R N I N G S **************************************/
        case WRN_LOCALID:
            StdLstErr( "Sub resources should have an identifier < 256." );
            break;

        case WRN_GLOBALID:
            StdLstErr( "Global resources should have an identifier >= 256." );
            break;

        case WRN_SUBINMEMBER:
            StdLstErr( "Sub resources are ignored." );
            break;

        case WRN_CONT_NOID:
            StdLstErr( "Resources without name are ignored." );
            break;

        case WRN_CONT_DOUBLEID:
            StdLstErr( "Two local resources have the same identifier." );
            break;

        case WRN_STR_REFNOTFOUND:
            StdLstErr( "String reference <" );
            StdLstErr( pMessage );
            StdLstErr( " > could not be resolved." );
            break;

        case WRN_MGR_REFNOTFOUND:
            StdLstErr( "Reference <" );
            StdLstErr( pMessage );
            StdLstErr( " > could not be resolved." );
            break;

        default:
            if( pMessage ){
                StdLstErr( "\nMessage: " );
                StdLstErr( pMessage );
            };
            break;
    }
}

/*************************************************************************
|*
|*    RscErrorFormat()
|*
*************************************************************************/
void RscError::ErrorFormat( const ERRTYPE& rError, RscTop * pClass,
                            const RscId & aId ){
    char buf[ 10 ];
    sal_uInt32 i;

    if( pFI )
    {
        pFI->SetError( rError );
        StdErr( "\n" );
        StdErr( pFI->GetLine() );
        StdErr( "\n" );
        // Fehlerposition anzeigen
        for( i = 0; (i +1) < pFI->GetScanPos(); i++ )
            StdLstErr( " " );
        LstOut( "     ^" ); //Zeilennummern beachten
        StdErr( "^" );
        StdLstErr( "\n" );
    }
    StdLstErr( "f" );
    sprintf( buf, "%u", (unsigned int)rError );
    StdLstErr( buf );

    if( pFI && pTC ){
        StdLstErr( ": \"" );
        StdLstErr( pTC->aFileTab.Get( pFI->GetFileIndex() )->aFileName.GetBuffer() );
        StdLstErr( "\", line " );
        sprintf( buf, "%u", (unsigned int)pFI->GetLineNo() );
        StdLstErr( buf );
    }

    if( rError.IsError() )
        StdLstErr( ": Error" );
    else
        StdLstErr( ": Warning" );

    if( pClass || aId.IsId() )
    {
        StdLstErr( " in the object (" );
        if( pClass )
        {
            StdLstErr( "Type: " );
            StdLstErr( pHS->getString( pClass->GetId() ).getStr() );
            if( aId.IsId() )
                StdLstErr( ", " );
        }
        if( aId.IsId() )
            StdLstErr( aId.GetName().GetBuffer() );
        StdLstErr( "):\n" );
    }
    else
        StdLstErr( ": " );
}

/*************************************************************************
|*
|*    RscError::Error()
|*
*************************************************************************/
void RscError::Error( const ERRTYPE& rError, RscTop * pClass,
                      const RscId & aId, const char * pMessage )
{
    if( WRN_LOCALID == rError ) // Keine Warning erzeugen
        return;
    if( rError.IsError() )
        nErrors++;
    if( rError.IsError() || rError.IsWarning() ){
        ErrorFormat( rError, pClass, aId );
        WriteError( rError, pMessage );
        StdLstErr( "\n" );
    };
}

/*************************************************************************
|*
|*    RscError::FatalError();
|*
*************************************************************************/
void RscError::FatalError( const ERRTYPE& rError, const RscId &aId,
                           const char * pMessage )
{
    if( ERR_USAGE != rError ){
        nErrors++;
        ErrorFormat( rError, NULL, aId );
        WriteError( rError, pMessage );
        StdLstErr( "\nTerminating compiler\n" );
    }
    else
        WriteError( rError, pMessage );

    exit( 1 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
