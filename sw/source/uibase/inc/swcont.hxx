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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWCONT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWCONT_HXX

#include <rtl/ustring.hxx>
#include <vcl/lstbox.hxx>

class SwContentType;

// synchronize order and number with ResIds!!
#define CONTENT_TYPE_OUTLINE        0
#define CONTENT_TYPE_TABLE          1
#define CONTENT_TYPE_FRAME          2
#define CONTENT_TYPE_GRAPHIC        3
#define CONTENT_TYPE_OLE            4
#define CONTENT_TYPE_BOOKMARK       5
#define CONTENT_TYPE_REGION         6
#define CONTENT_TYPE_URLFIELD       7
#define CONTENT_TYPE_REFERENCE      8
#define CONTENT_TYPE_INDEX          9
#define CONTENT_TYPE_POSTIT         10
#define CONTENT_TYPE_DRAWOBJECT     11
#define CONTENT_TYPE_MAX            CONTENT_TYPE_DRAWOBJECT +1

// strings for context menus
#define CONTEXT_COUNT   12
#define GLOBAL_CONTEXT_COUNT 14

// modes for Drag 'n Drop
enum class RegionMode
{
    NONE        = 0,
    LINK        = 1,
    EMBEDDED    = 2
};

//mini rtti
class SwTypeNumber
{
    sal_uInt8 nTypeId;

    public:
        SwTypeNumber(sal_uInt8 nId) :nTypeId(nId){}
        virtual ~SwTypeNumber();

        sal_uInt8 GetTypeId() const { return nTypeId;}
};

class SwContent : public SwTypeNumber
{
    const SwContentType*    pParent;
    OUString                sContentName;
    long                    nYPosition;
    bool                    bInvisible;
public:
        SwContent(const SwContentType* pCnt, const OUString& rName, long nYPos );

    virtual bool            IsProtect() const;
    const SwContentType*    GetParent() const {return pParent;}
    const OUString&         GetName()   const {return sContentName;}
    bool operator==(const SwContent& /*rCont*/) const
    {
        // they're never equal, otherwise they'd fall out of the array
        return false;
    }
    bool operator<(const SwContent& rCont) const
    {
        // at first sort by position and then by name
        if (nYPosition != rCont.nYPosition)
            return nYPosition < rCont.nYPosition;
        return ListBox::NaturalSortCompare(sContentName, rCont.sContentName) < 0;
    }

    long        GetYPos() const {return nYPosition;}

    bool        IsInvisible() const {return bInvisible;}
    void        SetInvisible(){ bInvisible = true;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
