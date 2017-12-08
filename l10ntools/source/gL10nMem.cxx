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
#include <iostream>
#include <fstream>
#include <sstream>
#include <rtl/character.hxx>
using namespace std;

#include "gL10nMem.hxx"
#include "gConvPo.hxx"



l10nMem *myMem;


class l10nMem_lang_list_entry
{
    public:
        explicit l10nMem_lang_list_entry(const string& sName)
                               : msName(sName),
                                 mbChanged(false)
        {}

        ~l10nMem_lang_list_entry() {};

        string msName;      // language Name
        bool        mbChanged;   // used for "convert", true if language is modified
};


class l10nMem_file_entry
{
    public:
        l10nMem_file_entry(const string& sFileName, int iStart)
                          : msFileName(sFileName),
                            miStart(iStart),
                            miEnd(iStart)
        {
            // Store fileName without relative path
            int i = msFileName.rfind("/");
            if (i == (int)string::npos)
                msPureName = msFileName;
            else
                msPureName = msFileName.substr(i + 1);
        }
        ~l10nMem_file_entry() {};

        string msFileName;  // file Name with relative path
        string msPureName;  // just filename
        int         miStart;     // start index of entries in mcMasterEntries (l10Mem_db::mcENUS)
        int         miEnd;       // last index of entries in mcMasterEntries (l10Mem_db::mcENUS)
};



class l10nMem_lang_entry
{
    public:
        l10nMem_lang_entry(const string& sMsgStr, bool bFuzzy)
                          : msMsgStr(sMsgStr),
                            mbFuzzy(bFuzzy)
        {}
        ~l10nMem_lang_entry() {};

        string msMsgStr;   // translated text from po file
        bool        mbFuzzy;    // fuzzy flag
};



class l10nMem_enus_entry
{
    public:
        l10nMem_enus_entry(const string&   sKey,
                           const string&   sMsgId,
                           const string&   sComment,
                           const string&   sResource,
                           const string&   sGroup,
                           int                  iLineNo,
                           int                  iFileInx,
                           int                  iLangSize,
                           l10nMem::ENTRY_STATE eState)
                          : msMsgId(sMsgId),
                            msComment(sComment),
                            msResource(sResource),
                            msGroup(sGroup),
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
            //FIX l10nMem::keyToUpper(msKey);
        }

        ~l10nMem_enus_entry() {};

        string                     msKey;      // key in po file and source file
        string                     msMsgId;    // en-US text from source file
        string                     msComment;  // Comment (to be used in msgcstr)
        string                     msResource; // Resource Id (to be used in msgcstr)
        string                     msGroup;    // Group Id (to be used in msgcstr)
        l10nMem::ENTRY_STATE       meState;    // status information
        int                        miFileInx;  // index of file name
        int                        miLineNo;   // line number
        vector<l10nMem_lang_entry> mcLangText; // language texts (index is languageId)
};



l10nMem::l10nMem()
                : mbVerbose(false),
                  mbDebug(false),
                  mbInError(false),
                  miCurFileInx(0),
                  miCurLangInx(0),
                  miCurENUSinx(0),
                  mbConvertMode(false),
                  mbStrictMode(false)
{
    myMem = this;
    msModuleName   = "default";
    mcFileList.push_back(l10nMem_file_entry("-genLang-", 0));
    mcLangList.push_back(l10nMem_lang_list_entry("-genLang-"));
    mcENUSlist.push_back(l10nMem_enus_entry("-genLang-", "-genLang-", "", "", "", 0, 0, 0, l10nMem::ENTRY_DELETED));
}



l10nMem::~l10nMem()
{
}



int l10nMem::showError(const string& sText, int iLineNo)
{
    myMem->mbInError = true;
    myMem->formatAndShowText("ERROR", iLineNo, sText);
    return 1;
}



void l10nMem::showWarning(const string& sText, int iLineNo)
{
    myMem->formatAndShowText("WARNING", iLineNo, sText);
}



