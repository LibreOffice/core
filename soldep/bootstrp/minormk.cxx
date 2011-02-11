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

#include "minormk.hxx"
#include "prodmap.hxx"
#include <soldep/appdef.hxx>
#include <tools/fsys.hxx>
#include <tools/geninfo.hxx>

//
// class MinorMk
//

/*****************************************************************************/
MinorMk::MinorMk(
    GenericInformationList *pList,
    const ByteString &rVersion,
    const ByteString &rWorkStamp,
    const ByteString &rEnvironment,
    const ByteString &rMinor
)
/*****************************************************************************/
{
    String sRoot( ProductMapper::GetVersionRoot( pList, rVersion ));
    if ( sRoot.Len()) {
        ByteString sPath( sRoot, RTL_TEXTENCODING_ASCII_US );
        sPath += "/";
        sPath += rEnvironment;
        sPath += "/";
        sPath += "inc";
        if ( rMinor.Len()) {
            sPath += ".";
            sPath += rMinor;
        }
#ifdef UNX
        sPath.SearchAndReplaceAll( "\\", "/" );
        while( sPath.SearchAndReplace( "//", "/" ) != STRING_NOTFOUND ) {};
#else
        sPath.SearchAndReplaceAll( "/", "\\" );
        while( sPath.SearchAndReplace( "\\\\", "\\" ) != STRING_NOTFOUND ) {};
#endif
        DirEntry aPath( String( sPath, RTL_TEXTENCODING_ASCII_US ));

        ByteString sFile( rWorkStamp );
        sFile += "minor.mk";

        DirEntry aFile( aPath );
        aFile += DirEntry( String( sFile, RTL_TEXTENCODING_ASCII_US ));
        if ( !aFile.Exists()) {
            sFile = sFile.Copy( 3 );
            aFile = aPath;
            aFile += DirEntry( String( sFile, RTL_TEXTENCODING_ASCII_US ));
        }
        ReadMinorMk( aFile.GetFull());
    }
}

/*****************************************************************************/
MinorMk::MinorMk( GenericInformationList *pList, const ByteString rVersion )
/*****************************************************************************/
{
    String sRoot( ProductMapper::GetVersionRoot( pList, rVersion ));
    if ( sRoot.Len()) {
        ByteString sPath( sRoot, RTL_TEXTENCODING_ASCII_US );
        sPath += "/src/solenv/inc";
#ifdef UNX
        sPath.SearchAndReplaceAll( "\\", "/" );
        while( sPath.SearchAndReplace( "//", "/" ) != STRING_NOTFOUND ) {};
#else
        sPath.SearchAndReplaceAll( "/", "\\" );
        while( sPath.SearchAndReplace( "\\\\", "\\" ) != STRING_NOTFOUND ) {};
#endif
        DirEntry aPath( String( sPath, RTL_TEXTENCODING_ASCII_US ));

         ByteString sFile( "minor.mk" );

        DirEntry aFile( aPath );
        aFile += DirEntry( String( sFile, RTL_TEXTENCODING_ASCII_US ));

        ReadMinorMk( aFile.GetFull());
    }
}

/*****************************************************************************/
MinorMk::MinorMk( const ByteString &rMinor, const ByteString &rEnvironment )
/*****************************************************************************/
                : bExists( sal_False )
{
    ByteString sDelimiter( DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );


    ByteString sSolver( GetEnv( "SOLARVERSION" ));
    sal_Bool bNewEnv = sal_True;
    if ( !sSolver.Len()) {
        sSolver = GetEnv( "SOLARVER" );
        bNewEnv = sal_False;
    }
    ByteString sUPD( GetEnv( "UPD" ));

    ByteString sMinorMk( sSolver );
    sMinorMk += sDelimiter;
    if ( !bNewEnv ) {
        sMinorMk += sUPD;
        sMinorMk += sDelimiter;
    }

    if ( rEnvironment.Len()) {
        sMinorMk += rEnvironment;
        sMinorMk += sDelimiter;
        sMinorMk += "inc";
        if( rMinor.Len()) {
            sMinorMk += ".";
            sMinorMk += rMinor;
        }
        sMinorMk += sDelimiter;
    }

    sMinorMk += sUPD;
    sMinorMk += "minor.mk";

    String sFile( sMinorMk, RTL_TEXTENCODING_ASCII_US );

    ReadMinorMk( sFile );
}

/*****************************************************************************/
void MinorMk::ReadMinorMk( const String &rFileName )
/*****************************************************************************/
{
    if ( DirEntry( rFileName ).Exists()) {
        bExists = sal_True;
        SvFileStream aStream( rFileName, STREAM_STD_READ );
        while ( !aStream.IsOpen())
            aStream.Open( rFileName, STREAM_STD_READ );

        ByteString sLine;
        while ( !aStream.IsEof()) {
            aStream.ReadLine( sLine );

            if ( sLine.GetToken( 0, '=' ) == "RSCVERSION" )
                sRSCVERSION = sLine.GetToken( 1, '=' );
            else if ( sLine.GetToken( 0, '=' ) == "RSCREVISION" )
                sRSCREVISION = sLine.GetToken( 1, '=' );
            else if ( sLine.GetToken( 0, '=' ) == "BUILD" )
                sBUILD = sLine.GetToken( 1, '=' );
            else if ( sLine.GetToken( 0, '=' ) == "LAST_MINOR" )
                sLAST_MINOR = sLine.GetToken( 1, '=' );
        }
        aStream.Close();

        sRSCVERSION.EraseTrailingChars();
        sRSCREVISION.EraseTrailingChars();
        sBUILD.EraseTrailingChars();
        sLAST_MINOR.EraseTrailingChars();
    }
}

/*****************************************************************************/
MinorMk::~MinorMk()
/*****************************************************************************/
{
}

