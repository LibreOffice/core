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

class l10nMem_enus_entry;
class l10nMem_file_entry;
class l10nMem_lang_list_entry;
class l10nMem
{
    public:
        l10nMem();
        ~l10nMem();

        typedef enum {
            ENTRY_DELETED,
            ENTRY_ADDED,
            ENTRY_CHANGED,
            ENTRY_NORMAL
        } ENTRY_STATE;

        static int  showError(const std::string& sText, int iLineNo = 0);
        static void showWarning(const std::string& sText, int iLineNo = 0);
        static void showDebug(const std::string& sText, int iLineNo = 0);
        static void showVerbose(const std::string& sText, int iLineNo = 0);
        static void keyToUpper(std::string& sKey);

        void setModuleName(const std::string& sModuleName);
        const std::string& getModuleName(void);
        void setLanguage(const std::string& sLanguage,
                         bool               bCreate);
        void setConvert(bool               bConvert,
                        bool               bStrict);
        void setVerbose(const bool doVerbose);
        void setDebug(const bool doDebug);

        void loadEntryKey(int                iLineNo,
                          const std::string& sSourceFile,
                          const std::string& sKey,
                          const std::string& sOrgText,
                          const std::string& sText,
                          bool               bIsFuzzy);
        void setSourceKey(int                iLineNo,
                          const std::string& sFilename,
                          const std::string& sKey,
                          const std::string& sText,
                          bool               bMustExist);

        void saveTemplates(const std::string& sTargetDir,
                           bool               bKid,
                           bool               bForce);
        void saveLanguages(l10nMem&           cMem,
                           const std::string& sTargetDir,
                           bool               bForce);
        void dumpMem(const std::string& sTargetDir);
        int  prepareMerge();
        bool getMergeLang(std::string& sLang,
                          std::string& sText);
        void showNOconvert();
        bool isError();
        void convertToInetString(std::string& sText);
        void convertFromInetString(std::string& sText);

    private:
        bool        mbVerbose;
        bool        mbDebug;
        bool        mbInError;
        std::string msModuleName;
        int                                  miCurFileInx;
        int                                  miCurLangInx;
        int                                  miCurENUSinx;
        bool                                 mbNeedWrite;
        bool                                 mbConvertMode;
        bool                                 mbStrictMode;
        std::vector<l10nMem_enus_entry>      mcENUSlist;
        std::vector<l10nMem_file_entry>      mcFileList;
        std::vector<l10nMem_lang_list_entry> mcLangList;

        void formatAndShowText(const std::string& sType, int iLineNo, const std::string& sText);
        bool needWrite(const std::string sFileName, bool bForce);
        bool convFilterWarning(const std::string& sSourceFile,
                               const std::string& sKey,
                               const std::string& sMsgId);
        void convEntryKey(int                iLineNo,
                          const std::string& sSourceFile,
                          const std::string& sKey,
                          const std::string& sMsgId,
                          const std::string& sMsgStr,
                          bool               bIsFuzzy);
        void saveTemplates(l10nMem&           cMem,
                           const std::string& sTargetDir,
                           bool               bKid,
                           bool               bForce);
        void loadENUSkey(int                iLineNo,
                         const std::string& sSourceFile,
                         const std::string& sKey,
                         const std::string& sMsgId);
        void loadLangKey(int                iLineNo,
                         const std::string& sSourceFile,
                         const std::string& sKey,
                         const std::string& sMsgId,
                         const std::string& sMsgStr,
                         bool               bFuzzy);
        void reorganize(bool               bConvert);
        bool locateKey(int                iLineNo,
                       const std::string& sSourceFile,
                       const std::string& sKey,
                       const std::string& sMsgId,
                       bool               bThrow);
        void addKey(int                  iLineNo,
                    const std::string&   sSourceFile,
                    const std::string&   sKey,
                    const std::string&   sMsgId,
                    l10nMem::ENTRY_STATE eStat);
        bool findFileName(const std::string& sSourceFile);
};
#endif
