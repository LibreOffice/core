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

class SdCustomShowList : private std::vector<SdCustomShow*>
{
private:
   sal_uInt16 mnCurPos;
public:
    using std::vector<SdCustomShow*>::operator[];
    using std::vector<SdCustomShow*>::size;
    using std::vector<SdCustomShow*>::empty;
    using std::vector<SdCustomShow*>::push_back;
    using std::vector<SdCustomShow*>::erase;
    using std::vector<SdCustomShow*>::begin;
    using std::vector<SdCustomShow*>::iterator;

    SdCustomShowList()
        : mnCurPos(0)
    {
    }

    sal_uInt16 GetCurPos() const { return mnCurPos; }
    void Seek(sal_uInt16 nNewPos) { mnCurPos = nNewPos; }

    SdCustomShow* First()
    {
        if( empty() )
            return NULL;
        mnCurPos = 0;
        return operator[](mnCurPos);
    }
    SdCustomShow* Next()
    {
        ++mnCurPos;
        return mnCurPos >= size() ? NULL : operator[](mnCurPos);
    }
    void Last()
    {
        if( !empty() )
            mnCurPos = size() - 1;
    }
    SdCustomShow* GetCurObject()
    {
        return empty() ? NULL : operator[](mnCurPos);
    }
    SdCustomShow* Remove(SdCustomShow* p)
    {
        iterator it = std::find(begin(), end(), p);
        if( it == end() )
            return NULL;
        erase(it);
        return p;
    }
};


#endif // INCLUDED_SD_INC_CUSTOMSHOWLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
