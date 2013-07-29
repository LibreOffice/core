/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/thread.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/macros.h"
#include "sal/main.h"
#include "sal/types.h"

#include "po.hxx"

using namespace std;

namespace {

struct AsciiString {
    char const * string;
    sal_Int32 length;
};

bool matchList(
    const OUString& rUrl, const AsciiString* pList, size_t nLength)
{
    for (size_t i = 0; i != nLength; ++i) {
        if (rUrl.endsWithAsciiL(pList[i].string, pList[i].length)) {
            return true;
        }
    }
    return false;
}

bool passesNegativeList(const OUString& rUrl) {
    static const AsciiString list[] = {
        { RTL_CONSTASCII_STRINGPARAM("/dictionaries.xcu") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dictionaries/da_DK/help/da/help.tree") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dictionaries/da_DK/help/da/"
            "org.openoffice.da.hunspell.dictionaries/page1.xhp") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dictionaries/da_DK/help/da/"
            "org.openoffice.da.hunspell.dictionaries/page2.xhp") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dictionaries/hu_HU/help/hu/help.tree") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dictionaries/hu_HU/help/hu/"
            "org.openoffice.hu.hunspell.dictionaries/page1.xhp") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/officecfg/registry/data/org/openoffice/Office/"
            "Accelerators.xcu") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/officecfg/registry/data/org/openoffice/Office/SFX.xcu") }
    };
    return !matchList(rUrl, list, SAL_N_ELEMENTS(list));
}

