
/*************************************************************************
 *
 *  $RCSfile: localize.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nf $ $Date: 2001-05-22 14:11:52 $
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

#include "srciter.hxx"
#include <bootstrp/appdef.hxx>
#include <bootstrp/command.hxx>
#include <stdio.h>

//
// SourceTreeLocalizer
//

const char *ExeTable[][4] = {
    { "src", "transex3", "-UTF8", "negative" },
    { "hrc", "transex3", "-UTF8", "positive" },
    { "lng", "lngex", "-UTF8", "negative" },
    { "xrb", "xmlex", "-UTF8", "negative" },
    { "xxl", "xmlex", "-UTF8", "negative" },
    { "xgf", "xmlex", "-UTF8 -t:xgf", "negative" },
    { "xcd", "cfgex", "-UTF8", "negative" },
    { "NULL", "NULL", "NULL", "NULL" }
};

const char *NegativeList[] = {
    "officecfg/data/org.openoffice.Office.Labels.xcd",
    "officecfg/data/org/openoffice/Office/Labels.xcd",
    "hidother.src",
    "NULL"
};

const char *PositiveList[] = {
    "svx/inc/globlmn.hrc",
    "sw/source/ui/inc/swmn.hrc",
    "sw/source/ui/inc/swacc.hrc",
    "sw/source/ui/inc/toolbox.hrc",
    "offmgr/inc/offmenu.hrc",
    "offmgr/source/offapp/intro/intro.hrc",
    "NULL"
};

const char PRJ_DIR_NAME[] = "prj";
const char DLIST_NAME[] = "d.lst";

#define LOCALIZE_NONE       0x0000
#define LOCALIZE_EXTRACT    0x0001
#define LOCALIZE_MERGE      0x0002

class SourceTreeLocalizer : public SourceTreeIterator
{
private:
    SvFileStream aSDF;
    USHORT nMode;

    const DirEntry GetTempFile();
    const ByteString GetProjectName( BOOL bAbs = FALSE );
    const ByteString GetProjectRootRel();

    BOOL CheckNegativeList( const ByteString &rFileName );
    BOOL CheckPositiveList( const ByteString &rFileName );

    void WorkOnFile(
        const ByteString &rFileName,
        const ByteString &rExecutable,
        const ByteString &rParameter
    );

    void WorkOnFileType(
        const ByteString &rDirectory,
        const ByteString &rExtension,
        const ByteString &rExecutable,
        const ByteString &rParameter,
        const ByteString &rCollectMode
    );
    void WorkOnDirectory( const ByteString &rDirectory );

public:
    SourceTreeLocalizer( const ByteString &rRoot, const ByteString &rVersion );
    ~SourceTreeLocalizer();

    BOOL Extract( const ByteString &rDestinationFile );
    BOOL Merge( const ByteString &rSourceFile );

    virtual void OnExecuteDirectory( const ByteString &rDirectory );
};

/*****************************************************************************/
SourceTreeLocalizer::SourceTreeLocalizer(
    const ByteString &rRoot, const ByteString &rVersion )
/*****************************************************************************/
                : SourceTreeIterator( rRoot, rVersion ),
                nMode( LOCALIZE_NONE )
{
}

/*****************************************************************************/
SourceTreeLocalizer::~SourceTreeLocalizer()
/*****************************************************************************/
{
}

/*****************************************************************************/
const ByteString SourceTreeLocalizer::GetProjectName( BOOL bAbs )
/*****************************************************************************/
{
    BOOL bFound = FALSE;
    DirEntry aCur;
    aCur.ToAbs();

    for ( ; ! bFound && aCur.Level() > 1; aCur.CutName() )
    {
        DirEntry aTest = aCur + DirEntry(PRJ_DIR_NAME) + DirEntry(DLIST_NAME);
        if ( aTest.Exists() )
        {
            if ( bAbs )
                return ByteString( aCur.GetFull(), RTL_TEXTENCODING_ASCII_US );
            else
                return ByteString( aCur.GetName(), RTL_TEXTENCODING_ASCII_US );
        }
    }

    return "";
}

