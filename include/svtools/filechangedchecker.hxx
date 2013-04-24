/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SVTOOLS_INCLUDED_FILECHANGEDCHECKER_HXX
#define SVTOOLS_INCLUDED_FILECHANGEDCHECKER_HXX

#include "svtools/svtdllapi.h"
#include <boost/function.hpp>
#include <osl/file.hxx>
#include <vcl/timer.hxx>

/** Periodically checks if a file has been modified

    Instances of this class setup a vcl timer to occasionally wake up
    check whether file modification time has changed.
 */
class SVT_DLLPUBLIC FileChangedChecker
{
private :
    Timer                    mTimer;
    OUString            mFileName;
    TimeValue                mLastModTime;
    ::boost::function0<void> mpCallback;

    bool SVT_DLLPRIVATE getCurrentModTime(TimeValue& o_rValue) const;
    DECL_LINK(TimerHandler, void *);

public :
    void resetTimer();
    bool hasFileChanged();
    FileChangedChecker(const OUString& rFilename, const ::boost::function0<void>& rCallback);
};

#endif
