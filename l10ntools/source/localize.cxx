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
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "boost/noncopyable.hpp"
#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "osl/thread.h"
#include "rtl/oustringostreaminserter.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/macros.h"
#include "sal/main.h"
#include "sal/types.h"

#include "po.hxx"

using namespace std;

namespace {

rtl::OUString getEnvironment(rtl::OUString const & variable) {
    rtl::OUString value;
    if (osl_getEnvironment(variable.pData, &value.pData) != osl_Process_E_None)
    {
        std::cerr
            << "Error: cannot get environment variable " << variable << '\n';
        throw false; //TODO
    }
    return value;
}

class TempFile: private boost::noncopyable {
public:
    TempFile() {
        if (osl::FileBase::createTempFile(0, 0, &url_) != osl::FileBase::E_None)
        {
            std::cerr << "osl::FileBase::createTempFile() failed\n";
            throw false; //TODO
        }
    }

    ~TempFile() {
        if (osl::File::remove(url_) != osl::FileBase::E_None) {
            std::cerr << "Warning: failure removing temporary " << url_ << '\n';
        }
    }

    rtl::OUString getUrl() const { return url_; }

private:
    rtl::OUString url_;
};

struct AsciiString {
    char const * string;
    sal_Int32 length;
};

bool matchList(
    rtl::OUString const & url, AsciiString const * list, std::size_t length)
{
    for (std::size_t i = 0; i != length; ++i) {
        if (url.endsWithAsciiL(list[i].string, list[i].length)) {
            return true;
        }
    }
    return false;
}

bool passesNegativeList(rtl::OUString const & url) {
    static AsciiString const list[] = {
        { RTL_CONSTASCII_STRINGPARAM("/dictionaries.xcu") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dictionaries/da_DK/help/da/"
            "org.openoffice.da.hunspell.dictionaries/page1.xhp") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dictionaries/da_DK/help/da/"
            "org.openoffice.da.hunspell.dictionaries/page2.xhp") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/dictionaries/hu_HU/help/hu/"
            "org.openoffice.hu.hunspell.dictionaries/page1.xhp") },
        { RTL_CONSTASCII_STRINGPARAM("/hidother.src") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/officecfg/registry/data/org/openoffice/Office/"
            "Accelerators.xcu") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/officecfg/registry/data/org/openoffice/Office/Labels.xcu") },
        { RTL_CONSTASCII_STRINGPARAM(
            "/officecfg/registry/data/org/openoffice/Office/SFX.xcu") }
    };
    return !matchList(url, list, SAL_N_ELEMENTS(list));
}

