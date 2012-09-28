/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Gert van Valkenhoef <g.h.m.van.valkenhoef@rug.nl>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <helpcompiler/HelpIndexer.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <string>
#include <iostream>

#include "LuceneHelper.hxx"

int main(int argc, char **argv) {
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

    std::string captionDir(dir + SAL_PATHDELIMITER + "caption");
    std::string contentDir(dir + SAL_PATHDELIMITER + "content");
    std::string indexDir(dir + SAL_PATHDELIMITER + module + ".idxl");

    rtl::OUString sDir;

    osl::File::getFileURLFromSystemPath(
        rtl::OUString(dir.c_str(), dir.size(), osl_getThreadTextEncoding()),
        sDir);

    rtl::OUString cwd;
    osl_getProcessWorkingDir(&cwd.pData);

    osl::File::getAbsoluteFileURL(cwd, sDir, sDir);

    HelpIndexer indexer(
        rtl::OUString(lang.c_str(), lang.size(), osl_getThreadTextEncoding()),
        rtl::OUString(module.c_str(), module.size(), osl_getThreadTextEncoding()),
        sDir, sDir);

    if (!indexer.indexDocuments()) {
        std::cerr << rtl::OUStringToOString(indexer.getErrorMessage(), osl_getThreadTextEncoding()).getStr()  << std::endl;
        return 2;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
