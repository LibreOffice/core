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
#include "gConv.hxx"
#include "gConvDB.hxx"
#include "gConvPo.hxx"
#include "gConvProp.hxx"
#include "gConvSrc.hxx"
#include "gConvTree.hxx"
#include "gConvUlf.hxx"
#include "gConvXcs.hxx"
#include "gConvXcu.hxx"
#include "gConvXhp.hxx"
#include "gConvXrm.hxx"
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#define OS_ACCESS(x,y) _access(x,y)
#define OS_MKDIR(x) _mkdir(x)
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#define OS_ACCESS(x,y) access(x,y)
#define OS_MKDIR(x)    mkdir(x,0777)
#endif


convert_gen * convert_gen::mcImpl = NULL;


/**********************   I M P L E M E N T A T I O N   **********************/
convert_gen::convert_gen(l10nMem&           cMemory)
    : mcMemory(cMemory)
{
    mcImpl = this;
}
convert_gen::~convert_gen()
{
}


convert_gen& convert_gen::createInstance(l10nMem&           cMemory,
                         const std::string& sSourceDir,
                         const std::string& sTargetDir,
                         const std::string& sSourceFile)
{
  // did the user give a .xxx with the source file ?
  int nInx = sSourceFile.rfind(".");
  if (nInx == (int)std::string::npos)
    throw l10nMem::showError("source file: "+sSourceFile+" missing extension");

  // find correct conversion class and create correct object
  std::string sExtension = sSourceFile.substr(nInx+1);
  convert_gen *x;
  if      (sExtension == "hrc")        x = new convert_src(cMemory);
  else if (sExtension == "src")        x = new convert_src(cMemory);
  else if (sExtension == "po")         x = new convert_po(cMemory);
  else if (sExtension == "pot")        x = new convert_po(cMemory);
  else if (sExtension == "tree")       x = new convert_tree(cMemory);
  else if (sExtension == "ulf")        x = new convert_ulf(cMemory);
  else if (sExtension == "xcu")        x = new convert_xcu(cMemory);
  else if (sExtension == "xhp")        x = new convert_xhp(cMemory);
  else if (sExtension == "xrm")        x = new convert_xrm(cMemory);
  else if (sExtension == "properties") x = new convert_prop(cMemory);
  else throw l10nMem::showError("unknown extension on source file: "+sSourceFile);

  // and set environment
  x->msSourceFile = sSourceFile;
  x->msTargetPath = sTargetDir;
  x->msSourcePath = sSourceDir + sSourceFile;
  return *x;
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool convert_gen::execute(const bool bMerge, const bool bKid)
{
  mbMergeMode  = bMerge;

  if (bKid)
      throw l10nMem::showError("not implemented");

  // and load file
  if (!prepareFile())
    return false;

  // and execute conversion
  execute();

  return true;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_gen::startSave(const std::string& sLanguage,
                            const std::string& sFile)
{
  std::string x;

  x = sLanguage;
  x = sFile;
  throw l10nMem::showError("startSave called with non .po file");
}



void convert_gen::save(const std::string& sFileName,
                       const std::string& sKey,
                       const std::string& sENUStext,
                       const std::string& sText,
                       bool               bFuzzy)
{
  std::string x;

  if (bFuzzy)
      x = sFileName + sKey + sENUStext + sText;
  throw l10nMem::showError("save called with non .po file");
}



void convert_gen::endSave()
{
  throw l10nMem::showError("endSave called with non .po file");
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool convert_gen::checkAccess(std::string& sFile)
{
  return (OS_ACCESS(sFile.c_str(), 0) == 0);
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool convert_gen::createDir(std::string& sDir, std::string& sFile)
{
  std::string sNewDir(sDir);
  int         newPos, oldPos;


  for (oldPos = 0;; oldPos = newPos +1)
  {
    newPos = sFile.find_first_of("/\\", oldPos);
    if (newPos == (int)std::string::npos)
      break;

    sNewDir += sFile.substr(oldPos, newPos-oldPos) + "/";

    if (!checkAccess(sNewDir))
    {
      OS_MKDIR(sNewDir.c_str());
    }

  }


  return true;
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool convert_gen::prepareFile()
{
  std::ifstream inputFile(msSourcePath.c_str(), std::ios::binary);


  if (!inputFile.is_open())
  {
    if (mbLoadMode)
    {
      l10nMem::showWarning("Cannot open file (" + msSourcePath + ")");
      return false;
    }
    else
      throw l10nMem::showError("Cannot open file (" + msSourcePath + ") for reading");
  }

  // get length of file:
  miSourceReadIndex = 0;
  inputFile.seekg (0, std::ios::end);
  msSourceBuffer.resize((unsigned int)inputFile.tellg());
  inputFile.seekg (0, std::ios::beg);

  // get size, prepare std::string and read whole file
  inputFile.read(const_cast<char *>(msSourceBuffer.c_str()), msSourceBuffer.size());
  if ((unsigned int)inputFile.gcount() != msSourceBuffer.size())
    throw l10nMem::showError("cannot read whole file");
  inputFile.close();

  if (mbMergeMode && !mbLoadMode)
  {
    // close previous file
    if (mcOutputFile.is_open())
      mcOutputFile.close();

    // open output file
    mcOutputFile.open((msTargetPath+msSourceFile).c_str(), std::ios::binary);
    if (mcOutputFile.is_open())
      return true;

    if (convert_gen::createDir(msTargetPath, msSourceFile))
    {
      mcOutputFile.open((msTargetPath+msSourceFile).c_str(), std::ios::binary);
      if (mcOutputFile.is_open())
        return true;
    }
    throw l10nMem::showError("Cannot open file (" + msTargetPath+msSourceFile + ") for writing");
  }

  return true;
}




/**********************   I M P L E M E N T A T I O N   **********************/
void convert_gen::lexRead(char *sBuf, int *nResult, int nMax_size)
{
  // did we hit eof
  if (miSourceReadIndex == -1)
  {
    *nResult = 0;
    return;
  }

  // assume we can copy all that are left.
  *nResult = msSourceBuffer.size() - miSourceReadIndex;

  // space enough for the whole line ?
  if (*nResult <= nMax_size)
  {
//    msSourceBuffer.copy(sBuf, *nResult, miSourceReadIndex);
      l10nMem::showDebug(sBuf);
    miSourceReadIndex = -1;
  }
  else
  {
//    msSourceBuffer.copy(sBuf, nMax_size, miSourceReadIndex);
    *nResult           = nMax_size;
    miSourceReadIndex += nMax_size;
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void convert_gen::writeSourceFile(const std::string& line)
{
  if (!line.size())
    return;

  if (mcOutputFile.is_open())
    mcOutputFile.write(line.c_str(), line.size());
}



/**********************   I M P L E M E N T A T I O N   **********************/
std::string& convert_gen::copySource(char const *yyText, bool bDoClear)
{
  int nL;


  if (!yyText)
  {
    msCopyText.clear();
    return msCopyText;
  }
  msCopyText = yyText;

  // write text for merge
  if (mbMergeMode)
    writeSourceFile(msCopyText);

  if (bDoClear)
    msCollector.clear();
  else
    msCollector += msCopyText;

  // remove any CR
  for (nL = 0; nL < (int)msCopyText.size(); ++nL)
  {
    if (msCopyText[nL] == '\r')
    {
      msCopyText.erase(nL, 1);
      --nL;
      continue;
    }
    if (msCopyText[nL] == '\n')
      ++miLineNo;
  }

  return msCopyText;
}
