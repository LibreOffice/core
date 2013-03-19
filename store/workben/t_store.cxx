/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#define _T_STORE_CXX
#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/time.h>
#include <rtl/ustring.hxx>
#include <store/store.hxx>

#include <stdio.h>

#if (defined(WNT) && defined(PROFILE))
extern "C"
{
    void StartCAP (void);
    void StopCAP  (void);
    void DumpCAP  (void);
}
#endif /* PROFILE */

using rtl::OUString;

/*========================================================================
 *
 * Internals.
 *
 *======================================================================*/
#define _DEMOSTOR_BUFSIZ          512  /* 4096, 1024, 512 */
#define _DEMOSTOR_LOOPS           1000 /* 1000, 2000 */

#define _DEMOSTOR_REMOVE          0
#define _DEMOSTOR_REBUILD         0

enum Options
{
    OPTION_HELP    = 0x0001,
    OPTION_FILE    = 0x0002,
    OPTION_PAUSE   = 0x0004,
    OPTION_REBUILD = 0x0008,

    OPTION_DUMP    = 0x0010,
    OPTION_ITER    = 0x0020,
    OPTION_LINK    = 0x0040,

    OPTION_READ    = 0x0100,
    OPTION_WRITE   = 0x0200,
    OPTION_CREAT   = 0x0400,
    OPTION_TRUNC   = 0x0800
};

inline sal_Char ascii_toLowerCase (sal_Char ch)
{
    if ((ch >= 0x41) && (ch <= 0x5A))
        return (ch + 0x20);
    else
        return (ch);
}

/*========================================================================
 *
 * Timing.
 *
 *======================================================================*/
struct OTime : public TimeValue
{
    OTime (void)
    {
        Seconds = 0;
        Nanosec = 0;
    }

    static OTime getSystemTime (void)
    {
        OTime tv;
        osl_getSystemTime (&tv);
        return tv;
    }

    OTime& operator-= (const OTime& rPast)
    {
        Seconds -= rPast.Seconds;
        if (Nanosec < rPast.Nanosec)
        {
            Seconds -= 1;
            Nanosec += 1000000000;
        }
        Nanosec -= rPast.Nanosec;
        return *this;
    }

    friend OTime operator- (const OTime& rTimeA, const OTime& rTimeB)
    {
        OTime aTimeC (rTimeA);
        aTimeC -= rTimeB;
        return aTimeC;
    }
};

/*========================================================================
 *
 * DirectoryTraveller.
 *
 *======================================================================*/
typedef store::OStoreDirectory Directory;

class DirectoryTraveller : public Directory::traveller
{
    typedef store::OStoreFile   file;
    typedef Directory::iterator iter;

    store::OStoreFile m_aFile;
    OUString          m_aPath;

    sal_uInt32  m_nOptions;
    unsigned int  m_nLevel;
    unsigned int  m_nCount;

public:
    DirectoryTraveller (
        const file&     rFile,
        const OUString &rPath,
        const OUString &rName,
        sal_uInt32      nOptions,
        unsigned int nLevel = 0);

    virtual ~DirectoryTraveller (void);

    virtual sal_Bool visit (const iter& it);
};

/*
 * DirectoryTraveller.
 */
DirectoryTraveller::DirectoryTraveller (
    const file&     rFile,
    const OUString &rPath,
    const OUString &rName,
    sal_uInt32      nOptions,
    unsigned int nLevel)
    : m_aFile    (rFile),
      m_aPath    (rPath),
      m_nOptions (nOptions),
      m_nLevel   (nLevel),
      m_nCount   (0)
{
    m_aPath += rName + "/";
}

/*
 * ~DirectoryTraveller.
 */
DirectoryTraveller::~DirectoryTraveller (void)
{
}

/*
 * visit.
 */
