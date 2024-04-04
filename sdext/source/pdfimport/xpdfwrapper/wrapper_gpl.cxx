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

#include "pdfioutdev_gpl.hxx"
#ifdef _WIN32
# include <io.h>
# include <fcntl.h>  /*_O_BINARY*/
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#ifndef SYSTEM_POPPLER
#include <string>         // std::string
#include <cstddef>        // std::size_t
#include <config_folders.h> //LIBO_SHARE_FOLDER
#endif

FILE* g_binary_out=stderr;

#ifdef _WIN32

// Use Unicode API

static const wchar_t *ownerPassword = nullptr;
static const wchar_t *userPassword  = nullptr;
static const wchar_t *outputFile    = nullptr;
static const wchar_t *options       = L"";

#define TO_STRING_VIEW(s) std::wstring_view(L##s)
using my_string = std::wstring;

// Poppler expects UTF-8 strings on Windows - see its openFile in poppler/goo/gfile.cc.
static std::string myStringToStdString(std::wstring_view s)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, s.data(), s.size(), nullptr, 0, nullptr, nullptr);
    char* buff = static_cast<char*>(_alloca(len * sizeof(char)));
    len = WideCharToMultiByte(CP_UTF8, 0, s.data(), s.size(), buff, len, nullptr, nullptr);
    return std::string(buff, len);
}

#else // ! _WIN32

static const char *ownerPassword = nullptr;
static const char *userPassword  = nullptr;
static const char *outputFile    = nullptr;
static const char *options       = "";

#define TO_STRING_VIEW(s) std::string_view(s)
using my_string = std::string;

static std::string myStringToStdString(std::string&& s) { return std::move(s); }

#endif

#ifdef _WIN32
int wmain(int argc, wchar_t **argv)
#else
int main(int argc, char **argv)
#endif
{
    int k = 1;
    while (k < argc)
    {
        if (argv[k] == TO_STRING_VIEW("-f"))
        {
            outputFile = argv[k+1];
            argc -= 2;
            for (int j = k; j < argc; ++j)
                argv[j] = argv[j+2];
        }
        else if (argv[k] == TO_STRING_VIEW("-o"))
        {
            options = argv[k+1];
            argc -= 2;
            for (int j = k; j < argc; ++j)
                argv[j] = argv[j+2];
        }

        else if (argv[k] == TO_STRING_VIEW("-opw"))
        {
            ownerPassword = argv[k+1];
            argc -= 2;
            for (int j = k; j < argc; ++j)
                argv[j] = argv[j+2];
        }
        else if (argv[k] == TO_STRING_VIEW("-upw"))
        {
            userPassword = argv[k+1];
            argc -= 2;
            for (int j = k; j < argc; ++j)
                argv[j] = argv[j+2];
        }
        ++k;
    }

    /* Get data directory location */
#ifdef SYSTEM_POPPLER
    const char* datadir = nullptr;
#else
    /* Creates an absolute path to the poppler_data directory, by taking the path
     * to the xpdfimport executable (provided in argv[0], and concatenating a
     * relative path to the poppler_data directory from the program directory. */
    const my_string execPath = argv[0];
    const std::size_t filenameStartPos = execPath.find_last_of(TO_STRING_VIEW("/\\")) + 1;
    const my_string programPath = execPath.substr(0, filenameStartPos);
    const std::string popplerDataPath = myStringToStdString(programPath + my_string(TO_STRING_VIEW("../" LIBO_SHARE_FOLDER "/xpdfimport/poppler_data")));
    const char* datadir = popplerDataPath.c_str();
#endif

    // read config file
#if POPPLER_CHECK_VERSION(0, 83, 0)
    globalParams = std::make_unique<GlobalParams>(datadir);
#else
    globalParams = new GlobalParams(datadir);
#endif
    globalParams->setErrQuiet(true);
#if defined(_MSC_VER)
    globalParams->setupBaseFonts(nullptr);
#endif

    // try to read a possible open password from stdin
    char aPwBuf[129];
    aPwBuf[128] = 0;
    if( ! fgets( aPwBuf, sizeof(aPwBuf)-1, stdin ) )
        aPwBuf[0] = 0; // mark as empty
    else
    {
        for( size_t i = 0; i < sizeof(aPwBuf); i++ )
        {
            if( aPwBuf[i] == '\n' )
            {
                aPwBuf[i] = 0;
                break;
            }
        }
    }

    // PDFDoc takes over ownership for all strings below
    GooString* pFileName = new GooString(myStringToStdString(argv[1]));

    // check for password string(s)
    GooString* pOwnerPasswordStr( aPwBuf[0] != 0
                                 ? new GooString( aPwBuf )
                                 : (ownerPassword
                                    ? new GooString(myStringToStdString(ownerPassword))
                                    : nullptr ) );
    GooString* pUserPasswordStr( aPwBuf[0] != 0
                                ? new GooString( aPwBuf )
                                : (userPassword
                                  ? new GooString(myStringToStdString(userPassword))
                                  : nullptr ) );
    if (outputFile)
#if defined _WIN32
        g_binary_out = _wfopen(outputFile, L"wb");
#else
        g_binary_out = fopen(outputFile,"wb");
#endif

#ifdef _WIN32
    // Win actually modifies output for O_TEXT file mode, so need to
    // revert to binary here
    _setmode( _fileno( g_binary_out ), _O_BINARY );
#endif

#if POPPLER_CHECK_VERSION(22, 6, 0)
    PDFDoc aDoc( std::make_unique<GooString>(pFileName),
                 std::optional<GooString>(pOwnerPasswordStr),
                 std::optional<GooString>(pUserPasswordStr) );
#else
    PDFDoc aDoc( pFileName,
                 pOwnerPasswordStr,
                 pUserPasswordStr );
#endif

    if (!aDoc.isOk())
        return aDoc.getErrorCode();

    pdfi::PDFOutDev aOutDev(&aDoc);
    if (options == TO_STRING_VIEW("SkipImages")) {
            aOutDev.setSkipImages(true);
    }

    // tell the receiver early - needed for proper progress calculation
    const int nPages = aDoc.getNumPages();
    pdfi::PDFOutDev::setPageNum(nPages);

    // virtual resolution of the PDF OutputDev in dpi
    static const int PDFI_OUTDEV_RESOLUTION = 7200;

    // do the conversion
    for (int i = 1; i <= nPages; ++i)
    {
        aDoc.displayPage(&aOutDev,
                i,
                PDFI_OUTDEV_RESOLUTION,
                PDFI_OUTDEV_RESOLUTION,
                0, true, true, true);
        aDoc.processLinks(&aOutDev, i);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
