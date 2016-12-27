/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ONLINEUPDATE_SERVICE_WINDOWSHELPER_HXX
#define INCLUDED_ONLINEUPDATE_SERVICE_WINDOWSHELPER_HXX

struct AutoHandle
{
    AutoHandle(HANDLE handle):
        mHandle(handle)
    {
    }

    ~AutoHandle()
    {
        release(mHandle);
    }

    void release(HANDLE handle)
    {
        if (handle && handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
        }
    }

    HANDLE get() const
    {
        return mHandle;
    }

    bool operator==(const AutoHandle& rhs)
    {
        return mHandle == rhs.mHandle;
    }

    HANDLE mHandle;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
