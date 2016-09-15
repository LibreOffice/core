/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/backupfilehelper.hxx>
#include <rtl/ustring.hxx>

namespace comphelper
{
    sal_uInt16 BackupFileHelper::mnMaxAllowedBackups = 10;
    bool BackupFileHelper::mbExitWasCalled = false;

    BackupFileHelper::BackupFileHelper(
        const OUString& rBaseURL,
        sal_uInt16 nNumBackups)
        : mrBaseURL(rBaseURL),
        mnNumBackups(::std::min(::std::max(nNumBackups, sal_uInt16(1)), mnMaxAllowedBackups)),
        maBase(),
        maExt(),
        maBaseFile(rBaseURL),
        mbBaseFileIsOpen(false)
    {
    }

    void BackupFileHelper::setExitWasCalled()
    {
        mbExitWasCalled = true;
    }

    bool BackupFileHelper::getExitWasCalled()
    {
        return mbExitWasCalled;
    }

    bool BackupFileHelper::tryPush()
    {
        if (isDifferentOrNew())
        {
            // the new file is different or new, create a new first backup
            // rename/move/cleanup other backup files, make space for new first backup
            push();

            // copy new one to now free 1st position
            osl::File::copy(mrBaseURL, getName(1));

            return true;
        }

        return false;
    }

    bool BackupFileHelper::isPopPossible()
    {
        return firstExists();
    }

    bool BackupFileHelper::tryPop()
    {
        if (firstExists())
        {
            // first copy exists, copy over original and delete
            const OUString aOneName(getName(1));
            maBaseFile.close();
            osl::File::copy(aOneName, mrBaseURL);
            osl::File::remove(aOneName);

            // rename/move/cleanup other backup files
            pop();

            return true;
        }

        return false;
    }

    rtl::OUString BackupFileHelper::getName(sal_uInt16 n)
    {
        if (maExt.isEmpty())
        {
            return OUString(maBase + "_" + OUString::number(n));
        }

        return OUString(maBase + "_" + OUString::number(n) + "." + maExt);
    }

    bool BackupFileHelper::firstExists()
    {
        if (baseFileOpen() && splitBaseURL())
        {
            // check if 1st copy exists
            osl::File aOneFile(getName(1));
            const osl::FileBase::RC aResult(aOneFile.open(osl_File_OpenFlag_Read));

            return (osl::File::E_None == aResult);
        }

        return false;
    }

    void BackupFileHelper::pop()
    {
        for (sal_uInt16 a(2); a < mnMaxAllowedBackups + 1; a++)
        {
            const OUString aSourceName(getName(a));

            if (a > mnNumBackups + 1)
            {
                // try to delete that file, it is out of scope
                osl::File::remove(aSourceName);
            }
            else
            {
                // rename that file by decreasing index by one
                osl::File::move(aSourceName, getName(a - 1));
            }
        }
    }

    void BackupFileHelper::push()
    {
        for (sal_uInt16 a(0); a < mnMaxAllowedBackups; a++)
        {
            const sal_uInt16 nIndex(mnMaxAllowedBackups - a);
            const OUString aSourceName(getName(nIndex));

            if (nIndex >= mnNumBackups)
            {
                // try to delete that file, it is out of scope
                osl::File::remove(aSourceName);
            }
            else
            {
                // rename that file by increasing index by one
                osl::File::move(aSourceName, getName(nIndex + 1));
            }
        }
    }

    bool BackupFileHelper::isDifferentOrNew()
    {
        if (baseFileOpen() && splitBaseURL())
        {
            osl::File aLastFile(getName(1));
            const osl::FileBase::RC aResult(aLastFile.open(osl_File_OpenFlag_Read));
            bool bDifferentOrNew(false);

            if (osl::File::E_None == aResult)
            {
                // exists, check for being equal
                bDifferentOrNew = !equalsBase(aLastFile);
            }
            else if (osl::File::E_NOENT == aResult)
            {
                // does not exist - also copy
                bDifferentOrNew = true;
            }

            return bDifferentOrNew;
        }

        return false;
    }

    bool BackupFileHelper::equalsBase(osl::File& rLastFile)
    {
        sal_uInt64 nBaseLen(0);
        sal_uInt64 nLastLen(0);
        maBaseFile.getSize(nBaseLen);
        rLastFile.getSize(nLastLen);

        if (nBaseLen == nLastLen)
        {
            // same filesize -> need to check content
            sal_uInt8 aArrayOld[1024];
            sal_uInt8 aArrayLast[1024];
            sal_uInt64 nBytesReadBase(0);
            sal_uInt64 nBytesReadLast(0);
            bool bDiffers(false);

            // both rewind on start
            maBaseFile.setPos(0, 0);
            rLastFile.setPos(0, 0);

            while (!bDiffers
                && osl::File::E_None == maBaseFile.read(static_cast<void*>(aArrayOld), 1024, nBytesReadBase)
                && osl::File::E_None == rLastFile.read(static_cast<void*>(aArrayLast), 1024, nBytesReadLast)
                && 0 != nBytesReadBase
                && nBytesReadBase == nBytesReadLast)
            {
                bDiffers = memcmp(aArrayOld, aArrayLast, nBytesReadBase);
            }

            return !bDiffers;
        }

        return false;
    }

    bool BackupFileHelper::splitBaseURL()
    {
        if (maBase.isEmpty() && !mrBaseURL.isEmpty())
        {
            const sal_Int32 nIndex(mrBaseURL.lastIndexOf('.'));

            if (-1 == nIndex)
            {
                maBase = mrBaseURL;
            }
            else if (nIndex > 0)
            {
                maBase = mrBaseURL.copy(0, nIndex);
            }

            if (mrBaseURL.getLength() > nIndex + 1)
            {
                maExt = mrBaseURL.copy(nIndex + 1);
            }
        }

        return !maBase.isEmpty();
    }

    bool BackupFileHelper::baseFileOpen()
    {
        if (!mbBaseFileIsOpen && !mrBaseURL.isEmpty())
        {
            mbBaseFileIsOpen = (osl::File::E_None == maBaseFile.open(osl_File_OpenFlag_Read));
        }

        return mbBaseFileIsOpen;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
