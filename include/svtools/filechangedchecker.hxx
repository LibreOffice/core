/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_FILECHANGEDCHECKER_HXX
#define INCLUDED_SVTOOLS_FILECHANGEDCHECKER_HXX

#include <svtools/svtdllapi.h>

#include <osl/file.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>

#include <functional>

/** Periodically checks if a file has been modified

    Instances of this class setup a vcl timer to occasionally wake up
    check whether file modification time has changed.
 */
class SVT_DLLPUBLIC FileChangedChecker
{
private:
    Idle                    mIdle;
    OUString            mFileName;
    TimeValue                mLastModTime;
    ::std::function<void ()> mpCallback;

    bool SVT_DLLPRIVATE getCurrentModTime(TimeValue& o_rValue) const;
    DECL_LINK_TYPED(TimerHandler, Idle *, void);

public:
    void resetTimer();
    bool hasFileChanged();
    FileChangedChecker(const OUString& rFilename,
            const ::std::function<void ()>& rCallback);
};

#endif
