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
#ifndef GLANG_HXX
#define GLANG_HXX
#include <string>
#include <vector>



class l10nMem_db;
class l10nMem
{
    public:
        l10nMem();
        ~l10nMem();

        static int  showError(const std::string& sText, int iLineNo = 0);
        static void showWarning(const std::string& sText, int iLineNo = 0);
        static void showDebug(const std::string& sText, int iLineNo = 0);
        static void showVerbose(const std::string& sText, int iLineNo = 0);
        static bool isError();

        void setModuleName (const std::string& sModuleName);
        const std::string& getModuleName (void);
        void setLanguage   (const std::string& sLanguage,
                            bool               bCreate);
        void setConvert    (bool               bConvert,
                            bool               bStrict);
        void loadEntryKey  (int                iLineNo,
                            const std::string& sSourceFile,
                            const std::string& sKey,
                            const std::string& sOrgText,
                            const std::string& sText,
                            bool               bIsFuzzy);

        void setSourceKey  (int                iLineNo,
                            const std::string& sFilename,
                            const std::string& sKey,
                            const std::string& sText,
                            bool               bMustExist);

        void saveTemplates (const std::string& sTargetDir,
                            bool               bKid,
                            bool               bForce);
        void saveLanguages(l10nMem&           cMem,
                           const std::string& sTargetDir,
                           bool               bForce);
        void dumpMem       (const std::string& sTargetDir);

        int  prepareMerge  ();
        bool getMergeLang  (std::string& sLang,
                            std::string& sText);
        void showNOconvert ();

        void convertToInetString(std::string& sText);
        void convertFromInetString(std::string& sText);

    private:
        l10nMem_db *mcDb;
        bool        mbVerbose;
        bool        mbDebug;
        bool        mbInError;
        typedef enum
        {
            ENTRY_DELETED,
            ENTRY_ADDED,
            ENTRY_CHANGED,
            ENTRY_NORMAL
        } ENTRY_STATE;
        std::string msModuleName;

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

        friend class handler;
        friend class l10nMem_enus_entry;
        friend class l10nMem_db;
};



class convert_gen
{
    public:
        convert_gen(l10nMem&           cMemory,
                    const std::string& sSourceDir,
                    const std::string& sTargetDir,
                    const std::string& sSourceFile);
        ~convert_gen();

        // do extract/merge
        bool execute(const bool bMerge, const bool bKid);

        // ONLY po should implement these functions
        void startSave(const std::string& sLanguage,
                       const std::string& sFile);
        void save(const std::string& sFileName,
                  const std::string& sKey,
                  const std::string& sENUStext,
                  const std::string& sText,
                  bool               bFuzzy);
        void endSave();
        static bool checkAccess(std::string& sFile);
        static bool createDir(std::string& sDir, std::string& sFile);
};
#endif