void l10nMem::showDebug(const string& sText, int iLineNo)
{
    if (myMem->mbDebug)
        myMem->formatAndShowText("DEBUG", iLineNo, sText);
}



void l10nMem::showVerbose(const string& sText, int iLineNo)
{
    if (myMem->mbVerbose)
        myMem->formatAndShowText("INFO", iLineNo, sText);
}



bool l10nMem::isError()
{
    return myMem->mbInError;
}



void l10nMem::setModuleName(const string& sModuleName)
{
    msModuleName = sModuleName;
}



const string& l10nMem::getModuleName()
{
    return msModuleName;
}



void l10nMem::setLanguage(const string& sLanguage,
                          bool               bCreate)
{
    int i, iSize;

    // regular load or convert of old po files
    miCurFileInx = 0;

    // With no languages selected only en-US is generated
    if (!sLanguage.size()) {
        miCurLangInx = 0;
        return;
    }

    // en-US is loaded as master and cannot be loaded again
    if (sLanguage == "en-US")
        throw l10nMem::showError("en-US is loaded automatically");

    // check if language is already loaded
    iSize = mcLangList.size();
    for (miCurLangInx = 0; miCurLangInx < iSize && mcLangList[miCurLangInx].msName != sLanguage; ++miCurLangInx);
    if (miCurLangInx < iSize) {
        if (bCreate)
            throw showError("loading " + sLanguage + " twice");
        return;
    }

    // language does not exist in db
    if (!bCreate)
        throw showError("language " + sLanguage + " not loaded");

    // create language
    mcLangList.push_back(l10nMem_lang_list_entry(sLanguage));

    // add language to all ENUS entries
    iSize = mcENUSlist.size();
    for (i = 0; i < iSize; ++i)
        mcENUSlist[i].mcLangText.push_back(l10nMem_lang_entry("", false));
}



void l10nMem::setConvert(bool bConvert,
                         bool bStrict)
{
    // regular load or convert of old po files
    mbConvertMode = bConvert;
    mbStrictMode = bStrict;
}



void l10nMem::setVerbose(bool doVerbose)
{
    mbVerbose = doVerbose;
}



void l10nMem::setDebug(bool doDebug)
{
    mbDebug = doDebug;
}



void l10nMem::loadEntryKey(int           iLineNo,
                           const string& sSourceFile,
                           const string& sKey,
                           const string& sMsgId,
                           const string& sMsgStr,
                           const string& sComment,
                           const string& sResource,
                           const string& sGroup,
                           bool          bIsFuzzy)
{
    if (mbConvertMode)
        convEntryKey(iLineNo, sSourceFile, sKey, sMsgId, sMsgStr, bIsFuzzy);
    else if (!miCurLangInx)
        loadENUSkey(iLineNo, sSourceFile, sKey, sMsgId, sComment, sResource, sGroup);
    else
        loadLangKey(iLineNo, sSourceFile, sKey, sMsgId, sMsgStr, bIsFuzzy);
}



