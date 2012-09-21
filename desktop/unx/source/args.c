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
 * The Initial Developer of the Original Code is
 *       Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *  Michael Meeks <michael.meeks@novell.com>
 * Portions created by the Ted are Copyright (C) 2010 Ted. All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <stdlib.h>
#include <string.h>
#include <osl/process.h>

#include "args.h"

/* do we start -env: */
static int
is_env_arg (rtl_uString *str)
{
    return !rtl_ustr_ascii_compare_WithLength (str->buffer, 5, "-env:");
}

static struct {
    const char   *name;
    unsigned int  bTwoArgs : 1;
    unsigned int  bInhibitSplash : 1;
    unsigned int  bInhibitPagein : 1;
    unsigned int  bInhibitJavaLdx : 1;
    const char   *pPageinType;
} pArgDescr[] = {
    /* have a trailing argument */
    { "pt",         1, 0, 0, 0, NULL },
    { "display",    1, 0, 0, 0, NULL },

    /* no splash */
    { "nologo",     0, 1, 0, 0, NULL },
    { "headless",   0, 1, 0, 0, NULL },
    { "invisible",  0, 1, 0, 0, NULL },
    { "quickstart", 0, 1, 0, 0, NULL },
    { "minimized",  0, 1, 0, 0, NULL },

    /* pagein bits */
    { "writer",     0, 0, 0, 0, "pagein-writer"  },
    { "calc",       0, 0, 0, 0, "pagein-calc"    },
    { "draw",       0, 0, 0, 0, "pagein-draw"    },
    { "impress",    0, 0, 0, 0, "pagein-impress" },

    /* nothing much */
    { "version",    0, 1, 1, 1, NULL },
    { "help",       0, 1, 1, 1, NULL },
    { "h",          0, 1, 1, 1, NULL },
    { "?",          0, 1, 1, 1, NULL },
};

Args *args_parse (void)
{
    Args *args;
    sal_uInt32 nArgs, i, j;

    nArgs = osl_getCommandArgCount();
    i = sizeof (Args) + sizeof (rtl_uString *) * nArgs;
    args = malloc (i);
    memset (args, 0, i);
    args->nArgsTotal = nArgs;

    j = 0;

    /* sort the -env: args to the front */
    for ( i = 0; i < nArgs; ++i )
    {
        rtl_uString *pTmp = NULL;
        osl_getCommandArg( i, &pTmp );
        if (is_env_arg (pTmp))
            args->ppArgs[j++] = pTmp;
        else
            rtl_uString_release (pTmp);
    }
    args->nArgsEnv = j;

    /* Then the other args */
    for ( i = 0; i < nArgs; ++i )
    {
        rtl_uString *pTmp = NULL;

        osl_getCommandArg( i, &pTmp );
        if (!is_env_arg (pTmp))
            args->ppArgs[j++] = pTmp;
        else
            rtl_uString_release (pTmp);
    }

    for ( i = args->nArgsEnv; i < args->nArgsTotal; i++ )
    {
        const sal_Unicode *arg = args->ppArgs[i]->buffer;
        sal_Int32 length = args->ppArgs[i]->length;

        /* grok only parameters */
        if (arg[0] != '-')
            continue;

        while (length > 2 && arg[0] == '-') {
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
    rtl_uString_release( args->pAppPath );
    free (args);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
