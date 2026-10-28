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

namespace
{
const struct ArgDescr
{
    const char   *name;
    bool bInhibitPagein : 1;
    bool bInhibitJavaLdx : 1;
    bool bInhibitPipe : 1;
    const char   *pPageinType;
} pArgDescr[] = {
    /* pagein bits */
    { "writer", false, false, false, "pagein-writer" },
    { "calc", false, false, false, "pagein-calc" },
    { "draw", false, false, false, "pagein-draw" },
    { "impress", false, false, false, "pagein-impress" },

    /* Do not send --help/--version over the pipe, as their output shall go to
       the calling process's stdout (ideally, this would also happen in the
       presence of unknown options); also prevent splash/pagein/javaldx overhead
       (as these options will be processed early in soffice_main): */
    { "version", true, true, true, nullptr },
    { "help", true, true, true, nullptr },
    { "h", true, true, true, nullptr },
    { "?", true, true, true, nullptr },
};
}

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

        for (const ArgDescr& rArgDescr : pArgDescr)
        {
            if (rtl_ustr_ascii_compare_WithLength(arg, length, rArgDescr.name) == 0)
            {
                args->bInhibitPagein |= rArgDescr.bInhibitPagein;
                args->bInhibitJavaLdx |= rArgDescr.bInhibitJavaLdx;
                args->bInhibitPipe |= rArgDescr.bInhibitPipe;
                if (rArgDescr.pPageinType)
                    args->pPageinType = rArgDescr.pPageinType;
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
