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
#include "gConvUlf.hxx"



void convert_ulf::doExecute()
{
    ulflex();
}



void convert_ulf::setKey(char *syyText)
{
    string sText = copySource(syyText);

    // locate key (is any)
    msKey = sText.substr(1,sText.size()-2);
}



void convert_ulf::setText(char *syyText, bool bIsEnUs)
{
    string sText = copySource(syyText);


    if (!bIsEnUs && sText != "x-comment =")
        l10nMem::showError(sText + " is not en-US");
}



void convert_ulf::setValue(char *syyText)
{
    string sLang, sText = copySource(syyText);
    int         nL;

    sText.erase(0,1);
    nL = sText.rfind("\"");
    sText.erase(nL);

    mcMemory.setSourceKey(miLineNo, msSourceFile, msKey, sText, "", "LngText", "", mbMergeMode);

    if (mbMergeMode) {
        // prepare to read all languages
        mcMemory.prepareMerge();
        for (; mcMemory.getMergeLang(sLang, sText);) {
            // Prepare tag
            sText = "\n" + sLang + " = \"" + sText + "\"";
            writeSourceFile(sText);
        }
    }
}
