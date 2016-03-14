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

l10nMem *myMem;



l10nMem::l10nMem()
                          : mbVerbose(false),
                            mbDebug(false),
                            mbInError(false)
{
    mcDb  = new l10nMem_db;
    myMem = this;
}



l10nMem::~l10nMem()
{
    delete mcDb;
}



int l10nMem::showError(const std::string& sText, int iLineNo)
{
    myMem->mbInError = true;
    myMem->formatAndShowText("ERROR", iLineNo, sText);
    return 1;
}



void l10nMem::showWarning(const std::string& sText, int iLineNo)
{
    myMem->formatAndShowText("WARNING", iLineNo, sText);
}



void l10nMem::showDebug(const std::string& sText, int iLineNo)
{
    if (myMem->mbDebug)
        myMem->formatAndShowText("DEBUG", iLineNo, sText);
}



void l10nMem::showVerbose(const std::string& sText, int iLineNo)
{
  if (myMem->mbVerbose)
    myMem->formatAndShowText("INFO", iLineNo, sText);
}



bool l10nMem::isError()
{
    return myMem->mbInError;
}



void l10nMem::setModuleName(const std::string& sModuleName)
{
    msModuleName = sModuleName;
}



const std::string& l10nMem::getModuleName()
{
    return msModuleName;
}



void l10nMem::setLanguage(const std::string& sLanguage,
                          bool               bCreate)
{
    mcDb->setLanguage(sLanguage, bCreate);
}



void l10nMem::setConvert(bool bConvert,
                         bool bStrict)
{
    mcDb->setConvert(bConvert, bStrict);
}



void l10nMem::loadEntryKey(int                iLineNo,
                          const std::string& sSourceFile,
                          const std::string& sKey,
                          const std::string& sMsgId,
                          const std::string& sMsgStr,
                          bool               bIsFuzzy)
{
    if (mcDb->mbConvertMode)
        convEntryKey(iLineNo, sSourceFile, sKey, sMsgId, sMsgStr, bIsFuzzy);
    else if (!mcDb->miCurLangInx)
        mcDb->loadENUSkey(iLineNo, sSourceFile, sKey, sMsgId);
    else
        mcDb->loadLangKey(iLineNo, sSourceFile, sKey, sMsgId, sMsgStr, bIsFuzzy);
}



void l10nMem::setSourceKey(int                iLineNo,
                           const std::string& sSourceFile,
                           const std::string& sKey,
                           const std::string& sMsgId,
                           bool               bMustExist)
{
    std::string newText(sMsgId);
    int         i;

    // time to escape " and \ if contained in text or key
    for (i = 0; (i = newText.find("\\", i)) != (int)std::string::npos;) {
        ++i;
        if (i < (int)newText.size() &&
            (newText[i] == '\\' || newText[i] == '<' || newText[i] == '>' ||
            newText[i] == 'n' || newText[i] == 't' || newText[i] == 'r' ||
            newText[i] == '\''))
            ++i;
        else {
            newText.insert(i - 1, "\\");
            ++i;
        }
    }
    for (i = 0; (i = newText.find("\"", i)) != (int)std::string::npos;) {
        newText.insert(i, "\\");
        i += 2;
    }

    // if key exist update state
    if (mcDb->locateKey(iLineNo, sSourceFile, sKey, newText, false))
        mcDb->mcENUSlist[mcDb->miCurENUSinx].meState = ENTRY_NORMAL;
    else {
        if (bMustExist)
            throw showError("key " + sKey + " does not exist");

        // add key, if changed text, this is wrong but handled in reorganize
        mcDb->addKey(iLineNo, sSourceFile, sKey, newText, ENTRY_ADDED);
    }
}



void l10nMem::saveTemplates(const std::string& sTargetDir, bool bKid, bool bForce)
{
//    int iEsize = mcDb->mcENUSlist.size();
    std::string sFileName = msModuleName + ".pot";

    // Dummy to satisfy compiler
    if (bKid)
        return;
    showError(sTargetDir);

    // and reorganize db if needed
    mcDb->miCurFileInx = 0;
    mcDb->reorganize(false);

    // no save if there has been errors
    if (!needWrite(sFileName, bForce))
        return;

    //JIX save HANDLE KID

    // Save en-US
#if 0
    convert_gen savePo(cMem, sTargetDir, sTargetDir, sFileName);

    savePo.startSave("templates/", sFileName);
    for (iE = 1; iE < iEsize; ++iE) {
        l10nMem_enus_entry& cE = mcDb->mcENUSlist[iE];

        // remove deleted entries
        if (cE.meState == ENTRY_DELETED)
            continue;

        savePo.save(mcDb->mcFileList[cE.miFileInx].msFileName, cE.msKey, cE.msMsgId, "", false);
    }
    savePo.endSave();
#endif
}



