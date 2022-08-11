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
#include <tools/long.hxx>
#include <vcl/naturalsort.hxx>

class SwContentType;

enum class ContentTypeId
{
    OUTLINE        = 0,
    TABLE          = 1,
    FRAME          = 2,
    GRAPHIC        = 3,
    OLE            = 4,
    BOOKMARK       = 5,
    REGION         = 6,
    URLFIELD       = 7,
    REFERENCE      = 8,
    INDEX          = 9,
    POSTIT         = 10,
    DRAWOBJECT     = 11,
    TEXTFIELD      = 12,
    FOOTNOTE       = 13,
    ENDNOTE        = 14,
    LAST           = ENDNOTE,
    UNKNOWN        = -1
};

// strings for context menus
#define CONTEXT_COUNT   17
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
    sal_uInt8 m_nTypeId;

    public:
        SwTypeNumber(sal_uInt8 nId) :m_nTypeId(nId){}
        virtual ~SwTypeNumber();

        sal_uInt8 GetTypeId() const { return m_nTypeId;}
};

class SwContent : public SwTypeNumber
{
    const SwContentType*    m_pParent;
    OUString                m_sContentName;
    double m_nYPosition;
        // some subclasses appear to use this for a tools/gen.hxx-style
        // geometric Y position, while e.g. SwOutlineContent wants to store
        // the index in its subtree
    bool                    m_bInvisible;
public:
        SwContent(const SwContentType* pCnt, OUString aName, double nYPos);

    virtual bool            IsProtect() const;
    const SwContentType*    GetParent() const {return m_pParent;}
    const OUString&         GetName()   const {return m_sContentName;}
    bool operator==(const SwContent& /*rCont*/) const
    {
        // they're never equal, otherwise they'd fall out of the array
        return false;
    }
    bool operator<(const SwContent& rCont) const
    {
        // at first sort by position and then by name
        if (m_nYPosition != rCont.m_nYPosition)
            return m_nYPosition < rCont.m_nYPosition;
        return vcl::NaturalSortCompare(m_sContentName, rCont.m_sContentName) < 0;
    }

    bool        IsInvisible() const {return m_bInvisible;}
    void        SetInvisible(){ m_bInvisible = true;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
