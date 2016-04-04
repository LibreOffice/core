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

        static int  showError  (const string& sText, int iLineNo = 0);
        static void showWarning(const string& sText, int iLineNo = 0);
        static void showDebug  (const string& sText, int iLineNo = 0);
        static void showVerbose(const string& sText, int iLineNo = 0);
        static void keyToLower (string& sKey);

        void setModuleName(const string& sModuleName);
        const string& getModuleName(void);
        void setLanguage(const string& sLanguage,
                         bool          bCreate);
        void setConvert(bool bConvert,
                        bool bStrict);
        void setVerbose(const bool doVerbose);
        void setDebug(const bool doDebug);

        void loadEntryKey(int           iLineNo,
                          const string& sSourceFile,
                          const string& sKey,
                          const string& sOrgText,
                          const string& sText,
                          const string& sComment,
                          const string& sResource,
                          const string& sGroup,
                          bool          bIsFuzzy);
        void setSourceKey(int           iLineNo,
                          const string& sFilename,
                          const string& sKey,
                          const string& sText,
                          const string& sComment,
                          const string& sResource,
                          const string& sGroup,
                          bool          bMustExist);

        void saveTemplates(const string& sTargetDir,
                           bool          bForce);
        void saveLanguages(l10nMem&      cMem,
                           const string& sTargetDir,
                           bool          bForce);
        void dumpMem(const string& sTargetDir);
        int  prepareMerge();
        bool getMergeLang(string& sLang,
                          string& sText);
        void showNOconvert();
        bool isError();
        void convertToInetString(string& sText);
        void convertFromInetString(string& sText);

    private:
        bool  mbVerbose;
        bool  mbDebug;
        bool  mbInError;
        string msModuleName;
        int  miCurFileInx;
        int  miCurLangInx;
        int  miCurENUSinx;
        bool mbConvertMode;
        bool mbStrictMode;
        vector<l10nMem_enus_entry>      mcENUSlist;
        vector<l10nMem_file_entry>      mcFileList;
        vector<l10nMem_lang_list_entry> mcLangList;

        void formatAndShowText(const string& sType, int iLineNo, const string& sText);
        bool needWrite(const string sFileName, bool bForce);
        bool convFilterWarning(const string& sSourceFile,
                               const string& sKey,
                               const string& sMsgId);
        void convEntryKey(int                iLineNo,
                          const string& sSourceFile,
                          const string& sKey,
                          const string& sMsgId,
                          const string& sMsgStr,
                          bool          bIsFuzzy);
        void loadENUSkey(int           iLineNo,
                         const string& sSourceFile,
                         const string& sKey,
                         const string& sMsgId,
                         const string& sComment,
                         const string& sResource,
                         const string& sGroup);
        void loadLangKey(int           iLineNo,
                         const string& sSourceFile,
                         const string& sKey,
                         const string& sMsgId,
                         const string& sMsgStr,
                         bool          bFuzzy);
        void reorganize(bool           bConvert);
        bool locateKey(int           iLineNo,
                       const string& sSourceFile,
                       const string& sKey,
                       const string& sMsgId,
                       bool          bThrow);
        void addKey(int           iLineNo,
                    const string& sSourceFile,
                    const string& sKey,
                    const string& sMsgId,
                    const string& sComment,
                    const string& sResource,
                    const string& sGroup,
                    l10nMem::ENTRY_STATE eStat);
        bool findFileName(const string& sSourceFile);
};
#endif
