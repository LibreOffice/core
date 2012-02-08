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

#include "sal/config.h"

#include <cstddef>
#include <fstream>

#include "srciter.hxx"
#include "export.hxx"
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include "tools/errcode.hxx"
#include "tools/fsys.hxx"
#include "tools/urlobj.hxx"

#include "helper.hxx"

using namespace std;

namespace {

DirEntry GetTempFile()
{
    rtl::OUString* sTempFilename = new rtl::OUString();

    // Create a temp file
    int nRC = osl::FileBase::createTempFile( 0 , 0 , sTempFilename );
    if( nRC ) printf(" osl::FileBase::createTempFile RC = %d",nRC);

    rtl::OUString strTmp( *sTempFilename  );

    INetURLObject::DecodeMechanism eMechanism = INetURLObject::DECODE_TO_IURI;
    rtl::OUString sDecodedStr = INetURLObject::decode( strTmp , '%' , eMechanism );
    rtl::OString sTmp(rtl::OUStringToOString(sDecodedStr , RTL_TEXTENCODING_UTF8));

#if defined(WNT)
    sTmp = comphelper::string::replace(sTmp,
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("file:///")),
        rtl::OString());
    sTmp = sTmp.replace('/', '\\');
#else
    // Set file permission to 644
    const sal_uInt64 nPerm = osl_File_Attribute_OwnRead | osl_File_Attribute_OwnWrite |
                             osl_File_Attribute_GrpRead | osl_File_Attribute_OthRead ;

    nRC = osl::File::setAttributes( *sTempFilename , nPerm );
    if( nRC ) printf(" osl::File::setAttributes RC = %d",nRC);

    sTmp = comphelper::string::replace(sTmp,
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("file://")),
        rtl::OString());
#endif
    DirEntry aDirEntry( sTmp );
    delete sTempFilename;
    return aDirEntry;
}

}

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
    std::ofstream aSDF;
    sal_uInt16 nMode;

    rtl::OString sLanguageRestriction;

    rtl::OString sOutputFile;

    int nFileCnt;

    const rtl::OString GetProjectName( sal_Bool bAbs = sal_False );
    const rtl::OString GetProjectRootRel();


    sal_Bool CheckNegativeList( const rtl::OString &rFileName );
    sal_Bool CheckPositiveList( const rtl::OString &rFileName );

    void WorkOnFile(
        const rtl::OString &rFileName,
        const rtl::OString &rExecutable,
        const rtl::OString &rParameter
    );

    void WorkOnFileType(
        const rtl::OString &rDirectory,
        const rtl::OString &rExtension,
        const rtl::OString &rExecutable,
        const rtl::OString &rParameter,
        const rtl::OString &rCollectMode
    );
    void WorkOnDirectory(const rtl::OString &rDirectory);
public:
    SourceTreeLocalizer(const rtl::OString &rRoot, bool skip_links);
    ~SourceTreeLocalizer();

    void SetLanguageRestriction( const rtl::OString& rRestrictions )
        { sLanguageRestriction = rRestrictions; }
    int getFileCnt();
    sal_Bool Extract(const rtl::OString &rDestinationFile);
    int GetFileCnt();
    virtual void OnExecuteDirectory( const rtl::OUString &rDirectory );
};

SourceTreeLocalizer::SourceTreeLocalizer(const rtl::OString &rRoot, bool skip_links)
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
const rtl::OString SourceTreeLocalizer::GetProjectName( sal_Bool bAbs )
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
const rtl::OString SourceTreeLocalizer::GetProjectRootRel()
/*****************************************************************************/
{
    rtl::OString sProjectRoot( GetProjectName( sal_True ));
    DirEntry aCur;
    aCur.ToAbs();
    rtl::OString sCur(rtl::OUStringToOString(aCur.GetFull(), RTL_TEXTENCODING_ASCII_US));

    if (helper::searchAndReplace(&sCur, sProjectRoot, rtl::OString()) == -1)
        return "";

    rtl::OString sDelimiter(rtl::OUStringToOString(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));

    helper::searchAndReplaceAll(&sCur, sDelimiter, "/");
    sCur = comphelper::string::stripStart(sCur, '/');
    sal_Int32 nCount = comphelper::string::getTokenCount(sCur, '/');

    rtl::OString sProjectRootRel;
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        if (!sProjectRootRel.isEmpty())
            sProjectRootRel += sDelimiter;
        sProjectRootRel += "..";
    }
    if (!sProjectRootRel.isEmpty())
        return sProjectRootRel;

    return ".";
}

