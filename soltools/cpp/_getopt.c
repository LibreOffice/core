/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include    <stdio.h>
#include    <string.h>

#define EPR                 fprintf(stderr,
#define ERR(str, chr)       if(opterr) { EPR "%s%c\n", str, chr); }

int opterr = 1;
int optind = 1;
int optopt;
char *optarg;

int
    stgetopt(int argc, char *const argv[], const char *opts)
{
    static int sp = 1;
    register int c;
    register char *cp;

    if (sp == 1)
    {
        if (optind >= argc ||
            argv[optind][0] != '-' || argv[optind][1] == '\0')
            return -1;
        else if (strcmp(argv[optind], "--") == 0)
            {
                optind++;
                return -1;
            }
        else if (strcmp(argv[optind], "-isysroot") == 0)
            {
                // skip Mac OS X SDK selection flags
                optind++; optind++;
            }
    }
    optopt = c = argv[optind][sp];
    if (c == ':' || (cp = strchr(opts, c)) == 0)
    {
        ERR(": illegal option -- ", c);
        if (argv[optind][++sp] == '\0')
        {
            optind++;
            sp = 1;
        }
        return '?';
    }
    if (*++cp == ':')
    {
        if (argv[optind][sp + 1] != '\0')
            optarg = &argv[optind++][sp + 1];
        else
            if (++optind >= argc)
            {
                ERR(": option requires an argument -- ", c);
                sp = 1;
                return '?';
            }
            else
                optarg = argv[optind++];
        sp = 1;
    }
    else
    {
        if (argv[optind][++sp] == '\0')
        {
            sp = 1;
            optind++;
        }
        optarg = 0;
    }
    return c;
}
