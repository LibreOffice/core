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

#include <sal/config.h>

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include <osl/file.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/string.h>
#include <rtl/string.hxx>
#include <rtl/textcvt.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/macros.h>
#include <sal/main.h>
#include <sal/types.h>

#include <po.hxx>

using namespace std;

namespace {

bool matchList(
    const OUString& rUrl, const OUStringLiteral* pList, size_t nLength)
{
    for (size_t i = 0; i != nLength; ++i) {
        if (rUrl.endsWith(pList[i])) {
            return true;
        }
    }
    return false;
}

bool passesNegativeList(const OUString& rUrl) {
    static const OUStringLiteral list[] = {
        "/desktop/test/deployment/passive/help/en/help.tree",
        "/desktop/test/deployment/passive/help/en/main.xhp",
        "/dictionaries.xcu",
        "/dictionaries/da_DK/help/da/help.tree",
        ("/dictionaries/da_DK/help/da/"
         "org.openoffice.da.hunspell.dictionaries/page1.xhp"),
        ("/dictionaries/da_DK/help/da/"
         "org.openoffice.da.hunspell.dictionaries/page2.xhp"),
        "/dictionaries/hu_HU/help/hu/help.tree",
        ("/dictionaries/hu_HU/help/hu/"
         "org.openoffice.hu.hunspell.dictionaries/page1.xhp"),
        "/officecfg/registry/data/org/openoffice/Office/Accelerators.xcu"
    };
    return !matchList(rUrl, list, SAL_N_ELEMENTS(list));
}

bool passesPositiveList(const OUString& rUrl) {
    static const OUStringLiteral list[] = {
        "/description.xml"
    };
    return matchList(rUrl, list, SAL_N_ELEMENTS(list));
}

void handleCommand(
    const OString& rInPath, const OString& rOutPath,
    const std::string& rExecutable)
{
    OStringBuffer buf;
    if (rExecutable == "uiex" || rExecutable == "hrcex")
    {
        buf.append(OString(getenv("SRC_ROOT")));
        buf.append("/solenv/bin/");
    }
    else
    {
        buf.append(OString(getenv("WORKDIR_FOR_BUILD")));
        buf.append("/LinkTarget/Executable/");
    }
    buf.append(rExecutable.data());
    buf.append(" -i ");
    buf.append(rInPath);
    buf.append(" -o ");
    buf.append(rOutPath);

    const OString cmd = buf.makeStringAndClear();
    if (system(cmd.getStr()) != 0)
    {
        cerr << "Error: Failed to execute " << cmd << '\n';
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
            cerr
                << ("Error: Cannot convert pathname to URL in " __FILE__
                    ", in line ")
                << __LINE__ << "\n       outDir: "
                << outDir
                << "\n";
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
            << rOutPath << "\n";
        throw false; //TODO
    }

    const sal_Int32 nProjectInd = rInPath.indexOf(rProject);
    const OString relativPath =
        rInPath.copy(nProjectInd, rInPath.lastIndexOf('/')- nProjectInd);

    PoHeader aTmp(relativPath);
    aPoOutPut.writeHeader(aTmp);
    aPoOutPut.close();
}

bool fileExists(const OString& fileName)
{
    FILE *f = fopen(fileName.getStr(), "r");

    if (f != nullptr)
    {
        fclose(f);
        return true;
    }

    return false;
}

OString gDestRoot;

bool handleFile(const OString& rProject, const OUString& rUrl, const OString& rPotDir)
{
    struct Command {
        OUStringLiteral extension;
        std::string executable;
        bool positive;
    };
    static Command const commands[] = {
        { OUStringLiteral(".hrc"), "hrcex", false },
        { OUStringLiteral(".ulf"), "ulfex", false },
        { OUStringLiteral(".xcu"), "cfgex", false },
        { OUStringLiteral(".xrm"), "xrmex", false },
        { OUStringLiteral("description.xml"), "xrmex", true },
        { OUStringLiteral(".xhp"), "helpex", false },
        { OUStringLiteral(".properties"), "propex", false },
        { OUStringLiteral(".ui"), "uiex", false },
        { OUStringLiteral(".tree"), "treex", false } };
    for (size_t i = 0; i != SAL_N_ELEMENTS(commands); ++i)
    {
        if (rUrl.endsWith(commands[i].extension) &&
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
                OString sOutPath;
                if (commands[i].executable == "uiex" || commands[i].executable == "hrcex")
                    sOutPath = gDestRoot + "/" + rProject + "/messages.pot";
                else
                    sOutPath = rPotDir.concat(".pot");

                if (!fileExists(sOutPath))
                    InitPoFile(rProject, sInPath, rPotDir, sOutPath);
                handleCommand(sInPath, sOutPath, commands[i].executable);

                {
                    //Delete pot file if it contain only the header
                    PoIfstream aPOStream(sOutPath);
                    PoEntry aPO;
                    aPOStream.readEntry( aPO );
                    bool bDel = aPOStream.eof();
                    aPOStream.close();
                    if (bDel)
                    {
                        if ( system(OString("rm " + sOutPath).getStr()) != 0 )
                        {
                            cerr
                                << "Error: Cannot remove entryless pot file: "
                                << sOutPath << "\n";
                            throw false; //TODO
                        }
                    }
                }

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

    for (auto const& elem : aFiles)
        handleFile(rProject, elem, rPotDir);
}

bool includeProject(const OString& rProject) {
    static const char *projects[] = {
        "include",
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
        "writerperfect",
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
/// @param nLevel 0 if this is the root directory (core repository)
/// that contains the individual modules. 1 if it is a toplevel module and
/// larger values for the subdirectories.
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
    std::map<OUString, std::map<OString, OString>> aSubDirs;
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
        switch (nLevel)
        {
            case 0: // a root directory
                if (stat.getFileType() == osl::FileStatus::Directory && includeProject(sDirName))
                    aSubDirs[stat.getFileURL()][sDirName] = rPotDir.concat("/").concat(sDirName);
                break;
            default:
                if (stat.getFileType() == osl::FileStatus::Directory)
                    aSubDirs[stat.getFileURL()][rProject] = rPotDir.concat("/").concat(sDirName);
                else
                    aFileNames.push_back(stat.getFileURL());
                break;
        }
    }

    OString aPotDir(rPotDir);
    if( !aFileNames.empty() )
    {
        OString aProject(rProject);
        if (aProject == "include" && nLevel > 1)
        {
            aProject = aPotDir.copy(aPotDir.lastIndexOf('/') + 1);
            aPotDir = aPotDir.copy(0, aPotDir.lastIndexOf("include")) + aProject + "/messages";
        }
        if (aProject != "include")
        {
            handleFilesOfDir(aFileNames, aProject, aPotDir);
        }
    }

    if (dir.close() != osl::FileBase::E_None) {
        cerr << "Error: Cannot close directory\n";
        throw false; //TODO
    }

    for (auto const& elem : aSubDirs)
        handleDirectory(elem.first, nLevel + 1, elem.second.begin()->first,
                        elem.second.begin()->second);

    //Remove empty pot directory
    OUString sPoPath =
        OStringToOUString(
            aPotDir.copy(0,aPotDir.lastIndexOf('/')), RTL_TEXTENCODING_UTF8);
    OUString sPoUrl;
    if (osl::FileBase::getFileURLFromSystemPath(sPoPath, sPoUrl)
        != osl::FileBase::E_None)
    {
        cerr
            << ("Error: Cannot convert pathname to URL in " __FILE__
                ", in line ")
            << __LINE__ << "\n"
            << sPoPath
            << "\n";
        throw false; //TODO
    }
    osl::Directory::remove(sPoUrl);
}

void handleProjects(char const * sSourceRoot, char const * sDestRoot)
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
        cerr
            << ("Error: Cannot convert pathname to URL in " __FILE__
                ", in line ")
            << __LINE__ << "\n       root16: "
            << root16
            << "\n";
        throw false; //TODO
    }
    gDestRoot = OString(sDestRoot);
    handleDirectory(rootUrl, 0, OString(), gDestRoot);
}
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    try
    {
        if (argc != 3)
        {
            cerr
                << ("localize (c)2001 by Sun Microsystems\n\n"
                    "As part of the L10N framework, localize extracts en-US\n"
                    "strings for translation out of the toplevel modules defined\n"
                    "in projects array in l10ntools/source/localize.cxx.\n\n"
                    "Syntax: localize <source-root> <outfile>\n");
            exit(EXIT_FAILURE);
        }
        handleProjects(argv[1],argv[2]);
    }
    catch (std::exception& e)
    {
        cerr << "exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (bool) //TODO
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
