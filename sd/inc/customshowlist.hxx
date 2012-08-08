/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CUSTOMSHOWLIST_HXX
#define _CUSTOMSHOWLIST_HXX

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


#endif // _CUSTOMSHOWLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
