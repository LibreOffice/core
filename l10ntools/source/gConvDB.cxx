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
#include <string>
#include <vector>

#include "gL10nMem.hxx"
#include "gConvDB.hxx"



convert_db::convert_db(l10nMem& crMemory) : convert_gen(crMemory) {}
convert_db::~convert_db()                                              {}



void convert_db::execute()
{
    std::string newKey;
    int         i;


    msSourceBuffer   += '\n';
    miSize            = msSourceBuffer.size() -1;
    miLineNo          = 0;

    while (collectLine()) {
        newKey = msFields[4];
        if (msFields[5].size())
            newKey += "." + msFields[5];
        if (msFields[3].size())
            newKey += "." + msFields[3];

        for (; (i = msFields[1].find('\\')) != (int)std::string::npos;)
            msFields[1][i] = '/';

        // handle en-US or lang
        mcMemory.loadEntryKey(miLineNo, msFields[1], newKey, msFields[10], msFields[10], false);
    }
}



bool convert_db::collectLine()
{
    int  i, iStart;
    bool bLineEnd;

    ++miLineNo;

    for (i = 0; i < NUMFIELD; ++i)
        msFields[i].clear();

    if (miSourceReadIndex >= miSize)
        return false;

    for (i = 0, bLineEnd = false, iStart = miSourceReadIndex; !bLineEnd; ++miSourceReadIndex) {
        if (msSourceBuffer[miSourceReadIndex] == '\r' ||
            msSourceBuffer[miSourceReadIndex] == '\n' ||
            miSourceReadIndex == miSize)
            bLineEnd = true;
        if (msSourceBuffer[miSourceReadIndex] == '\t' || bLineEnd) {
            if (i >= NUMFIELD) {
                l10nMem::showError("TOO many fields", miLineNo);
            }
            msFields[i++] = msSourceBuffer.substr(iStart, miSourceReadIndex - iStart);
            iStart       = miSourceReadIndex +1;
        }
    }
    return true;
}
