/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Command-line front end of desktop::l10n::translateDocument(): rewrites a
// "_"-marked ODF template that carries an "l10n" stream for one locale, the
// way the kit does when an app opens an intro document.  The preview
// renderer of the intro documents in online (browser/introdocs/
// render_previews.py) uses it to get a translated file to render.

#include <sal/config.h>

#include <cstdio>
#include <cstring>

#include <lib/l10ntranslate.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <sal/main.h>
#include <tools/extendapplicationenvironment.hxx>

namespace
{
// A file URL for a command-line argument: a URL as given, a system path
// resolved against the working directory.
OUString toFileURL(const char* pArgument)
{
    const OUString aArgument(pArgument, strlen(pArgument), osl_getThreadTextEncoding());
    if (aArgument.startsWithIgnoreAsciiCase("file:"))
        return aArgument;

    OUString aURL;
    if (osl::FileBase::getFileURLFromSystemPath(aArgument, aURL) != osl::FileBase::E_None)
        return aArgument;

    OUString aWorkingDir;
    OUString aAbsoluteURL;
    if (osl_getProcessWorkingDir(&aWorkingDir.pData) != osl_Process_E_None
        || osl::FileBase::getAbsoluteFileURL(aWorkingDir, aURL, aAbsoluteURL)
               != osl::FileBase::E_None)
        return aURL;
    return aAbsoluteURL;
}
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    if (argc != 4)
    {
        fputs("usage: l10ntranslate <input> <output> <bcp47-language>\n"
              "\n"
              "<input> and <output> are paths or file URLs and may name the same file.\n"
              "An empty language only strips the markers.  Prints \"translated\" when\n"
              "the language matched a locale of the stream, \"copied untranslated\"\n"
              "otherwise.\n",
              stderr);
        return 1;
    }

    // URE_BOOTSTRAP, so that the language tag data is found
    tools::extendApplicationEnvironment();

    const OUString aInput = toFileURL(argv[1]);
    const OUString aOutput = toFileURL(argv[2]);
    const OUString aLanguage = OUString::fromUtf8(argv[3]);

    switch (desktop::l10n::translateDocument(aInput, aOutput, aLanguage))
    {
        case desktop::l10n::TranslateResult::Translated:
            puts("translated");
            return 0;
        case desktop::l10n::TranslateResult::CopiedUntranslated:
            puts("copied untranslated");
            return 0;
        case desktop::l10n::TranslateResult::Error:
            break;
    }
    fprintf(stderr, "l10ntranslate: could not rewrite %s\n", argv[1]);
    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
