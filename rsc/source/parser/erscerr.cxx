/*************************************************************************
 *
 *  $RCSfile: erscerr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/parser/erscerr.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

************************************************************************/
#pragma hdrstop
#include <stdlib.h>
#include <stdio.h>

#include <tools/rcid.h>

#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif
#ifndef _RSCPAR_HXX
#include <rscpar.hxx>
#endif

#include "rsclex.hxx"

/*************************************************************************
|*
|*    ERRTYPE::operator = ;
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.09.91
|*    Letzte Aenderung  MM 25.09.91
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
|*    Beschreibung
|*    Ersterstellung    MM 06.05.91
|*    Letzte Aenderung  MM 06.05.91
|*
*************************************************************************/
void RscError::StdOut( const char * pStr )
{
#ifndef WIN
    if( pStr ){
        printf( "%s", pStr );
        fflush( stdout );
    }
#endif
}

/*************************************************************************
|*
|*    RscError::LstOut();
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.05.91
|*    Letzte Aenderung  MM 06.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 06.05.91
|*    Letzte Aenderung  MM 06.05.91
|*
*************************************************************************/
void RscError::StdLstOut( const char * pStr ){
    StdOut( pStr );
    LstOut( pStr );
}

/*************************************************************************
|*
|*    RscError::WriteError();
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.05.91
|*    Letzte Aenderung  MM 06.05.91
|*
*************************************************************************/
void RscError::WriteError( const ERRTYPE& rError, const char * pMessage )
{
    switch( rError )
    {
        case ERR_ERROR: {
            StdLstOut( "!! " );
            if( 1 == nErrors )
                StdLstOut( ByteString::CreateFromInt32( nErrors ).GetBuffer() );
            else
                StdLstOut( ByteString::CreateFromInt32( (USHORT)(nErrors -1) ).GetBuffer() );
            StdLstOut( " Error" );
            StdLstOut( " found!!" );
        }
        break;

        case ERR_UNKNOWN_METHOD:
            StdLstOut( "The used type is not allowed." );
        break;

        case ERR_OPENFILE:
            StdLstOut( "This file <" );
            StdLstOut( pMessage );
            StdLstOut( "> cannot be opened." );
        break;

        case ERR_FILESIZE:
            StdLstOut( "Wrong file <" );
            StdLstOut( pMessage );
            StdLstOut( "> length." );
        break;

        case ERR_FILEFORMAT:
            StdLstOut( "Wrong file type <" );
            StdLstOut( pMessage );
            StdLstOut( ">." );
        break;

        case ERR_NOCHAR:
            StdLstOut( "Character: '\\xxx'; The value xxx is greater than 255.");
            break;

        case ERR_NORSCINST:
            StdLstOut( "Internal error, instance invalid.");
            break;


        case ERR_NOINPUT:
            StdLstOut( "Input file was not specified.\n");
        case ERR_USAGE:
            StdLstOut( "Copyright (C) 1990-92 STAR DIVISION GmbH\n" );
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
            StdLstOut( "-pp<filename>  Use a different Preprocessor.\n" );
            StdLstOut( "-rc<filename>  Use a different system resource compiler.\n" );
            StdLstOut( "-fo<filename>  Renaming of the .res file.\n" );
            StdLstOut( "-fs<filename>  Renaming of the .rc file.\n" );
            StdLstOut( "-fp<filename>  Renaming of the .srs file.\n" );
            StdLstOut( "-fl<filename>  Listing file.\n" );
            StdLstOut( "-fh<filename>  Header file.\n" );
            StdLstOut( "-fc<filename>  Code file.\n" );
            StdLstOut( "-CHARSET_...  Convert to this character set.\n" );
            StdLstOut( "-BIGENDIAN    Format of number values.\n" );
            StdLstOut( "-LITTLEENDIAN Format of number values.\n" );
            StdLstOut( "-SMART        Generate smart names (cxx, hxx).\n" );
            StdLstOut( "-SrsDefault   Only write one language to srs file.\n" );
            break;

        case ERR_UNKNOWNSW:
            StdLstOut( "Unknown switch <" );
            StdLstOut( pMessage );
            StdLstOut( ">." );
            break;

        case ERR_REFTODEEP:
            StdLstOut( "Too many reference levels have been used (see Switch -RefDeep)." );
            break;

        case ERR_CONT_INVALIDPOS:
            StdLstOut( "Internal error, Container class: invalid position." );
            break;

        case ERR_CONT_INVALIDTYPE:
            StdLstOut( "Invalid type <" );
            StdLstOut( pMessage );
            StdLstOut( ">." );
            break;

        case ERR_ARRAY_INVALIDINDEX:
            StdLstOut( "Internal error, Array class: invalid index." );
            break;

        case ERR_RSCINST_NOVARNAME:
            StdLstOut( "Internal error, invalid name of variable." );
            break;

        case ERR_YACC:
            StdLstOut( pMessage );
            break;

        case ERR_DOUBLEID:
            StdLstOut( "Two global resources have the same identifier." );
            break;

        case ERR_FALSETYPE:
            StdLstOut( "Wrong type <" );
            StdLstOut( pMessage );
            StdLstOut( ">." );
            break;

        case ERR_NOVARIABLENAME:
            StdLstOut( "The variable <" );
            StdLstOut( pMessage );
            StdLstOut( "> must not be used here." );
            break;

        case ERR_RSCRANGE_OUTDEFSET:
            StdLstOut( "The used value is not in the expected domain." );
            break;

        case ERR_USHORTRANGE:
            StdLstOut( "Value is <" );
            StdLstOut( pMessage );
            StdLstOut( "> the allowed domain is from 0 up to 65535." );
            break;

        case ERR_IDRANGE:
            StdLstOut( "Value is <" );
            StdLstOut( pMessage );
            StdLstOut( "> the allowed domain is from 1 up to 32767." );
            break;

        case ERR_NOCOPYOBJ:
            StdLstOut( "Default resource <" );
            StdLstOut( pMessage );
            StdLstOut( "> not found." );
            break;

        case ERR_REFNOTALLOWED:
            StdLstOut( "The use of a reference is not allowed." );
            break;

        case ERR_COPYNOTALLOWED:
            StdLstOut( "The use of a default resource is not allowed." );
            break;

        case ERR_IDEXPECTED:
            StdLstOut( "An identifier needs to be specified." );
            break;

        case ERR_DOUBLEDEFINE:
            StdLstOut( "The symbol <" );
            StdLstOut( pMessage );
            StdLstOut( "> is defined twice." );
            break;

        case ERR_RSCINST_RESERVEDNAME:
            StdLstOut( "The symbol <" );
            StdLstOut( pMessage );
            StdLstOut( "> is a reserved name." );
            break;

        case ERR_ZERODIVISION:
            StdLstOut( "Attempt to divide by zero." );
            break;

        case ERR_PRAGMA:
            StdLstOut( "Error in a #pragma statement." );
            break;

        case ERR_DECLAREDEFINE:
            StdLstOut( "Error in the declaration part of the macro." );
            break;

        case ERR_NOTYPE:
            StdLstOut( "type expected." );
            break;

/****************** W A R N I N G S **************************************/
        case WRN_LOCALID:
            StdLstOut( "Sub resources should have an identifier < 256." );
            break;

        case WRN_GLOBALID:
            StdLstOut( "Global resources should have an identifier >= 256." );
            break;

        case WRN_SUBINMEMBER:
            StdLstOut( "Sub resources are ignored." );
            break;

        case WRN_CONT_NOID:
            StdLstOut( "Resources without name are ignored." );
            break;

        case WRN_CONT_DOUBLEID:
            StdLstOut( "Two local resources have the same identifier." );
            break;

        case WRN_STR_REFNOTFOUND:
            StdLstOut( "String reference <" );
            StdLstOut( pMessage );
            StdLstOut( " > could not be resolved." );
            break;

        case WRN_MGR_REFNOTFOUND:
            StdLstOut( "Reference <" );
            StdLstOut( pMessage );
            StdLstOut( " > could not be resolved." );
            break;

        default:
            if( pMessage ){
                StdLstOut( "\nMessage: " );
                StdLstOut( pMessage );
            };
            break;
    }
}

