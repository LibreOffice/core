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
#include "gConvXcu.hxx"
convert_xcu::convert_xcu(l10nMem& crMemory)
                        : convert_gen(crMemory),
                          mbNoCollectingData(true),
                          miLevel(0),
                          mbNoTranslate(false)
{
}



/************   I N T E R F A C E   I M P L E M E N T A T I O N   ************/
convert_xcu::~convert_xcu()
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
//namespace XcuWrap
//{
//#define IMPLptr convert_gen_impl::mcImpl
//#define LOCptr ((convert_xcu *)convert_gen_impl::mcImpl)
//#include "gConXcu_yy.c"
//}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_xcu::execute()
{
//  XcuWrap::yylex();
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_xcu::pushKey(char *syyText)
{
  std::string sKey, sTag = copySource(syyText);
  int    nL, nE;

  // find key in tag
  nL = sTag.find("oor:name=\"");
  if (nL != (int)std::string::npos)
  {
    // find end of key
    nL += 10;
    nE  = sTag.find("\"", nL);
    if (nE != (int)std::string::npos)
      sKey = sTag.substr(nL, nE - nL);
  }
  mcStack.push_back(sKey);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_xcu::popKey(char *syyText)
{
  copySource(syyText);

  // check for correct node/prop relations
  if (mcStack.size())
    mcStack.pop_back();

  mbNoTranslate = false;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_xcu::startCollectData(char *syyText)
{
  int nL;
  std::string sTag = copySource(syyText);

  if (mbNoTranslate)
    return;

  // locate object name
  nL = sTag.find("xml:lang=\"");
  if (nL != (int)std::string::npos)
  {
    // test langauge
    nL += 10;
    if (sTag.substr(nL,5) == "en-US")
      mbNoCollectingData = false;
    else if (sTag.substr(nL,14) == "x-no-translate")
      mbNoTranslate = true;
    else
    {
      std::string sErr = sTag.substr(nL,5) + " is not en-US";
      l10nMem::showError(sErr);
    }
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_xcu::stopCollectData(char *syyText)
{
  int    nL;
  std::string useKey, useText = msCollector;

  copySource(syyText);

  // time to do something ?
  if (mbNoCollectingData || mbNoTranslate)
    return;

  // remove any newline
  for (nL = 0;;)
  {
    nL = useText.find("\n");
    if (nL == (int)std::string::npos)
      break;
    useText.erase(nL,1);
  }

  mbNoCollectingData = true;

  if (useText.size())
  {
    // locate key and extract it
    for (nL = 0; nL < (int)mcStack.size(); ++nL)
      useKey += (useKey.size() ? "." : "" ) + mcStack[nL];
    mcMemory.setSourceKey(miLineNo, msSourceFile, useKey, useText, mbMergeMode);
  }

  if (mbMergeMode)
  {
    std::string sLang, sText, sNewLine;


    // prepare to read all languages
    mcMemory.prepareMerge();
    for (; mcMemory.getMergeLang(sLang, sText);)
    {
      sNewLine = "\n<value xml:lang=\"" + sLang + "\">" + sText + "</value>";
      mcMemory.convertToInetString(sNewLine);
      writeSourceFile(sNewLine);
    }
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_xcu::copySpecial(char *syyText)
{
  int         nx    = msCollector.size();
  std::string sText = copySource(syyText, mbNoCollectingData);

  if (!mbNoCollectingData)
  {
    msCollector.erase(nx);
    mcMemory.convertFromInetString(sText);
    msCollector += sText;
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_xcu::copyNL(char *syyText)
{
  int         nX    = msCollector.size();
  std::string sText = copySource(syyText, mbNoCollectingData);

  if (!mbNoCollectingData)
  {
    msCollector.erase(nX);
    msCollector += ' ';
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_xcu::addLevel()
{
  ++miLevel;
}
