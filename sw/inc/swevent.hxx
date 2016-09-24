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

#ifndef INCLUDED_SW_INC_SWEVENT_HXX
#define INCLUDED_SW_INC_SWEVENT_HXX

#include <tools/solar.h>
#include <sfx2/sfx.hrc>

#define     SW_EVENT_OBJECT_SELECT        ( EVENT_APP_START + 0 )
#define     SW_EVENT_START_INS_GLOSSARY   ( EVENT_APP_START + 1 )
#define     SW_EVENT_END_INS_GLOSSARY     ( EVENT_APP_START + 2 )
#define     SW_EVENT_MAIL_MERGE           ( EVENT_APP_START + 3 )
#define     SW_EVENT_FRM_KEYINPUT_ALPHA   ( EVENT_APP_START + 4 )
#define     SW_EVENT_FRM_KEYINPUT_NOALPHA ( EVENT_APP_START + 5 )
#define     SW_EVENT_FRM_RESIZE           ( EVENT_APP_START + 6 )
#define     SW_EVENT_FRM_MOVE             ( EVENT_APP_START + 7 )
#define     SW_EVENT_PAGE_COUNT           ( EVENT_APP_START + 8 )
#define     SW_EVENT_MAIL_MERGE_END       ( EVENT_APP_START + 9 )
#define     SW_EVENT_FIELD_MERGE          ( EVENT_APP_START + 10 )
#define     SW_EVENT_FIELD_MERGE_FINISHED ( EVENT_APP_START + 11 )
#define     SW_EVENT_LAYOUT_FINISHED      ( EVENT_APP_START + 12 )

#define     STR_SW_EVENT_PAGE_COUNT           0
#define     STR_SW_EVENT_MAIL_MERGE           1
#define     STR_SW_EVENT_MAIL_MERGE_END       2
#define     STR_SW_EVENT_FIELD_MERGE          3
#define     STR_SW_EVENT_FIELD_MERGE_FINISHED 4
#define     STR_SW_EVENT_LAYOUT_FINISHED      5
#define     STR_SW_EVENT_OBJECT_SELECT        6
#define     STR_SW_EVENT_START_INS_GLOSSARY   7
#define     STR_SW_EVENT_END_INS_GLOSSARY     8
#define     STR_SW_EVENT_FRM_KEYINPUT_ALPHA   9
#define     STR_SW_EVENT_FRM_KEYINPUT_NOALPHA 10
#define     STR_SW_EVENT_FRM_RESIZE           11
#define     STR_SW_EVENT_FRM_MOVE             12

class SwFrameFormat;
class SwFormatINetFormat;
class IMapObject;

// Enum for objects that call events into Basic or JavaScript.
enum SwCallEventObjectType
{
    EVENT_OBJECT_NONE = 0,          // Null is nothing at all.
    EVENT_OBJECT_IMAGE,
    EVENT_OBJECT_INETATTR,
    EVENT_OBJECT_URLITEM,
    EVENT_OBJECT_IMAGEMAP
};

// Structure for the exchange between UI/CORE.

struct SwCallMouseEvent
{
    SwCallEventObjectType eType;
    union
    {
        // EVENT_OBJECT_IMAGE/EVENT_OBJECT_URLITEM
        const SwFrameFormat* pFormat;

        // EVENT_OBJECT_INETATTR
        const SwFormatINetFormat* pINetAttr;

        // EVENT_OBJECT_IMAGEMAP
        struct
        {
            const SwFrameFormat* pFormat;
            const IMapObject* pIMapObj;
        } IMAP;
    } PTR;

    SwCallMouseEvent()
        : eType( EVENT_OBJECT_NONE )
        { PTR.pFormat = nullptr; PTR.IMAP.pIMapObj = nullptr; }

    void Set( SwCallEventObjectType eTyp, const SwFrameFormat* pFormat )
        { eType = eTyp; PTR.pFormat = pFormat; PTR.IMAP.pIMapObj = nullptr; }

    void Set( const SwFrameFormat* pFormat, const IMapObject* pIMapObj )
        { eType = EVENT_OBJECT_IMAGEMAP; PTR.pFormat = pFormat; PTR.IMAP.pIMapObj = pIMapObj; }

    void Set( const SwFormatINetFormat* pINetAttr )
        { eType = EVENT_OBJECT_INETATTR; PTR.pINetAttr = pINetAttr; PTR.IMAP.pIMapObj = nullptr; }

    bool operator==( const SwCallMouseEvent& rEvent ) const
        {
            return eType == rEvent.eType &&
                    PTR.pFormat == rEvent.PTR.pFormat &&
                    PTR.IMAP.pIMapObj == rEvent.PTR.IMAP.pIMapObj;
        }
    bool operator!=( const SwCallMouseEvent& rEvent ) const
        {   return !( *this == rEvent );    }

    void Clear()
        { eType = EVENT_OBJECT_NONE; PTR.pFormat = nullptr; PTR.IMAP.pIMapObj = nullptr; }

    bool HasEvent() const { return EVENT_OBJECT_NONE != eType; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
