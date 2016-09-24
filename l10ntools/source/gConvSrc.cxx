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
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
using namespace std;

#include "gL10nMem.hxx"
#include "gConvSrc.hxx"


convert_src::convert_src(l10nMem& crMemory)
                        : convert_gen(crMemory),
                          miLevel(0),
                          mbMacroActive(false)
#if 0
                          mbExpectValue(false),
                          mbEnUs(false),
                          mbExpectName(false),
                          mbAutoPush(false),
                          mbValuePresent(false),
                          mbInList(false),
                          mbInListItem(false)
#endif
{
}



void convert_src::doExecute()
{
    srclex();
}



void convert_src::setValue(string& syyText)
{
    int stackSize = mcStack.size();
    string cleanValue = syyText.substr(1, syyText.size() - 1);
    {
        cout << "test value\n";
        for (int i = 0; i < stackSize; i++)
            cout << i << ")  " << mcStack[i] << "\n";
    }
    string subid = (stackSize > 3) ? mcStack[stackSize - 1] : "";
    string stringid = mcStack[stackSize - 2];
    l10nMem::keyToLower(stringid);

    mcMemory.setSourceKey(miLineNo, msSourceFile, mcStack[1], cleanValue, "", stringid, subid, false);
}



bool convert_src::setLang(string& syyText)
{
    if (syyText == "en-US")
        return true;
    else if (syyText != "x-comment")
        l10nMem::showError(syyText + " non legal language");
    return false;
}



void convert_src::setName(string& syyText, bool revertLevel)
{
    if (revertLevel)
        mcStack.pop_back();
    mcStack.push_back(syyText);
}



void convert_src::setCmd(string& syyText)
{
    mcStack.push_back(syyText);
}



void convert_src::startBlock()
{
    unsigned int cnt = 2 * ++miLevel;

    while (cnt > mcStack.size())
        mcStack.push_back("");
}



void convert_src::stopBlock()
{
    if (miLevel > 0)
        miLevel--;

    // check for correct node/prop relations
    if (mcStack.size()) {
        if (miLevel) {
            mcStack.pop_back();
            mcStack.pop_back();
        }
        else
            mcStack.clear();
    }
}



void convert_src::defMacro()
{
    if (!miLevel)
        miLevel++;
    mbMacroActive = true;
}



void convert_src::endMacro()
{
    if (mbMacroActive) {
        mbMacroActive = false;
        miLevel = 0;
        mcStack.clear();
    }
}


#if 0
void convert_src::setId(char *syyText, bool bId)
{
    copySource(syyText);
    if (bId || !mcStack.back().size())
        mbExpectName = mbAutoPush = true;
}
#endif



#if 0
void convert_src::setText(char *syyText)
{
    msTextName = copySource(syyText);
    mbExpectValue = true;
    mbEnUs = false;
    trim(msTextName);
}
#endif



#if 0
void convert_src::setList(char *syyText)
{
    msCmd = copySource(syyText);
    miListCount = 0;
    mbInList = true;
    trim(msCmd);
    l10nMem::keyToLower(msCmd);
}
#endif



#if 0
void convert_src::setNL(char *syyText, bool bMacro)
{
    int         nL;
    string sKey;

    copySource(syyText);

    if (msTextName.size() && mbValuePresent && mbEnUs) {
        // locate key and extract it
        buildKey(sKey);

        for (nL = -1;;) {
            nL = msValue.find("\\\"", nL + 1);
            if (nL == (int)string::npos)
                break;
            msValue.erase(nL, 1);
        }
        for (nL = -1;;) {
            nL = msValue.find("\\\\", nL + 1);
            if (nL == (int)string::npos)
                break;
            msValue.erase(nL, 1);
        }

        //FIX        sKey += "." + msCmd + "." + msTextName;
        if (msValue.size() && msValue != "-") {
            mcMemory.setSourceKey(miLineNo, msSourceFile, sKey, msValue, "", msCmd, msGroup, mbMergeMode);
            if (mbMergeMode)
                insertLanguagePart(sKey, msTextName);
        }
        msGroup.clear();
    }

    if (!bMacro && mbExpectMacro) {
        while ((int)mcStack.size() > miMacroLevel)
            mcStack.pop_back();
        mbEnUs =
            mbExpectMacro = false;
    }

    mbValuePresent =
        mbExpectName =
        mbAutoPush = false;
    msValue.clear();
    msTextName.clear();
}
#endif



#if 0
void convert_src::setListItem(char const *syyText, bool bIsStart)
{
    copySource(syyText);

    if (bIsStart) {
        if (!miListCount) {
            mcStack.pop_back();
            msName = "dummy";
            mcStack.push_back(msName);
        }
        msTextName    = "item";
        mbExpectValue =
        mbExpectName  =
        mbInListItem  = true;
        msName.clear();
    }
    else
    {
        if (mbInListItem) {
            stringstream ssBuf;
            string       myKey;

            ++miListCount;
            mcStack.pop_back();
            if (mbExpectName) {
                ssBuf  << miListCount;
                msName  = "item" + ssBuf.str();
            }
            mcStack.push_back(msName);
            mbInListItem =
            mbExpectName = false;

            // check key or add seq.
            buildKey(myKey);
        }
    }
}
#endif



#if 0
void convert_src::trim(string& sText)
{
    int nL;


    while (sText[0] == ' ' || sText[0] == '\t')
        sText.erase(0,1);
    for (nL = sText.size(); sText[nL-1] == ' ' || sText[nL-1] == '\t'; --nL)
        ;
    if (nL != (int)sText.size())
        sText.erase(nL);
}
#endif



#if 0
void convert_src::buildKey(string& sKey)
{
    int nL;


    sKey.clear();
    for (nL = 0; nL < (int)mcStack.size(); ++nL)
        if (mcStack[nL].size())
            sKey += (sKey.size() ? "." : "") + mcStack[nL];

    // FIX jan
    sKey = mcStack[0];
}
#endif



#if 0
void convert_src::insertLanguagePart(string& sKey, string& sTextType)
{
    string sLang, sText, sTagText;


    // just to please compiler
    sKey = sKey;

    // prepare to read all languages
    mcMemory.prepareMerge();
    for (; mcMemory.getMergeLang(sLang, sText);) {
        // Prepare tag start and end
        sTagText = sTextType + "[ " + sLang + " ] = \"" + sText + "\" ;" +
                   (mbExpectMacro ? "\\" : "") + "\n";
        writeSourceFile(sTagText);
    }
}
#endif
