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
#include "gConvXhp.hxx"



convert_xhp::convert_xhp(l10nMem& crMemory)
                        : convert_gen(crMemory),
                          meExpectValue(VALUE_NOT_USED),
                          msLangText(nullptr),
                          mcOutputFiles(nullptr),
                          miCntLanguages(0)
{
    // xhp files are written through a local routine
    mbLoadMode = true;
}



convert_xhp::~convert_xhp()
{
    if (mcOutputFiles) {
        for (int i = 0; i < miCntLanguages; ++i)
            mcOutputFiles[i].close();
        delete[] mcOutputFiles;
    }
    if (msLangText)
        delete[] msLangText;
}



void convert_xhp::doExecute()
{
    string sLang;
    string sFile, sFile2;

    // prepare list with languages
    miCntLanguages = mcMemory.prepareMerge();
    if (mbMergeMode) {
        mcOutputFiles  = new ofstream[miCntLanguages];
        msLangText     = new string[miCntLanguages];

        for (int i = 0; mcMemory.getMergeLang(sLang, sFile); ++i) {
            sFile2 = sLang + "/text/" + mcMemory.getModuleName().substr(5) + "/" + msSourceFile;
            sFile  = msTargetPath + sFile2;
            mcOutputFiles[i].open(sFile.c_str(), ios::binary);
            if (!mcOutputFiles[i].is_open()) {
                if (!createDir(msTargetPath, sFile2))
                    throw l10nMem::showError("Cannot create missing directories (" + sFile + ") for writing");

                mcOutputFiles[i].open(sFile.c_str(), ios::binary);
                if (!mcOutputFiles[i].is_open())
                    throw l10nMem::showError("Cannot open file (" + sFile + ") for writing");
            }
            msLangText[i] = "xml-lang=\"" + sLang + "\"";
        }
    }

    // run analyzer
    xhplex();

    // dump last line
    copySourceSpecial(nullptr,3);
}



void convert_xhp::setString(char *yytext)
{
    copySourceSpecial(yytext, 0);
}



void convert_xhp::openTag(char *yytext)
{
    if (meExpectValue == VALUE_IS_VALUE) {
        meExpectValue  = VALUE_IS_VALUE_TAG;
    //FIX    msCollector   += "\\";
    }
    copySourceSpecial(yytext, 0);
}



void convert_xhp::closeTag(char *yytext)
{
    STATE newState = meExpectValue;

    switch (meExpectValue) {
        case VALUE_IS_VALUE_TAG:
             newState = VALUE_IS_VALUE;
             //FIX         msCollector   += "\\";
             break;

        case VALUE_IS_TAG_TRANS:
             if (msKey.size())
                 newState = VALUE_IS_VALUE;
             break;

        case VALUE_IS_TAG:
             msKey.clear();
             newState = VALUE_NOT_USED;
             break;

        case VALUE_NOT_USED:
        case VALUE_IS_VALUE:
             break;
    }
    copySourceSpecial(yytext, 0);
    meExpectValue = newState;
}



void convert_xhp::closeTagNOvalue(char *yytext)
{
    copySourceSpecial(yytext, 0);
    if (meExpectValue == VALUE_IS_VALUE_TAG)
        meExpectValue = VALUE_IS_VALUE;
    else
        meExpectValue = VALUE_NOT_USED;
}



void convert_xhp::setId(char *yytext)
{
    int          nL, nE;
    string& sText = copySourceSpecial(yytext, 0);


    nL = sText.find("\"");
    nE = sText.find("\"", nL+1);
    if (nL == (int)string::npos || nE == (int)string::npos)
        return;

    switch (meExpectValue) {
        case VALUE_IS_TAG:
        case VALUE_IS_TAG_TRANS:
             msKey = sText.substr(nL+1, nE - nL -1) + msKey;
             break;

        case VALUE_IS_VALUE_TAG:
        case VALUE_NOT_USED:
        case VALUE_IS_VALUE:
             break;
    }
}



void convert_xhp::setLang(char *yytext)
{
    int          nL, nE;
    string  sLang;
    string& sText = copySourceSpecial(yytext, 1);


    nL = sText.find("\"");
    nE = sText.find("\"", nL+1);
    if (nL == (int)string::npos || nE == (int)string::npos)
        return;

    switch (meExpectValue) {
        case VALUE_IS_TAG:
             sLang = sText.substr(nL+1, nE - nL -1);
             if (sLang == "en-US")
                 meExpectValue = VALUE_IS_TAG_TRANS;
             else
                 l10nMem::showError(sLang + " is no en-US language");
             break;

        case VALUE_IS_VALUE_TAG:
             msCollector.erase(msCollector.size() - sText.size() -1);
             break;

        case VALUE_NOT_USED:
        case VALUE_IS_TAG_TRANS:
        case VALUE_IS_VALUE:
             break;
    }
}



