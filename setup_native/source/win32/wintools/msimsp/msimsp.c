/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <sys/stat.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void usage(void)
{
    printf(
        "Usage: msimsp.exe -s [pcp_path] -p [msp_path] {options}\n"
        "\nOptions:\n"
        "-s <pcpfile>  Required. Path to the patch creation properties (.pcp) file.\n"
        "-p <mspfile>  Required. Path to patch package being created (.msp)\n"
        "-f <tmpdir>   Path to temporary directory (default: %%TMP%%/~pcw_tmp.tmp/\n"
        "-k            Fail if temporary directory already exists\n"
        "-l <logfile>  Path to log file of patch creation process and errors\n"
/*      "-lp <logfile> Path to log file (includes performance data).\n"
        "-d            Displays dialog on success\n"*/
        "-? or -h      Display usage\n");
}

void createPatch(char * pcpFile, char * mspFile, char * tmpDir, char * logFile)
{
}

int main(int argc, char *argv[])
{
    char * pcpFile = 0;
    char * mspFile = 0;
    char * tmpDir = 0;
    char * logFile = 0;
    BOOL chkTmpDir = FALSE;
    struct stat s;

    /* Get parameters */
    while (argv[1] && (argv[1][0] == '-' || argv[1][0] == '/'))
    {
        switch(tolower(argv[1][1]))
        {
        case 's':
            argv++; argc++;
            pcpFile = argv[1];
            break;
        case 'p':
            argv++; argc++;
            mspFile = argv[1];
            break;
        case 'f':
            argv++; argc++;
            tmpDir = argv[1];
            break;
        case 'k':
            chkTmpDir = TRUE;
            break;
        case 'l':
            argv++; argc++;
            logFile = argv[1];
            break;
        case '?':
        case 'h':
            usage();
            return 0;
        }
        argv++; argc++;
    }

    if (chkTmpDir && stat(tmpDir, &s) == 0 && S_ISDIR(s.st_mode))
    {
        printf("Temporary directory exists, please specify another or omit -k\n");
        return 2;
    }
    createPatch(pcpFile, mspFile, tmpDir, logFile);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