bool passesPositiveList(const OUString& rUrl) {
    static const AsciiString list[] = {
        { RTL_CONSTASCII_STRINGPARAM(
            "/chart2/source/controller/dialogs/res_DataLabel_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/chart2/source/controller/dialogs/res_ErrorBar_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/chart2/source/controller/dialogs/res_LegendPosition_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/chart2/source/controller/dialogs/"
            "res_SecondaryAxisCheckBoxes_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/chart2/source/controller/dialogs/res_Statistic_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/chart2/source/controller/dialogs/res_Titlesx_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/chart2/source/controller/dialogs/res_Trendline_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/chart2/source/controller/menu/MenuItems_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dbaccess/source/ui/dlg/AutoControls_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dbaccess/source/ui/inc/toolbox_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM("/description.xml") },
        { RTL_CONSTASCII_STRINGPARAM("/android/sdremote/res/values/strings.xml") },
        { RTL_CONSTASCII_STRINGPARAM("/offmgr/inc/offmenu_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/offmgr/source/offapp/intro/intro_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM("/svx/inc/globlmn_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM("/svx/source/intro/intro_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/svx/source/unodialogs/textconversiondlgs/"
            "chinese_direction_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM("/sw/source/ui/inc/swacc_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM("/sw/source/ui/inc/swmn_tmpl.hrc") },
        { RTL_CONSTASCII_STRINGPARAM("/sw/source/ui/inc/toolbox_tmpl.hrc") }
    };
    return matchList(rUrl, list, SAL_N_ELEMENTS(list));
}

void handleCommand(
    const OString& rInPath, const OString& rOutPath,
    const OString& rExecutable)
{
    OStringBuffer buf(OString(getenv("SOLARVER")));
    buf.append('/');
    buf.append(OString(getenv("INPATH_FOR_BUILD")));
    buf.append("/bin/");
    buf.append(rExecutable);
    buf.append(" -i ");
    buf.append(rInPath);
    buf.append(" -o ");
    buf.append(rOutPath);

    const OString cmd = buf.makeStringAndClear();
    if (system(cmd.getStr()) != 0)
    {
        cerr << "Error: Failed to execute " << cmd.getStr() << '\n';
        throw false; //TODO
    }
}

void InitPoFile(
    const OString& rProject, const OString& rInPath,
    const OString& rPotDir, const OString& rOutPath )
{
    //Create directory for po file
    {
        OUString outDir =
            OStringToOUString(
                rPotDir.copy(0,rPotDir.lastIndexOf('/')), RTL_TEXTENCODING_UTF8);
        OUString outDirUrl;
        if (osl::FileBase::getFileURLFromSystemPath(outDir, outDirUrl)
            != osl::FileBase::E_None)
        {
            cerr << "Error: Cannot convert pathname to URL in " << __FILE__ << ", in line " << __LINE__ << "\n"
            << "       outDir: " << OUStringToOString(outDir, RTL_TEXTENCODING_ASCII_US).getStr() << "\n";
            throw false; //TODO
        }
        osl::Directory::createPath(outDirUrl);
    }

    //Add header to the po file
    PoOfstream aPoOutPut;
    aPoOutPut.open(rOutPath.getStr());
    if (!aPoOutPut.isOpen())
    {
        cerr
            << "Error: Cannot open po file "
            << rOutPath.getStr() << "\n";
        throw false; //TODO
    }

    const sal_Int32 nProjectInd = rInPath.indexOf(rProject);
    const OString relativPath =
        rInPath.copy(nProjectInd, rInPath.lastIndexOf('/')- nProjectInd);

    PoHeader aTmp(relativPath);
    aPoOutPut.writeHeader(aTmp);
    aPoOutPut.close();
}

bool handleFile(
    const OString& rProject, const OUString& rUrl,
    const OString& rPotDir, bool bInitPoFile )
{
    struct Command {
        char const * extension;
        sal_Int32 extensionLength;
        OString executable;
        bool positive;
    };
    static Command const commands[] = {
        { RTL_CONSTASCII_STRINGPARAM(".src"), "transex3", false },
        { RTL_CONSTASCII_STRINGPARAM(".hrc"), "transex3", true },
        { RTL_CONSTASCII_STRINGPARAM(".ulf"), "ulfex", false },
        { RTL_CONSTASCII_STRINGPARAM(".xcu"), "cfgex", false },
        { RTL_CONSTASCII_STRINGPARAM(".xrm"), "xrmex", false },
        { RTL_CONSTASCII_STRINGPARAM("description.xml"), "xrmex", true },
        { RTL_CONSTASCII_STRINGPARAM("strings.xml"), "stringex", true },
        { RTL_CONSTASCII_STRINGPARAM(".xhp"), "helpex", false },
        { RTL_CONSTASCII_STRINGPARAM(".properties"), "propex", false },
        { RTL_CONSTASCII_STRINGPARAM(".ui"), "uiex", false },
        { RTL_CONSTASCII_STRINGPARAM(".tree"), "treex", false } };
    for (size_t i = 0; i != SAL_N_ELEMENTS(commands); ++i)
    {
        if (rUrl.endsWithAsciiL(
                commands[i].extension, commands[i].extensionLength) &&
            (commands[i].executable != "propex" || rUrl.indexOf("en_US") != -1))
        {
            if (commands[i].positive ? passesPositiveList(rUrl) : passesNegativeList(rUrl))
            {
                //Get input file path
                OString sInPath;
                {
                    OUString sInPathTmp;
                    if (osl::FileBase::getSystemPathFromFileURL(rUrl, sInPathTmp) !=
                        osl::FileBase::E_None)
                    {
                        cerr << "osl::FileBase::getSystemPathFromFileURL(" << rUrl << ") failed\n";
                        throw false; //TODO
                    }
                    sInPath = OUStringToOString( sInPathTmp, RTL_TEXTENCODING_UTF8 );
                }
                OString sOutPath = rPotDir.concat(".pot");

                if ( bInitPoFile )
                {
                    InitPoFile(rProject, sInPath, rPotDir, sOutPath);
                }
                handleCommand(sInPath, sOutPath, commands[i].executable);
                return true;
            }
            break;
        }
    }
    return false;
}

void handleFilesOfDir(
    std::vector<OUString>& aFiles, const OString& rProject,
    const OString& rPotDir )
{
    ///Handle files in lexical order
    std::sort(aFiles.begin(), aFiles.end());

    typedef std::vector<OUString>::const_iterator citer_t;

    bool bFirstLocFile = true; ///< First file in directory which needs localization

    for( citer_t aIt = aFiles.begin(); aIt != aFiles.end(); ++aIt )
    {
        if (handleFile( rProject, *aIt, rPotDir, bFirstLocFile))
        {
            bFirstLocFile = false;
        }
    }

    if( !bFirstLocFile )
    {
        //Delete pot file if it contain only the header
        OString sPotFile = rPotDir.concat(".pot");
        PoIfstream aPOStream( sPotFile );
        PoEntry aPO;
        aPOStream.readEntry( aPO );
        bool bDel = aPOStream.eof();
        aPOStream.close();
        if( bDel )
        {
            if ( system(OString("rm " + sPotFile).getStr()) != 0 )
            {
                cerr
                    << "Error: Cannot remove entryless pot file: "
                    << sPotFile.getStr() << "\n";
                    throw false; //TODO
            }
        }
    }
}

bool includeProject(const OString& rProject) {
    static const OString projects[] = {
        "accessibility",
        "android",
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
        "extras",
        "filter",
        "forms",
        "formula",
        "fpicker",
        "framework",
        "helpcontent2",
        "instsetoo_native",
        "librelogo",
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
        "tubes",
        "uui",
        "vcl",
        "wizards",
        "xmlsecurity" };
    for (size_t i = 0; i != SAL_N_ELEMENTS(projects); ++i) {
        if (rProject == projects[i]) {
            return true;
        }
    }
    return false;
}

/// Handle one directory in the hierarchy.
///
/// Ignores symlinks and instead explicitly descends into clone/* or src/*,
/// as the Cygwin symlinks are not supported by osl::Directory on Windows.
///
/// @param rUrl the absolute file URL of this directory
///
/// @param nLevel 0 if this is either the root directory that contains the
/// projects or one of the clone/* or src/* directories that contain the
/// additional projects; -1 if this is the clone directory; 1 if this
/// is a project directory; 2 if this is a directory inside a project
///
/// @param rProject the name of the project (empty and ignored if nLevel <= 0)
/// @param rPotDir the path of pot directory
void handleDirectory(
    const OUString& rUrl, int nLevel,
    const OString& rProject, const OString& rPotDir)
{
    osl::Directory dir(rUrl);
    if (dir.open() != osl::FileBase::E_None) {
        cerr
            << "Error: Cannot open directory: " << rUrl << '\n';
        throw false; //TODO
    }
    std::vector<OUString> aFileNames;
    for (;;) {
        osl::DirectoryItem item;
        osl::FileBase::RC e = dir.getNextItem(item);
        if (e == osl::FileBase::E_NOENT) {
            break;
        }
        if (e != osl::FileBase::E_None) {
            cerr << "Error: Cannot read directory\n";
            throw false; //TODO
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName
            | osl_FileStatus_Mask_FileURL);
        if (item.getFileStatus(stat) != osl::FileBase::E_None) {
            cerr << "Error: Cannot get file status\n";
            throw false; //TODO
        }
        const OString sDirName =
            OUStringToOString(stat.getFileName(),RTL_TEXTENCODING_UTF8);
        switch (nLevel) {
        case -1: // the clone or src directory
            if (stat.getFileType() == osl::FileStatus::Directory) {
                handleDirectory(
                    stat.getFileURL(), 0, OString(), rPotDir);
            }
            break;
        case 0: // a root directory
            if (stat.getFileType() == osl::FileStatus::Directory) {
                if (includeProject(sDirName)) {
                    handleDirectory(
                        stat.getFileURL(), 1, sDirName, rPotDir.concat("/").concat(sDirName));
                } else if ( sDirName == "clone" ||
                            sDirName == "src" )
                {
                    handleDirectory( stat.getFileURL(), -1, OString(), rPotDir);
                }
            }
            break;
        default:
            if (stat.getFileType() == osl::FileStatus::Directory)
            {
                handleDirectory(
                    stat.getFileURL(), 2, rProject, rPotDir.concat("/").concat(sDirName));
            }
            else
            {
                aFileNames.push_back(stat.getFileURL());
            }
            break;
        }
    }

    if( !aFileNames.empty() )
    {
        handleFilesOfDir( aFileNames, rProject, rPotDir );
    }

    if (dir.close() != osl::FileBase::E_None) {
        cerr << "Error: Cannot close directory\n";
        throw false; //TODO
    }

    //Remove empty pot directory
    OUString sPoPath =
        OStringToOUString(
            rPotDir.copy(0,rPotDir.lastIndexOf('/')), RTL_TEXTENCODING_UTF8);
    OUString sPoUrl;
    if (osl::FileBase::getFileURLFromSystemPath(sPoPath, sPoUrl)
        != osl::FileBase::E_None)
    {
        cerr << "Error: Cannot convert pathname to URL in " << __FILE__ << ", in line " << __LINE__ << "\n"
             << OUStringToOString(sPoPath, RTL_TEXTENCODING_UTF8).getStr() << "\n";
        throw false; //TODO
    }
    osl::Directory::remove(sPoUrl);
}

void handleProjects(char * sSourceRoot, char const * sDestRoot)
{
    OUString root16;
    if (!rtl_convertStringToUString(
            &root16.pData, sSourceRoot, rtl_str_getLength(sSourceRoot),
            osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        cerr << "Error: Cannot convert pathname to UTF-16\n";
        throw false; //TODO
    }
    OUString rootUrl;
    if (osl::FileBase::getFileURLFromSystemPath(root16, rootUrl)
        != osl::FileBase::E_None)
    {
        cerr << "Error: Cannot convert pathname to URL in " << __FILE__ << ", in line " << __LINE__ << "\n"
             << "       root16: " << OUStringToOString(root16, RTL_TEXTENCODING_ASCII_US).getStr() << "\n";
        throw false; //TODO
    }
    handleDirectory(rootUrl, 0, OString(), OString(sDestRoot));
}
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    if (argc != 3) {
        cerr
            << ("localize (c)2001 by Sun Microsystems\n\n"
                "As part of the L10N framework, localize extracts en-US\n"
                "strings for translation out of the toplevel modules defined\n"
                "in projects array in l10ntools/source/localize.cxx.\n\n"
                "Syntax: localize <source-root> <outfile>\n");
        exit(EXIT_FAILURE);
    }
    try {
        handleProjects(argv[1],argv[2]);
    } catch (bool) { //TODO
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
