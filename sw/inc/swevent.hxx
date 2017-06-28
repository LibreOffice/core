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
#include <calbck.hxx>
#include <frmfmt.hxx>

#define     SW_EVENT_OBJECT_SELECT        ( EVENT_APP_START + 0 )
#define     SW_EVENT_START_INS_GLOSSARY   ( EVENT_APP_START + 1 )
#define     SW_EVENT_END_INS_GLOSSARY     ( EVENT_APP_START + 2 )
#define     SW_EVENT_FRM_KEYINPUT_ALPHA   ( EVENT_APP_START + 4 )
#define     SW_EVENT_FRM_KEYINPUT_NOALPHA ( EVENT_APP_START + 5 )
#define     SW_EVENT_FRM_RESIZE           ( EVENT_APP_START + 6 )
#define     SW_EVENT_FRM_MOVE             ( EVENT_APP_START + 7 )

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
    : public SwClient
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

    SwCallMouseEvent(SwCallMouseEvent const& rOther)
        : SwClient(rOther.GetRegisteredInNonConst())
        , eType(rOther.eType)
    {
        memcpy(&PTR, &rOther.PTR, sizeof(PTR));
    }

    void Set( SwCallEventObjectType eTyp, const SwFrameFormat* pFormat )
        { Clear(); eType = eTyp; PTR.pFormat = pFormat; PTR.IMAP.pIMapObj = nullptr; assert(pFormat); const_cast<SwFrameFormat*>(pFormat)->Add(this); }

    void Set( const SwFrameFormat* pFormat, const IMapObject* pIMapObj )
        { Clear(); eType = EVENT_OBJECT_IMAGEMAP; PTR.pFormat = pFormat; PTR.IMAP.pIMapObj = pIMapObj; assert(pFormat); const_cast<SwFrameFormat*>(pFormat)->Add(this); }

    void Set( const SwFormatINetFormat* pINetAttr )
        { Clear(); eType = EVENT_OBJECT_INETATTR; PTR.pINetAttr = pINetAttr; PTR.IMAP.pIMapObj = nullptr; }

    bool operator==( const SwCallMouseEvent& rEvent ) const
        {
            return eType == rEvent.eType &&
                    PTR.pFormat == rEvent.PTR.pFormat &&
                    PTR.IMAP.pIMapObj == rEvent.PTR.IMAP.pIMapObj;
        }
    bool operator!=( const SwCallMouseEvent& rEvent ) const
        {   return !( *this == rEvent );    }

    void Clear()
        {
            if (EVENT_OBJECT_IMAGE == eType || EVENT_OBJECT_URLITEM == eType || EVENT_OBJECT_IMAGEMAP == eType)
            {
                // note: pFormat is not necessarily the same as
                // GetRegisteredIn() here; see ~SwFormat()
                assert(PTR.pFormat);
                GetRegisteredInNonConst()->Remove(this);
            }
            eType = EVENT_OBJECT_NONE; PTR.pFormat = nullptr; PTR.IMAP.pIMapObj = nullptr;
        }

    bool HasEvent() const { return EVENT_OBJECT_NONE != eType; }

    virtual void Modify(SfxPoolItem const*const pOldValue, SfxPoolItem const*const pNewValue) override
    {
        assert(EVENT_OBJECT_IMAGE == eType || EVENT_OBJECT_URLITEM == eType || EVENT_OBJECT_IMAGEMAP == eType);
        SwClient::Modify(pOldValue, pNewValue);
        if (!GetRegisteredIn()
            || (RES_FMT_CHG == pOldValue->Which()
                && static_cast<SwFormatChg const*>(pOldValue)->pChangedFormat == PTR.pFormat)
            || (RES_REMOVE_UNO_OBJECT == pOldValue->Which()
                && static_cast<SwPtrMsgPoolItem const*>(pOldValue)->pObject == PTR.pFormat))
        {
            Clear();
        }
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