bool passesPositiveList(rtl::OUString const & url) {
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
    rtl::OUString const & project, rtl::OUString const & projectRoot,
    rtl::OUString const & url, rtl::OUString const & actualDir,
    std::ofstream & outPut, rtl::OUString const & executable, bool positive)
{
    if (positive ? passesPositiveList(url) : passesNegativeList(url)) {
        rtl::OUString inPath;
        if (osl::FileBase::getSystemPathFromFileURL(url, inPath) !=
            osl::FileBase::E_None)
        {
            std::cerr
                << "osl::FileBase::getSystemPathFromFileURL(" << url
                << ") failed\n";
            throw false; //TODO
        }
        TempFile temp;
        rtl::OUString outPath;
        if (osl::FileBase::getSystemPathFromFileURL(temp.getUrl(), outPath)
            != osl::FileBase::E_None)
        {
            std::cerr
                << "osl::FileBase::getSystemPathFromFileURL(" << temp.getUrl()
                << ") failed\n";
            throw false; //TODO
        }
        rtl::OUStringBuffer buf(
            getEnvironment(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SOLARVER"))));
        buf.append('/');
        buf.append(
            getEnvironment(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("INPATH_FOR_BUILD"))));
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("/bin/"));
        buf.append(executable);
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" -e -p "));
        buf.append(project);
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" -r "));
        buf.append(projectRoot);
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" -i "));
        buf.append(inPath);
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" -o "));
        buf.append(outPath);
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" -l en-US"));
        rtl::OString cmd;
        if (!buf.makeStringAndClear().convertToString(
                &cmd, osl_getThreadTextEncoding(),
                (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                 | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
        {
            std::cerr << "Error: Cannot convert command from UTF-16\n";
            throw false; //TODO
        }
        if (system(cmd.getStr()) != 0) {
            std::cerr << "Error: Failed to execute " << cmd.getStr() << '\n';
            throw false; //TODO
        }
        rtl::OString outPath8;
        if (!outPath.convertToString(
                &outPath8, osl_getThreadTextEncoding(),
                (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                 | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
        {
            std::cerr << "Error: Cannot convert pathname from UTF-16\n";
            throw false; //TODO
        }
        std::ifstream in(outPath8.getStr());
        if (!in.is_open()) {
            std::cerr << "Error: Cannot open " << outPath8.getStr() << "\n";
            throw false; //TODO
        }

        std::string s;
        std::getline(in, s);
        if (!in.eof() && !outPut.is_open())
        {
            rtl::OUString outDirUrl;
            if (osl::FileBase::getFileURLFromSystemPath(actualDir.
                copy(0,actualDir.lastIndexOf('/')), outDirUrl)
                != osl::FileBase::E_None)
            {
                std::cerr << "Error: Cannot convert pathname to URL\n";
                throw false; //TODO
            }
            osl::Directory::createPath(outDirUrl);

            rtl::OString outFilePath;
            if (!actualDir.concat(".pot").
                convertToString(
                &outFilePath, osl_getThreadTextEncoding(),
                (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
            {
                std::cerr << "Error: Cannot convert pathname from UTF-16\n";
                throw false; //TODO
            }
            outPut.open(outFilePath.getStr(),
                        std::ios_base::out | std::ios_base::trunc);
            rtl::OString relativPath;
            if (!inPath.copy(inPath.indexOf(project),
                inPath.lastIndexOf('/')-inPath.indexOf(project)).
                convertToString(&relativPath, osl_getThreadTextEncoding(),
                (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
            {
                std::cerr << "Error: Cannot convert pathname from UTF-16\n";
                throw false; //TODO
            }
            PoHeader(relativPath).writeToFile(outPut);
        }
        while (!in.eof())
        {
            OString sLine = OString(s.data(),s.length());

            if (!sLine.getToken(PoEntry::TEXT,'\t').isEmpty())
                PoEntry(sLine).writeToFile(outPut);
            if (!sLine.getToken(PoEntry::QUICKHELPTEXT,'\t').isEmpty())
                PoEntry(sLine,PoEntry::TQUICKHELPTEXT).writeToFile(outPut);
            if (!sLine.getToken(PoEntry::TITLE,'\t').isEmpty())
                PoEntry(sLine,PoEntry::TTITLE).writeToFile(outPut);
            std::getline(in, s);
        };
        in.close();
    }
}

void handleFile(
    rtl::OUString const & project, rtl::OUString const & projectRoot,
    rtl::OUString const & url, rtl::OUString const & actualDir,
    std::ofstream &  outPut)
{
    struct Command {
        char const * extension;
        sal_Int32 extensionLength;
        char const * executable;
        bool positive;
    };
    static Command const commands[] = {
        { RTL_CONSTASCII_STRINGPARAM(".src"), "transex3", false },
        { RTL_CONSTASCII_STRINGPARAM(".hrc"), "transex3", true },
        { RTL_CONSTASCII_STRINGPARAM(".ulf"), "ulfex", false },
        { RTL_CONSTASCII_STRINGPARAM(".xcu"), "cfgex", false },
        { RTL_CONSTASCII_STRINGPARAM(".xrm"), "xrmex", false },
        { RTL_CONSTASCII_STRINGPARAM(".xml"), "xrmex", true },
        { RTL_CONSTASCII_STRINGPARAM(".xhp"), "helpex", false },
        { RTL_CONSTASCII_STRINGPARAM(".properties"), "propex", false } };
    for (std::size_t i = 0; i != SAL_N_ELEMENTS(commands); ++i) {
        if (url.endsWithAsciiL(
                commands[i].extension, commands[i].extensionLength))
        {
            handleCommand(
                project, projectRoot, url, actualDir, outPut,
                rtl::OUString::createFromAscii(commands[i].executable),
                commands[i].positive);
            break;
        }
    }
}

bool includeProject(rtl::OUString const & project) {
    static char const * projects[] = {
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
        "xmlsecurity" };
    for (std::size_t i = 0; i != SAL_N_ELEMENTS(projects); ++i) {
        if (project.equalsAscii(projects[i])) {
            return true;
        }
    }
    return false;
}

bool excludeDirectory(rtl::OUString const & directory) {
    // Cf. OUTPATH=* in configure.in:
    static AsciiString const excluded[] = {
        { RTL_CONSTASCII_STRINGPARAM("callcatcher") },
        { RTL_CONSTASCII_STRINGPARAM("unxaig") },
        { RTL_CONSTASCII_STRINGPARAM("unxand") },
        { RTL_CONSTASCII_STRINGPARAM("unxdfly") },
        { RTL_CONSTASCII_STRINGPARAM("unxfbsd") },
        { RTL_CONSTASCII_STRINGPARAM("unxios") },
        { RTL_CONSTASCII_STRINGPARAM("unxkfg") },
        { RTL_CONSTASCII_STRINGPARAM("unxlng") },
        { RTL_CONSTASCII_STRINGPARAM("unxmac") },
        { RTL_CONSTASCII_STRINGPARAM("unxnbsd") },
        { RTL_CONSTASCII_STRINGPARAM("unxobsd") },
        { RTL_CONSTASCII_STRINGPARAM("unxsog") },
        { RTL_CONSTASCII_STRINGPARAM("unxsol") },
        { RTL_CONSTASCII_STRINGPARAM("unxubt") },
        { RTL_CONSTASCII_STRINGPARAM("wntmsc") } };
    for (std::size_t i = 0; i != SAL_N_ELEMENTS(excluded); ++i) {
        if (directory.matchAsciiL(excluded[i].string, excluded[i].length)) {
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
void handleDirectory(
    rtl::OUString const & url, int level, rtl::OUString const & project,
    rtl::OUString const & projectRoot, rtl::OUString const & actualDir)
{
    std::ofstream output;
    osl::Directory dir(url);
    if (dir.open() != osl::FileBase::E_None) {
        std::cerr
            << "Error: Cannot open directory: "
            << rtl::OUStringToOString(url, osl_getThreadTextEncoding()).getStr()
            << '\n';
        throw false; //TODO
    }
    for (;;) {
        osl::DirectoryItem item;
        osl::FileBase::RC e = dir.getNextItem(item);
        if (e == osl::FileBase::E_NOENT) {
            break;
        }
        if (e != osl::FileBase::E_None) {
            std::cerr << "Error: Cannot read directory\n";
            throw false; //TODO
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName
            | osl_FileStatus_Mask_FileURL);
        if (item.getFileStatus(stat) != osl::FileBase::E_None) {
            std::cerr << "Error: Cannot get file status\n";
            throw false; //TODO
        }
        switch (level) {
        case -1: // the clone or src directory
            if (stat.getFileType() == osl::FileStatus::Directory) {
                handleDirectory(
                    stat.getFileURL(), 0, rtl::OUString(),
                    rtl::OUString(), actualDir);
            }
            break;
        case 0: // a root directory
            if (stat.getFileType() == osl::FileStatus::Directory) {
                if (includeProject(stat.getFileName())) {
                    handleDirectory(
                        stat.getFileURL(), 1, stat.getFileName(),
                        rtl::OUString(), actualDir.concat("/").
                        concat(stat.getFileName()));
                } else if ( stat.getFileName() == "clone" ||
                            stat.getFileName() == "src" )
                {
                    handleDirectory(
                        stat.getFileURL(), -1, rtl::OUString(),
                        rtl::OUString(), actualDir);
                }
            }
            break;
        default:
            if (stat.getFileType() == osl::FileStatus::Directory) {
                if (level == 2 || !excludeDirectory(stat.getFileName())) {
                    rtl::OUString pr(projectRoot);
                    if (!pr.isEmpty()) {
                        pr += rtl::OUString('/');
                    }
                    pr += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".."));
                    handleDirectory(stat.getFileURL(), 2, project, pr,
                                    actualDir.concat("/").
                                    concat(stat.getFileName()));
                }
            } else {
                handleFile(project, projectRoot,
                           stat.getFileURL(), actualDir, output);
            }
            break;
        }
    }
    if (output.is_open())
        output.close();
    if (dir.close() != osl::FileBase::E_None) {
        std::cerr << "Error: Cannot close directory\n";
        throw false; //TODO
    }
}

void handleProjects(char const * sourceRoot, char const * destRoot) {
    rtl::OUString root16;
    if (!rtl_convertStringToUString(
            &root16.pData, sourceRoot, rtl_str_getLength(sourceRoot),
            osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        std::cerr << "Error: Cannot convert pathname to UTF-16\n";
        throw false; //TODO
    }
    rtl::OUString rootUrl;
    if (osl::FileBase::getFileURLFromSystemPath(root16, rootUrl)
        != osl::FileBase::E_None)
    {
        std::cerr << "Error: Cannot convert pathname to URL\n";
        throw false; //TODO
    }
    rtl::OUString outPutRoot;
    if (!rtl_convertStringToUString(
            &outPutRoot.pData, destRoot, rtl_str_getLength(destRoot),
            osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        std::cerr << "Error: Cannot convert pathname to UTF-16\n";
        throw false; //TODO
    }
    handleDirectory(rootUrl, 0, rtl::OUString(), rtl::OUString(), outPutRoot);
}
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    if (argc != 3) {
        std::cerr
            << ("localize (c)2001 by Sun Microsystems\n\n"
                "As part of the L10N framework, localize extracts en-US\n"
                "strings for translation out of the toplevel modules defined\n"
                "in projects array in l10ntools/source/localize.cxx.\n\n"
                "Syntax: localize <source-root> <outfile>\n");
        std::exit(EXIT_FAILURE);
    }
    try {
        handleProjects(argv[1],argv[2]);
    } catch (bool) { //TODO
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