sal_Bool DirectoryTraveller::visit (const iter& it)
{
    m_nCount++;
    if (m_nOptions & OPTION_DUMP)
    {
        rtl::OString aName (it.m_pszName, it.m_nLength, RTL_TEXTENCODING_UTF8);
        printf ("Visit(%u,%u): %s [0x%08x] %d [Bytes]\n",
                m_nLevel, m_nCount,
                aName.pData->buffer, (unsigned int)(it.m_nAttrib), (unsigned int)(it.m_nSize));
    }
    if (it.m_nAttrib & STORE_ATTRIB_ISDIR)
    {
        OUString  aName (it.m_pszName, it.m_nLength);
        if (aName.compareToAscii ("XTextViewCursorSupplier") == 0)
        {
            m_nCount += 1 - 1;
        }
        Directory aSubDir;

        storeError eErrCode = aSubDir.create (
            m_aFile, m_aPath, aName, store_AccessReadOnly);
        if (eErrCode == store_E_None)
        {
            sal_uInt32 nRefCount = 0;
            m_aFile.getRefererCount (nRefCount);

            DirectoryTraveller aSubTraveller (
                m_aFile, m_aPath, aName, m_nOptions, m_nLevel + 1);
            aSubDir.travel (aSubTraveller);
        }
    }
    return sal_True;
}