void l10nMem::setSourceKey(int           iLineNo,
                           const string& sSourceFile,
                           const string& sKey,
                           const string& sMsgId,
                           const string& sComment,
                           const string& sResource,
                           const string& sGroup,
                           bool          bMustExist)
{
    string newText(sMsgId);
    int         i;

    // time to escape " and \ if contained in text or key
    for (i = 0; (i = newText.find("\\", i)) != (int)string::npos;) {
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
    for (i = 0; (i = newText.find("\"", i)) != (int)string::npos;) {
        newText.insert(i, "\\");
        i += 2;
    }

    // if key exist update state
    if (locateKey(iLineNo, sSourceFile, sKey, newText, false))
        mcENUSlist[miCurENUSinx].meState = ENTRY_NORMAL;
    else {
        if (bMustExist)
            throw showError("key " + sKey + " does not exist");

        // add key, if changed text, this is wrong but handled in reorganize
        addKey(iLineNo, sSourceFile, sKey, newText, sComment, sResource, sGroup, ENTRY_ADDED);
    }
}



void l10nMem::saveTemplates(const string& sTargetDir, bool bForce)
{
    string target(msModuleName + ".pot");
    int iE, iEsize = mcENUSlist.size();

    // and reorganize db if needed
    miCurFileInx = 0;
    reorganize(false);

    // no save if there has been errors
    if (!needWrite(target, bForce))
        return;

    // Save en-US
    convert_po savePo(*this);

    savePo.startSave(msModuleName, sTargetDir, target);
    for (iE = 1; iE < iEsize; ++iE) {
        l10nMem_enus_entry& cE = mcENUSlist[iE];

        // remove deleted entries
        if (cE.meState == ENTRY_DELETED)
            continue;

        savePo.save(mcFileList[cE.miFileInx].msFileName, cE.msKey, cE.msMsgId, "", cE.msComment, cE.msResource, cE.msGroup, false);
    }
    savePo.endSave();
}



void l10nMem::saveLanguages(l10nMem& cMem, const string& sTargetDir, bool bForce)
{
    //    int iE, iEsize = mcENUSlist.size();
    //    int iEsize = mcENUSlist.size();
    int iL, iLsize = mcLangList.size();
    string sFileName = msModuleName + ".po";

    cMem.dumpMem("jan");
    showDebug(sTargetDir);
    if (bForce)
        showDebug("debug");

    // and reorganize db if needed
    miCurFileInx = 0;
    reorganize(true);

    // no save if there has been errors
    if (!needWrite(sFileName, bForce))
        return;

    // save all languages
    for (iL = 1; iL < iLsize; ++iL) {
        // only save language file if modified
        if (!mcLangList[iL].mbChanged)
            continue;

        mcLangList[iL].mbChanged = false;
#if 0
        convert_gen savePo(cMem, sTargetDir, sTargetDir, sFileName);

        savePo.startSave(mcLangList[iL].msName + "/", sFileName);
        for (iE = 1; iE < iEsize; ++iE) {
            l10nMem_enus_entry& cE = mcENUSlist[iE];
            l10nMem_lang_entry& cL = cE.mcLangText[iL];
            bool                bF = cL.mbFuzzy;

            // remove deleted entries
            if (cE.meState == ENTRY_DELETED)
                continue;

            savePo.save(mcFileList[cE.miFileInx].msFileName, cE.msKey, cE.msMsgId, cL.msMsgStr, bF);
        }
        savePo.endSave();
#endif
    }
}



void l10nMem::showNOconvert()
{
    int iE, iEsize = mcENUSlist.size();

    for (iE = 1; iE < iEsize; ++iE) {
        l10nMem_enus_entry& cE = mcENUSlist[iE];

        if (cE.meState == ENTRY_DELETED)
            showError("template key(" + cE.msKey + ") msgId(" + cE.msMsgId + ") not in pot file", 0);
    }
}



void l10nMem::convertToInetString(string& sText)
{
    static const char *replacingStr[] = { "&", "\'", ">", "<", "\"", nullptr };
    static const int   replacingLen[] = { 1, 1, 1, 1, 1, 0 };
    static const char *newStr[] = { "&amp;", "&apos;", "&gt;", "&lt;", "&quot;", nullptr };
    static const int   newLen[] = { 5, 6, 4, 4, 6, 0 };
    int pos;

    for (int i = 0; replacingStr[i]; i++) {
        pos = 0;
        while ((pos = sText.find(replacingStr[i], pos)) != (int)string::npos) {
            sText.replace(pos, replacingLen[i], newStr[i]);
            pos += newLen[i];
        }
    }
}



void l10nMem::convertFromInetString(string& sText)
{
    static const char *replacingStr[] = { "&amp;", "&apos;", "&gt;", "&lt;", "&quot;", nullptr };
    static const int   replacingLen[] = { 5, 6, 4, 4, 6, 0 };
    static const char *newStr[] = { "&", "\'", ">", "<", "\"", nullptr };
    static const int   newLen[] = { 1, 1, 1, 1, 1, 0 };
    int i, pos;

    for (i = 0; replacingStr[i]; i++) {
        pos = 0;
        while ((pos = sText.find(replacingStr[i], pos)) != (int)string::npos) {
            sText.replace(pos, replacingLen[i], newStr[i]);
            pos += newLen[i];
        }
    }
}



int  l10nMem::prepareMerge()
{
    miCurLangInx = 0;
    return mcLangList.size();
}



void l10nMem::dumpMem(const string& sFileName)
{
    // and reorganize db if needed
    reorganize(false);

    // no save if there has been errors
    if (!needWrite(sFileName, true))
        return;

    // JIX (dumpMem)
}



bool l10nMem::getMergeLang(string& sLang, string& sMsgStr)
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



void l10nMem::formatAndShowText(const string& sType, int iLineNo, const string& sText)
{
    cout << sType;
    if (miCurFileInx > 0)
        cout << " in " << mcFileList[miCurFileInx].msFileName;
    if (iLineNo)
        cout << "(" << iLineNo << ")";
     cout << ":  " << sText << endl;
}



bool l10nMem::needWrite(const string sFileName, bool bForce)
{
    int iE, iEsize  = mcENUSlist.size();
    int iCntDeleted = 0, iCntChanged = 0, iCntAdded = 0;

    // no save if there has been errors
    if (mbInError) {
        if (bForce)
            showError("forced save, due to errors");
        else
            throw showError("Cannot save due to previous errors");
    }
    // Check number of changes
    for (iE = 1; iE < iEsize; ++iE) {
        l10nMem_enus_entry& cur = mcENUSlist[iE];
        if (cur.meState == ENTRY_ADDED)
            ++iCntAdded;
        if (cur.meState == ENTRY_CHANGED) {
            ++iCntChanged;
            if (mbStrictMode)
                cur.meState = ENTRY_NORMAL;
        }
        if (cur.meState == ENTRY_DELETED)
            ++iCntDeleted;
    }
    if (!mbConvertMode)
        iCntDeleted -= iCntChanged;
    if (!iCntAdded && !iCntChanged && !iCntDeleted) {
        cout << "genLang: No changes in " <<   sFileName;
        if (bForce)
            cout << ", -s switch used, so files are saved" << endl;
        else
            cout << " skipping \"save\"" << endl;
        return bForce;
    }

    cout << "genLang statistics: " << iCntDeleted << " deleted, "
                                        << iCntChanged << " changed, "
                                        << iCntAdded   << " added entries in "
                                        << sFileName   << endl;
    return true;
}



bool l10nMem::convFilterWarning(const string& sSourceFile,
                                const string& sKey,
                                const string& sMsgId)
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
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/SlideTransitions.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/CustomAnimations.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/MasterPages.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/Layouts.UIName") ||
            (sSourceFile == "registry/data/org/openoffice/Office/UI/ImpressWindowState.xcu" &&
             sKey        == "ImpressWindowState.UIElements.States.private:resource/toolpanel/DrawingFramework/TableDesign.UIName") )
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
    if (sSourceFile == "source/svdraw/svdstr.src" && sKey == "SIP_XA_FORMTXTSTDFORM.String.Text")
        return true;
    return false;
}



