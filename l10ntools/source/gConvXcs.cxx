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
#include "gConvXcs.hxx"



convert_xcs::convert_xcs(l10nMem& crMemory)
                        : convert_gen(crMemory),
                          mbCollectingData(false)
{
}



void convert_xcs::doExecute()
{
    if (mbMergeMode)
        throw l10nMem::showError("Merge not implemented");

    // currently no .xcs files generate en-US translation, so stop trying
    xcslex();
}



void convert_xcs::setKey(char *syyText)
{
    int    nL;
    string sHead, sText = copySource(syyText);

    // is it to be translated
    if (sText.find("oor:localized=") == string::npos)
        return;

    // locate key (is any)
    nL = sText.find("oor:name=\"");
    if (nL == (int)string::npos)
        return;
    sHead = sText.substr(nL+10);
    nL    = sHead.find("\"");
    msKey = sHead.substr(0,nL);
}



void convert_xcs::unsetKey(char *syyText)
{
    copySource(syyText);
}



void convert_xcs::startCollectData(char *syyText)
{
    copySource(syyText);
    if (!msKey.size())
        return;
}



void convert_xcs::stopCollectData(char *syyText)
{
    string sHead, sKey, sLang, sText, sCollectedText = copySource(syyText, false);
    int    nL;


    // get type of tag
    msCollector += sCollectedText;
    nL = msCollector.find("<p");
    if (nL != (int)string::npos)
        sHead = msCollector.substr(nL+1, 1);
    else {
        nL = msCollector.find("<h");
        sHead = msCollector.substr(nL+1, 2);
    }

    // locate key and extract it
    nL    = msCollector.find("id=") +4;
    sKey  = msCollector.substr(nL, msCollector.find("\"", nL+1) - nL);
    nL    = msCollector.find("xml:lang=\"") + 10;
    sLang = msCollector.substr(nL, msCollector.find("\"", nL+1) - nL);
    nL    = msCollector.find(">") +1;
    sText = msCollector.substr(nL, msCollector.find("\"", nL+1) - nL);
    msCollector.clear();

    if (mbMergeMode) {
#if 0
        // get all languages (includes en-US)
        vector<l10nMem_entry *>& cExtraLangauges = mcMemory.getLanguagesForKey(sKey);
        string                   sNewLine;
        nL = cExtraLangauges.size();

        for (int i = 0; i < nL; ++i) {
            sNewLine = "\n<" + sHead + " id=\"" + sKey + "\"" + " xml:lang=\"" +
                       cExtraLangauges[i]->msLanguage + "\">" +
                       cExtraLangauges[i]->msText +
                       "</" + sHead + ">";

            writeSourceFile(sNewLine);
        }
#endif
    }

    mcMemory.setSourceKey(miLineNo, msSourceFile, sKey, sText, "", "", "", mbMergeMode);
    mbCollectingData = false;
}
