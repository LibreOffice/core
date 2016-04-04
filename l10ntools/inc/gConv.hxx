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
                                           const string& sSourceDir,
                                           const string& sTargetDir,
                                           const string& sSourceFile);

        // do extract/merge
        bool execute(const bool bMerge);

        // all converters MUST implement this function
        virtual void doExecute() = 0;

        // utility functions for converters
        void lexRead(char *sBuf, size_t *result, size_t nMax_size);
        static void lexStrncpy(char* s1, const char * s2, int n);
        string& copySource(char const *yyText, bool bDoClear = true);

    protected:
        // generic variables
        bool         mbMergeMode;
        bool         mbLoadMode;
        string  msSourcePath;
        string  msTargetPath;
        string  msSourceFile;
        l10nMem&     mcMemory;
        string  msCollector;
        int          miLineNo;
        string  msSourceBuffer, msCopyText;
        int          miSourceReadIndex;

        bool prepareFile();

        // utility functions for converters
        void writeSourceFile(const string& line);
        static bool createDir(const string& sDir, const string& sFile);
private:
        ofstream mcOutputFile;
        static bool checkAccess(string& sFile);
};
#endif
