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

#ifndef INCLUDED_SD_INC_CUSTOMSHOWLIST_HXX
#define INCLUDED_SD_INC_CUSTOMSHOWLIST_HXX

#include <vector>

class SdCustomShow;

class SdCustomShowList
{
private:
    std::vector<SdCustomShow*> mShows;
   sal_uInt16 mnCurPos;
public:
    SdCustomShowList()
        : mShows(), mnCurPos(0)
    {
    }

    bool empty() const {return mShows.empty();}

    size_t size() const {return mShows.size();}

    SdCustomShow* &operator[](size_t i) {return mShows[i];}

    std::vector<SdCustomShow*>::iterator begin() {return mShows.begin();}

    void erase(std::vector<SdCustomShow*>::iterator it) {mShows.erase(it);}

    void push_back(SdCustomShow* p) {mShows.push_back(p);}

    sal_uInt16 GetCurPos() const { return mnCurPos; }
    void Seek(sal_uInt16 nNewPos) { mnCurPos = nNewPos; }

    SdCustomShow* First()
    {
        if( mShows.empty() )
            return nullptr;
        mnCurPos = 0;
        return mShows[mnCurPos];
    }
    SdCustomShow* Next()
    {
        ++mnCurPos;
        return mnCurPos >= mShows.size() ? nullptr : mShows[mnCurPos];
    }
    void Last()
    {
        if( !mShows.empty() )
            mnCurPos = mShows.size() - 1;
    }
    SdCustomShow* GetCurObject()
    {
        return mShows.empty() ? nullptr : mShows[mnCurPos];
    }
    SdCustomShow* Remove(SdCustomShow* p)
    {
        std::vector<SdCustomShow*>::iterator it = std::find(mShows.begin(), mShows.end(), p);
        if( it == mShows.end() )
            return nullptr;
        mShows.erase(it);
        return p;
    }
};

#endif // INCLUDED_SD_INC_CUSTOMSHOWLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