void l10nMem::convEntryKey(int           iLineNo,
                           const string& sSourceFile,
                           const string& sKey,
                           const string& sMsgId,
                           const string& sMsgStr,
                           bool          bIsFuzzy)
{
    vector<int> ivEntryList;
    string      curFileName;
    string      curKeyUpper;
    int              curFileIndex, curENUSindex, i, iSize;


    // filter out dummy messages, silently
    if (convFilterWarning(sSourceFile, sKey, sMsgId))
        return;

    // check for directory names in file name
    i = sSourceFile.rfind("/");
    curFileName = (i > 0) ? sSourceFile.substr(i + 1) : sSourceFile;

    // Find all matching file names (old system does not have directory.
    // build list of potential entries
    iSize = mcFileList.size();
    for (curFileIndex = 1; curFileIndex < iSize; ++curFileIndex)
        if (curFileName == mcFileList[curFileIndex].msPureName) {
            int j    = mcFileList[curFileIndex].miStart;
            int iEnd = mcFileList[curFileIndex].miEnd;

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
        l10nMem_enus_entry& curE = mcENUSlist[ivEntryList[i]];

        // The entry must be unconverted if strict mode (comparing .pot files)
        if (mbStrictMode && curE.meState != ENTRY_DELETED)
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
    if (curENUSindex == -1 || mbStrictMode) {
        // make copy of key in upper case
        curKeyUpper = sKey;
        keyToLower(curKeyUpper);

        // Loop through all potential en-US entries
        for (i = 0; i < iSize; ++i) {
            l10nMem_enus_entry& curE = mcENUSlist[ivEntryList[i]];

            // compare keys, but be aware of different length
            if (curKeyUpper.find(curE.msKey) != string::npos) {
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
    l10nMem_enus_entry& curE = mcENUSlist[curENUSindex];
    l10nMem_lang_entry& curL = curE.mcLangText[miCurLangInx];

    if (sMsgStr != curL.msMsgStr) {
        curL.msMsgStr = sMsgStr;
        curL.mbFuzzy  = bIsFuzzy;
        curE.meState  = ENTRY_CHANGED;
        mcLangList[miCurLangInx].mbChanged = true;
    }
}



void l10nMem::loadENUSkey(int           iLineNo,
                          const string& sSourceFile,
                          const string& sKey,
                          const string& sMsgId,
                          const string& sComment,
                          const string& sResource,
                          const string& sGroup)
{
    // add it to vector and update file pointer
    addKey(iLineNo, sSourceFile, sKey, sMsgId, sComment, sResource, sGroup, ENTRY_DELETED);
}



void l10nMem::loadLangKey(int           iLineNo,
                          const string& sSourceFile,
                          const string& sKey,
                          const string& sMsgId,
                          const string& sMsgStr,
                          bool          bFuzzy)
{
    if (!locateKey(iLineNo, sSourceFile, sKey, sMsgId, true))
        throw l10nMem::showError(".po file contains unknown filename: " + sSourceFile + " or key: " + sKey);

    l10nMem_lang_entry& xCur = mcENUSlist[miCurENUSinx].mcLangText[miCurLangInx];
    xCur.msMsgStr = sMsgStr;
    xCur.mbFuzzy = bFuzzy;
}



void l10nMem::reorganize(bool bConvert)
{
    int iE, iEsize = mcENUSlist.size();
    int iD, iDsize;
    vector<int> listDel, listAdd;


    // Check number of changes
    for (iE = 1; iE < iEsize; ++iE) {
        l10nMem_enus_entry& cur = mcENUSlist[iE];
        if (cur.meState == l10nMem::ENTRY_ADDED)
            listAdd.push_back(iE);
        if (cur.meState == l10nMem::ENTRY_DELETED) {
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
    for (iE = 0; iE < iEsize; ++iE) {
        l10nMem_enus_entry& curAdd = mcENUSlist[listAdd[iE]];
        for (iD = 0; iD < iDsize; ++iD) {
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
        curDel.msMsgId = curAdd.msMsgId;
        curDel.msKey = curAdd.msKey;
        curDel.meState = l10nMem::ENTRY_CHANGED;
        curAdd.meState = l10nMem::ENTRY_DELETED;
    }
}



bool l10nMem::locateKey(int                iLineNo,
                        const string& sSourceFile,
                        const string& sKey,
                        const string& sMsgId,
                        bool               bThrow)
{
    string sUpperKey(sKey);
    int         i, iSize = sUpperKey.size();
    char        ch;

    // Position file pointer
    if (!findFileName(sSourceFile))
        return false;

    // convert key to upper case
    for (i = 0; i < iSize; ++i) {
        ch = sUpperKey[i];
        if (ch == ' ' || ch == '*' || ch == '+' || ch == '%')
            sUpperKey[i] = '_';
        else
            sUpperKey[i] = rtl::toAsciiUpperCase(sUpperKey[i]);
    }

    // Fast check, to see if next key is the one (normal with load and source without change)
    if (++miCurENUSinx < (int)mcENUSlist.size()) {
        l10nMem_enus_entry& nowEntry = mcENUSlist[miCurENUSinx];
        if (nowEntry.msMsgId == sMsgId && nowEntry.msKey == sUpperKey)
            return true;
    }

    // Start from beginning of file and to end
    l10nMem_file_entry& cCur = mcFileList[miCurFileInx];

    // Find match with key and text
    for (miCurENUSinx = cCur.miStart; miCurENUSinx <= cCur.miEnd; ++miCurENUSinx) {
        l10nMem_enus_entry& cEntry = mcENUSlist[miCurENUSinx];
        if (cEntry.msMsgId == sMsgId && cEntry.msKey == sUpperKey)
            return true;
    }

    if (bThrow)
        throw l10nMem::showError("cannot find key(" + sUpperKey + ") with text(" + sMsgId + ")", iLineNo);
    return false;
}



void l10nMem::addKey(int             iLineNo,
                     const string&   sSourceFile,
                     const string&   sKey,
                     const string&   sMsgId,
                     const string&   sComment,
                     const string&   sResource,
                     const string&   sGroup,
                     l10nMem::ENTRY_STATE eStat)
{
    // check file
    if (!findFileName(sSourceFile)) {
        // prepare for new entry
        miCurENUSinx = mcENUSlist.size();
        miCurFileInx = mcFileList.size();

        // Create file
        mcFileList.push_back(l10nMem_file_entry(sSourceFile, miCurENUSinx));

        // and add entry at the back (no problem since it is a new file)
        mcENUSlist.push_back(l10nMem_enus_entry(sKey, sMsgId, sComment, sResource, sGroup, iLineNo, miCurFileInx,
            mcLangList.size(), eStat));
        mcFileList[miCurFileInx].miEnd = miCurENUSinx;
    }
    else {
        int iFsize = mcFileList.size();
        l10nMem_file_entry& curF = mcFileList[miCurFileInx];
        vector<l10nMem_enus_entry>::iterator it = mcENUSlist.begin();

        // file is registered, so we need to add the entry at the end of the file range
        curF.miEnd++;
        miCurENUSinx = curF.miEnd;
        mcENUSlist.insert(it + curF.miEnd,
            l10nMem_enus_entry(sKey, sMsgId, sComment, sResource, sGroup, iLineNo, miCurFileInx,
                          mcLangList.size(), eStat));
        for (int i = miCurFileInx + 1; i < iFsize; ++i) {
            l10nMem_file_entry& curF2 = mcFileList[i];
            if (curF2.miStart >= curF.miEnd)
                curF2.miStart++;
            if (curF2.miEnd >= curF.miEnd)
                curF2.miEnd++;
        }
    }
}


bool l10nMem::findFileName(const string& sSourceFile)
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
    ++miCurFileInx);
    if (miCurFileInx == iSize) {
        miCurFileInx = 0;
        return false;
    }
    else
        return true;
}




void l10nMem::keyToLower(string& sKey)
{
    int i, iSize;

    iSize = sKey.size();
    for (i = 0; i < iSize; ++i) {
        char ch = sKey[i];
        if (ch == ' ' || ch == '*' || ch == '+' || ch == '%')
            sKey[i] = '_';
        else
            sKey[i] = toAsciiLowerCase(ch);
    }
}
