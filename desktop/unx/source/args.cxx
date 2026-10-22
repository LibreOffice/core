/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "args.hxx"

#include <string.h>
#include <osl/process.h>

/* do we start -env: */
static bool is_env_arg(rtl_uString const* str)
{
    return !rtl_ustr_ascii_compare_WithLength (str->buffer, 5, "-env:");
}

const struct {
    const char   *name;
    bool bInhibitSplash : 1;
    bool bInhibitPagein : 1;
    bool bInhibitJavaLdx : 1;
    bool bInhibitPipe : 1;
    const char   *pPageinType;
} pArgDescr[] = {
    /* have a trailing argument */
    { "pt", true, false, false, false, nullptr },
    { "p", true, false, false, false, nullptr },
    { "display", false, false, false, false, nullptr },

    /* no splash */
    { "nologo", true, false, false, false, nullptr },
    { "headless", true, false, false, false, nullptr },
    { "invisible", true, false, false, false, nullptr },
    { "quickstart", true, false, false, false, nullptr },
    { "minimized", true, false, false, false, nullptr },
    { "convert-to", true, false, false, false, nullptr },
    { "cat", true, false, false, false, nullptr },

    /* pagein bits */
    { "writer", false, false, false, false, "pagein-writer" },
    { "calc", false, false, false, false, "pagein-calc" },
    { "draw", false, false, false, false, "pagein-draw" },
    { "impress", false, false, false, false, "pagein-impress" },

    /* Do not send --help/--version over the pipe, as their output shall go to
       the calling process's stdout (ideally, this would also happen in the
       presence of unknown options); also prevent splash/pagein/javaldx overhead
       (as these options will be processed early in soffice_main): */
    { "version", true, true, true, true, nullptr },
    { "help", true, true, true, true, nullptr },
    { "h", true, true, true, true, nullptr },
    { "?", true, true, true, true, nullptr },
};

Args *args_parse (void)
{
    const sal_uInt32 nArgs = osl_getCommandArgCount();
    Args* args = new Args;

    sal_uInt32 j = 0;

    for (sal_uInt32 i = 0; i < nArgs; ++i)
    {
        rtl_uString *pTmp = nullptr;
        osl_getCommandArg( i, &pTmp );
        if (is_env_arg (pTmp))
        {
            /* sort the -env: args to the front */
            args->aArgs.insert(args->aArgs.begin() + j, pTmp);
            ++j;
        }
        else
        {
            /* Then the other args */
            args->aArgs.push_back(pTmp);
        }
    }
    args->nArgsEnv = j;

    for (size_t i = args->nArgsEnv; i < args->aArgs.size(); i++)
    {
        const sal_Unicode* arg = args->aArgs.at(i)->buffer;
        sal_Int32 length = args->aArgs.at(i)->length;

        /* grok only parameters */
        if (arg[0] != '-')
            continue;

        while (length > 1 && arg[0] == '-') {
            arg++;
            length--;
        }

        for ( j = 0; j < SAL_N_ELEMENTS (pArgDescr); ++j ) {
            if (rtl_ustr_ascii_compare_WithLength(
                    arg, length, pArgDescr[j].name)
                == 0)
            {
                args->bInhibitSplash  |= pArgDescr[j].bInhibitSplash;
                args->bInhibitPagein  |= pArgDescr[j].bInhibitPagein;
                args->bInhibitJavaLdx |= pArgDescr[j].bInhibitJavaLdx;
                args->bInhibitPipe    |= pArgDescr[j].bInhibitPipe;
                if (pArgDescr[j].pPageinType)
                    args->pPageinType = pArgDescr[j].pPageinType;
                break;
            }
        }
    }

    return args;
}

void
args_free (Args *args)
{
    /* FIXME: free ppArgs */
    delete args;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
