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

