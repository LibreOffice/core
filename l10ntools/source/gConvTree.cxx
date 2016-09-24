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
#include "gConvTree.hxx"


convert_tree::convert_tree(l10nMem& crMemory)
                          : convert_gen(crMemory),
                            mcOutputFiles(nullptr),
                            meStateTag(STATE_TAG_NONE),
                            meStateVal(STATE_VAL_NONE),
                            miCntLanguages(0)

{
    // tree files are written through a local routine
    mbLoadMode = true;
}



convert_tree::~convert_tree()
{
    if (mcOutputFiles) {
        for (int i = 0; i < miCntLanguages; ++i)
            mcOutputFiles[i].close();
        delete[] mcOutputFiles;
    }
}



void convert_tree::doExecute()
{
    string sLang;
    string sFile, sFile2;

    if (mbMergeMode)
        throw l10nMem::showError("Merge not implemented");

    // prepare list with languages
    if (mbMergeMode) {
        miCntLanguages = mcMemory.prepareMerge();
        mcOutputFiles  = new ofstream[miCntLanguages];

        for (int i = 0; mcMemory.getMergeLang(sLang, sFile); ++i) {
            sFile2 = sLang + "/" + msSourceFile;
            sFile  = msTargetPath + sFile2;
            mcOutputFiles[i].open(sFile.c_str(), ios::binary);
            if (!mcOutputFiles[i].is_open()) {
                if (!createDir(msTargetPath, sFile2))
                    throw l10nMem::showError("Cannot create missing directories (" + sFile + ") for writing");

                mcOutputFiles[i].open(sFile.c_str(), ios::binary);
                if (!mcOutputFiles[i].is_open())
                    throw l10nMem::showError("Cannot open file (" + sFile + ") for writing");
            }
        }
    }

    // run analyzer
    treelex();

    // dump last line
    copySourceSpecial(nullptr,3);
}



void convert_tree::setString(char *yytext)
{
    switch (meStateVal) {
        case STATE_VAL_NONE:
             copySourceSpecial(yytext, 0);
             break;

        case STATE_VAL_APPL:
             msAppl = copySourceSpecial(yytext, 0);
             break;

        case STATE_VAL_ID:
             msId = copySourceSpecial(yytext, 0);
             msId.erase(msId.size()-1);
             break;

        case STATE_VAL_TITLE:
             string sText = copySourceSpecial(yytext, 1);
             sText.erase(sText.size()-1);
             mcMemory.setSourceKey(miLineNo, msSourceFile, msId, sText, "", "help_section", "", mbMergeMode);
             break;
    }
    meStateVal = STATE_VAL_NONE;
}



void convert_tree::setState(char *yytext, STATE_TAG eNewStateTag, STATE_VAL eNewStateVAL, char *sModule)
{
    copySourceSpecial(yytext, 0);
    msCollector.clear();
    meStateTag = eNewStateTag;
    meStateVal = eNewStateVAL;
    if (sModule)
        msModule = sModule;
}



void convert_tree::setValue(char *yytext)
{
    mcMemory.setSourceKey(miLineNo, msSourceFile, msId, msCollector, "", msModule, "", mbMergeMode);
    copySourceSpecial(yytext, 2);

    meStateTag = STATE_TAG_NONE;
    meStateVal = STATE_VAL_NONE;
}



string& convert_tree::copySourceSpecial(char *yytext, int iType)
{
    string& sText = copySource(yytext, false);
    string  sLang, sTemp;
    int          i;

    // Handling depends on iType
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

        case 1: // Used for title token, are to replaced with languages
             if (mbMergeMode) {
                 mcMemory.prepareMerge();
                 for (i = 0; i < miCntLanguages; ++i) {
                     writeSourceFile(msLine, i);
                     mcMemory.getMergeLang(sLang, sTemp);
                     writeSourceFile(sTemp,i);
                 }
                 msLine.clear();
             }
             break;

        case 2: // Used for token at end of value, language text are to be inserted and then token written
             if (mbMergeMode) {
                 mcMemory.prepareMerge();
                 for (i = 0; i < miCntLanguages; ++i) {
                      writeSourceFile(msLine, i);
                      mcMemory.getMergeLang(sLang, sTemp);
                      writeSourceFile(sTemp,i);
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



void convert_tree::writeSourceFile(string& sText, int inx)
{
    if (sText.size() && mcOutputFiles[inx].is_open())
        mcOutputFiles[inx].write(sText.c_str(), sText.size());
}
