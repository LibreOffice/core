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
using namespace std;



#include "gL10nMem.hxx"
#include "gConvXcu.hxx"



convert_xcu::convert_xcu(l10nMem& crMemory)
                        : convert_gen(crMemory),
                          mbNoCollectingData(true),
                          miLevel(0),
                          mbNoTranslate(false)
{
}



void convert_xcu::doExecute()
{
    xculex();
}



void convert_xcu::pushKey(char *syyText)
{
    string sKey, sTag = copySource(syyText);

    // find key in tag
    int nL = sTag.find("oor:name=\"");
    if (nL != (int)string::npos) {
        // find end of key
        nL += 10;
        int nE  = sTag.find("\"", nL);
        if (nE != (int)string::npos)
            sKey = sTag.substr(nL, nE - nL);
    }
    mcStack.push_back(sKey);
}



void convert_xcu::popKey(char *syyText)
{
    copySource(syyText);

    // check for correct node/prop relations
    if (mcStack.size())
        mcStack.pop_back();

    mbNoTranslate = false;
}



void convert_xcu::startCollectData(char *syyText)
{
    int nL;
    string sTag = copySource(syyText);

    if (mbNoTranslate)
        return;

    // locate object name
    nL = sTag.find("xml:lang=\"");
    if (nL != (int)string::npos) {
        // test language
        nL += 10;
        if (sTag.substr(nL,5) == "en-US")
            mbNoCollectingData = false;
        else if (sTag.substr(nL,14) == "x-no-translate")
            mbNoTranslate = true;
        else {
            string sErr = sTag.substr(nL,5) + " is not en-US";
            l10nMem::showError(sErr);
        }
    }
}



void convert_xcu::stopCollectData(char *syyText)
{
    int    nL;
    string useKey, useText = msCollector;

    copySource(syyText);

    // time to do something ?
    if (mbNoCollectingData || mbNoTranslate)
        return;

    // remove any newline
    for (nL = 0;;) {
        nL = useText.find("\n");
        if (nL == (int)string::npos)
            break;
        useText.erase(nL,1);
    }

    mbNoCollectingData = true;

    if (useText.size()) {
        // locate key and extract it
        int max = (int)mcStack.size() - 1;
        for (nL = 0; nL < max; ++nL) {
            useKey += mcStack[nL];
            if (nL < max -1)
                useKey += ".";
        }
        mcMemory.setSourceKey(miLineNo, msSourceFile, useKey, useText, "", "value", mcStack[nL], mbMergeMode);
    }

    if (mbMergeMode) {
        string sLang, sText, sNewLine;

        // prepare to read all languages
        mcMemory.prepareMerge();
        for (; mcMemory.getMergeLang(sLang, sText);) {
            sNewLine = "\n<value xml:lang=\"" + sLang + "\">" + sText + "</value>";
            mcMemory.convertToInetString(sNewLine);
            writeSourceFile(sNewLine);
        }
    }
}



void convert_xcu::copySpecial(char *syyText)
{
    int         nx    = msCollector.size();
    string sText = copySource(syyText, mbNoCollectingData);

    if (!mbNoCollectingData) {
        msCollector.erase(nx);
        mcMemory.convertFromInetString(sText);
        msCollector += sText;
    }
}



void convert_xcu::copyNL(char *syyText)
{
    int         nX    = msCollector.size();
    string sText = copySource(syyText, mbNoCollectingData);

    if (!mbNoCollectingData) {
        msCollector.erase(nX);
        msCollector += ' ';
    }
}



void convert_xcu::addLevel()
{
    ++miLevel;
}
