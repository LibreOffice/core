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

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#undef WINVER
#define WINVER 0x0500

#include <windows.h>
#include <../tools/msiprop.hxx>
#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <systools/win32/uwinapi.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <stdlib.h>
#include <string.h>
using namespace std;
namespace
{

    static const int MAXLINE = 1024*64;


    void generateIndex(const char *inputFile, const char *outputFile)
    {

        ifstream in(inputFile);
        char inputBuffer[MAXLINE];
        map<string, size_t> entries;
        map<string,size_t>::iterator ret(entries.begin());

        int line(1);
        in.getline(inputBuffer, MAXLINE);
        const string encoding(inputBuffer);
        size_t currentOffset(encoding.size()+1);
        while (true)
        {
                // Extract the next word, but not the entry count
                in.getline(inputBuffer, MAXLINE, '|');

                if (in.eof()) break;

                string word(inputBuffer);
                ret = entries.insert(ret, pair<string, size_t>(word, currentOffset));
                currentOffset += word.size() + 1;
                // Next is the entry count
                in.getline(inputBuffer, MAXLINE);
                if (!in.good())
                {
                        return;

                }
                currentOffset += strlen(inputBuffer)+1;
                int entryCount(strtol(inputBuffer, NULL, 10));
                for (int i(0); i < entryCount; ++i)
                {
                        in.getline(inputBuffer, MAXLINE);
                        currentOffset += strlen(inputBuffer)+1;
                        ++line;
                }
        }

        // Use binary mode to prevent any translation of LF to CRLF on Windows
        ofstream outputStream(outputFile, ios_base::binary| ios_base::trunc|ios_base::out);
        if (!outputStream.is_open())
        {
                //cerr << "Unable to open output file " << outputFile << endl;
                return;
        }

        cerr << "Doing it now" << outputFile << endl;
        outputStream << encoding << '\n' << entries.size() << '\n';

        for (map<string, size_t>::const_iterator ii(entries.begin());
                ii != entries.end();
                ++ii
        )
        {
                outputStream << ii->first << '|' << ii->second << '\n';
        }
        outputStream.close();
    }

    void generateIndex(const string &datFile)
    {
        string idxFile(datFile.substr(0, datFile.size()-3)+"idx");
        generateIndex(datFile.c_str(), idxFile.c_str());
    }
    void createIndexesForThesaurusFiles(const string & sOfficeInstallPath)
    {
        vector<string> thesaurusPaths;

        string sExtensionsPath = sOfficeInstallPath + "\\share\\extensions\\";

        string dictionariesWildcard = sExtensionsPath + "dict-*";

        struct _finddatai64_t dictFind;
        long h = _findfirsti64(dictionariesWildcard.c_str(),&dictFind);
        if (h > 0)
        {
            do
            {
                if (dictFind.attrib & _A_SUBDIR)
                {
                    struct _finddatai64_t datFind;
                    // Find any .dat files in the subdirectory
                    string dictPath = sExtensionsPath + dictFind.name + "\\";
                    string datWildCard = dictPath + "*.dat";
                    long h2 = _findfirsti64(datWildCard.c_str(), &datFind);
                    if (h2 > 0)
                    {
                        do
                        {
                            thesaurusPaths.push_back(dictPath + datFind.name);

                        } while (_findnexti64(h2,&datFind) == 0);
                    }
                }

            } while (_findnexti64(h,&dictFind) == 0);
        }
        for (vector<string>::const_iterator ii(thesaurusPaths.begin());
                ii != thesaurusPaths.end();
                ++ii
            )
        {
            generateIndex(*ii);
        }
    }

} // namespace


// Creates the thesaurus .idx files for all installed .dat
// thesaurus files
extern "C" UINT __stdcall CreateIndexes( MSIHANDLE handle )
{

    string sOfficeInstallPath = GetMsiPropValue(handle, TEXT("INSTALLLOCATION"));
    createIndexesForThesaurusFiles(sOfficeInstallPath);
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
