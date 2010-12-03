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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "svtools/filechangedchecker.hxx"
#include <osl/time.h>
#include "com/sun/star/system/XSystemShellExecute.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

FileChangedChecker::FileChangedChecker(const rtl::OUString& rFilename, const ::boost::function0<void>& rCallback) :
    mTimer(),
    mFileName(rFilename),
    mLastModTime(),
    mpCallback(rCallback)
{
    // Get the curren last file modified Status
    getCurrentModTime(mLastModTime);

    // associate the callback function for the timer
    mTimer.SetTimeoutHdl(LINK(this, FileChangedChecker, TimerHandler));

    //start the timer
    resetTimer();
}

void FileChangedChecker::resetTimer()
{
    //Start the timer if its not active
    if(!mTimer.IsActive())
        mTimer.Start();

    // Set a timeout of 3 seconds
    mTimer.SetTimeout(3000);
}

bool FileChangedChecker::getCurrentModTime(TimeValue& o_rValue) const
{
    // Need a Directory item to fetch file status
    osl::DirectoryItem aItem;
    osl::DirectoryItem::get(mFileName, aItem);

    // Retrieve the status - we are only interested in last File
    // Modified time
    osl::FileStatus aStatus( FileStatusMask_ModifyTime );
    if( osl::FileBase::E_None != aItem.getFileStatus(aStatus) )
        return false;

    o_rValue = aStatus.getModifyTime();
    return true;
}

bool FileChangedChecker::hasFileChanged()
{
    // Get the current file Status
    TimeValue newTime={0,0};
    if( !getCurrentModTime(newTime) )
        return true; // well. hard to answer correctly here ...

    // Check if the seconds time stamp has any difference
    // If so, then our file has changed meanwhile
    if( newTime.Seconds != mLastModTime.Seconds ||
        newTime.Seconds != mLastModTime.Seconds )
    {
        // Since the file has changed, set the new status as the file status and
        // return True
        mLastModTime = newTime ;

        return true;
    }
    else
        return false;
}

IMPL_LINK(FileChangedChecker, TimerHandler, Timer *, EMPTYARG)
{
    // If the file has changed, then update the graphic in the doc
    OSL_TRACE("Timeout Called \n");
    if(hasFileChanged())
    {
        OSL_TRACE("File modified \n");
        mpCallback();
    }

    // Reset the timer in any case
    resetTimer();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
