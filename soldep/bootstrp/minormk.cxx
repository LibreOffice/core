/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: minormk.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 08:09:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "minormk.hxx"
#include <appdef.hxx>
#include <tools/fsys.hxx>
#include <tools/geninfo.hxx>
#include "prodmap.hxx"

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
                : bExists( FALSE )
{
    ByteString sDelimiter( DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );


    ByteString sSolver( GetEnv( "SOLARVERSION" ));
    BOOL bNewEnv = TRUE;
    if ( !sSolver.Len()) {
        sSolver = GetEnv( "SOLARVER" );
        bNewEnv = FALSE;
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
        bExists = TRUE;
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

