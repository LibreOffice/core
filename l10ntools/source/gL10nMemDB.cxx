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

#include "gL10nMem.hxx"
#include <iostream>
#include <fstream>
#include <sstream>





/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_lang_entry::l10nMem_lang_entry(const std::string& sMsgStr, bool bFuzzy)
                                      :
                                       msMsgStr(sMsgStr),
                                       mbFuzzy(bFuzzy)
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_lang_entry::~l10nMem_lang_entry()
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_enus_entry::l10nMem_enus_entry(const std::string&   sKey,
                                       const std::string&   sMsgId,
                                       int                  iLineNo,
                                       int                  iFileInx,
                                       int                  iLangSize,
                                       l10nMem::ENTRY_STATE eState)
                                      :
                                       msMsgId(sMsgId),
                                       meState(eState),
                                       miFileInx(iFileInx),
                                       miLineNo(iLineNo)
{
  int i;

  // add dummy language entries
  for (i = 0; i < iLangSize; ++i)
    mcLangText.push_back(l10nMem_lang_entry("", false));

  // convert key to upper case
  msKey = sKey;
  l10nMem_db::keyToUpper(msKey);
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_enus_entry::~l10nMem_enus_entry()
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_file_entry::l10nMem_file_entry(const std::string& sFileName, int iStart)
                                      :
                                       msFileName(sFileName),
                                       miStart(iStart),
                                       miEnd(iStart)
{
  // Store fileName without relative path
  int i = msFileName.rfind("/");
  if (i == (int)std::string::npos)
    msPureName = msFileName;
  else
    msPureName = msFileName.substr(i+1);
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_file_entry::~l10nMem_file_entry()
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_lang_list_entry::l10nMem_lang_list_entry(const std::string& sName)
                                                :
                                                  msName(sName),
                                                  mbChanged(false)
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_lang_list_entry::~l10nMem_lang_list_entry()
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_db::l10nMem_db()
                      :
                       miCurFileInx(0),
                       miCurLangInx(0),
                       miCurENUSinx(0),
                       mbNeedWrite(false),
                       mbConvertMode(false),
                       mbStrictMode(false)
{
  mcFileList.push_back(l10nMem_file_entry("-genLang-", 0));
  mcLangList.push_back(l10nMem_lang_list_entry("-genLang-"));
  mcENUSlist.push_back(l10nMem_enus_entry("-genLang-", "-genLang-", 0, 0, 0, l10nMem::ENTRY_DELETED));
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_db::~l10nMem_db()
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_db::loadENUSkey(int                iLineNo,
                             const std::string& sSourceFile,
                             const std::string& sKey,
                             const std::string& sMsgId)
{
  // add it to vector and update file pointer
  addKey(iLineNo, sSourceFile, sKey, sMsgId, l10nMem::ENTRY_DELETED);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_db::setLanguage(const std::string& sLanguage,
                             bool               bCreate)
{
  int i, iSize;

  // regular load or convert of old po files
  miCurFileInx  = 0;

  // With no languages selected only en-US is generated
  if (!sLanguage.size())
  {
    miCurLangInx = 0;
    return;
  }

  // en-US is loaded as master and cannot be loaded again
  if (sLanguage == "en-US")
    throw l10nMem::showError("en-US is loaded automatically");

  // check if language is already loaded
  iSize = mcLangList.size();
  for (miCurLangInx = 0; miCurLangInx < iSize && mcLangList[miCurLangInx].msName != sLanguage; ++miCurLangInx) ;
  if (miCurLangInx < iSize)
  {
    if (bCreate)
      throw l10nMem::showError("loading " + sLanguage + " twice");
    return;
  }

  // language does not exist in db
  if (!bCreate)
    throw l10nMem::showError("language " + sLanguage + " not loaded");

  // create language
  mcLangList.push_back(sLanguage);

  // add language to all ENUS entries
  iSize = mcENUSlist.size();
  for (i = 0; i < iSize; ++i)
    mcENUSlist[i].mcLangText.push_back(l10nMem_lang_entry("", false));
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_db::setConvert(bool bConvert,
                            bool bStrict)
{
  // regular load or convert of old po files
  mbConvertMode = bConvert;
  mbStrictMode  = bStrict;
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool l10nMem_db::findFileName(const std::string& sSourceFile)
{
  int iSize = mcFileList.size();

  // Check this or next file
  if (mcFileList[miCurFileInx].msFileName == sSourceFile || mcFileList[miCurFileInx].msPureName == sSourceFile)
    return true;
  if (++miCurFileInx < iSize && mcFileList[miCurFileInx].msFileName == sSourceFile)
    return true;

  for (miCurFileInx = 1;
       miCurFileInx < iSize && mcFileList[miCurFileInx].msFileName != sSourceFile &&
       mcFileList[miCurFileInx].msPureName != sSourceFile;
       ++miCurFileInx) ;
  if (miCurFileInx == iSize)
  {
    miCurFileInx = 0;
    return false;
  }
  else
    return true;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_db::loadLangKey(int                iLineNo,
                             const std::string& sSourceFile,
                             const std::string& sKey,
                             const std::string& sMsgId,
                             const std::string& sMsgStr,
                             bool               bFuzzy)
{
  if (!locateKey(iLineNo, sSourceFile, sKey, sMsgId, true))
    throw l10nMem::showError(".po file contains unknown filename: " + sSourceFile + " or key: " + sKey);

  l10nMem_lang_entry& xCur = mcENUSlist[miCurENUSinx].mcLangText[miCurLangInx];
  xCur.msMsgStr  = sMsgStr;
  xCur.mbFuzzy = bFuzzy;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_db::reorganize(bool bConvert)
{
  int iE, iEsize  = mcENUSlist.size();
  int iD, iDsize;
  std::vector<int> listDel, listAdd;


  // Check number of changes
  for (iE = 1; iE < iEsize; ++iE)
  {
    l10nMem_enus_entry& cur = mcENUSlist[iE];
    if (cur.meState == l10nMem::ENTRY_ADDED)
      listAdd.push_back(iE);
    if (cur.meState == l10nMem::ENTRY_DELETED)
    {
      if (bConvert)
        cur.meState = l10nMem::ENTRY_NORMAL;
      else
        listDel.push_back(iE);
    }
  }

  if (!listDel.size() || !listAdd.size())
    return;

  // loop through added text and see if key match deleted text
  iEsize = listAdd.size();
  iDsize = listDel.size();
  for (iE = 0; iE < iEsize; ++iE)
  {
    l10nMem_enus_entry& curAdd = mcENUSlist[listAdd[iE]];
    for (iD = 0; iD < iDsize; ++iD)
    {
      l10nMem_enus_entry& curE = mcENUSlist[listDel[iD]];

      if (curE.miFileInx != curAdd.miFileInx)
        continue;
      if (curE.msKey == curAdd.msKey)
        break;
      if (curE.msMsgId == curAdd.msMsgId)
        break;
    }
    if (iD == iDsize)
      continue;

    // Update deleted entry (original), because lang is connected here
    l10nMem_enus_entry& curDel = mcENUSlist[listDel[iD]];
    curDel.msMsgId   = curAdd.msMsgId;
    curDel.msKey     = curAdd.msKey;
    curDel.meState   = l10nMem::ENTRY_CHANGED;
    curAdd.meState   = l10nMem::ENTRY_DELETED;
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool l10nMem_db::locateKey(int                iLineNo,
                           const std::string& sSourceFile,
                           const std::string& sKey,
                           const std::string& sMsgId,
                           bool               bThrow)
{
  std::string sUpperKey(sKey);
  int         i, iSize = sUpperKey.size();
  char        ch;

  // Position file pointer
  if (!findFileName(sSourceFile))
    return false;

  // convert key to upper case
  for (i = 0; i < iSize; ++i)
  {
    ch = sUpperKey[i];
    if (ch == ' ' || ch == '*' || ch == '+' || ch == '%')
      sUpperKey[i] = '_';
    else
      sUpperKey[i] = toupper(sUpperKey[i]);
  }

  // Fast check, to see if next key is the one (normal with load and source without change)
  if (++miCurENUSinx < (int)mcENUSlist.size())
  {
    l10nMem_enus_entry& nowEntry = mcENUSlist[miCurENUSinx];
    if (nowEntry.msMsgId == sMsgId && nowEntry.msKey == sUpperKey)
      return true;
  }

  // Start from beginning of file and to end
  l10nMem_file_entry& cCur  = mcFileList[miCurFileInx];

  // Find match with key and text
  for (miCurENUSinx = cCur.miStart; miCurENUSinx <= cCur.miEnd; ++miCurENUSinx)
  {
    l10nMem_enus_entry& cEntry = mcENUSlist[miCurENUSinx];
    if (cEntry.msMsgId == sMsgId && cEntry.msKey == sUpperKey)
      return true;
  }

  if (bThrow)
    throw l10nMem::showError("cannot find key(" + sUpperKey +") with text(" + sMsgId + ")", iLineNo);
  return false;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_db::addKey(int                  iLineNo,
                        const std::string&   sSourceFile,
                        const std::string&   sKey,
                        const std::string&   sMsgId,
                        l10nMem::ENTRY_STATE eStat)
{
  // check file
  if (!findFileName(sSourceFile))
  {
    // prepare for new entry
    miCurENUSinx = mcENUSlist.size();
    miCurFileInx = mcFileList.size();

    // Create file
    mcFileList.push_back(l10nMem_file_entry(sSourceFile, miCurENUSinx));

    // and add entry at the back (no problem since it is a new file)
    mcENUSlist.push_back(l10nMem_enus_entry(sKey, sMsgId, iLineNo, miCurFileInx,
                                               mcLangList.size(), eStat));
    mcFileList[miCurFileInx].miEnd = miCurENUSinx;
  }
  else
  {
    int iFsize                                   = mcFileList.size();
    l10nMem_file_entry& curF                     = mcFileList[miCurFileInx];
    std::vector<l10nMem_enus_entry>::iterator it = mcENUSlist.begin();

    // file is registred, so we need to add the entry at the end of the file range
    curF.miEnd++;
    miCurENUSinx = curF.miEnd;
    mcENUSlist.insert(it + curF.miEnd,
                      l10nMem_enus_entry(sKey, sMsgId, iLineNo, miCurFileInx,
                                         mcLangList.size(), eStat));
    for (int i = miCurFileInx+1; i < iFsize; ++i)
    {
      l10nMem_file_entry& curF2 = mcFileList[i];
      if (curF2.miStart >= curF.miEnd)
        curF2.miStart++;
      if (curF2.miEnd >= curF.miEnd)
        curF2.miEnd++;
    }
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
int l10nMem_db::prepareMerge()
{
  miCurLangInx = 0;
  return mcLangList.size();
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool l10nMem_db::getMergeLang(std::string& sLang,
                              std::string& sMsgStr)
{
  miCurLangInx++;
  if (miCurLangInx >= (int)mcLangList.size())
    return false;

  // update pointers
  sLang = mcLangList[miCurLangInx].msName;
  if (!sMsgStr.size())
    sMsgStr = "NOT TRANSLATED";
  else
    sMsgStr = mcENUSlist[miCurENUSinx].mcLangText[miCurLangInx].msMsgStr;
  return true;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_db::keyToUpper(std::string& sKey)
{
  int i, iSize;

  iSize = sKey.size();
  for (i = 0; i < iSize; ++i)
  {
    char ch = sKey[i];
    if (ch == ' ' || ch == '*' || ch == '+' || ch == '%')
      sKey[i] = '_';
    else
      sKey[i] = toupper(ch);
  }
}
