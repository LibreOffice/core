/*************************************************************************
 *
 *  $RCSfile: localize.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:44:02 $
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
#include "export.hxx"
#include <bootstrp/appdef.hxx>
#include <bootstrp/command.hxx>
#include <stdio.h>

// #define EHAM02_TEST

//
// SourceTreeLocalizer
//

const char *ExeTable[][5] = {
    { "src", "transex3", "-UTF8 -e", "negative", "noiso" },
    { "hrc", "transex3", "-UTF8 -e", "positive", "noiso" },
    { "lng", "lngex", "-UTF8 -e", "negative", "noiso" },
    { "ulf", "lngex", "-UTF8 -ULF -e", "negative", "noiso" },
    { "xrb", "xmlex", "-UTF8 -e", "negative", "iso" },
    { "xxl", "xmlex", "-UTF8 -e", "negative", "iso" },
    { "xgf", "xmlex", "-UTF8 -e -t:xgf", "negative", "iso" },
    { "xcd", "cfgex", "-UTF8 -e", "negative", "iso" },
    { "xcu", "cfgex", "-UTF8 -e", "negative", "iso" },
    { "xcs", "cfgex", "-UTF8 -e -f", "negative", "iso" },
    { "xrm", "xrmex", "-UTF8 -e", "negative", "iso" },
    { "NULL", "NULL", "NULL", "NULL", "NULL" }
};

const char *NegativeList[] = {
    "officecfg/data/org.openoffice.Office.Labels.xcd",
    "officecfg/data/org/openoffice/Office/Labels.xcd",
    "officecfg/data/org/openoffice/Office/SFX.xcd",
    "officecfg/registry/data/org/openoffice/Office/Labels.xcu",
    "officecfg/registry/data/org/openoffice/Office/SFX.xcu",
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
    "dbaccess/source/ui/inc/toolbox.hrc",
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

    ByteString sLanguageRestriction;
    ByteString sIsoCode99;

    const ByteString GetProjectName( BOOL bAbs = FALSE );
    const ByteString GetProjectRootRel();

    BOOL CheckNegativeList( const ByteString &rFileName );
    BOOL CheckPositiveList( const ByteString &rFileName );

    void WorkOnFile(
        const ByteString &rFileName,
        const ByteString &rExecutable,
        const ByteString &rParameter,
        const ByteString &rIso
    );

    void WorkOnFileType(
        const ByteString &rDirectory,
        const ByteString &rExtension,
        const ByteString &rExecutable,
        const ByteString &rParameter,
        const ByteString &rCollectMode,
        const ByteString &rIso
    );
    void WorkOnDirectory( const ByteString &rDirectory );
    BOOL ExecuteMerge();
    BOOL MergeSingleFile(
        const ByteString &rPrj,
        const ByteString &rFile,
        const ByteString &rSDFFile
    );

public:
    SourceTreeLocalizer( const ByteString &rRoot, const ByteString &rVersion );
    ~SourceTreeLocalizer();

    void SetLanguageRestriction( const ByteString& rRestrictions )
        { sLanguageRestriction = rRestrictions; }
    void SetIsoCode99( const ByteString& rIsoCode )
        { sIsoCode99 = rIsoCode; }

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
             // HACK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            if (( ByteString( aCur.GetName(), RTL_TEXTENCODING_ASCII_US ).Equals("webinstall") ) ||
                               ( ByteString( aCur.GetName(), RTL_TEXTENCODING_ASCII_US ).Equals("portal") ) ||
                               ( ByteString( aCur.GetName(), RTL_TEXTENCODING_ASCII_US ).Equals("xulclient") ) ||
                               ( ByteString( aCur.GetName(), RTL_TEXTENCODING_ASCII_US ).Search( "wdk_" ) == 0 ))
                    return "";
            // end HACK !!!!!!!!!!!!!!!!!!!!!!!!!



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
void SourceTreeLocalizer::WorkOnFile(
    const ByteString &rFileName, const ByteString &rExecutable,
    const ByteString &rParameter, const ByteString &rIso )
/*****************************************************************************/
{
    if (( rIso.Equals("noiso") ) || sIsoCode99.Len()) {
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
            DirEntry aTemp( Export::GetTempFile());
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
            if ( sLanguageRestriction.Len()) {
                sCommand += " -l ";
                sCommand += sLanguageRestriction;
            }
            if ( rIso.Equals("iso") && sIsoCode99.Len()) {
                sCommand += " -ISO99 ";
                sCommand += sIsoCode99;
            }

            system( sCommand.GetBuffer());

            SvFileStream aSDFIn( aTemp.GetFull(), STREAM_READ );
            ByteString sLine;
            while ( !aSDFIn.IsEof()) {
                aSDFIn.ReadLine( sLine );
                if ( sLine.Len()) {

#ifdef EHAM02_TEST
                    if ( sLine.GetToken( 9, '\t' ) == "99" ) {
                        ByteString sTmp;
                        for ( USHORT i = 0; i < sLine.GetTokenCount( '\t' ); i++ ) {
                            if ( i == 10 )
                                sTmp += "X_";
                            sTmp += sLine.GetToken( i, '\t' );
                            sTmp += "\t";
                        }
                        sTmp.EraseTrailingChars( '\t' );
                        sLine = sTmp;
                    }
#endif

                    aSDF.WriteLine( sLine );
                }
            }
            aSDFIn.Close();

            aTemp.Kill();
        }
        // reset current working directory
        aOldCWD.SetCWD();
    }
    else {
        fprintf( stdout, "ERROR: Iso code required for file %s\n", rFileName.GetBuffer());
    }
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
    while( !sNegative.Equals( "NULL" ) && bReturn ) {
        sNegative.SearchAndReplaceAll( "\\", sDelimiter );
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
    while( !sNegative.Equals( "NULL" ) && !bReturn ) {
        sNegative.SearchAndReplaceAll( "\\", sDelimiter );
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
    const ByteString &rCollectMode, const ByteString &rIso
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

        if ( rCollectMode.Equals( "negative" ))
            bAllowed = CheckNegativeList( sFile );
        else if ( rCollectMode.Equals( "positive" ))
            bAllowed = CheckPositiveList( sFile );

        if ( bAllowed )
            WorkOnFile( sFile, rExecutable, rParameter, rIso );
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
    ByteString sIso( ExeTable[ nIndex ][ 4 ] );

    while( !sExtension.Equals( "NULL" )) {
        WorkOnFileType(
            rDirectory,
            sExtension,
            sExecutable,
            sParameter,
            sCollectMode,
            sIso
        );

        nIndex++;

        sExtension = ExeTable[ nIndex ][ 0 ];
        sExecutable = ExeTable[ nIndex ][ 1 ];
        sParameter = ExeTable[ nIndex ][ 2 ];
        sCollectMode = ExeTable[ nIndex ][ 3 ];
        sIso = ExeTable[ nIndex ][ 4 ];
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
    aSDF.SetLineDelimiter( LINEEND_CRLF );

    BOOL bReturn = aSDF.IsOpen();
    if ( bReturn ) {
        bReturn = StartExecute();
        aSDF.Close();
    }

    nMode = LOCALIZE_NONE;
    return bReturn;
}

/*****************************************************************************/
BOOL SourceTreeLocalizer::MergeSingleFile(
    const ByteString &rPrj,
    const ByteString &rFile,
    const ByteString &rSDFFile
)
/*****************************************************************************/
{
    if ( !rFile.Len())
        return TRUE;

    ByteString sRoot( GetEnv( "SRC_ROOT" ));
    DirEntry aEntry( String( sRoot, RTL_TEXTENCODING_ASCII_US ));
    aEntry += DirEntry( String( rPrj, RTL_TEXTENCODING_ASCII_US ));

    ByteString sDelimiter(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

    ByteString sCur( rFile );
    sCur.SearchAndReplaceAll( "\\", sDelimiter );
    sCur.SearchAndReplaceAll( "/", sDelimiter );

    aEntry += DirEntry( String( sCur, RTL_TEXTENCODING_ASCII_US ));
    ByteString sFile( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );

    ByteString sBCur( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
    fprintf( stdout, "##### %s #####\n", sBCur.GetBuffer());

    ULONG nIndex = 0;
    ByteString sExtension( aEntry.GetExtension(), RTL_TEXTENCODING_ASCII_US );
    ByteString sCandidate( ExeTable[ nIndex ][ 0 ] );

    while( !sCandidate.Equals ("NULL") && !sCandidate.Equals(sExtension) )
        sCandidate = ExeTable[ ++nIndex ][ 0 ];

    ByteString sIso( ExeTable[ nIndex ][ 4 ] );

    if ( !sCandidate.Equals( "NULL" ) ) {
        if( !aEntry.Exists()) {
            DirEntryKind theDir=FSYS_KIND_FILE;
            Dir myDir( aEntry.GetPath(), theDir);
            DirEntry current;
            BOOL found=FALSE;
            for(int x=0; x < myDir.Count() && !found;){
                current=myDir[x++];
                StringCompare result=current.GetName().CompareIgnoreCaseToAscii( aEntry.GetName() );
                if( result==COMPARE_EQUAL ){
                    fprintf(stderr,"WARNING: %s not found\n", ByteString(aEntry.GetFull(),RTL_TEXTENCODING_ASCII_US).GetBuffer() );
                    fprintf(stderr,"but use  %s instead \n" , ByteString(current.GetFull(), RTL_TEXTENCODING_ASCII_US).GetBuffer() );
                    aEntry=current;
                    found=TRUE;
                }
            }
            if(!found)  return TRUE;

        }

        DirEntry aOut( Export::GetTempFile() );
        ByteString sOutput( aOut.GetFull(), RTL_TEXTENCODING_ASCII_US );

        ByteString sCommand( ExeTable[ nIndex ][ 1 ] );
        sCommand += " -i ";
        sCommand += ByteString( aEntry.GetName(), RTL_TEXTENCODING_ASCII_US );
        sCommand += " -m ";
        sCommand += rSDFFile;
        sCommand += " -o ";
        sCommand += sOutput;
        sCommand += " ";
        sCommand += ByteString( ExeTable[ nIndex ][ 2 ] );
        if ( sIso.Equals( "iso" ) && sIsoCode99.Len()) {
            sCommand += " -ISO99 ";
            sCommand += sIsoCode99;
        }
        if ( sLanguageRestriction.Len()) {
            sCommand += " -l ";
            sCommand += sLanguageRestriction;
        }

        DirEntry aPath( aEntry.GetPath());
        DirEntry aOldCWD;
        aPath.SetCWD();

        system( sCommand.GetBuffer());

        SvFileStream aInStream( aOut.GetFull(), STREAM_READ );
        if ( !aInStream.IsOpen()) {
            fprintf( stderr,
                "ERROR: Unable to open file %s for reading!\n",
                sOutput.GetBuffer());
        }
        else {
            FileStat::SetReadOnlyFlag( aEntry, FALSE );
            String myStr2(aEntry.GetFull());
            String aTemp22 = String::CreateFromAscii("_tmp");
            myStr2.Append(aTemp22);

            ByteString test(myStr2,RTL_TEXTENCODING_ASCII_US);
            SvFileStream aOutStream( myStr2, STREAM_STD_WRITE | STREAM_TRUNC );

            if ( !aOutStream.IsOpen()) {
                ByteString test2(myStr2,RTL_TEXTENCODING_ASCII_US);
                fprintf( stderr,"ERROR: Unable to open file %s for modification!\n", test2.GetBuffer());
                aInStream.Close();
            }

            else {
                ByteString sLine;
                aOutStream.SetLineDelimiter( LINEEND_CRLF );
                while ( !aInStream.IsEof()) {
                    aInStream.ReadLine( sLine );
                    sLine.EraseAllChars( '\r' );
                    aOutStream.WriteLine( sLine );
                }
                aInStream.Close();
                aOutStream.Close();

                DirEntry myTempFile(ByteString(myStr2,RTL_TEXTENCODING_ASCII_US));      // xxx_tmp ->
                DirEntry myFile(ByteString(aEntry.GetFull(),RTL_TEXTENCODING_ASCII_US));// xxx

                DirEntry oldFile(ByteString(aEntry.GetFull(),RTL_TEXTENCODING_ASCII_US));

                if(oldFile.Kill()==ERRCODE_NONE){
                    if(myTempFile.MoveTo(myFile)!=ERRCODE_NONE){
                        fprintf( stderr, "ERROR: Can't rename file %s\n",ByteString(myStr2,RTL_TEXTENCODING_ASCII_US).GetBuffer());
                    }
                }
                else{
                    fprintf( stderr, "ERROR: Can't remove file %s\n",ByteString(aEntry.GetFull(),RTL_TEXTENCODING_ASCII_US).GetBuffer());
                }
            } // else

            aOldCWD.SetCWD();
            aOut.Kill();
        }   // else

    }       // if ( !sCandidate.Equals( "NULL" ) )

    return TRUE;
}

/*****************************************************************************/
BOOL SourceTreeLocalizer::ExecuteMerge()
/*****************************************************************************/
{
    DirEntry aEntry( Export::GetTempFile());
    BOOL bReturn = TRUE;

    ByteString sFileName;
    ByteString sLine;

    SvFileStream aFile;

    while ( !aSDF.IsEof()) {
        aSDF.ReadLine( sLine );
        ByteString sOldFileName( sFileName );
        sFileName = sLine.GetToken( 0, '\t' );
        sFileName += "#";
        sFileName += sLine.GetToken( 1, '\t' );
        if ( sFileName.Len() && ( !sOldFileName.Equals(sFileName) )) {
            if ( aFile.IsOpen()) {
                aFile.Close();

                ByteString sPrj( sOldFileName.GetToken( 0, '#' ));
                ByteString sFile( sOldFileName.GetToken( 1, '#' ));
                ByteString sSDFFile( aFile.GetFileName(), RTL_TEXTENCODING_ASCII_US );

                if ( !MergeSingleFile( sPrj, sFile, sSDFFile ))
                    bReturn = FALSE;
            }
            aFile.Open( aEntry.GetFull(),
                STREAM_STD_WRITE |STREAM_TRUNC );
        }
        if ( aFile.IsOpen() && sLine.Len())
            aFile.WriteLine( sLine );
    }
    if ( aFile.IsOpen()) {
        aFile.Close();

        ByteString sPrj( sLine.GetToken( 0, '\t' ));
        ByteString sFile( sLine.GetToken( 1, '\t' ));
        ByteString sSDFFile( aFile.GetFileName(), RTL_TEXTENCODING_ASCII_US );

        if ( !MergeSingleFile( sPrj, sFile, sSDFFile ))
            bReturn = FALSE;
    }

    aEntry.Kill();

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
        bReturn = ExecuteMerge();
        aSDF.Close();
    }

    nMode = LOCALIZE_NONE;
    return bReturn;
}

#define STATE_NONE      0x0000
#define STATE_EXPORT    0x0001
#define STATE_MERGE     0x0002
#define STATE_ISOCODE   0x0003
#define STATE_LANGUAGES 0x0004
#define STATE_FILENAME  0x0005

/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout,
        "localize (c)2001 by Sun Microsystems\n"
        "====================================\n" );
    fprintf( stdout,
        "As part of the L10N framework, localize extracts and merges translations\n"
        "out of and into the whole source tree.\n\n"
        "Syntax: localize -e|-m -i ISO-Code -l l1[=f1][,l2[=f2]][...] -f FileName\n"
        "Parameter:\n"
        "\t-e: Extract mode\n"
        "\t-m: Merge mode\n"
        "\tFileName: Output file when extract mode, input file when merge mode\n"
        "\tl1...ln: supported languages (\"all\" for all languages).\n"
        "\tf1...fn: fallback languages for supported languages\n"
        "\tISO-Code: The full qualified ISO language code for language 99 (en-US, de, ...)"
    );

    fprintf( stdout,
        "Valid language codes for l1...ln and f1...fn are:\n" );

    for ( USHORT i = 0; i < LANGUAGES; i++ ) {
        ByteString sId;
        if ( Export::LangId[ i ] < 10 ) {
            sId = "0";
        }
        sId += ByteString::CreateFromInt32( Export::LangId[ i ] );
        ByteString sLanguage( Export::LangName[ i ] );
        fprintf( stdout,
            "\t%s => %s\n",
            sId.GetBuffer(),
            sLanguage.GetBuffer()
        );
    }
    fprintf( stdout,
        "\nExample 1:\n"
        "==========\n"
        "localize -e -i de-CH -l 01,99=35 -f MyFile\n\n"
        "All strings will be extracted for language 01 and language 99.\n"
        "If 99 is empty, language 35 will be fallback.\n"
    );
    fprintf( stdout,
        "\nExample 2:\n"
        "==========\n"
        "localize -m -i de-CH -l 99 -f MyFile\n\n"
        "All strings in MyFile will be merged into language 99 in the\n"
        "source code.\n"
    );
}

/*****************************************************************************/
int Error()
/*****************************************************************************/
{
    Help();
    return 1;
}

/*****************************************************************************/
BOOL CheckLanguages( ByteString &rLanguages )
/*****************************************************************************/
{
    BOOL bReturn = TRUE;

    ByteString sTmp( rLanguages );
    /* Using gcc-2.95.3 and STLport-4.5 .Equals() must
     * be used.. using == causes a compile error */
    if ( sTmp.ToUpperAscii().Equals("ALL") ) {
        rLanguages = "";
        for ( USHORT i = 0; i < LANGUAGES; i++ ) {
            if ( LANGUAGE_ALLOWED( i )) {
                if ( rLanguages.Len())
                    rLanguages += ",";
                rLanguages += ByteString::CreateFromInt32( Export::LangId[ i ] );
            }
        }
        fprintf( stdout, "\nExpanded -l all to %s\n", rLanguages.GetBuffer());
    }
    for ( USHORT i = 0; i < rLanguages.GetTokenCount( ',' ); i++ ) {
        ByteString sCur = rLanguages.GetToken( i, ',' );
        ByteString sLang = sCur.GetToken( 0, '=' );
        USHORT nLang = ( USHORT ) sLang.ToInt32();

        ByteString sFallback = sCur.GetToken( 1, '=' );
        USHORT nFallback = ( USHORT ) sFallback.ToInt32();

        if ( Export::GetLangIndex( nLang ) == 0xFFFF ) {
            fprintf( stderr, "ERROR: Unknown language %s\n",
                sLang.GetBuffer());
            bReturn = FALSE;
        }
        else if ( sFallback.Len() && ( Export::GetLangIndex( nFallback )==0xFFFF )){
            fprintf( stderr, "ERROR: Unknown fallback languges %s\n",
                sFallback.GetBuffer());
            bReturn = FALSE;
        }
    }

    if ( bReturn ) {
        if ( !rLanguages.Len()) {
            rLanguages = "01,99=01";
        }
/*      if ( rLanguages.Search( "99" ) == STRING_NOTFOUND ) {
            rLanguages += ",99=01";
        } */
    }

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
    String sTempBase( String::CreateFromAscii( "loc" ));
    DirEntry::SetTempNameBase( sTempBase );

    USHORT nState = STATE_NONE;

    BOOL bExport = FALSE;
    BOOL bMerge = FALSE;

    ByteString sIsoCode;
    ByteString sLanguages;
    ByteString sFileName;

    for( int i = 1; i < argc; i++ ) {
        if ( ByteString( argv[ i ]).ToUpperAscii().Equals( "-E" )) {
            nState = STATE_EXPORT;
            if ( bMerge )
                return Error();
            bExport = TRUE;
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii().Equals( "-M" )) {
            nState = STATE_MERGE;
            if ( bExport )
                return Error();
            bMerge = TRUE;
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii().Equals( "-I" ) )
            nState = STATE_ISOCODE;
        else if ( ByteString( argv[ i ]).ToUpperAscii().Equals( "-L" ) )
            nState = STATE_LANGUAGES;
        else if ( ByteString( argv[ i ]).ToUpperAscii().Equals( "-F" ) )
            nState = STATE_FILENAME;
        else {
            switch ( nState ) {
                case STATE_NONE:
                    return Error();
                break;
                case STATE_ISOCODE:
                    if ( sIsoCode.Len())
                        return Error();
                    sIsoCode = ByteString( argv[ i ] );
                    nState = STATE_NONE;
                break;
                case STATE_LANGUAGES:
                    if ( sLanguages.Len())
                        return Error();
                    sLanguages = ByteString( argv[ i ] );
                    nState = STATE_NONE;
                break;
                case STATE_FILENAME:
                    if ( sFileName.Len())
                        return Error();
                    sFileName = ByteString( argv[ i ] );
                    nState = STATE_NONE;
                break;

                default:
                    return Error();
                break;
            }
        }
    }
    if ( !bMerge && !bExport ) {
        Help();
        return 1;
    }

    ByteString sRoot( GetEnv( "SRC_ROOT" ));
    DirEntry aRoot( String( sRoot, RTL_TEXTENCODING_ASCII_US ));
    sRoot = ByteString( aRoot.GetFull(), RTL_TEXTENCODING_ASCII_US );
    ByteString sVersion( GetEnv( "WORK_STAMP" ));

    if ( !sRoot.Len() || !sVersion.Len()) {
        fprintf( stderr, "ERROR: No environment set!\n" );
        return 1;
    }

    if ( !CheckLanguages( sLanguages ))
        return 2;

    if ( !sIsoCode.Len() && ( sLanguages.Search( "99" ) != STRING_NOTFOUND )) {
        fprintf( stderr, "ERROR: No ISO code given\n" );
        return 3;
    }

    if ( !sFileName.Len()) {
        fprintf( stderr, "ERROR: No filename given\n" );
        return 3;
    }

    ByteString sMode( "merge" );
    if ( bExport )
        sMode = "extract";

    ByteString sICode( sIsoCode );
    if ( !sICode.Len())
        sICode = "not given, support for language 99 disabled";
    fprintf( stdout,
        "\n"
        "============================================================\n"
        "Current settings:\n"
        "============================================================\n"
        "Mode:          %s\n"
        "Workspace:     %s\n"
        "Source tree:   %s\n"
        "Languages:     %s\n"
        "ISO code (99): %s\n"
        "Filename:      %s\n"
        "============================================================\n"
        "\n"
        ,
        sMode.GetBuffer(),
        sVersion.GetBuffer(),
        sRoot.GetBuffer(),
        sLanguages.GetBuffer(),
        sICode.GetBuffer(),
        sFileName.GetBuffer()
     );

    SourceTreeLocalizer aIter( sRoot, sVersion );
    aIter.SetLanguageRestriction( sLanguages );
     aIter.SetIsoCode99( sIsoCode );

    if ( bExport )
          aIter.Extract( sFileName );
    else {
        DirEntry aEntry( String( sFileName, RTL_TEXTENCODING_ASCII_US ));
        if ( !aEntry.Exists())
            return FALSE;
        aIter.Merge( sFileName );
    }

    return 0;
}
