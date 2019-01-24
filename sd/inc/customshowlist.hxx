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

#include "sddllapi.h"
#include "cusshow.hxx"
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>

class SD_DLLPUBLIC SdCustomShowList
{
private:
    std::vector<std::unique_ptr<SdCustomShow>> mShows;
   sal_uInt16 mnCurPos;
public:
    SdCustomShowList()
        : mShows(), mnCurPos(0)
    {
    }

    SdCustomShowList& operator=( SdCustomShowList const & ) = delete; // MSVC2017 workaround
    SdCustomShowList( SdCustomShowList const & ) = delete; // MSVC2017 workaround

    bool empty() const {return mShows.empty();}

    size_t size() const {return mShows.size();}

    std::unique_ptr<SdCustomShow>& operator[](size_t i) {return mShows[i];}

    std::vector<std::unique_ptr<SdCustomShow>>::iterator begin() {return mShows.begin();}

    void erase(std::vector<std::unique_ptr<SdCustomShow>>::iterator it);

    void push_back(std::unique_ptr<SdCustomShow> p) {mShows.push_back(std::move(p));}

    sal_uInt16 GetCurPos() const { return mnCurPos; }
    void Seek(sal_uInt16 nNewPos) { mnCurPos = nNewPos; }

    SdCustomShow* First()
    {
        if( mShows.empty() )
            return nullptr;
        mnCurPos = 0;
        return mShows[mnCurPos].get();
    }
    SdCustomShow* Next()
    {
        ++mnCurPos;
        return mnCurPos >= mShows.size() ? nullptr : mShows[mnCurPos].get();
    }
    void Last()
    {
        if( !mShows.empty() )
            mnCurPos = mShows.size() - 1;
    }
    SdCustomShow* GetCurObject()
    {
        return mShows.empty() ? nullptr : mShows[mnCurPos].get();
    }
    void erase(const SdCustomShow* p)
    {
        auto it = std::find_if(mShows.begin(), mShows.end(),
                [&] (std::unique_ptr<SdCustomShow> const &i) { return i.get() == p; });
        assert( it != mShows.end() );
        if( it != mShows.end() )
            mShows.erase(it);
    }
};

#endif // INCLUDED_SD_INC_CUSTOMSHOWLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
