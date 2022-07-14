/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svtools/svtdllapi.h>

#include <osl/time.h>
#include <rtl/ustring.hxx>
#include <vcl/timer.hxx>

#include <functional>

/** Periodically checks if a file has been modified

    Instances of this class setup a vcl timer to occasionally wake up
    check whether file modification time has changed.
 */
class SVT_DLLPUBLIC FileChangedChecker
{
private:
    Timer                    mTimer;
    OUString                 mFileName;
    TimeValue                mLastModTime;
    ::std::function<void ()> mpCallback;

    bool SVT_DLLPRIVATE getCurrentModTime(TimeValue& o_rValue) const;
    DECL_DLLPRIVATE_LINK(TimerHandler, Timer *, void);

public:
    void resetTimer();
    // bUpdate = true  when file has changed, get the return and the object get the new time
    //         = false when file has changed, only get the return, not change the object
    bool hasFileChanged(bool bUpdate = true);
    FileChangedChecker(OUString aFilename,
            ::std::function<void ()> aCallback);
    // without Timer function
    FileChangedChecker(OUString aFilename);
};