bool skipProject( rtl::OString sPrj )
{
    int nIndex = 0;
    bool bReturn = true;
    rtl::OString sModule( ModuleList[ nIndex ] );
    while (!sModule.equalsL(RTL_CONSTASCII_STRINGPARAM("NULL")) && bReturn) {
        if (sPrj == sModule)
            bReturn = false;
        nIndex++;
        sModule = ModuleList[ nIndex ];
    }
    return bReturn;
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnFile(
    const rtl::OString &rFileName, const rtl::OString &rExecutable,
    const rtl::OString &rParameter )
/*****************************************************************************/
{
    rtl::OUString sFull(
        rtl::OStringToOUString(rFileName, RTL_TEXTENCODING_ASCII_US));
        DirEntry aEntry( sFull );
        rtl::OString sFileName(rtl::OUStringToOString(aEntry.GetName(), RTL_TEXTENCODING_ASCII_US));

        // set current working directory
        DirEntry aPath( aEntry.GetPath());
        DirEntry aOldCWD;
        aPath.SetCWD();

        rtl::OString sPrj( GetProjectName());
        if (!sPrj.isEmpty() && !skipProject( sPrj ) )
        {
            rtl::OString sRoot( GetProjectRootRel());

            DirEntry aTemp(GetTempFile());
            rtl::OString sTempFile(rtl::OUStringToOString(aTemp.GetFull(), RTL_TEXTENCODING_ASCII_US));

            rtl::OString sDel;
#if defined(WNT)
            sDel=rtl::OString("\\");
#else
            sDel=rtl::OString("/");
#endif
            rtl::OString sPath1( Export::GetEnv("SOLARVER") );
            rtl::OString sPath2( Export::GetEnv("INPATH_FOR_BUILD") );
            rtl::OString sPath3( "bin" );
            rtl::OString sExecutable( sPath1 );
#if defined(WNT)
            sExecutable = sExecutable.replace('/', '\\');
#endif
            sExecutable += sDel ;
            sExecutable += sPath2 ;
            sExecutable += sDel;
            sExecutable += sPath3 ;
            sExecutable += sDel ;
            sExecutable += rExecutable ;


        rtl::OString sCommand( sExecutable );
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
        if (!sLanguageRestriction.isEmpty()) {
            sCommand += " -l ";
            sCommand +=  sLanguageRestriction;
        }

            //printf("DBG: %s\n",sCommand.GetBuffer());
            if (system(sCommand.getStr()) == -1)
                fprintf(stderr, "%s failed\n", sCommand.getStr());
            nFileCnt++;

            ifstream aSDFIn(
                rtl::OUStringToOString(
                    aTemp.GetFull(), osl_getThreadTextEncoding()).
                getStr());
            while (aSDFIn.is_open() && !aSDFIn.eof())
            {
                std::string s;
                std::getline(aSDFIn, s);
                if (!s.empty())
                    aSDF << s << '\n';
            }
            aSDFIn.close();

            aTemp.Kill();

        }
        // reset current working directory
        aOldCWD.SetCWD();
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::CheckNegativeList( const rtl::OString &rFileName )
/*****************************************************************************/
{
    std::size_t nIndex = 0;
    sal_Bool bReturn  = sal_True;

    rtl::OString sDelimiter(rtl::OUStringToOString(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));

    rtl::OString sFileName(rFileName.toAsciiLowerCase());

    rtl::OString sNegative( NegativeList[ nIndex ] );
    while (!sNegative.equalsL(RTL_CONSTASCII_STRINGPARAM("NULL")) && bReturn) {
        helper::searchAndReplaceAll(&sNegative, "\\", sDelimiter);
        helper::searchAndReplaceAll(&sNegative, "/", sDelimiter);
        sNegative = sNegative.toAsciiLowerCase();

        if (sFileName.indexOf(sNegative)
            == sFileName.getLength() - sNegative.getLength())
        {
            bReturn = false;
        }

        nIndex++;
        sNegative = NegativeList[ nIndex ];
    }

    return bReturn;
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::CheckPositiveList( const rtl::OString &rFileName )
/*****************************************************************************/
{
    std::size_t nIndex = 0;
    sal_Bool bReturn  = sal_False;

    rtl::OString sDelimiter(rtl::OUStringToOString(
        DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));

    rtl::OString sFileName(rFileName.toAsciiLowerCase());

    rtl::OString sNegative( PositiveList[ nIndex ] );
    while (!sNegative.equalsL(RTL_CONSTASCII_STRINGPARAM("NULL")) && !bReturn) {
        helper::searchAndReplaceAll(&sNegative, "\\", sDelimiter);
        helper::searchAndReplaceAll(&sNegative, "/", sDelimiter);
        sNegative = sNegative.toAsciiLowerCase();

        if (sFileName.indexOf(sNegative)
            == sFileName.getLength() - sNegative.getLength())
        {
            bReturn = true;
        }

        nIndex++;
        sNegative = PositiveList[ nIndex ];
    }

    return bReturn;
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnFileType(
    const rtl::OString &rDirectory, const rtl::OString &rExtension,
    const rtl::OString &rExecutable, const rtl::OString &rParameter,
    const rtl::OString &rCollectMode
)
/*****************************************************************************/
{
    rtl::OUString sWild(
        rtl::OStringToOUString(rDirectory, RTL_TEXTENCODING_ASCII_US));
    sWild += DirEntry::GetAccessDelimiter();
    sWild += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*."));
    sWild += rtl::OStringToOUString(rExtension, RTL_TEXTENCODING_ASCII_US);

    Dir aDir(DirEntry(sWild), FSYS_KIND_FILE);

    for ( sal_uInt16 i = 0; i < aDir.Count(); i++ )
    {
        DirEntry aFile( aDir[ i ] );
        rtl::OString sFile(rtl::OUStringToOString(aFile.GetFull(), RTL_TEXTENCODING_ASCII_US));

        sal_Bool bAllowed = sal_True;

        if (rCollectMode.equalsL(RTL_CONSTASCII_STRINGPARAM("negative")))
            bAllowed = CheckNegativeList( sFile );
        else if (rCollectMode.equalsL(RTL_CONSTASCII_STRINGPARAM("positive")))
            bAllowed = CheckPositiveList( sFile );

        if ( bAllowed )
            WorkOnFile( sFile, rExecutable, rParameter );
    }
}

void SourceTreeLocalizer::WorkOnDirectory(const rtl::OString &rDirectory)
{
    //printf("Working on Directory %s\n",rDirectory.getStr());
    std::size_t nIndex = 0;
    rtl::OString sExtension( ExeTable[ nIndex ][ 0 ] );
    rtl::OString sExecutable( ExeTable[ nIndex ][ 1 ] );
    rtl::OString sParameter( ExeTable[ nIndex ][ 2 ] );
    rtl::OString sCollectMode( ExeTable[ nIndex ][ 3 ] );

    while (!sExtension.equalsL(RTL_CONSTASCII_STRINGPARAM("NULL"))) {
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

    aSDF.open(
        rDestinationFile.getStr(), std::ios_base::out | std::ios_base::app);

    sal_Bool bReturn = aSDF.is_open();
    if ( bReturn )
    {
        bReturn = StartExecute();
    }
    else
    {
        printf("ERROR: Can't create file %s\n", rDestinationFile.getStr());
    }
    nMode = LOCALIZE_NONE;
    aSDF.close();
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
    rtl::OUString sTempBase(RTL_CONSTASCII_USTRINGPARAM("loc"));
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
    rtl::OUString sFullEntry(aEntry.GetFull());
    rtl::OString sFileABS(rtl::OUStringToOString(aEntry.GetFull(), osl_getThreadTextEncoding()));
    sFileName = sFileABS;

    string pwd;
    Export::getCurrentDir( pwd );
    cout << "Localizing directory " << pwd << "\n";
    SourceTreeLocalizer aIter( rtl::OString( pwd.c_str() ) , bSkipLinks );
    aIter.SetLanguageRestriction( sLanguages );
    aIter.Extract( sFileName );
    printf("\n%d files found!\n",aIter.GetFileCnt());
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