void convert_xhp::setRef(char *yytext)
{
    int          nL, nE;
    string& sText = copySourceSpecial(yytext, 0);


    nL = sText.find("\"");
    nE = sText.find("\"", nL+1);
    if (nL == (int)string::npos || nE == (int)string::npos)
        return;

    switch (meExpectValue) {
        case VALUE_IS_TAG:
        case VALUE_IS_TAG_TRANS:
             msKey += "." + sText.substr(nL+1, nE - nL -1);
             break;

        case VALUE_IS_VALUE_TAG:
        case VALUE_NOT_USED:
        case VALUE_IS_VALUE:
             break;
    }
}



void convert_xhp::openTransTag(char *yytext)
{
    copySourceSpecial(yytext, 0);
    msKey.clear();
    meExpectValue = VALUE_IS_TAG;
}



void convert_xhp::closeTransTag(char *yytext)
{
    int iType = 0;


    if (meExpectValue == VALUE_IS_VALUE || meExpectValue == VALUE_IS_VALUE_TAG) {
        if (msCollector.size() && msCollector != "-") {
            string newString(msCollector);
            if (newString[newString.length() - 1] == ' ')
                newString = newString.substr(0, newString.length() - 1);
            mcMemory.setSourceKey(miLineNo, msSourceFile, msKey, newString, "", "help", "", mbMergeMode);
        }
        msKey.clear();
        iType = 2;
    }
    meExpectValue = VALUE_NOT_USED;
    copySourceSpecial(yytext, iType);
}



void convert_xhp::stopTransTag(char *yytext)
{
    copySourceSpecial(yytext, 0);
    meExpectValue = VALUE_NOT_USED;
}



void convert_xhp::startComment(char *yytext)
{
    mePushValue   = meExpectValue;
    msPushCollect = msCollector;
    meExpectValue = VALUE_NOT_USED;
    copySourceSpecial(yytext, 0);
}



void convert_xhp::stopComment(char *yytext)
{
    copySourceSpecial(yytext, 0);
    meExpectValue = mePushValue;
    msCollector   = msPushCollect;
}



void convert_xhp::handleSpecial(char *yytext)
{
    if (meExpectValue != VALUE_IS_VALUE || meExpectValue != VALUE_IS_VALUE_TAG) {
        string sText(yytext);
        mcMemory.convertFromInetString(sText);
        msCollector += sText;
    }
    else
        copySourceSpecial(yytext, 0);
}



void convert_xhp::handleDataEnd(char *yytext)
{
    int nX = msCollector.size();
    copySourceSpecial(yytext, 0);

    if (meExpectValue == VALUE_IS_VALUE || meExpectValue == VALUE_IS_VALUE_TAG)
        msCollector.erase(nX);
}



void convert_xhp::duplicate(char *yytext)
{
    copySourceSpecial(yytext, 0);

    if (meExpectValue == VALUE_IS_VALUE || meExpectValue == VALUE_IS_VALUE_TAG)
        msCollector += msCollector[msCollector.size()-1];
}



string& convert_xhp::copySourceSpecial(char *yytext, int iType)
{
    bool         doingValue = (meExpectValue == VALUE_IS_VALUE || meExpectValue == VALUE_IS_VALUE_TAG);
    string& sText      = copySource(yytext, !doingValue);
    string  sLang;
    int          i;


    // Do NOT write data while collecting a value (will be replaced by language text)
    if (doingValue)
        return sText;

    // Handling depends o
    switch (iType) {
        case 0: // Used for tokens that are to be copied 1-1,
             if (mbMergeMode) {
                 msLine += yytext;
                 if (*yytext == '\n') {
                     for (i = 0; i < miCntLanguages; ++i)
                         writeSourceFile(msLine, i);
                     msLine.clear();
                 }
             }
             break;

        case 1: // Used for language token, are to replaced with languages
             if (mbMergeMode) {
                 for (i = 0; i < miCntLanguages; ++i) {
                     writeSourceFile(msLine, i);
                     writeSourceFile(msLangText[i], i);
                 }
                 msLine.clear();
             }
             break;

        case 2: // Used for token at end of value, language text are to be inserted and then token written
             if (mbMergeMode) {
                 mcMemory.prepareMerge();
                 for (i = 0; i < miCntLanguages; ++i) {
                     writeSourceFile(msLine, i);
                     mcMemory.getMergeLang(sLang, sText);
                     writeSourceFile(sText,i);
                     string sYY(yytext);
                     writeSourceFile(sYY, i);
                 }
                 msLine.clear();
             }
             break;

        case 3: // Used for EOF
             if (mbMergeMode) {
                 for (i = 0; i < miCntLanguages; ++i)
                     writeSourceFile(msLine, i);
             }
             break;
    }
    return sText;
}



void convert_xhp::writeSourceFile(string& sText, int inx)
{
    if (sText.size() && mcOutputFiles[inx].is_open())
        mcOutputFiles[inx].write(sText.c_str(), sText.size());
}