void l10nMem::saveLanguages(l10nMem& cMem, const std::string& sTargetDir, bool bForce)
{
    int iE, iEsize = mcDb->mcENUSlist.size();
    int iL, iLsize = mcDb->mcLangList.size();
    std::string sFileName = msModuleName + ".po";


    // and reorganize db if needed
    mcDb->miCurFileInx = 0;
    mcDb->reorganize(true);

    // no save if there has been errors
    if (!needWrite(sFileName, bForce))
        return;

    // save all languages
    for (iL = 1; iL < iLsize; ++iL) {
        // only save language file if modified
        if (!mcDb->mcLangList[iL].mbChanged)
            continue;

        mcDb->mcLangList[iL].mbChanged = false;
        convert_gen savePo(cMem, sTargetDir, sTargetDir, sFileName);

        savePo.startSave(mcDb->mcLangList[iL].msName + "/", sFileName);
        for (iE = 1; iE < iEsize; ++iE) {
            l10nMem_enus_entry& cE = mcDb->mcENUSlist[iE];
            l10nMem_lang_entry& cL = cE.mcLangText[iL];
            bool                bF = cL.mbFuzzy;

            // remove deleted entries
            if (cE.meState == ENTRY_DELETED)
                continue;

            savePo.save(mcDb->mcFileList[cE.miFileInx].msFileName, cE.msKey, cE.msMsgId, cL.msMsgStr, bF);
        }
        savePo.endSave();
    }
}



void l10nMem::showNOconvert()
{
    int iE, iEsize = mcDb->mcENUSlist.size();

    for (iE = 1; iE < iEsize; ++iE) {
        l10nMem_enus_entry& cE = mcDb->mcENUSlist[iE];

        if (cE.meState == ENTRY_DELETED)
            showError("template key(" + cE.msKey + ") msgId(" + cE.msMsgId + ") not in pot file", 0);
    }
}



void l10nMem::convertToInetString(std::string& sText)
{
    static const char *replacingStr[] = { "&", "\'", ">", "<", "\"", nullptr };
    static const int   replacingLen[] = { 1, 1, 1, 1, 1, 0 };
    static const char *newStr[] = { "&amp;", "&apos;", "&gt;", "&lt;", "&quot;", nullptr };
    static const int   newLen[] = { 5, 6, 4, 4, 6, 0 };
    int i, pos;

    for (i = 0; replacingStr[i]; i++) {
        pos = 0;
        while ((pos = sText.find(replacingStr[i], pos)) != (int)std::string::npos) {
            sText.replace(pos, replacingLen[i], newStr[i]);
            pos += newLen[i];
        }
    }
}



void l10nMem::convertFromInetString(std::string& sText)
{
    static const char *replacingStr[] = { "&amp;", "&apos;", "&gt;", "&lt;", "&quot;", nullptr };
    static const int   replacingLen[] = { 5, 6, 4, 4, 6, 0 };
    static const char *newStr[] = { "&", "\'", ">", "<", "\"", nullptr };
    static const int   newLen[] = { 1, 1, 1, 1, 1, 0 };
    int i, pos;

    for (i = 0; replacingStr[i]; i++) {
        pos = 0;
        while ((pos = sText.find(replacingStr[i], pos)) != (int)std::string::npos) {
            sText.replace(pos, replacingLen[i], newStr[i]);
            pos += newLen[i];
        }
    }
}



int  l10nMem::prepareMerge()
{
    return mcDb->prepareMerge();
}



void l10nMem::dumpMem(const std::string& sFileName)
{
    // and reorganize db if needed
    mcDb->reorganize(false);

    // no save if there has been errors
    if (!needWrite(sFileName, true))
        return;

    // JIX (dumpMem)
}



bool l10nMem::getMergeLang(std::string& sL, std::string& sT)
{
    return mcDb->getMergeLang(sL, sT);
}



