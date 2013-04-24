/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
