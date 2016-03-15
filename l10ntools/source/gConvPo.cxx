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
    std::string sFileName, sNewKey;
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



void convert_po::execute()
{
    if (mbMergeMode)
        throw l10nMem::showError("Merge not implemented");

    //  PoWrap::yylex();
    startLook();
}



void convert_po::startSave(const std::string& sLanguage,
                           const std::string& sFile)
{
    std::string sFilePath = msTargetPath + sLanguage + sFile;
    outBuffer.open(sFilePath.c_str(), std::ios::out);

    if (!outBuffer.is_open())
        throw l10nMem::showError("Cannot open " + sFilePath + " for writing");

    l10nMem::showDebug("writing file (" + sFilePath + ")");

    std::ostream outFile(&outBuffer);

    // Set license header
    outFile << "#*************************************************************" << std::endl
            << "#*"                                                             << std::endl
            << "#* This file is part of the LibreOffice project.              " << std::endl
            << "#*"                                                             << std::endl
            << "#* This Source Code Form is subject to the terms of the       " << std::endl
            << "#* Mozilla Public License, v. 2.0. If a copy of the MPL was   " << std::endl
            << "#* not distributed with this file, You can obtain one at      " << std::endl
            << "#* http://mozilla.org/MPL/2.0/."                                << std::endl
            << "#*"                                                             << std::endl
            << "#* This file incorporates work covered by the following       " << std::endl
            << "#* license notice :"                                            << std::endl
            << "#*"                                                             << std::endl
            << "#* Licensed to the Apache Software Foundation (ASF) under one " << std::endl
            << "#* or more contributor license agreements. See the NOTICE file" << std::endl
            << "#* distributed with this work for additional information      " << std::endl
            << "#* regarding copyright ownership. The ASF licenses this file  " << std::endl
            << "#* to you under the Apache License, Version 2.0               " << std::endl
            << "#* (the \"License\"); you may not use this file except in     " << std::endl
            << "#* compliance with the License.You may obtain a copy of the   " << std::endl
            << "#* License at http ://www.apache.org/licenses/LICENSE-2.0 .   " << std::endl
            << "#*"                                                             << std::endl
            << "#************************************************************"  << std::endl
            << "msgid \"\""                                                     << std::endl
            << "msgstr \"\""                                                    << std::endl
            << "\"Project-Id-Version: AOO-4-xx\\n\""                            << std::endl
            << "\"POT-Creation-Date: \\n\""                                     << std::endl
            << "\"PO-Revision-Date: \\n\""                                      << std::endl
            << "\"Last-Translator: genLang (build process)\\n\""                << std::endl
            << "\"Language-Team: \\n\""                                         << std::endl
            << "\"MIME-Version: 1.0\\n\""                                       << std::endl
            << "\"Content-Type: text/plain; charset=UTF-8\\n\""                 << std::endl
            << "\"Content-Transfer-Encoding: 8bit\\n\""                         << std::endl
            << "\"X-Generator: genLang\\n\""                                    << std::endl
            << std::endl;
}



void convert_po::save(const std::string& sFileName,
                      const std::string& sKey,
                      const std::string& sENUStext,
                      const std::string& sText,
                      bool               bFuzzy)
{
    std::ostream outFile(&outBuffer);

    outFile << std::endl << "#: " << sFileName << "#" << sKey << std::endl;
    if (bFuzzy)
        outFile << "#, fuzzy" << std::endl;
    outFile << "msgid  \"" << sENUStext << "\"" << std::endl
            << "msgstr \"" << sText     << "\"" << std::endl;
}




void convert_po::endSave()
{
    outBuffer.close();
}
