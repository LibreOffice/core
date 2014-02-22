/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "sal/config.h"

#include "svtools/filechangedchecker.hxx"

FileChangedChecker::FileChangedChecker(const OUString& rFilename, const ::boost::function0<void>& rCallback) :
    mTimer(),
    mFileName(rFilename),
    mLastModTime(),
    mpCallback(rCallback)
{
    
    getCurrentModTime(mLastModTime);

    
    mTimer.SetTimeoutHdl(LINK(this, FileChangedChecker, TimerHandler));

    
    resetTimer();
}

void FileChangedChecker::resetTimer()
{
    
    if(!mTimer.IsActive())
        mTimer.Start();

    
    mTimer.SetTimeout(3000);
}

bool FileChangedChecker::getCurrentModTime(TimeValue& o_rValue) const
{
    
    osl::DirectoryItem aItem;
    osl::DirectoryItem::get(mFileName, aItem);

    
    
    osl::FileStatus aStatus( osl_FileStatus_Mask_ModifyTime );
    if( osl::FileBase::E_None != aItem.getFileStatus(aStatus) )
        return false;

    o_rValue = aStatus.getModifyTime();
    return true;
}

bool FileChangedChecker::hasFileChanged()
{
    
    TimeValue newTime={0,0};
    if( !getCurrentModTime(newTime) )
        return true; 

    
    
    if( newTime.Seconds != mLastModTime.Seconds ||
        newTime.Nanosec != mLastModTime.Nanosec )
    {
        
        
        mLastModTime = newTime ;

        return true;
    }
    else
        return false;
}

IMPL_LINK_NOARG(FileChangedChecker, TimerHandler)
{
    
    OSL_TRACE("Timeout Called");
    if(hasFileChanged())
    {
        OSL_TRACE("File modified");
        mpCallback();
    }

    
    resetTimer();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
