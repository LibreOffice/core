/*************************************************************************
 *
 *  $RCSfile: t_store.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-13 21:15:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#define _T_STORE_CXX "$Revision: 1.3 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _STORE_STORE_HXX_
#include <store/store.hxx>
#endif

#include <stdio.h>

#if (defined(WNT) && defined(PROFILE))
extern "C"
{
    void StartCAP (void);
    void StopCAP  (void);
    void DumpCAP  (void);
}
#endif /* PROFILE */

using namespace rtl;

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
    sal_uInt32  m_nLevel;
    sal_uInt32  m_nCount;

public:
    DirectoryTraveller (
        const file&     rFile,
        const OUString &rPath,
        const OUString &rName,
        sal_uInt32      nOptions,
        sal_uInt32      nLevel = 0);

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
    sal_uInt32      nLevel)
    : m_aFile    (rFile),
      m_aPath    (rPath),
      m_nOptions (nOptions),
      m_nLevel   (nLevel),
      m_nCount   (0)
{
    m_aPath += rName;
    m_aPath += OUString::createFromAscii("/");
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
        OString aName (it.m_pszName, it.m_nLength, RTL_TEXTENCODING_UTF8);
        printf ("Visit(%d,%d): %s [0x%08x] %d [Bytes]\n",
                m_nLevel, m_nCount,
                aName.pData->buffer, it.m_nAttrib, it.m_nSize);
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
    OTime aStartTime (OTime::getSystemTime());
#endif /* PROFILE */

    store::OStoreFile aFile;
    storeError eErrCode = store_E_None;

    sal_uInt32 nOptions = 0;
    for (int i = 1; i < argc; i++)
    {
        const char *opt = argv[i];
        if (opt[0] == '-')
        {
            switch (rtl_char_toLowerCase(opt[1]))
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
        printf ("Options:\n");
        printf ("-f\tfilename\n");
        printf ("-h\thelp\n");
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
    OUString aPath (RTL_CONSTASCII_USTRINGPARAM("/"));
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

        for (sal_Int32 i = 0; i < _DEMOSTOR_LOOPS; i++)
        {
            OUString aName (RTL_CONSTASCII_USTRINGPARAM("demostor-"));
            aName += OUString::valueOf ((sal_Int32)(i + 1), 10);
            aName += OUString::createFromAscii (".dat");

#if (_DEMOSTOR_REMOVE == 1)
            eErrCode = aFile.remove (aPath, aName);
            if ((eErrCode != store_E_None     ) &&
                (eErrCode != store_E_NotExists)    )
                break;
#endif /* _REMOVE */

            store::OStoreStream aStream;
            eErrCode = aStream.create (aFile, aPath, aName, eMode);
            if (eErrCode != store_E_None)
                break;

            if (nOptions & OPTION_TRUNC)
            {
                eErrCode = aStream.setSize(0);
                if (eErrCode != store_E_None)
                    break;
            }

            sal_uInt32 nDone = 0;
            if (nOptions & OPTION_WRITE)
            {
                eErrCode = aStream.writeAt (
                    0, pBuffer, sizeof(pBuffer), nDone);
                if (eErrCode != store_E_None)
                    break;
            }

            if (nOptions & OPTION_READ)
            {
                sal_uInt32 nOffset = 0;
                for (;;)
                {
                    eErrCode = aStream.readAt (
                        nOffset, pBuffer, sizeof(pBuffer), nDone);
                    if (eErrCode != store_E_None)
                        break;
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

        printf ("Total(rd,wr): %d[usec]\n", nDelta);
#endif /* PROFILE */
    }

    // Link/Rename.
    if (nOptions & OPTION_LINK)
    {
        // Create symlink to (root) directory.
        eErrCode = aFile.symlink (
            aPath,      OUString::createFromAscii("000000/"),
            OUString(), aPath);
        OSL_POSTCOND(
            ((eErrCode == store_E_None         ) ||
             (eErrCode == store_E_AlreadyExists)    ),
            "t_store::main(): store_symlink() failed");

        // Create symlink to file.
        OUString aLinkName (RTL_CONSTASCII_USTRINGPARAM("demostor-1.lnk"));

        eErrCode = aFile.symlink (
            aPath, aLinkName,
            aPath, OUString::createFromAscii("demostor-1.dat"));
        OSL_POSTCOND(
            ((eErrCode == store_E_None         ) ||
             (eErrCode == store_E_AlreadyExists)    ),
            "t_store::main(): store_symlink() failed");
        if ((eErrCode == store_E_None         ) ||
            (eErrCode == store_E_AlreadyExists)    )
        {
            OUString aShortcut (
                RTL_CONSTASCII_USTRINGPARAM("Shortcut to demostor-1.dat"));
            eErrCode = aFile.rename (
                aPath, aLinkName,
                aPath, aShortcut);
            OSL_POSTCOND(
                ((eErrCode == store_E_None         ) ||
                 (eErrCode == store_E_AlreadyExists)    ),
                "t_store::main(): store_rename() failed");
        }

        // Create directory.
        OUString aDirName (RTL_CONSTASCII_USTRINGPARAM("demostor-1.dir"));
        store::OStoreDirectory aDir;

        eErrCode = aDir.create (
            aFile, aPath, aDirName, store_AccessReadCreate);
        OSL_POSTCOND(
            (eErrCode == store_E_None),
            "t_store::main(): store_createDirectory() failed");
        if (eErrCode == store_E_None)
        {
#if 0  /* NYI */
            // Rename directory.
            eErrCode = aFile.rename (
                aPath, "demostor-1.dir/",
                aPath, "Renamed demostor-1.dir");
            OSL_POSTCOND(
                ((eErrCode == store_E_None         ) ||
                 (eErrCode == store_E_AlreadyExists)    ),
                "t_store::main(): store_rename() failed");

            eErrCode = aFile.rename (
                aPath, "Renamed demostor-1.dir/",
                aPath, "demostor-1.dir");
            OSL_POSTCOND(
                (eErrCode == store_E_None),
                "t_store::main(): store_rename() failed");
#endif  /* NYI */
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
                aFile, aPath, OUString::createFromAscii("000000"),
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

        printf ("Total(iter): %d[usec]\n", nDelta);
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
    OTime aDelta (OTime::getSystemTime() - aStartTime);

    sal_uInt32 nDelta = aDelta.Seconds * 1000000;
    nDelta += (aDelta.Nanosec / 1000);

    printf ("Total: %d[usec]\n", nDelta);
#endif /* PROFILE */

    return 0;
}

