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
#include <msi.h>
#include <msiquery.h>

#define MSITRANSFORM_ERROR_ADDEXISTINGROW   0x01
#define MSITRANSFORM_ERROR_DELMISSINGROW    0x02
#define MSITRANSFORM_ERROR_ADDEXISTINGTABLE 0x04
#define MSITRANSFORM_ERROR_DELMISSINGTABLE  0x08
#define MSITRANSFORM_ERROR_UPDATEMISSINGROW 0x10
#define MSITRANSFORM_ERROR_CHANGECODEPAGE   0x20

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

BOOL generateTransform(char * basedb, char * refdb, char * transFile, unsigned int err)
{
    MSIHANDLE dbHandle, refHandle;
    UINT r;
    r = MsiOpenDatabase(basedb, MSIDBOPEN_READONLY, &dbHandle);
    if (r != ERROR_SUCCESS) return FALSE;
    r = MsiOpenDatabase (refdb, MSIDBOPEN_READONLY, &refHandle);
    if (r != ERROR_SUCCESS) return FALSE;

    /* TODO: This isn't implemented in Wine */
    r = MsiDatabaseGenerateTransform(dbHandle, refHandle, transFile, 0, 0);
    if (r != ERROR_SUCCESS)
    {
        MsiCloseHandle(dbHandle);
        MsiCloseHandle(refHandle);
        return FALSE;
    }
    r = MsiCreateTransformSummaryInfo(dbHandle, refHandle, transFile, err, 0);
    MsiCloseHandle(dbHandle);
    MsiCloseHandle(refHandle);

    return TRUE;
}

int main(int argc, char *argv[])
{
    char * genFiles[3] = {0, 0, 0};
    unsigned int i, err = 0;
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
            case 'a': err |= MSITRANSFORM_ERROR_ADDEXISTINGROW; break;
            case 'b': err |= MSITRANSFORM_ERROR_DELMISSINGROW; break;
            case 'c': err |= MSITRANSFORM_ERROR_ADDEXISTINGTABLE; break;
            case 'd': err |= MSITRANSFORM_ERROR_DELMISSINGTABLE; break;
            case 'e': err |= MSITRANSFORM_ERROR_UPDATEMISSINGROW; break;
            case 'f': err |= MSITRANSFORM_ERROR_CHANGECODEPAGE; break;
              }
        }
    }

    /* Perform transformation */
    if (!generateTransform(genFiles[0], genFiles[1], genFiles[2], err))
    {
        printf("An error occurred and the transform could not be generated\n");
        return 3;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