/*************************************************************************
|*
|*    RscErrorFormat()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.05.91
|*    Letzte Aenderung  MM 06.05.91
|*
*************************************************************************/
void RscError::ErrorFormat( const ERRTYPE& rError, RscTop * pClass,
                            const RscId & aId ){
    char buf[ 10 ];
    USHORT i;

    if( pFI )
    {
        pFI->SetError( rError );
        StdOut( "\n" );
        StdOut( pFI->GetLine() );
        StdOut( "\n" );
        // Fehlerposition anzeigen
        for( i = 0; (USHORT)(i +1) < pFI->GetScanPos(); i++ )
            StdLstOut( " " );
        LstOut( "     ^" ); //Zeilennummern beachten
        StdOut( "^" );
        StdLstOut( "\n" );
    }
    StdLstOut( "f" );
    sprintf( buf, "%u", (USHORT)rError );
    StdLstOut( buf );

    if( pFI && pTC ){
        StdLstOut( ": \"" );
        StdLstOut( pTC->aFileTab.Get( pFI->GetFileIndex() )->aFileName.GetBuffer() );
        StdLstOut( "\", line " );
        sprintf( buf, "%d", pFI->GetLineNo() );
        StdLstOut( buf );
    }

    if( rError.IsError() )
        StdLstOut( ": Error" );
    else
        StdLstOut( ": Warning" );

    if( pClass || aId.IsId() )
    {
        StdLstOut( " in the object (" );
        if( pClass )
        {
            StdLstOut( "Type: " );
            StdLstOut( pHS->Get( pClass->GetId() ) );
            if( aId.IsId() )
                StdLstOut( ", " );
        }
        if( aId.IsId() )
            StdLstOut( aId.GetName().GetBuffer() );
        StdLstOut( "):\n" );
    }
    else
        StdLstOut( ": " );
}

/*************************************************************************
|*
|*    RscError::Error()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.05.91
|*    Letzte Aenderung  MM 06.05.91
|*
*************************************************************************/
void RscError::Error( const ERRTYPE& rError, RscTop * pClass,
                      const RscId & aId, const char * pMessage )
{
    if( WRN_LOCALID == (USHORT)rError ) // Keine Warning erzeugen
        return;
    if( rError.IsError() )
        nErrors++;
    if( rError.IsError() || rError.IsWarning() ){
        ErrorFormat( rError, pClass, aId );
        WriteError( rError, pMessage );
        StdLstOut( "\n" );
    };
}

/*************************************************************************
|*
|*    RscError::FatalError();
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.05.91
|*    Letzte Aenderung  MM 06.05.91
|*
*************************************************************************/
void RscError::FatalError( const ERRTYPE& rError, const RscId &aId,
                           const char * pMessage )
{
    if( ERR_USAGE != (USHORT)rError ){
        nErrors++;
        ErrorFormat( rError, NULL, aId );
        WriteError( rError, pMessage );
        StdLstOut( "\nTerminating compiler\n" );
    }
    else
        WriteError( rError, pMessage );

    exit( 1 );
}

