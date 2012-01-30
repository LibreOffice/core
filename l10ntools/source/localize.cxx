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
    { "properties", "propex", " -e", "negative" },
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
    "nlpsolver",
    "officecfg",
    "padmin",
    "readlicense_oo",
    "reportbuilder",
    "reportdesign",
    "sc",
    "scaddins",
    "sccomp",
    "scp2",
    "scripting",
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
    void WorkOnDirectory(const rtl::OString &rDirectory);
public:
    SourceTreeLocalizer(const ByteString &rRoot, bool skip_links);
    ~SourceTreeLocalizer();

    void SetLanguageRestriction( const ByteString& rRestrictions )
        { sLanguageRestriction = rRestrictions; }
    int getFileCnt();
    sal_Bool Extract(const rtl::OString &rDestinationFile);
    int GetFileCnt();
    virtual void OnExecuteDirectory( const rtl::OUString &rDirectory );
};

SourceTreeLocalizer::SourceTreeLocalizer(const ByteString &rRoot, bool skip_links)
    : SourceTreeIterator(rRoot)
    , nMode( LOCALIZE_NONE )
    , nFileCnt( 0 )
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
                return rtl::OUStringToOString(aCur.GetFull(), RTL_TEXTENCODING_ASCII_US);
            else
                return rtl::OUStringToOString(aCur.GetName(), RTL_TEXTENCODING_ASCII_US);
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
    ByteString sCur(rtl::OUStringToOString(aCur.GetFull(), RTL_TEXTENCODING_ASCII_US));

    if( sCur.SearchAndReplace( sProjectRoot, "" ) == STRING_NOTFOUND )
        return "";

    rtl::OString sDelimiter(rtl::OUStringToOString(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));

    sCur.SearchAndReplaceAll( sDelimiter, "/" );
    sCur = comphelper::string::stripStart(sCur, '/');
    sal_Int32 nCount = comphelper::string::getTokenCount(sCur, '/');

    ByteString sProjectRootRel;
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
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
        rtl::OString sFileName(rtl::OUStringToOString(aEntry.GetName(), RTL_TEXTENCODING_ASCII_US));

        // set current working directory
        DirEntry aPath( aEntry.GetPath());
        DirEntry aOldCWD;
        aPath.SetCWD();

        ByteString sPrj( GetProjectName());
        if ( sPrj.Len() && !skipProject( sPrj ) )
        {
            ByteString sRoot( GetProjectRootRel());

            DirEntry aTemp( Export::GetTempFile());
            rtl::OString sTempFile(rtl::OUStringToOString(aTemp.GetFull(), RTL_TEXTENCODING_ASCII_US));

            ByteString sDel;
#if defined(WNT)
            sDel=ByteString("\\");
#else
            sDel=ByteString("/");
#endif
            ByteString sPath1( Export::GetEnv("SOLARVER") );
            ByteString sPath2( Export::GetEnv("INPATH_FOR_BUILD") );
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
            sCommand +=  sLanguageRestriction;
        }

            //printf("DBG: %s\n",sCommand.GetBuffer());
            if (system(sCommand.GetBuffer()) == -1)
                fprintf(stderr, "%s failed\n", sCommand.GetBuffer());
            nFileCnt++;

            SvFileStream aSDFIn( aTemp.GetFull(), STREAM_READ );
            rtl::OString sLine;
            while ( aSDFIn.IsOpen() && !aSDFIn.IsEof())
            {
                aSDFIn.ReadLine( sLine );
                if (!sLine.isEmpty())
                    aSDF.WriteLine( sLine );
            }
            aSDFIn.Close();

            aTemp.Kill();

        }
        // reset current working directory
        aOldCWD.SetCWD();
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::CheckNegativeList( const ByteString &rFileName )
/*****************************************************************************/
{
    sal_uLong nIndex = 0;
    sal_Bool bReturn  = sal_True;

    rtl::OString sDelimiter(rtl::OUStringToOString(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));

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

    rtl::OString sDelimiter(rtl::OUStringToOString(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));

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

    for ( sal_uInt16 i = 0; i < aDir.Count(); i++ )
    {
        DirEntry aFile( aDir[ i ] );
        rtl::OString sFile(rtl::OUStringToOString(aFile.GetFull(), RTL_TEXTENCODING_ASCII_US));

        sal_Bool bAllowed = sal_True;

        if ( rCollectMode.Equals( "negative" ))
            bAllowed = CheckNegativeList( sFile );
        else if ( rCollectMode.Equals( "positive" ))
            bAllowed = CheckPositiveList( sFile );

        if ( bAllowed )
            WorkOnFile( sFile, rExecutable, rParameter );
    }
}