/*****************************************************************************/
const ByteString SourceTreeLocalizer::GetProjectRootRel()
/*****************************************************************************/
{
    ByteString sProjectRoot( GetProjectName( TRUE ));
    DirEntry aCur;
    aCur.ToAbs();
    ByteString sCur( aCur.GetFull(), RTL_TEXTENCODING_ASCII_US );

    if( sCur.SearchAndReplace( sProjectRoot, "" ) == STRING_NOTFOUND )
        return "";

    ByteString sDelimiter(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

    sCur.SearchAndReplaceAll( sDelimiter, "/" );
    sCur.EraseLeadingChars( '/' );
    ULONG nCount = sCur.GetTokenCount( '/' );

    ByteString sProjectRootRel;
    for ( ULONG i = 0; i < nCount; i++ ) {
        if ( sProjectRootRel.Len())
            sProjectRootRel += sDelimiter;
        sProjectRootRel += "..";
    }
    if ( sProjectRootRel.Len())
        return sProjectRootRel;

    return ".";
}

/*****************************************************************************/
const DirEntry SourceTreeLocalizer::GetTempFile()
/*****************************************************************************/
{
#ifdef WNT
    String sTempDir( GetEnv( "TEMP" ), RTL_TEXTENCODING_ASCII_US );
#else
    String sTempDir( GetEnv( "HOME" ), RTL_TEXTENCODING_ASCII_US );
#endif
    DirEntry aTemp( sTempDir );
    return aTemp.TempName();
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnFile(
    const ByteString &rFileName, const ByteString &rExecutable,
    const ByteString &rParameter )
/*****************************************************************************/
{
    String sFull( rFileName, RTL_TEXTENCODING_ASCII_US );
    DirEntry aEntry( sFull );
    ByteString sFileName( aEntry.GetName(), RTL_TEXTENCODING_ASCII_US );

    // set current working directory
    DirEntry aPath( aEntry.GetPath());
    DirEntry aOldCWD;
    aPath.SetCWD();

    ByteString sPrj( GetProjectName());
    if ( sPrj.Len()) {
        ByteString sRoot( GetProjectRootRel());

        // get temp file
        DirEntry aTemp( GetTempFile());
        ByteString sTempFile( aTemp.GetFull(), RTL_TEXTENCODING_ASCII_US );

        ByteString sExecutable( rExecutable );
#ifdef WNT
        sExecutable += ".exe";
        String sPath( GetEnv( "PATH" ), RTL_TEXTENCODING_ASCII_US );
#else
        String sPath( GetEnv( "LD_LIBRARY_PATH" ), RTL_TEXTENCODING_ASCII_US );
#endif

        DirEntry aExecutable( String( sExecutable, RTL_TEXTENCODING_ASCII_US ));
        aExecutable.Find( sPath );

        ByteString sCommand( aExecutable.GetFull(), RTL_TEXTENCODING_ASCII_US );
        sCommand += " ";
        sCommand += rParameter;
        sCommand += " -p ";
        sCommand += sPrj;
        sCommand += " -r ";
        sCommand += sRoot;
        sCommand += " -i ";
        sCommand += sFileName;
        sCommand += " -o ";
        sCommand += sTempFile;

        system( sCommand.GetBuffer());

        SvFileStream aSDFIn( aTemp.GetFull(), STREAM_STD_READ );
        ByteString sLine;
        while ( !aSDFIn.IsEof()) {
            aSDFIn.ReadLine( sLine );
            if ( sLine.Len())
                aSDF.WriteLine( sLine );
        }
        aSDFIn.Close();

        aTemp.Kill();
    }
    // reset current working directory
    aOldCWD.SetCWD();
}

/*****************************************************************************/
BOOL SourceTreeLocalizer::CheckNegativeList( const ByteString &rFileName )
/*****************************************************************************/
{
    ULONG nIndex = 0;
    BOOL bReturn  = TRUE;

    ByteString sDelimiter(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

    ByteString sFileName( rFileName );
    sFileName.ToLowerAscii();

    ByteString sNegative( NegativeList[ nIndex ] );
    while( sNegative != "NULL" && bReturn ) {
        sNegative.SearchAndReplaceAll( "/", sDelimiter );
        sNegative.ToLowerAscii();

        if( sFileName.Search( sNegative ) == sFileName.Len() - sNegative.Len())
            bReturn = FALSE;

        nIndex++;
        sNegative = NegativeList[ nIndex ];
    }

    return bReturn;
}

/*****************************************************************************/
BOOL SourceTreeLocalizer::CheckPositiveList( const ByteString &rFileName )
/*****************************************************************************/
{
    ULONG nIndex = 0;
    BOOL bReturn  = FALSE;

    ByteString sDelimiter(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

    ByteString sFileName( rFileName );
    sFileName.ToLowerAscii();

    ByteString sNegative( PositiveList[ nIndex ] );
    while( sNegative != "NULL" && !bReturn ) {
        sNegative.SearchAndReplaceAll( "/", sDelimiter );
        sNegative.ToLowerAscii();

        if( sFileName.Search( sNegative ) == sFileName.Len() - sNegative.Len())
            bReturn = TRUE;

        nIndex++;
        sNegative = PositiveList[ nIndex ];
    }

    return bReturn;
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnFileType(
    const ByteString &rDirectory, const ByteString &rExtension,
    const ByteString &rExecutable, const ByteString &rParameter,
    const ByteString &rCollectMode
)
/*****************************************************************************/
{
    String sWild( rDirectory, RTL_TEXTENCODING_ASCII_US );
    sWild += DirEntry::GetAccessDelimiter();
    sWild += String::CreateFromAscii( "*." );
    sWild += String( rExtension, RTL_TEXTENCODING_ASCII_US );

    DirEntry aEntry( sWild );
    Dir aDir( sWild, FSYS_KIND_FILE );

    for ( USHORT i = 0; i < aDir.Count(); i++ ) {
        DirEntry aFile( aDir[ i ] );
        ByteString sFile( aFile.GetFull(), RTL_TEXTENCODING_ASCII_US );

        BOOL bAllowed = TRUE;

        if ( rCollectMode == "negative" )
            bAllowed = CheckNegativeList( sFile );
        else if ( rCollectMode == "positive" )
            bAllowed = CheckPositiveList( sFile );

        if ( bAllowed ) {
            fprintf( stdout, "%s\n", sFile.GetBuffer());
            WorkOnFile( sFile, rExecutable, rParameter );
        }
    }
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnDirectory( const ByteString &rDirectory )
/*****************************************************************************/
{
    ULONG nIndex = 0;
    ByteString sExtension( ExeTable[ nIndex ][ 0 ] );
    ByteString sExecutable( ExeTable[ nIndex ][ 1 ] );
    ByteString sParameter( ExeTable[ nIndex ][ 2 ] );
    ByteString sCollectMode( ExeTable[ nIndex ][ 3 ] );

    while( sExtension != "NULL" ) {
        WorkOnFileType(
            rDirectory,
            sExtension,
            sExecutable,
            sParameter,
            sCollectMode
        );

        nIndex++;

        sExtension = ExeTable[ nIndex ][ 0 ];
        sExecutable = ExeTable[ nIndex ][ 1 ];
        sParameter = ExeTable[ nIndex ][ 2 ];
        sCollectMode = ExeTable[ nIndex ][ 3 ];
    }
}

/*****************************************************************************/
void SourceTreeLocalizer::OnExecuteDirectory( const ByteString &rDirectory )
/*****************************************************************************/
{
    if ( nMode == LOCALIZE_NONE )
        fprintf( stdout, "%s\n", rDirectory.GetBuffer());
    else
        WorkOnDirectory( rDirectory );
}

/*****************************************************************************/
BOOL SourceTreeLocalizer::Extract( const ByteString &rDestinationFile )
/*****************************************************************************/
{
    nMode = LOCALIZE_EXTRACT;
    aSDF.Open( String( rDestinationFile, RTL_TEXTENCODING_ASCII_US ),
        STREAM_STD_WRITE | STREAM_TRUNC );

    BOOL bReturn = aSDF.IsOpen();
    if ( bReturn ) {
        bReturn = StartExecute();
        aSDF.Close();
    }

    nMode = LOCALIZE_NONE;
    return bReturn;
}

/*****************************************************************************/
BOOL SourceTreeLocalizer::Merge( const ByteString &rSourceFile )
/*****************************************************************************/
{
    nMode = LOCALIZE_MERGE;
    aSDF.Open( String( rSourceFile, RTL_TEXTENCODING_ASCII_US ),
        STREAM_STD_READ );

    BOOL bReturn = aSDF.IsOpen();
    if ( bReturn ) {
        bReturn = StartExecute();
        aSDF.Close();
    }

    nMode = LOCALIZE_NONE;
    return bReturn;
}

/*****************************************************************************/
#if defined( UNX ) || defined( MAC )
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    ByteString sRoot( GetEnv( "SOLARVERSION" ));
    DirEntry aRoot( String( sRoot, RTL_TEXTENCODING_ASCII_US ));
    aRoot += DirEntry( String::CreateFromAscii( "src" ));
    sRoot = ByteString( aRoot.GetFull(), RTL_TEXTENCODING_ASCII_US );
    ByteString sVersion( GetEnv( "WORK_STAMP" ));

    if ( !sRoot.Len() || !sVersion.Len()) {
        fprintf( stderr, "ERROR: No environment set!\n" );
        return 1;
    }

    SourceTreeLocalizer aIter( sRoot, sVersion );
    aIter.Extract( "x:\\nf\\test.txt" );

    return 0;
}
