/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef enum ERRCOND { ERRA = 0x1, ERRB = 0x2, ERRC = 0x4, ERRD = 0x8, ERRE = 0x10, ERRF = 0x20 } ERRCOND;
static unsigned int err;

void usage(void)
{
    printf(
        "Usage: msitran.exe -g {basedb}{refdb}{transformfile}[{errorconditions}]\n"
        "\nOptions:\n"
        "  -g        Generate a transform\n"
        "  -? or -h  Display usage\n"
        "\nError conditions (specify in {errorconditions} to suppress):\n"
        "  a  Add existing row\n"
        "  b  Delete non-existing row\n"
        "  c  Add existing table\n"
        "  d  Delete non-existing table\n"
        "  e  Modify existing row\n"
        "  f  Change codepage\n");
}

void generatePatch(char * basedb, char * refdb, char * transFile)
{
}

int main(int argc, char *argv[])
{
    char * genFiles[3] = {0, 0, 0};
    unsigned int i = 0;
    err = 0;

    /* Get parameters */
    while (argv[1] && (argv[1][0] == '-' || argv[1][0] == '/'))
    {
        switch(tolower(argv[1][1]))
        {
        case 'g':
            for (i = 0; i < 3; i++)
            {
                if (!argv[1])
                {
                    printf("Please supply all arguments for generating a transform\n");
                    return 1;
                }
                argv++; argc--;
                genFiles[i] = argv[1];
            }
            break;
        case '?':
        case 'h':
            usage();
            return 0;
        }
        argv++; argc++;
    }

    /* Record error suppression conditions */
    if (argv[1])
    {
        for (i = 0; i < strlen(argv[1]); i++)
        {
            switch(tolower(argv[1][i]))
            {
            case 'a': err |= ERRA; break;
            case 'b': err |= ERRB; break;
            case 'c': err |= ERRC; break;
            case 'd': err |= ERRD; break;
            case 'e': err |= ERRE; break;
            case 'f': err |= ERRF; break;
            }
        }
    }

    generatePatch(genFiles[0], genFiles[1], genFiles[2]);

    return 0;
}
