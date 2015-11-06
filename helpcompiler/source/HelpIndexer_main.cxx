/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <helpcompiler/HelpIndexer.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <string>
#include <iostream>

#include "LuceneHelper.hxx"

int main(int argc, char **argv)
{
    try
    {
        const std::string pLang("-lang");
        const std::string pModule("-mod");
        const std::string pDir("-dir");

        std::string lang;
        std::string module;
        std::string dir;

        bool error = false;
        for (int i = 1; i < argc; ++i) {
            if (pLang.compare(argv[i]) == 0) {
                if (i + 1 < argc) {
                    lang = argv[++i];
                } else {
                    error = true;
                }
            } else if (pModule.compare(argv[i]) == 0) {
                if (i + 1 < argc) {
                    module = argv[++i];
                } else {
                    error = true;
                }
            } else if (pDir.compare(argv[i]) == 0) {
                if (i + 1 < argc) {
                    dir = argv[++i];
                } else {
                    error = true;
                }
            } else {
                error = true;
            }
        }

        if (error) {
            std::cerr << "Error parsing command-line arguments" << std::endl;
        }

        if (error || lang.empty() || module.empty() || dir.empty()) {
            std::cerr << "Usage: HelpIndexer -lang ISOLangCode -mod HelpModule -dir Dir" << std::endl;
            return 1;
        }

        OUString sDir;

        osl::File::getFileURLFromSystemPath(
            OUString(dir.c_str(), dir.size(), osl_getThreadTextEncoding()),
            sDir);

        OUString cwd;
        osl_getProcessWorkingDir(&cwd.pData);

        osl::File::getAbsoluteFileURL(cwd, sDir, sDir);

        HelpIndexer indexer(
            OUString(lang.c_str(), lang.size(), osl_getThreadTextEncoding()),
            OUString(module.c_str(), module.size(), osl_getThreadTextEncoding()),
            sDir, sDir);

        if (!indexer.indexDocuments()) {
            std::cerr << OUStringToOString(indexer.getErrorMessage(), osl_getThreadTextEncoding()).getStr()  << std::endl;
            return 2;
        }
        return 0;
    }
    catch (std::exception& e)
    {
        std::cerr << "failure: " << e.what() << std::endl;
        return 2;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
