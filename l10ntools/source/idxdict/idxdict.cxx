/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Steven Butler <sebutler@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <stdlib.h>
#include <string.h>

static const int MAXLINE = 1024*64;

using namespace std;

int main(int argc, char *argv[])
{
        if (argc != 3 || strcmp(argv[1],"-o"))
        {
                cout << "Usage: idxdict -o outputfile < input\n";
                ::exit(99);
        }
        // This call improves performance by approx 5x
        cin.sync_with_stdio(false);

        const char * outputFile(argv[2]);
        char inputBuffer[MAXLINE];
        multimap<string, size_t> entries;
        multimap<string,size_t>::iterator ret(entries.begin());

        int line(1);
        cin.getline(inputBuffer, MAXLINE);
        const string encoding(inputBuffer);
        size_t currentOffset(encoding.size()+1);
        while (true)
        {
                // Extract the next word, but not the entry count
                cin.getline(inputBuffer, MAXLINE, '|');

                if (cin.eof()) break;

                string word(inputBuffer);
                ret = entries.insert(ret, pair<string, size_t>(word, currentOffset));
                currentOffset += word.size() + 1;
                // Next is the entry count
                cin.getline(inputBuffer, MAXLINE);
                if (!cin.good())
                {
                        cerr << "Unable to read entry - insufficient buffer?.\n";
                        exit(99);
                }
                currentOffset += strlen(inputBuffer)+1;
                int entryCount(strtol(inputBuffer, NULL, 10));
                for (int i(0); i < entryCount; ++i)
                {
                        cin.getline(inputBuffer, MAXLINE);
                        currentOffset += strlen(inputBuffer)+1;
                        ++line;
                }
        }

        // Use binary mode to prevent any translation of LF to CRLF on Windows
        ofstream outputStream(outputFile, ios_base::binary| ios_base::trunc|ios_base::out);
        if (!outputStream.is_open())
        {
                cerr << "Unable to open output file " << outputFile << endl;
                ::exit(99);
        }

        outputStream << encoding << '\n' << entries.size() << '\n';

        for (multimap<string, size_t>::const_iterator ii(entries.begin());
                ii != entries.end();
                ++ii
        )
        {
                outputStream << ii->first << '|' << ii->second << '\n';
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