void l10nMem::formatAndShowText(const std::string& sType, int iLineNo, const std::string& sText)
{
    std::cout << sType;
    if (mcDb->miCurFileInx > 0)
        std::cout << " in " << mcDb->mcFileList[mcDb->miCurFileInx].msFileName;
    if (iLineNo)
        std::cout << "(" << iLineNo << ")";
     std::cout << ":  " << sText << std::endl;
}



bool l10nMem::needWrite(const std::string sFileName, bool bForce)
{
    int iE, iEsize  = mcDb->mcENUSlist.size();
    int iCntDeleted = 0, iCntChanged = 0, iCntAdded = 0;

    // no save if there has been errors
    if (mbInError)
        throw showError("Cannot save due to previous errors");

    // Check number of changes
    for (iE = 1; iE < iEsize; ++iE) {
        l10nMem_enus_entry& cur = mcDb->mcENUSlist[iE];
        if (cur.meState == ENTRY_ADDED)
            ++iCntAdded;
        if (cur.meState == ENTRY_CHANGED) {
            ++iCntChanged;
            if (mcDb->mbStrictMode)
                cur.meState = ENTRY_NORMAL;
        }
        if (cur.meState == ENTRY_DELETED)
            ++iCntDeleted;
    }
    if (!mcDb->mbConvertMode)
        iCntDeleted -= iCntChanged;
    if (!iCntAdded && !iCntChanged && !iCntDeleted) {
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



bool l10nMem::convFilterWarning(const std::string& sSourceFile,
                                const std::string& sKey,
                                const std::string& sMsgId)
{
    // silent ignore deleted messages
    if (sMsgId == "-" || sMsgId == "")
        return true;
    if (msModuleName == "help_sbasic" && sSourceFile == "sbasic.tree")
        return true;
    if (msModuleName == "help_scalc" && sSourceFile == "scalc.tree")
        return true;
    if (msModuleName == "help_schart" && sSourceFile == "schart.tree")
        return true;
    if (msModuleName == "help_shared" && sSourceFile == "shared.tree")
        return true;
    if (msModuleName == "help_simpress" && sSourceFile == "simpress.tree")
        return true;
    if (msModuleName == "help_smath" && sSourceFile == "smath.tree")
        return true;
    if (msModuleName == "help_swriter" && sSourceFile == "swriter.tree")
        return true;
    if (msModuleName == "officecfg") {
        if ((sSourceFile == "registry/data/org/openoffice/Office/Writer.xcu" &&
             sKey        == "Writer.Insert.Caption.CaptionOrderNumberingFirst") ||
            (sSourceFile == "registry/data/org/openoffice/Office/Writer.xcu" &&
             sKey        == "Writer.Layout.Other.TabStop") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/WriterCommands.xcu" &&
             sKey        == "WriterCommands.UserInterface.Commands..uno:FlipVertical.Label") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/WriterCommands.xcu" &&
             sKey        == "WriterCommands.UserInterface.Commands..uno:FlipHorizontal.Label") ||
            (sSourceFile == "registry/data/org/openoffice/Office/Common.xcu" &&
             sKey        == "Common.View.Localisation.AutoMnemonic") ||
            (sSourceFile == "registry/data/org/openoffice/Office/Common.xcu" &&
             sKey        == "Common.View.Localisation.DialogScale") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/SlideTransitions.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/CustomAnimations.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/MasterPages.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/Layouts.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/TableDesign.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/TableDesign.UIName"))
            return true;
    }
    if (msModuleName == "readlicense_oo") {
        if (sSourceFile == "docs/readme/readme.xrm") {
            if ((sKey == "BDB11" || sKey == "BDB2a" ||
                 sKey == "BDB3a" || sKey == "BDB4a"))
                return true;
        }
    }
    if (msModuleName == "scp2") {
        if ((sSourceFile == "source/binfilter/module_binfilter.ulf" ||
             sSourceFile == "source/binfilter/registryitem_binfilter.ulf"))
            return true;
    }
    if (msModuleName == "sdext") {
        if (sSourceFile == "source/minimizer/registry/data/org/openoffice/Office/Addons.xcu" ||
            sSourceFile == "source/minimizer/registry/data/org/openoffice/Office/extension/SunPresentationMinimizer.xcu" ||
            sSourceFile == "source/presenter/help/en-US/com.sun.PresenterScreen/presenter.xhp" ||
            sSourceFile == "source/presenter/registry/data/org/openoffice/Office/extension/PresenterScreen.xcu")
            return true;
    }
    if (msModuleName == "sd") {
        if (sSourceFile == "source/ui/dlg/celltempl.src")
            return true;
    }
    if (msModuleName == "svx") {
        if (sSourceFile == "source/dialog/fontwork.src") {
            if (sKey == "RID_SVXSTR_FONTWORK_FORM1.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM2.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM3.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM4.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM5.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM6.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM7.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM8.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM9.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM10.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM11.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_FORM12.String.Text" ||
                sKey == "RID_SVXSTR_FONTWORK_UNDOCREATE.String.Text")
                return true;
        }
    }
    if (sSourceFile == "source/dialog/sdstring.src") {
        if (sKey == "RID_SVXSTR_LIGHTGREY.String.Text" ||
            sKey == "RID_SVXSTR_LIGHTBLUE.String.Text" ||
            sKey == "RID_SVXSTR_LIGHTGREEN.String.Text" ||
            sKey == "RID_SVXSTR_LIGHTCYAN.String.Text" ||
            sKey == "RID_SVXSTR_LIGHTRED.String.Text" ||
            sKey == "RID_SVXSTR_LIGHTMAGENTA.String.Text" ||
            sKey == "RID_SVXSTR_COLOR_SUN.String.Text")
            return true;
    }
    if (sSourceFile == "source/svdraw/svdstr.src" && sKey == "SIP_XA_FORMTXTSTDFORM.String.Text")
        return true;
    return false;
}



void l10nMem::convEntryKey(int                iLineNo,
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
    curFileName = (i > 0) ? sSourceFile.substr(i + 1) : sSourceFile;

    // Find all matching file names (old system does not have directory.
    // build list of potential entries
    iSize = mcDb->mcFileList.size();
    for (curFileIndex = 1; curFileIndex < iSize; ++curFileIndex)
        if (curFileName == mcDb->mcFileList[curFileIndex].msPureName) {
            int j    = mcDb->mcFileList[curFileIndex].miStart;
            int iEnd = mcDb->mcFileList[curFileIndex].miEnd;

            for (; j <= iEnd; ++j)
                ivEntryList.push_back(j);
        }

    // Did we find one or more files ?
    iSize = ivEntryList.size();
    if (!iSize) {
        showWarning("source file(" + curFileName + ") not in template file", iLineNo);
        return;
    }

    // Loop through all potential en-US entries
    for (curENUSindex = -1, i = 0; i < iSize; ++i) {
        l10nMem_enus_entry& curE = mcDb->mcENUSlist[ivEntryList[i]];

        // The entry must be unconverted if strict mode (comparing .pot files)
        if (mcDb->mbStrictMode && curE.meState != ENTRY_DELETED)
            continue;

        // msgId must match
        if (sMsgId == curE.msMsgId) {
            curENUSindex = ivEntryList[i];
            if (curE.meState == ENTRY_DELETED)
                curE.meState = ENTRY_NORMAL;
            break;
        }
    }

    // do we need to do advanced find
    if (curENUSindex == -1 || mcDb->mbStrictMode) {
        // make copy of key in upper case
        curKeyUpper = sKey;
        l10nMem_db::keyToUpper(curKeyUpper);

        // Loop through all potential en-US entries
        for (i = 0; i < iSize; ++i) {
            l10nMem_enus_entry& curE = mcDb->mcENUSlist[ivEntryList[i]];

            // compare keys, but be aware of different length
            if (curKeyUpper.find(curE.msKey) != std::string::npos) {
                curENUSindex = ivEntryList[i];
                bIsFuzzy     = true;
                break;
            }
        }
    }

    if (curENUSindex == -1) {
        showWarning("file(" + curFileName + ") key(" + sKey + ") msgId(" + sMsgId + ") not found in templates", iLineNo);
        return;
    }

    // update language text
    l10nMem_enus_entry& curE = mcDb->mcENUSlist[curENUSindex];
    l10nMem_lang_entry& curL = curE.mcLangText[mcDb->miCurLangInx];

    if (sMsgStr != curL.msMsgStr) {
        curL.msMsgStr = sMsgStr;
        curL.mbFuzzy  = bIsFuzzy;
        curE.meState  = ENTRY_CHANGED;
        mcDb->mcLangList[mcDb->miCurLangInx].mbChanged = true;
    }
}
