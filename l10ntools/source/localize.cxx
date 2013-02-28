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
#include <fstream>
#include <iostream>
#include <string>

#include "boost/noncopyable.hpp"
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

class TempFile: private boost::noncopyable {
public:
    TempFile() {
        if (osl::FileBase::createTempFile(0, 0, &url_) != osl::FileBase::E_None)
        {
            cerr << "osl::FileBase::createTempFile() failed\n";
            throw false; //TODO
        }
    }

    ~TempFile() {
        if (osl::File::remove(url_) != osl::FileBase::E_None) {
            cerr << "Warning: failure removing temporary " << url_ << '\n';
        }
    }

    OUString getUrl() const { return url_; }

private:
    OUString url_;
};

struct AsciiString {
    char const * string;
    sal_Int32 length;
};

bool matchList(
    OUString const & url, AsciiString const * list, size_t length)
{
    for (size_t i = 0; i != length; ++i) {
        if (url.endsWithAsciiL(list[i].string, list[i].length)) {
            return true;
        }
    }
    return false;
}

bool passesNegativeList(OUString const & url) {
    static AsciiString const list[] = {
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
    return !matchList(url, list, SAL_N_ELEMENTS(list));
}

bool passesPositiveList(OUString const & url) {
    static AsciiString const list[] = {
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
    return matchList(url, list, SAL_N_ELEMENTS(list));
}

void handleCommand(
    OString const & project, OString const & projectRoot,
    OUString const & url, OString const & actualPotDir,
    PoOfstream & rPoOutPut, OString const & executable, bool positive)
{
    if (positive ? passesPositiveList(url) : passesNegativeList(url)) {

        //Get input file path
        OString inPath;
        {
            OUString inPathTmp;
            if (osl::FileBase::getSystemPathFromFileURL(url, inPathTmp) !=
                osl::FileBase::E_None)
            {
                cerr
                    << "osl::FileBase::getSystemPathFromFileURL(" << url
                    << ") failed\n";
                throw false; //TODO
            }
            inPath = OUStringToOString( inPathTmp, RTL_TEXTENCODING_UTF8 );
        }

        //Get output file path
        TempFile temp;
        OString outPath;
        {
            OUString outPathTmp;
            if (osl::FileBase::getSystemPathFromFileURL(temp.getUrl(),outPathTmp)
                != osl::FileBase::E_None)
            {
                cerr
                    << "osl::FileBase::getSystemPathFromFileURL("
                    << temp.getUrl() << ") failed\n";
                throw false; //TODO
            }
            outPath = OUStringToOString( outPathTmp, RTL_TEXTENCODING_UTF8 );
        }

        //Call the executable
        {
            OStringBuffer buf(OString(getenv("SOLARVER")));
            buf.append('/');
            buf.append(OString(getenv("INPATH_FOR_BUILD")));
            buf.append("/bin/");
            buf.append(executable);
            buf.append(" -p ");
            buf.append(project);
            buf.append(" -r ");
            buf.append(projectRoot);
            buf.append(" -i ");
            buf.append(inPath);
            buf.append(" -o ");
            buf.append(outPath);
            buf.append(" -l en-US");

            const OString cmd = buf.makeStringAndClear();
            if (system(cmd.getStr()) != 0) {
                cerr << "Error: Failed to execute " << cmd.getStr() << '\n';
                throw false; //TODO
            }
        }

        ifstream in(outPath.getStr());
        if (!in.is_open()) {
            cerr << "Error: Cannot open " << outPath.getStr() << "\n";
            throw false; //TODO
        }

        string s;
        getline(in, s);
        if (!in.eof() && !rPoOutPut.isOpen())
        {
            //Create directory for po file
            {
                OUString outDir =
                    OStringToOUString(
                        actualPotDir.copy(0,actualPotDir.lastIndexOf('/')),
                        RTL_TEXTENCODING_UTF8);
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

            //Open po file
            {
                OString outFilePath = actualPotDir.concat(".pot");
                rPoOutPut.open(outFilePath.getStr());
                if (!rPoOutPut.isOpen())
                {
                    cerr
                        << "Error: Cannot open po file "
                        << outFilePath.getStr() << "\n";
                    throw false; //TODO
                }
            }

            //Add header to po file
            {
                const sal_Int32 nProjectInd = inPath.indexOf(project);
                const OString relativPath =
                    inPath.copy(
                        nProjectInd, inPath.lastIndexOf('/')- nProjectInd);

                PoHeader aTmp(relativPath);
                rPoOutPut.writeHeader(aTmp);
            }
        }
        while (!in.eof())
        {
            OString sLine = OString(s.data(),s.length());
            try
            {
                if (!sLine.getToken(PoEntry::TEXT,'\t').isEmpty())
                    rPoOutPut.writeEntry(PoEntry(sLine));
                if (!sLine.getToken(PoEntry::QUICKHELPTEXT,'\t').isEmpty())
                    rPoOutPut.writeEntry(PoEntry(sLine,PoEntry::TQUICKHELPTEXT));
                if (!sLine.getToken(PoEntry::TITLE,'\t').isEmpty())
                    rPoOutPut.writeEntry(PoEntry(sLine,PoEntry::TTITLE));
            }
            catch(PoEntry::Exception& aException)
            {
                if(aException == PoEntry::INVALIDSDFLINE)
                {
                    cerr
                        << executable.getStr()
                        << "'s output is invalid:\n"
                        << sLine.replaceAll("\t","\\t").getStr()
                        << endl;
                    throw false; //TODO
                }
            }
            getline(in, s);
        };
        in.close();
    }
}

void handleFile(
    OString const & project, OString const & projectRoot,
    OUString const & url, OString const & actualPotDir,
    PoOfstream & rPoOutPut)
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
        if (url.endsWithAsciiL(
                commands[i].extension, commands[i].extensionLength) &&
            (commands[i].executable != "propex" || url.indexOf("en_US") != -1)
)
        {
            handleCommand(
                project, projectRoot, url, actualPotDir, rPoOutPut,
                commands[i].executable, commands[i].positive);
            break;
        }
    }
}

bool includeProject(OString const & project) {
    static OString projects[] = {
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
        if (project == projects[i]) {
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
/// @param url the absolute file URL of this directory
///
/// @param level 0 if this is either the root directory that contains the
/// projects or one of the clone/* or src/* directories that contain the
/// additional projects; -1 if this is the clone directory; 1 if this
/// is a project directory; 2 if this is a directory inside a project
///
/// @param project the name of the project (empty and ignored if level <= 0)
///
/// @param the relative path back to the project root (empty and ignored if
/// level <= 0)
/// @param actualPotDir the path of pot directory
void handleDirectory(
    OUString const & url, int level, OString const & project,
    OString const & projectRoot, OString const & actualPotDir)
{
    PoOfstream aPoOutPut;
    osl::Directory dir(url);
    if (dir.open() != osl::FileBase::E_None) {
        cerr
            << "Error: Cannot open directory: " << url << '\n';
        throw false; //TODO
    }
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
        const OString sFileName =
            OUStringToOString(stat.getFileName(),RTL_TEXTENCODING_UTF8);
        switch (level) {
        case -1: // the clone or src directory
            if (stat.getFileType() == osl::FileStatus::Directory) {
                handleDirectory(
                    stat.getFileURL(), 0, OString(),
                    OString(), actualPotDir);
            }
            break;
        case 0: // a root directory
            if (stat.getFileType() == osl::FileStatus::Directory) {
                if (includeProject(sFileName)) {
                    handleDirectory(
                        stat.getFileURL(), 1, sFileName,
                        OString(), actualPotDir.concat("/").
                        concat(sFileName));
                } else if ( sFileName == "clone" ||
                            sFileName == "src" )
                {
                    handleDirectory(
                        stat.getFileURL(), -1, OString(),
                        OString(), actualPotDir);
                }
            }
            break;
        default:
            if (stat.getFileType() == osl::FileStatus::Directory) {
                if (level == 2) {
                    OString pr(projectRoot);
                    if (!pr.isEmpty()) {
                        pr += OString('/');
                    }
                    pr += OString("..");
                    handleDirectory(stat.getFileURL(), 2, project, pr,
                                    actualPotDir.concat("/").concat(sFileName));
                }
            } else {
                handleFile(project, projectRoot,
                           stat.getFileURL(), actualPotDir, aPoOutPut);
            }
            break;
        }
    }
    if (aPoOutPut.isOpen())
        aPoOutPut.close();
    if (dir.close() != osl::FileBase::E_None) {
        cerr << "Error: Cannot close directory\n";
        throw false; //TODO
    }
}

void handleProjects(char * sourceRoot, char const * destRoot)
{
    OUString root16;
    if (!rtl_convertStringToUString(
            &root16.pData, sourceRoot, rtl_str_getLength(sourceRoot),
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
    handleDirectory(rootUrl, 0, OString(), OString(), OString(destRoot));
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