/*========================================================================
 *
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
#if (defined(WNT) && defined(PROFILE))
    StartCAP();
#else
    OTime aMainStartTime (OTime::getSystemTime());
#endif /* PROFILE */

    store::OStoreFile aFile;
    storeError eErrCode = store_E_None;

    sal_uInt32 nOptions = 0;
    for (int i = 1; i < argc; i++)
    {
        const char *opt = argv[i];
        if (opt[0] == '-')
        {
            switch (ascii_toLowerCase(sal_Char(opt[1])))
            {
                case 'f':
                    nOptions |= OPTION_FILE;
                    break;

                case 'd':
                    nOptions |= OPTION_DUMP;
                    break;
                case 'i':
                    nOptions |= OPTION_ITER;
                    break;
                case 'l':
                    nOptions |= OPTION_LINK;
                    break;

                case 'r':
                    nOptions |= OPTION_READ;
                    break;
                case 'w':
                    nOptions |= OPTION_WRITE;
                    break;
                case 'c':
                    nOptions |= OPTION_CREAT;
                    break;
                case 't':
                    nOptions |= OPTION_TRUNC;
                    break;

                case 'p':
                    nOptions |= OPTION_PAUSE;
                    break;

                case 'h':
                default:
                    nOptions |= OPTION_HELP;
                    break;
            }
        }
        else
        {
            if (nOptions & OPTION_FILE)
            {
                OUString aName (
                    argv[i], rtl_str_getLength(argv[i]),
                    osl_getThreadTextEncoding());
                if ((nOptions & OPTION_CREAT) && (nOptions & OPTION_TRUNC))
                    eErrCode = aFile.create (aName, store_AccessCreate);
                else if (nOptions & OPTION_CREAT)
                    eErrCode = aFile.create (aName, store_AccessReadCreate);
                else if (nOptions & OPTION_WRITE)
                    eErrCode = aFile.create (aName, store_AccessReadWrite);
                else
                    eErrCode = aFile.create (aName, store_AccessReadOnly);
                if (eErrCode != store_E_None)
                {
                    printf ("Error: can't open file: %s\n", argv[i]);
                    exit (0);
                }
            }
        }
    }

    if ((nOptions == 0) || (nOptions & OPTION_HELP))
    {
        printf ("Usage:\tt_store "
                "[[-c] [-t] [-r] [-w]] [[-i] [-d] [-h]] "
                "[-f filename]\n");

        printf ("\nOptions:\n");
        printf ("-c\tcreate\n");
        printf ("-t\ttruncate\n");
        printf ("-r\tread\n");
        printf ("-w\twrite\n");
        printf ("-i\titerate\n");
        printf ("-d\tdump\n");
        printf ("-h\thelp\n");
        printf ("-f\tfilename\n");

        printf ("\nExamples:");
        printf ("\nt_store -c -w -f t_store.rdb\n");
        printf ("\tCreate file 't_store.rdb',\n"
                "\twrite fixed number (1000) of streams.\n");
        printf ("\nt_store -c -i -d -f t_store.rdb\n");
        printf ("\tOpen file 't_store.rdb', "
                "create '/' directory,\n"
                "\titerate directory tree, "
                "dump directory info.\n");

        exit (0);
    }

    if (!aFile.isValid())
    {
        eErrCode = aFile.createInMemory();
        if (eErrCode != store_E_None)
        {
            printf ("Error: can't create memory file\n");
            exit (0);
        }
    }

    // Stream Read/Write.
    OUString aPath ("/");
    if ((nOptions & OPTION_READ) || (nOptions & OPTION_WRITE))
    {
        // Mode.
        storeAccessMode eMode = store_AccessReadOnly;
        if (nOptions & OPTION_WRITE)
            eMode = store_AccessReadWrite;
        if (nOptions & OPTION_CREAT)
            eMode = store_AccessCreate;

        // Buffer.
        char pBuffer[_DEMOSTOR_BUFSIZ] = "Hello World";
        pBuffer[_DEMOSTOR_BUFSIZ - 2] = 'B';
        pBuffer[_DEMOSTOR_BUFSIZ - 1] = '\0';

        // Load/Save.
#ifndef PROFILE
        OTime aStartTime (OTime::getSystemTime());
#endif /* PROFILE */

        for (int i = 0; i < _DEMOSTOR_LOOPS; i++)
        {
            OUString aName ("demostor-");
            aName += OUString::valueOf ((sal_Int32)(i + 1), 10);
            aName += ".dat";

#if (_DEMOSTOR_REMOVE == 1)
            eErrCode = aFile.remove (aPath, aName);
            if ((eErrCode != store_E_None     ) &&
                (eErrCode != store_E_NotExists)    )
                break;
#endif /* _REMOVE */

            store::OStoreStream aStream;
            eErrCode = aStream.create (aFile, aPath, aName, eMode);
            if (eErrCode != store_E_None)
            {
                OSL_TRACE("OStoreStream(%d)::create(): error: %d", i, eErrCode);
                break;
            }

            if (nOptions & OPTION_TRUNC)
            {
                eErrCode = aStream.setSize(0);
                if (eErrCode != store_E_None)
                {
                    OSL_TRACE("OStoreStream(%d)::setSize(0): error: %d", i, eErrCode);
                    break;
                }
            }

            sal_uInt32 nDone = 0;
            if (nOptions & OPTION_WRITE)
            {
                eErrCode = aStream.writeAt (
                    0, pBuffer, sizeof(pBuffer), nDone);
                if (eErrCode != store_E_None)
                {
                    OSL_TRACE("OStoreStream(%d)::writeAt(): error: %d", i, eErrCode);
                    break;
                }
            }

            if (nOptions & OPTION_READ)
            {
                sal_uInt32 nOffset = 0;
                for (;;)
                {
                    eErrCode = aStream.readAt (
                        nOffset, pBuffer, sizeof(pBuffer), nDone);
                    if (eErrCode != store_E_None)
                    {
                        OSL_TRACE("OStoreStream(%d)::readAt(): error: %d", i, eErrCode);
                        break;
                    }
                    if (nDone == 0)
                        break;
                    nOffset += nDone;
                }
            }

            aStream.close();

#ifndef PROFILE
            if (((i + 1) % (_DEMOSTOR_LOOPS/10)) == 0)
            {
                OTime aDelta (OTime::getSystemTime() - aStartTime);

                sal_uInt32 nDelta = aDelta.Seconds * 1000000;
                nDelta += (aDelta.Nanosec / 1000);

                printf ("%d: %12.4g[usec]\n", (i+1),
                        (double)(nDelta)/(double)(i+1));
            }
#endif /* PROFILE */
        }

#ifndef PROFILE
        OTime aDelta (OTime::getSystemTime() - aStartTime);

        sal_uInt32 nDelta = aDelta.Seconds * 1000000;
        nDelta += (aDelta.Nanosec / 1000);

        printf ("Total(rd,wr): %d[usec]\n", (unsigned int)(nDelta));
#endif /* PROFILE */
    }

    // Link/Rename.
    if (nOptions & OPTION_LINK)
    {
        // Create symlink to (root) directory.
        eErrCode = aFile.symlink (
            aPath,      OUString("000000/"),
            OUString(), aPath);
        OSL_POSTCOND(
            ((eErrCode == store_E_None         ) ||
             (eErrCode == store_E_AlreadyExists)    ),
            "t_store::main(): store_symlink() failed");

        // Create symlink to file.
        OUString aLinkName ("demostor-1.lnk");

        eErrCode = aFile.symlink (
            aPath, aLinkName,
            aPath, OUString("demostor-1.dat"));
        OSL_POSTCOND(
            ((eErrCode == store_E_None         ) ||
             (eErrCode == store_E_AlreadyExists)    ),
            "t_store::main(): store_symlink() failed");
        if ((eErrCode == store_E_None         ) ||
            (eErrCode == store_E_AlreadyExists)    )
        {
            OUString aShortcut (
                "Shortcut to demostor-1.dat");
            eErrCode = aFile.rename (
                aPath, aLinkName,
                aPath, aShortcut);
            OSL_POSTCOND(
                ((eErrCode == store_E_None         ) ||
                 (eErrCode == store_E_AlreadyExists)    ),
                "t_store::main(): store_rename() failed");
        }

        // Create directory.
        OUString aDirName ("demostor-1.dir");
        store::OStoreDirectory aDir;

        eErrCode = aDir.create (
            aFile, aPath, aDirName, store_AccessReadCreate);
        OSL_POSTCOND(
            (eErrCode == store_E_None),
            "t_store::main(): store_createDirectory() failed");
        if (eErrCode == store_E_None)
        {
        }
    }

    // Directory iteration.
    if (nOptions & OPTION_ITER)
    {
#ifndef PROFILE
        OTime aStartTime (OTime::getSystemTime());
#endif /* PROFILE */
        OUString aEmpty;

        // Root directory.
        store::OStoreDirectory aRootDir;
        if (nOptions & OPTION_LINK)
        {
            // Open symlink entry.
            eErrCode = aRootDir.create (
                aFile, aPath, OUString("000000"),
                store_AccessReadOnly);
        }
        else
        {
            // Open direct entry.
            if (nOptions & OPTION_CREAT)
                eErrCode = aRootDir.create (
                    aFile, aEmpty, aEmpty, store_AccessReadCreate);
            else if (nOptions & OPTION_WRITE)
                eErrCode = aRootDir.create (
                    aFile, aEmpty, aEmpty, store_AccessReadWrite);
            else
                eErrCode = aRootDir.create (
                    aFile, aEmpty, aEmpty, store_AccessReadOnly);
        }

        if (eErrCode == store_E_None)
        {
            // Traverse directory tree.
            DirectoryTraveller aTraveller (
                aFile, aEmpty, aEmpty, nOptions, 0);
            aRootDir.travel (aTraveller);
        }
        else
        {
            // Failure.
            printf ("Error: can't open directory: \"/\"\n");
        }

#ifndef PROFILE
        OTime aDelta (OTime::getSystemTime() - aStartTime);

        sal_uInt32 nDelta = aDelta.Seconds * 1000000;
        nDelta += (aDelta.Nanosec / 1000);

        printf ("Total(iter): %d[usec]\n", (unsigned int)(nDelta));
#endif /* PROFILE */
    }

    if (nOptions & OPTION_PAUSE)
    {
        TimeValue tv;
        tv.Seconds = 300;
        tv.Nanosec = 0;
        osl_waitThread (&tv);
    }

    // Size.
    sal_uInt32 nSize = 0;
    aFile.getSize (nSize);

    // Done.
    aFile.close();

#if (defined(WNT) && defined(PROFILE))
    StopCAP();
    DumpCAP();
#endif /* PROFILE */
#ifndef PROFILE
    OTime aDelta (OTime::getSystemTime() - aMainStartTime);

    sal_uInt32 nDelta = aDelta.Seconds * 1000000;
    nDelta += (aDelta.Nanosec / 1000);

    printf ("Total: %d[usec]\n", (unsigned int)(nDelta));
#endif /* PROFILE */

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
