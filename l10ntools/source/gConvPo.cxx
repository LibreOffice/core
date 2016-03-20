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
#include "gConvPo.hxx"
#include <iostream>
#include <fstream>
#include <cstdlib>



convert_po::convert_po(l10nMem& crMemory)
                      : convert_gen(crMemory),
                        mbExpectId(false),
                        mbExpectStr(false),
                        mbFuzzy(false)
{
    // Po files are handled special
    mbLoadMode = true;
}



convert_po::~convert_po()
{
}



void convert_po::startLook()
{
    string sFileName, sNewKey;
    int         i;


    if (!msKey.size() || !msId.size())
        return;

    // split key into filename and real key
    i = msKey.find("#");
    sFileName = msKey.substr(0, i);
    sNewKey   = msKey.substr(i+1);

    // load in db
    if (msId.size())
        mcMemory.loadEntryKey(miLineNo, sFileName, sNewKey, msId, msStr, mbFuzzy);

    // and prepare for new entry
    msKey.clear();
    msId.clear();
    msStr.clear();
}



void convert_po::setValue(char *syyText, int iLineCnt)
{
    if (mbExpectId)
        msId = syyText;
    if (mbExpectStr)
        msStr = syyText;
    mbExpectId  =
    mbExpectStr = false;
    miLineNo   += iLineCnt;
}



void convert_po::setFuzzy()
{
    mbFuzzy = true;
}



void convert_po::setKey(char *syyText)
{
    int i;


    // Activate "look for msg" mode.
    startLook();

    // skip "#:" and any blanks
    for (syyText += 2; *syyText == ' ' || *syyText == '\t'; ++syyText) ;
        msKey = syyText;

    // remove trailing blanks
    for (i = msKey.size() -1; msKey[i] == '\r' || msKey[i] == ' ' || msKey[i] == '\t'; --i) ;
        msKey.erase(i+1);
}



void convert_po::setMsgId()
{
    mbExpectId = true;
}



void convert_po::setMsgStr()
{
    mbExpectStr = true;
}



void convert_po::handleNL()
{
    ++miLineNo;
}



extern int polex(void);
void convert_po::doExecute()
{
    if (mbMergeMode)
        throw l10nMem::showError("Merge not implemented");

    polex();
    startLook();
}



void convert_po::startSave(const string& sName,
                           const string& sTargetDir,
                           const string& sFile)
{
    string sFilePath = sTargetDir + "/" + sFile;

    // create directories as needed
    createDir(string(""), sFilePath);
    outBuffer.open(sFilePath.c_str(), ios::out + ios::binary);

    if (!outBuffer.is_open())
        throw l10nMem::showError("Cannot open " + sFilePath + " for writing");

    l10nMem::showDebug("writing file (" + sFilePath + ")");

    ostream outFile(&outBuffer);

    // Set license header
    outFile << "#. extracted from " << sName                       << endl
            << "msgid \"\""                                        << endl
            << "msgstr \"\""                                       << endl
            << "\"Project-Id-Version: PACKAGE VERSION\\n\""        << endl
            << "\"Report-Msgid-Bugs-To: "
            << "https://bugs.libreoffice.org/enter_bug.cgi?"
            << "product=LibreOffice&bug_status=UNCONFIRMED"
            << "&component=UI\\n\""                                << endl
            << "\"POT-Creation-Date: \\n\""                        << endl
            << "\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\""    << endl
            << "\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"" << endl
            << "\"Language-Team: LANGUAGE <LL@li.org>\\n\""        << endl
            << "\"MIME-Version: 1.0\\n\""                          << endl
            << "\"Content-Type: text/plain; charset=UTF-8\\n\""    << endl
            << "\"Content-Transfer-Encoding: 8bit\\n\""            << endl
            << "\"X-Generator: LibreOffice\\n\""                   << endl
            << "\"X-Accelerator-Marker: ~\\n\""                    << endl;
}



void convert_po::save(const string& sFileName,
                      const string& sKey,
                      const string& sENUStext,
                      const string& sText,
                      bool               bFuzzy)
{
    string sName;
    ostream outFile(&outBuffer);
    int newPos;

    // isolate filename
    newPos = sFileName.find_last_of("/\\", sFileName.length());
    sName = sFileName.substr(newPos + 1, sFileName.length());

    outFile << "#. xxxxx" << endl
            << "#: " << sName << endl
            << "msgctxt \"\"" << endl
            << "\"" << sName << "\\n\"" << endl;
    if (bFuzzy)
        outFile << "#, fuzzy" << endl;
    outFile << "msgid  \"" << sENUStext << "\"" << endl
            << "msgstr \"" << sText     << "\"" << endl;
}




void convert_po::endSave()
{
    outBuffer.close();
}
