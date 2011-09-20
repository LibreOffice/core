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
#include "precompiled_l10ntools.hxx"

#include "srciter.hxx"
#include "export.hxx"
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include "tools/errcode.hxx"
#include "tools/fsys.hxx"

#ifndef L10NTOOLS_FILE_HXX
#define L10NTOOLS_FILE_HXX
#include <l10ntools/file.hxx>
#endif

using namespace std;

namespace transex3
{

//
// SourceTreeLocalizer
//

const char *ExeTable[][4] = {
    { "src", "transex3", " -e", "negative" },
    { "hrc", "transex3", " -e", "positive" },
    { "ulf", "ulfex", " -e", "negative" },
    { "xcu", "cfgex", " -e", "negative" },
    { "xrm", "xrmex", " -e", "negative" },
    { "xml", "xrmex", " -e", "positive" },
    { "xhp", "helpex", " -e", "negative" },
    { "NULL", "NULL", "NULL", "NULL" }
};

const char *NegativeList[] = {
    "officecfg/registry/data/org/openoffice/Office/Labels.xcu",
    "officecfg/registry/data/org/openoffice/Office/SFX.xcu",
    "officecfg/registry/data/org/openoffice/Office/Accelerators.xcu",
    "dictionaries.xcu",
    "hidother.src",
    "dictionaries/da_DK/help/da/org.openoffice.da.hunspell.dictionaries/page1.xhp",
    "dictionaries/hu_HU/help/hu/org.openoffice.hu.hunspell.dictionaries/page1.xhp",
    "NULL"
};

const char *PositiveList[] = {
    "description.xml",
    "svx/inc/globlmn_tmpl.hrc",
    "sw/source/ui/inc/swmn_tmpl.hrc",
    "sw/source/ui/inc/swacc_tmpl.hrc",
    "sw/source/ui/inc/toolbox_tmpl.hrc",
    "offmgr/inc/offmenu_tmpl.hrc",
    "offmgr/source/offapp/intro/intro_tmpl.hrc",
    "dbaccess/source/ui/inc/toolbox_tmpl.hrc",
    "svx/source/intro/intro_tmpl.hrc",
    "dbaccess/source/ui/dlg/AutoControls_tmpl.hrc",
    "svx/source/unodialogs/textconversiondlgs/chinese_direction_tmpl.hrc",
    "chart2/source/controller/dialogs/res_DataLabel_tmpl.hrc",
    "chart2/source/controller/dialogs/res_LegendPosition_tmpl.hrc",
    "chart2/source/controller/dialogs/res_Statistic_tmpl.hrc",
    "chart2/source/controller/dialogs/res_Titlesx_tmpl.hrc",
    "chart2/source/controller/dialogs/res_SecondaryAxisCheckBoxes_tmpl.hrc",
    "chart2/source/controller/menu/MenuItems_tmpl.hrc",
    "chart2/source/controller/dialogs/res_ErrorBar_tmpl.hrc",
    "chart2/source/controller/dialogs/res_Trendline_tmpl.hrc",
       "svx.link/inc/globlmn_tmpl.hrc",
    "sw.link/source/ui/inc/swmn_tmpl.hrc",
    "sw.link/source/ui/inc/swacc_tmpl.hrc",
    "sw.link/source/ui/inc/toolbox_tmpl.hrc",
    "offmgr.link/inc/offmenu_tmpl.hrc",
    "offmgr.link/source/offapp/intro/intro_tmpl.hrc",
    "dbaccess.link/source/ui/inc/toolbox_tmpl.hrc",
    "svx.link/source/intro/intro_tmpl.hrc",
    "dbaccess.link/source/ui/dlg/AutoControls_tmpl.hrc",
    "svx.link/source/unodialogs/textconversiondlgs/chinese_direction_tmpl.hrc",
    "chart2.link/source/controller/dialogs/res_DataLabel_tmpl.hrc",
    "chart2.link/source/controller/dialogs/res_LegendPosition_tmpl.hrc",
    "chart2.link/source/controller/dialogs/res_Statistic_tmpl.hrc",
    "chart2.link/source/controller/dialogs/res_Titlesx_tmpl.hrc",
    "chart2.link/source/controller/dialogs/res_SecondaryAxisCheckBoxes_tmpl.hrc",
    "chart2.link/source/controller/menu/MenuItems_tmpl.hrc",
    "chart2.link/source/controller/dialogs/res_ErrorBar_tmpl.hrc",
    "chart2.link/source/controller/dialogs/res_Trendline_tmpl.hrc",
    "NULL"
};

const char *ModuleList[] = {
    "accessibility",
    "avmedia",
    "basctl",
    "basic",
    "chart2",
    "connectivity",
    "crashrep",
    "cui",
    "dbaccess",
    "desktop",
    "dictionaries",
    "editeng",
    "extensions",
    "filter",
    "forms",
    "formula",
    "fpicker",
    "framework",
    "helpcontent2",
    "instsetoo_native",
    "mysqlc",
    "officecfg",
    "padmin",
    "readlicense_oo",
    "reportbuilder",
    "reportdesign",
    "sc",
    "scaddins",
    "sccomp",
    "scp2",
    "sd",
    "sdext",
    "setup_native",
    "sfx2",
    "shell",
    "starmath",
    "svl",
    "svtools",
    "svx",
    "sw",
    "swext",
    "sysui",
    "uui",
    "vcl",
    "wizards",
    "xmlsecurity",
    "NULL",
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
    sal_uInt16 nMode;

    ByteString sLanguageRestriction;

    ByteString sOutputFile;

    int nFileCnt;

    const ByteString GetProjectName( sal_Bool bAbs = sal_False );
    const ByteString GetProjectRootRel();


    sal_Bool CheckNegativeList( const ByteString &rFileName );
    sal_Bool CheckPositiveList( const ByteString &rFileName );

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
    SourceTreeLocalizer( const ByteString &rRoot, const ByteString &rVersion , bool bLocal , bool skip_links );
    ~SourceTreeLocalizer();

    ByteString getSourceLanguages( ByteString sLanguageRestriction , ByteString sCommand );

    void SetLanguageRestriction( const ByteString& rRestrictions )
        { sLanguageRestriction = rRestrictions; }
    int getFileCnt();
    sal_Bool Extract( const ByteString &rDestinationFile );
    int GetFileCnt();
    virtual void OnExecuteDirectory( const rtl::OUString &rDirectory );
};

/*****************************************************************************/
SourceTreeLocalizer::SourceTreeLocalizer(
    const ByteString &rRoot, const ByteString &rVersion, bool bLocal_in , bool skip_links )
/*****************************************************************************/
                : SourceTreeIterator( rRoot, rVersion , bLocal_in ),
                nMode( LOCALIZE_NONE ),
                nFileCnt( 0 )
{
        bSkipLinks  = skip_links ;
}

/*****************************************************************************/
SourceTreeLocalizer::~SourceTreeLocalizer()
/*****************************************************************************/
{
}

/*****************************************************************************/
const ByteString SourceTreeLocalizer::GetProjectName( sal_Bool bAbs )
/*****************************************************************************/
{
    sal_Bool bFound = sal_False;
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
int SourceTreeLocalizer::GetFileCnt(){
/*****************************************************************************/
    return nFileCnt;
}

/*****************************************************************************/
const ByteString SourceTreeLocalizer::GetProjectRootRel()
/*****************************************************************************/
{
    ByteString sProjectRoot( GetProjectName( sal_True ));
    DirEntry aCur;
    aCur.ToAbs();
    ByteString sCur( aCur.GetFull(), RTL_TEXTENCODING_ASCII_US );

    if( sCur.SearchAndReplace( sProjectRoot, "" ) == STRING_NOTFOUND )
        return "";

    ByteString sDelimiter(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

    sCur.SearchAndReplaceAll( sDelimiter, "/" );
    sCur.EraseLeadingChars( '/' );
    sal_uLong nCount = sCur.GetTokenCount( '/' );

    ByteString sProjectRootRel;
    for ( sal_uLong i = 0; i < nCount; i++ ) {
        if ( sProjectRootRel.Len())
            sProjectRootRel += sDelimiter;
        sProjectRootRel += "..";
    }
    if ( sProjectRootRel.Len())
        return sProjectRootRel;

    return ".";
}

bool skipProject( ByteString sPrj )
{
    int nIndex = 0;
    bool bReturn = true;
    ByteString sModule( ModuleList[ nIndex ] );
    while( !sModule.Equals( "NULL" ) && bReturn ) {
        if( sPrj.Equals ( sModule ) )
            bReturn = false;
        nIndex++;
        sModule = ModuleList[ nIndex ];
    }
    return bReturn;
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
        if ( sPrj.Len() && !skipProject( sPrj ) )
        {
            ByteString sRoot( GetProjectRootRel());

            DirEntry aTemp( Export::GetTempFile());
            ByteString sTempFile( aTemp.GetFull(), RTL_TEXTENCODING_ASCII_US );

            ByteString sDel;
#if defined(WNT)
            sDel=ByteString("\\");
#else
            sDel=ByteString("/");
#endif
            ByteString sPath1( Export::GetEnv("SOLARVER") );
            ByteString sPath2( Export::GetEnv("INPATH") );
            ByteString sPath3( "bin" );
            ByteString sExecutable( sPath1 );
#if defined(WNT)
            sExecutable.SearchAndReplaceAll( "/", sDel );
#endif
            sExecutable += sDel ;
            sExecutable += sPath2 ;
            sExecutable += sDel;
            sExecutable += sPath3 ;
            sExecutable += sDel ;
            sExecutable += rExecutable ;


        ByteString sCommand( sExecutable );
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
        sCommand += getSourceLanguages( sLanguageRestriction , sCommand );
        }

            //printf("DBG: %s\n",sCommand.GetBuffer());
            if (system(sCommand.GetBuffer()) == -1)
                fprintf(stderr, "%s failed\n", sCommand.GetBuffer());
            nFileCnt++;

            SvFileStream aSDFIn( aTemp.GetFull(), STREAM_READ );
            ByteString sLine;
            while ( aSDFIn.IsOpen() && !aSDFIn.IsEof()) {
                aSDFIn.ReadLine( sLine );
                if ( sLine.Len()) {
                    aSDF.WriteLine( sLine );
                }
            }
            aSDFIn.Close();

            aTemp.Kill();

        }
        // reset current working directory
        aOldCWD.SetCWD();
}

ByteString SourceTreeLocalizer::getSourceLanguages( ByteString sLanguageRestriction_inout , ByteString sCommand )
{
    // Source languages in helpcontent2 and macromigration en-US only!
    if( sCommand.Search("helpex") != STRING_NOTFOUND ) {
        sLanguageRestriction_inout.Assign( ByteString("en-US") );
    }
    else if( sCommand.Search("xmlex") != STRING_NOTFOUND ){
        sLanguageRestriction_inout.Assign( ByteString("en-US") );
    }
    return sLanguageRestriction_inout;
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::CheckNegativeList( const ByteString &rFileName )
/*****************************************************************************/
{
    sal_uLong nIndex = 0;
    sal_Bool bReturn  = sal_True;

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
            bReturn = sal_False;

        nIndex++;
        sNegative = NegativeList[ nIndex ];
    }

    return bReturn;
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::CheckPositiveList( const ByteString &rFileName )
/*****************************************************************************/
{
    sal_uLong nIndex = 0;
    sal_Bool bReturn  = sal_False;

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
            bReturn = sal_True;

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

    for ( sal_uInt16 i = 0; i < aDir.Count(); i++ ) {
        DirEntry aFile( aDir[ i ] );
        ByteString sFile( aFile.GetFull(), RTL_TEXTENCODING_ASCII_US );

        sal_Bool bAllowed = sal_True;

        if ( rCollectMode.Equals( "negative" ))
            bAllowed = CheckNegativeList( sFile );
        else if ( rCollectMode.Equals( "positive" ))
            bAllowed = CheckPositiveList( sFile );

        if ( bAllowed )
            WorkOnFile( sFile, rExecutable, rParameter );
    }
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnDirectory( const ByteString &rDirectory )
/*****************************************************************************/
{
    //printf("Working on Directory %s\n",rDirectory.GetBuffer());
    sal_uLong nIndex = 0;
    ByteString sExtension( ExeTable[ nIndex ][ 0 ] );
    ByteString sExecutable( ExeTable[ nIndex ][ 1 ] );
    ByteString sParameter( ExeTable[ nIndex ][ 2 ] );
    ByteString sCollectMode( ExeTable[ nIndex ][ 3 ] );

    while( !sExtension.Equals( "NULL" )) {
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

void SourceTreeLocalizer::OnExecuteDirectory( const rtl::OUString &aDirectory )
{
    ByteString rDirectory( rtl::OUStringToOString( aDirectory , RTL_TEXTENCODING_UTF8 , aDirectory.getLength() ) ) ;
    if ( nMode == LOCALIZE_NONE ){
    }
    else
        WorkOnDirectory( rDirectory );
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::Extract( const ByteString &rDestinationFile )
/*****************************************************************************/
{
    nMode = LOCALIZE_EXTRACT;

    aSDF.Open( String( rDestinationFile , RTL_TEXTENCODING_ASCII_US ) , STREAM_STD_WRITE );
    aSDF.SetLineDelimiter( LINEEND_CRLF );

    sal_Bool bReturn = aSDF.IsOpen();
    if ( bReturn ) {
        aSDF.Seek( STREAM_SEEK_TO_END );
        bReturn = StartExecute();
        aSDF.Close();
    }
    else{
        printf("ERROR: Can't create file %s\n", rDestinationFile.GetBuffer() );
    }
    nMode = LOCALIZE_NONE;
    aSDF.Close();
    return bReturn;
}

}
using namespace transex3;

#define STATE_NONE      0x0000
#define STATE_EXPORT    0x0001
#define STATE_MERGE     0x0002
#define STATE_ISOCODE   0x0003
#define STATE_LANGUAGES 0x0004
#define STATE_FILENAME  0x0005
#define STATE_OUTPUT    0x0006

/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout,
        "localize (c)2001 by Sun Microsystems\n"
        "====================================\n" );
    fprintf( stdout,
        "As part of the L10N framework, localize extracts and merges translations\n"
        "out of and into the toplevel modules defined in ModuleList array in\n"
        "l10ntools/source/localize.cxx.\n\n"
        "Syntax: localize -e -l en-US -f FileName \n"
        "Parameter:\n"
        "\t-e: Extract mode\n"
        "\tFileName: Output file when extract mode, input file when merge mode\n"
        "\tl1...ln: supported languages (\"all\" for all languages).\n"
    );

    fprintf( stdout,
        "Valid language codes for l1...ln and f1...fn are:\n" );
    fprintf( stdout,
        "\nExample 1:\n"
        "==========\n"
        "localize -e -l en-US -f MyFile\n\n"
        "All strings will be extracted for language en-US.\n"
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
sal_Bool CheckLanguages( ByteString &rLanguages )
/*****************************************************************************/
{
    ByteString sTmp( rLanguages );
    return true;
}

/*****************************************************************************/
#if defined(UNX)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    String sTempBase( String::CreateFromAscii( "loc" ));
    DirEntry::SetTempNameBase( sTempBase );
    sal_uInt16 nState   = STATE_NONE;

    sal_Bool bExport    = sal_False;
    sal_Bool bMerge     = sal_False;
    bool bSkipLinks = false;

    ByteString sLanguages;
    ByteString sFileName;
    ByteString sOutput;

    bExport = sal_True;

    for( int i = 1; i < argc; i++ ) {
        ByteString sSwitch( argv[ i ] );
        sSwitch.ToUpperAscii();

        if ( sSwitch.Equals( "-E" )) {
            nState = STATE_EXPORT;
            if ( bMerge )
                return Error();
            bExport = sal_True;
        }
        else if ( sSwitch.Equals( "-I" ) )
            nState = STATE_ISOCODE;
        else if ( sSwitch.Equals( "-L" ) )
            nState = STATE_LANGUAGES;
        else if ( sSwitch.Equals( "-F" ) )
            nState = STATE_FILENAME;
        else if ( ByteString( argv[ i ]).ToUpperAscii().Equals( "-O" ) )
            nState = STATE_OUTPUT;
        else {
            switch ( nState ) {
                case STATE_NONE:
                    return Error();
                case STATE_OUTPUT:
                    if ( sOutput.Len())
                        return Error();
                    sOutput = ByteString( argv[ i ] );
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
            }
        }
    }
    if ( !bMerge && !bExport ) {
        Help();
        return 1;
    }

    ByteString sSolarVer( Export::GetEnv( "WORK_STAMP" ));
    ByteString sVersion( Export::GetEnv( "WORK_STAMP" ));

    if ( !sSolarVer.Len() || !sVersion.Len()) {
        fprintf( stderr, "ERROR: No environment set!\n" );
        return 1;
    }

    if ( !CheckLanguages( sLanguages ))
        return 2;

    if ( !sFileName.Len()) {
        fprintf( stderr, "ERROR: No filename given\n" );
        return 3;
    }

    DirEntry aEntry( String( sFileName , RTL_TEXTENCODING_ASCII_US ));
    aEntry.ToAbs();
    String sFullEntry = aEntry.GetFull();
    ByteString sFileABS(aEntry.GetFull(), osl_getThreadTextEncoding());
    //printf("B %s\nA %s\n",rDestinationFile.GetBuffer(), sFile.GetBuffer());
    sFileName = sFileABS;

    string pwd;
    Export::getCurrentDir( pwd );
    cout << "Localizing directory " << pwd << "\n";
    SourceTreeLocalizer aIter( ByteString( pwd.c_str() ) , sVersion , (sOutput.Len() > 0) , bSkipLinks );
    aIter.SetLanguageRestriction( sLanguages );
    if ( bExport ){
        fflush( stdout );
        aIter.Extract( sFileName );
        printf("\n%d files found!\n",aIter.GetFileCnt());
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
