/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 3.
 *
 *
 *    GNU Lesser General Public License Version 3
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/



#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#undef WINVER
#define WINVER 0x0500

#include <windows.h>
#include <msiquery.h>
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

    string GetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        string  result;
        TCHAR   szDummy[1] = TEXT("");
        DWORD   nChars = 0;

        if (MsiGetProperty(handle, sProperty.c_str(), szDummy, &nChars) == ERROR_MORE_DATA)
        {
            DWORD nBytes = ++nChars * sizeof(TCHAR);
            LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
            ZeroMemory( buffer, nBytes );
            MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
            result = buffer;
        }
        return result;
    }

    inline bool IsSetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        return (GetMsiProperty(handle, sProperty).length() > 0);
    }

    inline void UnsetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        MsiSetProperty(handle, sProperty.c_str(), NULL);
    }

    inline void SetMsiProperty(MSIHANDLE handle, const string& sProperty, const string&)
    {
        MsiSetProperty(handle, sProperty.c_str(), TEXT("1"));
    }

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

    string sOfficeInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));
    createIndexesForThesaurusFiles(sOfficeInstallPath);
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