void SourceTreeLocalizer::WorkOnDirectory(const rtl::OString &rDirectory)
{
    //printf("Working on Directory %s\n",rDirectory.getStr());
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

void SourceTreeLocalizer::OnExecuteDirectory(const rtl::OUString &aDirectory)
{
    if ( nMode != LOCALIZE_NONE )
    {
        rtl::OString rDirectory(rtl::OUStringToOString(aDirectory, RTL_TEXTENCODING_UTF8));
        WorkOnDirectory( rDirectory );
    }
}

sal_Bool SourceTreeLocalizer::Extract(const rtl::OString &rDestinationFile)
{
    nMode = LOCALIZE_EXTRACT;

    aSDF.Open(rtl::OStringToOUString(rDestinationFile, RTL_TEXTENCODING_ASCII_US) , STREAM_STD_WRITE);
    aSDF.SetLineDelimiter( LINEEND_CRLF );

    sal_Bool bReturn = aSDF.IsOpen();
    if ( bReturn )
    {
        aSDF.Seek( STREAM_SEEK_TO_END );
        bReturn = StartExecute();
        aSDF.Close();
    }
    else
    {
        printf("ERROR: Can't create file %s\n", rDestinationFile.getStr());
    }
    nMode = LOCALIZE_NONE;
    aSDF.Close();
    return bReturn;
}

}
using namespace transex3;

/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout,
        "localize (c)2001 by Sun Microsystems\n"
        "====================================\n" );
    fprintf( stdout,
        "As part of the L10N framework, localize extracts en-US strings for\n"
        "translation out of the toplevel modules defined in ModuleList array in\n"
        "l10ntools/source/localize.cxx.\n\n"
        "Syntax: localize -f FileName \n"
        "Parameter:\n"
        "\tFileName: Output file\n"
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
#if defined(UNX)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    String sTempBase( String::CreateFromAscii( "loc" ));
    DirEntry::SetTempNameBase( sTempBase );

    bool bSkipLinks = false;

    rtl::OString sFileName;

    rtl::OString sLanguages(RTL_CONSTASCII_STRINGPARAM("en-US"));

    rtl::OString sSwitch(rtl::OString(argv[1]).toAsciiUpperCase());

    if ( ( argc == 3 ) && sSwitch.equalsL(RTL_CONSTASCII_STRINGPARAM("-F")) )
        sFileName = rtl::OString( argv[ 2 ] );
    else
        return Error();

    DirEntry aEntry(rtl::OStringToOUString(sFileName, RTL_TEXTENCODING_ASCII_US));
    aEntry.ToAbs();
    String sFullEntry = aEntry.GetFull();
    rtl::OString sFileABS(rtl::OUStringToOString(aEntry.GetFull(), osl_getThreadTextEncoding()));
    sFileName = sFileABS;

    string pwd;
    Export::getCurrentDir( pwd );
    cout << "Localizing directory " << pwd << "\n";
    SourceTreeLocalizer aIter( ByteString( pwd.c_str() ) , bSkipLinks );
    aIter.SetLanguageRestriction( sLanguages );
    aIter.Extract( sFileName );
    printf("\n%d files found!\n",aIter.GetFileCnt());
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
