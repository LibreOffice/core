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
#ifndef GCON_HXX
#define GCON_HXX
#include <iostream>
#include <fstream>

class convert_gen
{
    public:
        static convert_gen *mcImpl;

        convert_gen(l10nMem& cMemory);
        virtual ~convert_gen();

        // Create instance
        static convert_gen& createInstance(l10nMem&           cMemory,
                                           const std::string& sSourceDir,
                                           const std::string& sTargetDir,
                                           const std::string& sSourceFile);

        // do extract/merge
        bool execute(const bool bMerge, const bool bKid);

        // all converters MUST implement this function
        virtual void execute() = 0;

        // ONLY po should implement these functions
        virtual void startSave(const std::string& sLanguage,
                               const std::string& sFile);
        virtual void save(const std::string& sFileName,
                          const std::string& sKey,
                          const std::string& sENUStext,
                          const std::string& sText,
                          bool               bFuzzy);
        virtual void endSave();
        static bool checkAccess(std::string& sFile);
        static bool createDir(std::string& sDir, std::string& sFile);

        // utility functions for converters
        void lexRead(char *sBuf, int *nResult, int nMax_size);
        std::string& copySource(char const *yyText, bool bDoClear = true);

    protected:
        // generic variables
        bool         mbMergeMode;
        bool         mbLoadMode;
        std::string  msSourcePath;
        std::string  msTargetPath;
        std::string  msSourceFile;
        l10nMem&     mcMemory;
        std::string  msCollector;
        int          miLineNo;
        std::string  msSourceBuffer, msCopyText;
        int          miSourceReadIndex;

        bool prepareFile();

        // utility functions for converters
        void writeSourceFile(const std::string& line);

    private:
        std::ofstream mcOutputFile;
};
#endif
