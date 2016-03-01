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

#ifndef GL10NMEM_HXX
#define GL10NMEM_HXX
#include "gLang.hxx"



/*****************************************************************************
 ************************   G L 1 0 N M E M . H X X   ************************
 *****************************************************************************
 * This is the class definition header of the l10n localizer program,
 * all global classes and their interrelations is defined here
 *****************************************************************************/



/********************   C L A S S   D E F I N I T I O N   ********************/
class l10nMem_lang_entry
{
  public:
    l10nMem_lang_entry(const std::string& sMsgStr, bool bFuzzy);
    ~l10nMem_lang_entry();

    std::string msMsgStr;   // translated text from po file
    bool        mbFuzzy;    // fuzzy flag
};



/********************   C L A S S   D E F I N I T I O N   ********************/
class l10nMem_enus_entry
{
  public:
    l10nMem_enus_entry(const std::string&   sKey,
                       const std::string&   sMsgId,
                       int                  iLineNo,
                       int                  iFileInx,
                       int                  iLangSize,
                       l10nMem::ENTRY_STATE eState);
    ~l10nMem_enus_entry();

    std::string                     msKey;      // key in po file and source file
    std::string                     msMsgId;    // en-US text from source file
    l10nMem::ENTRY_STATE            meState;    // status information
    int                             miFileInx;  // index of file name
    int                             miLineNo;   // line number
    std::vector<l10nMem_lang_entry> mcLangText; // language texts (index is languageId)
};



/********************   C L A S S   D E F I N I T I O N   ********************/
class l10nMem_file_entry
{
  public:
    l10nMem_file_entry(const std::string& sFileName, int iStart);
    ~l10nMem_file_entry();

    std::string msFileName;  // file Name with relative path
    std::string msPureName;  // just filename
    int         miStart;     // start index of entries in mcMasterEntries (l10Mem_db::mcENUS)
    int         miEnd;       // last index of entries in mcMasterEntries (l10Mem_db::mcENUS)
};



/********************   C L A S S   D E F I N I T I O N   ********************/
class l10nMem_lang_list_entry
{
  public:
    l10nMem_lang_list_entry(const std::string& sName);
    ~l10nMem_lang_list_entry();

    std::string msName;      // language Name
    bool        mbChanged;   // used for "convert", true if language is modified
};



/********************   C L A S S   D E F I N I T I O N   ********************/
class l10nMem_db
{
  public:
    l10nMem_db();
    ~l10nMem_db();

    int                                  miCurFileInx;
    int                                  miCurLangInx;
    int                                  miCurENUSinx;
    bool                                 mbNeedWrite;
    bool                                 mbConvertMode;
    bool                                 mbStrictMode;
    std::vector<l10nMem_enus_entry>      mcENUSlist;
    std::vector<l10nMem_file_entry>      mcFileList;
    std::vector<l10nMem_lang_list_entry> mcLangList;


    void loadENUSkey    (int                iLineNo,
                         const std::string& sSourceFile,
                         const std::string& sKey,
                         const std::string& sMsgId);
    void setLanguage    (const std::string& sLanguage,
                         bool               bCreate);
    void setConvert     (bool               bConvert,
                         bool               bStrict);
    bool findFileName   (const std::string& sSourceFile);
    void loadLangKey    (int                iLineNo,
                         const std::string& sSourceFile,
                         const std::string& sKey,
                         const std::string& sMsgId,
                         const std::string& sMsgStr,
                         bool               bFuzzy);


    bool locateKey      (int                iLineNo,
                         const std::string& sSourceFile,
                         const std::string& sKey,
                         const std::string& sMsgId,
                         bool               bThrow);
    void reorganize     (bool               bConvert);
    void addKey         (int                  iLineNo,
                         const std::string&   sSourceFile,
                         const std::string&   sKey,
                         const std::string&   sMsgId,
                         l10nMem::ENTRY_STATE eStat);

    int  prepareMerge   ();
    bool getMergeLang   (std::string& sLang,
                         std::string& sText);
    bool getLangList    (std::string& sLang);

static void keyToUpper(std::string& sKey);
};




/********************   C L A S S   D E F I N I T I O N   ********************/
class l10nMem_impl
{
  public:
    l10nMem_impl();
    ~l10nMem_impl();

    int  showError     (const std::string& sText, int iLineNo);
    int  showWarning   (const std::string& sText, int iLineNo);
    void showDebug     (const std::string& sText, int iLineNo);
    void showVerbose   (const std::string& sText, int iLineNo);

    void setModuleName (const std::string& sModuleName);
    const std::string& getModuleName (void);
    void loadEntryKey  (int                iLineNo,
                        const std::string& sSourceFile,
                        const std::string& sKey,
                        const std::string& sMsgId,
                        const std::string& sMsgStr,
                        bool               bIsFuzzy);
    void setSourceKey  (int                iLineNo,
                        const std::string& sFilename,
                        const std::string& sKey,
                        const std::string& sMsgId,
                        bool               bMustExist);

    void saveTemplates (l10nMem& cMem,
                        const std::string& sTargetDir,
                        bool               bKid,
                        bool               bForce);
    void saveLanguages (l10nMem& cMem,
                        const std::string& sTargetDir,
                        bool               bForce);
    void dumpMem      (const std::string& sTargetDir);
    void showNOconvert();

    void convertToInetString(std::string& sText);
    void convertFromInetString(std::string& sText);

  private:
    static bool                         mbVerbose;
    static bool                         mbDebug;
    static l10nMem_impl                *mcImpl;
    l10nMem_db                          mcDb;
    std::string                         msModuleName;
    bool                                mbInError;

    void formatAndShowText(const std::string& sType, int iLineNo, const std::string& sText);
    bool needWrite        (const std::string sFileName, bool bForce);
    bool convFilterWarning(const std::string& sSourceFile,
                           const std::string& sKey,
                           const std::string& sMsgId);
    void convEntryKey     (int                iLineNo,
                           const std::string& sSourceFile,
                           const std::string& sKey,
                           const std::string& sMsgId,
                           const std::string& sMsgStr,
                           bool               bIsFuzzy);

    friend class l10nMem;
};
#endif
