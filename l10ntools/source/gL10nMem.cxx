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
//#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>



/*****************************************************************************
 ************************   G L 1 0 N M E M . C X X   ************************
 *****************************************************************************
 * This is the interface to translation memory that links between the converts
 * and to the language files. The memory contains the actual text info
 ***********************d******************************************************/



/*******************   G L O B A L   D E F I N I T I O N   *******************/
l10nMem_impl * l10nMem_impl::mcImpl    = NULL;
bool           l10nMem_impl::mbVerbose = false;
bool           l10nMem_impl::mbDebug   = false;



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem::l10nMem()
{
  l10nMem_impl::mcImpl = new l10nMem_impl();
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem::~l10nMem()
{
  delete l10nMem_impl::mcImpl;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem::setShowVerbose()
{
  l10nMem_impl::mbVerbose = true;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem::setShowDebug()
{
  l10nMem_impl::mbDebug = true;
}



/**********************   I M P L E M E N T A T I O N   **********************/
int l10nMem::showError(const std::string& sText, int iLineNo)
     { return l10nMem_impl::mcImpl->showError(sText, iLineNo); }
int  l10nMem::showWarning(const std::string& sText, int iLineNo)
     { return l10nMem_impl::mcImpl->showWarning(sText, iLineNo); }
void l10nMem::showDebug(const std::string& sText, int iLineNo)
     { l10nMem_impl::mcImpl->showDebug(sText, iLineNo); }
void l10nMem::showVerbose(const std::string& sText, int iLineNo)
     { l10nMem_impl::mcImpl->showVerbose(sText, iLineNo); }
bool l10nMem::isError()
     { return l10nMem_impl::mcImpl->mbInError; }
void l10nMem::setModuleName(const std::string& sM)
     { l10nMem_impl::mcImpl->setModuleName(sM); }
const std::string& l10nMem::getModuleName ()
     { return l10nMem_impl::mcImpl->getModuleName(); }
void l10nMem::setLanguage(const std::string& sL, bool bC)
     { l10nMem_impl::mcImpl->mcDb.setLanguage(sL, bC); }
void l10nMem::setConvert(bool bC, bool bS)
     { l10nMem_impl::mcImpl->mcDb.setConvert(bC, bS); }
void l10nMem::loadEntryKey(int iL, const std::string& sS, const std::string& sK, const std::string& sO, const std::string& sT, bool               bI)
     { l10nMem_impl::mcImpl->loadEntryKey(iL, sS, sK, sO, sT, bI); }
void l10nMem::setSourceKey(int iL, const std::string& sF, const std::string& sK, const std::string& sT, bool bM)
     { l10nMem_impl::mcImpl->setSourceKey(iL, sF, sK, sT, bM); }
void l10nMem::saveTemplates(const std::string& sT, bool bK, bool bF)
     { l10nMem_impl::mcImpl->saveTemplates(*this, sT, bK, bF); }
void l10nMem::saveLanguages(const std::string& sT, bool bF)
     { l10nMem_impl::mcImpl->saveLanguages(*this, sT, bF); }
void l10nMem::showNOconvert ()
     { l10nMem_impl::mcImpl->showNOconvert(); }
void l10nMem::convertToInetString(std::string& sT)
     { l10nMem_impl::mcImpl->convertToInetString(sT); }
void l10nMem::convertFromInetString(std::string& sT)
     { l10nMem_impl::mcImpl->convertFromInetString(sT); }

int  l10nMem::prepareMerge()
     { return l10nMem_impl::mcImpl->mcDb.prepareMerge(); }
void l10nMem::dumpMem(const std::string& sT)
     { l10nMem_impl::mcImpl->dumpMem(sT); }
bool l10nMem::getMergeLang(std::string& sL, std::string& sT)
     { return l10nMem_impl::mcImpl->mcDb.getMergeLang(sL, sT); }



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_impl::l10nMem_impl()
                          :
                            mbInError(false)
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
l10nMem_impl::~l10nMem_impl()
{
}



/**********************   I M P L E M E N T A T I O N   **********************/
int l10nMem_impl::showError(const std::string& sText, int iLineNo)
{
  mbInError = true;
  formatAndShowText("ERROR", iLineNo, sText);
  return 1;
}



/**********************   I M P L E M E N T A T I O N   **********************/
int l10nMem_impl::showWarning(const std::string& sText, int iLineNo)
{
  formatAndShowText("WARNING", iLineNo, sText);
  return 2;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::showDebug(const std::string& sText, int iLineNo)
{
  if (mbDebug)
    formatAndShowText("DEBUG", iLineNo, sText);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::showVerbose(const std::string& sText, int iLineNo)
{
  if (mbVerbose)
    formatAndShowText("INFO", iLineNo, sText);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::setModuleName(const std::string& sModuleName)
{
  msModuleName = sModuleName;
}



/**********************   I M P L E M E N T A T I O N   **********************/
const std::string& l10nMem_impl::getModuleName()
{
  return msModuleName;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::loadEntryKey(int                iLineNo,
                               const std::string& sSourceFile,
                               const std::string& sKey,
                               const std::string& sMsgId,
                               const std::string& sMsgStr,
                               bool               bIsFuzzy)
{
  if (mcDb.mbConvertMode)
    convEntryKey(iLineNo, sSourceFile, sKey, sMsgId, sMsgStr, bIsFuzzy);
  else if (!mcDb.miCurLangInx)
    mcDb.loadENUSkey(iLineNo, sSourceFile, sKey, sMsgId);
  else
    mcDb.loadLangKey(iLineNo, sSourceFile, sKey, sMsgId, sMsgStr, bIsFuzzy);
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::setSourceKey(int                iLineNo,
                                const std::string& sSourceFile,
                                const std::string& sKey,
                                const std::string& sMsgId,
                                bool               bMustExist)
{
  std::string newText(sMsgId);
  int         i;

  // time to escape " and \ if contained in text or key
  for (i = 0; (i = newText.find("\\", i)) != (int)std::string::npos;)
  {
    ++i;
    if (i < (int)newText.size() &&
        (newText[i] == '\\' || newText[i] == '<' || newText[i] == '>' ||
         newText[i] == 'n'  || newText[i] == 't' || newText[i] == 'r' ||
         newText[i] == '\''))
      ++i;
    else
    {
      newText.insert(i-1, "\\");
      ++i;
    }
  }
  for (i = 0; (i = newText.find("\"", i)) != (int)std::string::npos;)
  {
    newText.insert(i, "\\");
    i += 2;
  }

  // if key exist update state
  if (mcDb.locateKey(iLineNo, sSourceFile, sKey, newText, false))
  {
    mcDb.mcENUSlist[mcDb.miCurENUSinx].meState = l10nMem::ENTRY_NORMAL;
  }
  else
  {
    if (bMustExist)
      throw l10nMem::showError("key " + sKey + " does not exist");
    else
      // add key, if changed text, this is wrong but handled in reorganize
      mcDb.addKey(iLineNo, sSourceFile, sKey, newText, l10nMem::ENTRY_ADDED);
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::saveTemplates(l10nMem& cMem, const std::string& sTargetDir, bool bKid, bool bForce)
{
  int iE, iEsize  = mcDb.mcENUSlist.size();
  std::string sFileName = msModuleName + ".pot";

  // Dummy to satisfy compiler
  if (bKid)
    return;

  // and reorganize db if needed
   mcDb.miCurFileInx = 0;
   mcDb.reorganize(false);

  // no save if there has been errors
  if(!needWrite(sFileName, bForce))
    return;

  //JIX save HANDLE KID

  // Save en-US
  convert_gen savePo(cMem, sTargetDir, sTargetDir, sFileName);

  savePo.startSave("templates/", sFileName);
  for (iE = 1; iE < iEsize; ++iE)
  {
    l10nMem_enus_entry& cE = mcDb.mcENUSlist[iE];

    // remove deleted entries
    if (cE.meState == l10nMem::ENTRY_DELETED)
      continue;

    savePo.save(mcDb.mcFileList[cE.miFileInx].msFileName, cE.msKey, cE.msMsgId, "", false);
  }
  savePo.endSave();
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::saveLanguages(l10nMem& cMem, const std::string& sTargetDir, bool bForce)
{
  int iE, iEsize  = mcDb.mcENUSlist.size();
  int iL, iLsize  = mcDb.mcLangList.size();
  std::string sFileName = msModuleName + ".po";


  // and reorganize db if needed
   mcDb.miCurFileInx = 0;
   mcDb.reorganize(true);

  // no save if there has been errors
  if(!needWrite(sFileName, bForce))
    return;

  // save all languages
  for (iL = 1; iL < iLsize; ++iL)
  {
    // only save language file if modified
    if (!mcDb.mcLangList[iL].mbChanged)
      continue;

    mcDb.mcLangList[iL].mbChanged = false;
    convert_gen savePo(cMem, sTargetDir, sTargetDir, sFileName);

    savePo.startSave(mcDb.mcLangList[iL].msName + "/", sFileName);
    for (iE = 1; iE < iEsize; ++iE)
    {
      l10nMem_enus_entry& cE = mcDb.mcENUSlist[iE];
      l10nMem_lang_entry& cL = cE.mcLangText[iL];
      bool                bF = cL.mbFuzzy;

      // remove deleted entries
      if (cE.meState == l10nMem::ENTRY_DELETED)
        continue;

      savePo.save(mcDb.mcFileList[cE.miFileInx].msFileName, cE.msKey, cE.msMsgId, cL.msMsgStr, bF);
    }
    savePo.endSave();
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::showNOconvert ()
{
  int iE, iEsize  = mcDb.mcENUSlist.size();

  for (iE = 1; iE < iEsize; ++iE)
  {
    l10nMem_enus_entry& cE = mcDb.mcENUSlist[iE];

    if (cE.meState == l10nMem::ENTRY_DELETED)
    {
      showError("template key(" + cE.msKey + ") msgId(" + cE.msMsgId + ") not in pot file", 0);
    }
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::dumpMem(const std::string& sFileName)
{
  // and reorganize db if needed
  mcDb.reorganize(false);

  // no save if there has been errors
  if(!needWrite(sFileName, true))
    return;

  // JIX (dumpMem)
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::formatAndShowText(const std::string& sType, int iLineNo, const std::string& sText)
{
  std::cout << sType;
  if (mcDb.miCurFileInx > 0)
    std::cout << " in " << mcDb.mcFileList[mcDb.miCurFileInx].msFileName;
  if (iLineNo)
    std::cout << "(" << iLineNo << ")";
  std::cout << ":  " << sText << std::endl;
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool l10nMem_impl::needWrite(const std::string sFileName, bool bForce)
{
  int iE, iEsize  = mcDb.mcENUSlist.size();
  int iCntDeleted = 0, iCntChanged = 0, iCntAdded = 0;

  // no save if there has been errors
  if (mbInError)
    throw l10nMem::showError("Cannot save due to previous errors");

  // Check number of changes
  for (iE = 1; iE < iEsize; ++iE)
  {
    l10nMem_enus_entry& cur = mcDb.mcENUSlist[iE];
    if (cur.meState == l10nMem::ENTRY_ADDED)
      ++iCntAdded;
    if (cur.meState == l10nMem::ENTRY_CHANGED)
    {
      ++iCntChanged;
      if (mcDb.mbStrictMode)
        cur.meState = l10nMem::ENTRY_NORMAL;
    }
    if (cur.meState == l10nMem::ENTRY_DELETED)
      ++iCntDeleted;
  }
  if (!mcDb.mbConvertMode)
    iCntDeleted -= iCntChanged;
  if (!iCntAdded && !iCntChanged && !iCntDeleted)
  {
    std::cout << "genLang: No changes in " <<   sFileName;
    if (bForce)
      std::cout << ", -o switch used, so files are saved" << std::endl;
    else
      std::cout << " skipping \"save\"" << std::endl;
    return bForce;
  }

  std::cout << "genLang statistics: " << iCntDeleted << " deleted, "
                                      << iCntChanged << " changed, "
                                      << iCntAdded   << " added entries in "
                                      << sFileName   << std::endl;
  return true;
}



/**********************   I M P L E M E N T A T I O N   **********************/
bool l10nMem_impl::convFilterWarning(const std::string& sSourceFile,
                                     const std::string& sKey,
                                     const std::string& sMsgId)
{
  // silent ignore deleted messages
  if (sMsgId == "-" || sMsgId == "")
    return true;

  if (msModuleName == "help_sbasic")
  {
    if (sSourceFile == "sbasic.tree")
      return true;
  }
  if (msModuleName == "help_scalc")
  {
    if (sSourceFile == "scalc.tree")
      return true;
  }
  if (msModuleName == "help_schart")
  {
    if (sSourceFile == "schart.tree")
      return true;
  }
  if (msModuleName == "help_shared")
  {
    if (sSourceFile == "shared.tree")
      return true;
  }
  if (msModuleName == "help_simpress")
  {
    if (sSourceFile == "simpress.tree")
      return true;
  }
  if (msModuleName == "help_smath")
  {
    if (sSourceFile == "smath.tree")
      return true;
  }
  if (msModuleName == "help_swriter")
  {
    if (sSourceFile == "swriter.tree")
      return true;
  }
  if (msModuleName == "officecfg")
  {
    if (sSourceFile == "registry/data/org/openoffice/Office/Writer.xcu" && sKey == "Writer.Insert.Caption.CaptionOrderNumberingFirst")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/Writer.xcu" && sKey == "Writer.Layout.Other.TabStop")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/UI/WriterCommands.xcu" && sKey == "WriterCommands.UserInterface.Commands..uno:FlipVertical.Label")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/UI/WriterCommands.xcu" && sKey == "WriterCommands.UserInterface.Commands..uno:FlipHorizontal.Label")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/Common.xcu" && sKey == "Common.View.Localisation.AutoMnemonic")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/Common.xcu" && sKey == "Common.View.Localisation.DialogScale")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" && sKey == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/SlideTransitions.UIName")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" && sKey == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/CustomAnimations.UIName")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" && sKey == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/MasterPages.UIName")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" && sKey == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/Layouts.UIName")
      return true;
    if (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" && sKey == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/TableDesign.UIName")
      return true;
  }
  if (msModuleName == "readlicense_oo")
  {
    if (sSourceFile == "docs/readme/readme.xrm")
    {
      if (sKey == "BDB11")
        return true;
      if (sKey == "BDB2a")
        return true;
      if (sKey == "BDB3a")
        return true;
      if (sKey == "BDB4a")
        return true;
    }
  }
  if (msModuleName == "scp2")
  {
    if (sSourceFile == "source/binfilter/module_binfilter.ulf")
      return true;
    if (sSourceFile == "source/binfilter/registryitem_binfilter.ulf")
      return true;
  }
  if (msModuleName == "sdext")
  {
    if (sSourceFile == "source/minimizer/registry/data/org/openoffice/Office/Addons.xcu")
      return true;
    if (sSourceFile == "source/minimizer/registry/data/org/openoffice/Office/extension/SunPresentationMinimizer.xcu")
      return true;
    if (sSourceFile == "source/presenter/help/en-US/com.sun.PresenterScreen/presenter.xhp")
      return true;
    if (sSourceFile == "source/presenter/registry/data/org/openoffice/Office/extension/PresenterScreen.xcu")
      return true;
  }
  if (msModuleName == "sd")
  {
    if (sSourceFile == "source/ui/dlg/celltempl.src")
      return true;
  }
  if (msModuleName == "svx")
  {
    if (sSourceFile == "source/dialog/fontwork.src")
    {
      if (sKey == "RID_SVXSTR_FONTWORK_FORM1.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM2.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM3.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM4.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM5.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM6.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM7.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM8.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM9.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM10.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM11.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_FORM12.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_FONTWORK_UNDOCREATE.String.Text")
        return true;
    }
    if (sSourceFile == "source/dialog/sdstring.src")
    {
      if (sKey == "RID_SVXSTR_LIGHTGREY.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_LIGHTBLUE.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_LIGHTGREEN.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_LIGHTCYAN.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_LIGHTRED.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_LIGHTMAGENTA.String.Text")
        return true;
      if (sKey == "RID_SVXSTR_COLOR_SUN.String.Text")
        return true;
    }
    if (sSourceFile == "source/svdraw/svdstr.src" && sKey == "SIP_XA_FORMTXTSTDFORM.String.Text")
        return true;
  }
  return false;
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::convEntryKey(int                iLineNo,
                               const std::string& sSourceFile,
                               const std::string& sKey,
                               const std::string& sMsgId,
                               const std::string& sMsgStr,
                               bool               bIsFuzzy)
{
  std::vector<int> ivEntryList;
  std::string      curFileName;
  std::string      curKeyUpper;
  int              curFileIndex, curENUSindex, i, iSize;


  // filter out dummy messages, silently
  if (convFilterWarning(sSourceFile, sKey, sMsgId))
    return;

  // check for directory names in file name
  i = sSourceFile.rfind("/");
  if (i > 0)
    curFileName = sSourceFile.substr(i+1);
  else
    curFileName = sSourceFile;

  // Find all matching file names (old system does not have directory.
  // build list of potential entries
  iSize = mcDb.mcFileList.size();
  for (curFileIndex = 1; curFileIndex < iSize; ++curFileIndex)
    if (curFileName == mcDb.mcFileList[curFileIndex].msPureName)
    {
      int j    = mcDb.mcFileList[curFileIndex].miStart;
      int iEnd = mcDb.mcFileList[curFileIndex].miEnd;

      for (; j <= iEnd; ++j)
        ivEntryList.push_back(j);
    }

  // Did we find one or more files ?
  iSize = ivEntryList.size();
  if (!iSize)
  {
    showWarning("source file(" + curFileName + ") not in template file", iLineNo);
    return;
  }

  // Loop through all potential en-US entries
  for (curENUSindex = -1, i = 0; i < iSize; ++i)
  {
    l10nMem_enus_entry& curE = mcDb.mcENUSlist[ivEntryList[i]];


    // The entry must be unconverted if strict mode (comparing .pot files)
    if (mcDb.mbStrictMode && curE.meState != l10nMem::ENTRY_DELETED)
      continue;

    // msgId must match
    if (sMsgId == curE.msMsgId)
    {
      curENUSindex = ivEntryList[i];
      if (curE.meState == l10nMem::ENTRY_DELETED)
        curE.meState = l10nMem::ENTRY_NORMAL;
      break;
    }
  }

  // do we need to do advanced find
  if (curENUSindex == -1 || mcDb.mbStrictMode)
  {
    // make copy of key in upper case
    curKeyUpper = sKey;
    l10nMem_db::keyToUpper(curKeyUpper);

    // Loop through all potential en-US entries
    for (i = 0; i < iSize; ++i)
    {
      l10nMem_enus_entry& curE = mcDb.mcENUSlist[ivEntryList[i]];

      // compare keys, but be aware of different length
      if (curKeyUpper.find(curE.msKey) != (int)std::string::npos)
      {
        curENUSindex = ivEntryList[i];
        bIsFuzzy     = true;
        break;
      }
    }
  }

  if (curENUSindex == -1)
  {
    showWarning("file(" + curFileName + ") key(" + sKey + ") msgId(" + sMsgId + ") not found in templates", iLineNo);
    return;
  }

  // update language text
  l10nMem_enus_entry& curE = mcDb.mcENUSlist[curENUSindex];
  l10nMem_lang_entry& curL = curE.mcLangText[mcDb.miCurLangInx];

  if (sMsgStr != curL.msMsgStr)
  {
    curL.msMsgStr = sMsgStr;
    curL.mbFuzzy  = bIsFuzzy;
    curE.meState  = l10nMem::ENTRY_CHANGED;
    mcDb.mcLangList[mcDb.miCurLangInx].mbChanged = true;
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::convertToInetString(std::string& sText)
{
static const char *replacingStr[] = {"&",     "\'",     ">",     "<",   "\"",     NULL };
static const int   replacingLen[] = {1,       1,        1,      1,      1,        0    };
static const char *newStr[]       = {"&amp;", "&apos;", "&gt;", "&lt;", "&quot;", NULL };
static const int   newLen[]       = {5,       6,        4,      4,      6,        0    };
  int i, pos;

  for (i = 0; replacingStr[i]; i++)
  {
    pos = 0;
    while((pos = sText.find(replacingStr[i], pos)) != std::string::npos) {
         sText.replace(pos, replacingLen[i], newStr[i]);
         pos += newLen[i];
    }
  }
}



/**********************   I M P L E M E N T A T I O N   **********************/
void l10nMem_impl::convertFromInetString(std::string& sText)
{
static const char *replacingStr[] = {"&amp;", "&apos;", "&gt;", "&lt;", "&quot;", NULL };
static const int   replacingLen[] = {5,       6,        4,      4,      6,        0    };
static const char *newStr[]       = {"&",     "\'",     ">",     "<",   "\"",     NULL };
static const int   newLen[]       = {1,       1,        1,      1,      1,        0    };
  int i, pos;

  for (i = 0; replacingStr[i]; i++)
  {
    pos = 0;
    while((pos = sText.find(replacingStr[i], pos)) != std::string::npos) {
         sText.replace(pos, replacingLen[i], newStr[i]);
         pos += newLen[i];
    }
  }
}
