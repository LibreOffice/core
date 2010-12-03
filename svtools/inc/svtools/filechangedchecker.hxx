/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Surendran Mahendran <surenspost@gmail.com>]
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef SVTOOLS_INCLUDED_FILECHANGEDCHECKER_HXX
#define SVTOOLS_INCLUDED_FILECHANGEDCHECKER_HXX

#include <boost/function.hpp>
#include <osl/file.hxx>
#include <vcl/timer.hxx>

/** Periodically checks if a file has been modified

    Instances of this class setup a vcl timer to occasionally wake up
    check whether file modification time has changed.
 */
class FileChangedChecker
{
private :
    Timer                    mTimer;
    rtl::OUString            mFileName;
    TimeValue                mLastModTime;
    ::boost::function0<void> mpCallback;

    bool getCurrentModTime(TimeValue& o_rValue) const;

public :
    DECL_LINK(TimerHandler, Timer *);
    void resetTimer();
    bool hasFileChanged();
    FileChangedChecker(const rtl::OUString& rFilename, const ::boost::function0<void>& rCallback);
};

#endif
