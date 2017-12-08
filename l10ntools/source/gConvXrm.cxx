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
#include "gConvXrm.hxx"



convert_xrm::convert_xrm(l10nMem& crMemory)
                        : convert_gen(crMemory),
                          mbNoCollectingData(true),
                          mbIsTag(false),
                          mbIsLang(false)
{
}



void convert_xrm::doExecute()
{
    xrmlex();

    // write last part of file.
    if (mbMergeMode)
        writeSourceFile(msCollector);
}



void convert_xrm::setId(char *yytext)
{
    string& sText = copySource(yytext, mbNoCollectingData);

    if (mbIsTag) {
        int nL = sText.find("\"");
        int nE = sText.find("\"", nL+1);
        if (nL == (int)string::npos || nE == (int)string::npos)
            return;

        msKey = sText.substr(nL+1, nE - nL -1);
    }
}



void convert_xrm::setLang(char *yytext)
{
    string& sText = copySource(yytext, mbNoCollectingData);

    if (mbIsTag) {
        int nL = sText.find("\"");
        int nE = sText.find("\"", nL+1);
        if (nL == (int)string::npos || nE == (int)string::npos)
            return;

        string sLang = sText.substr(nL+1, nE - nL -1);
        if (sLang == "en-US")
            mbIsLang = true;
        else
            l10nMem::showError(sLang + " is no en-US language");
    }
}



void convert_xrm::setTag(char *yytext)
{
    msTag = copySource(yytext);

    msKey.clear();
    mbIsLang = false;
    mbIsTag  = true;
}



void convert_xrm::startCollectData(char *yytext)
{
    copySource(yytext, mbNoCollectingData);

    if (mbIsTag && mbIsLang && msKey.size())
        mbNoCollectingData = false;

    mbIsTag = mbIsLang = false;
}



void convert_xrm::stopCollectData(char *yytext)
{
    string sTagText, sTagEnd, sLang, sText = msCollector;

    copySource(yytext);
    if (!mbNoCollectingData) {
        mcMemory.setSourceKey(miLineNo, msSourceFile, msKey, sText, "", "readmeitem", "", mbMergeMode);
        mbNoCollectingData = true;
        if (mbMergeMode) {
            sTagEnd  = "</" + msTag.substr(1,msTag.size()-2) + ">\n";
            msTag   += "id=\"" + msKey + "\" xml:lang=\"";

            // prepare to read all languages
            mcMemory.prepareMerge();
            for (; mcMemory.getMergeLang(sLang, sText);) {
                // replace \" with "
                for (int i = 0; (i = sText.find("\\\"", i)) != (int)string::npos;)
                    sText.erase(i,1);

                // Prepare tag start and end
                sTagText = msTag + sLang + "\">" + sText + sTagEnd;
                writeSourceFile(sTagText);
            }
        }
        msKey.clear();
    }
    mbIsTag = false;
}
