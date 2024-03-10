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

#include "calbck.hxx"
#include "frmfmt.hxx"
#include "hints.hxx"

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

struct SwCallMouseEvent final
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

    void Clear()
        {
            if (EVENT_OBJECT_IMAGE == eType || EVENT_OBJECT_URLITEM == eType || EVENT_OBJECT_IMAGEMAP == eType)
            {
                // note: pFormat is not necessarily the same as
                // GetRegisteredIn() here; see ~SwFormat()
                assert(PTR.pFormat);
                EndListeningAll();
            }
            eType = EVENT_OBJECT_NONE; PTR.pFormat = nullptr; PTR.IMAP.pIMapObj = nullptr;
        }

    bool HasEvent() const { return EVENT_OBJECT_NONE != eType; }

    virtual void SwClientNotify(const SwModify& rMod, const SfxHint& rHint) override
    {
        if (rHint.GetId() != SfxHintId::SwLegacyModify)
            return;
        auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
        assert(EVENT_OBJECT_IMAGE == eType || EVENT_OBJECT_URLITEM == eType || EVENT_OBJECT_IMAGEMAP == eType);
        SwClient::SwClientNotify(rMod, rHint);
        bool bClear = !GetRegisteredIn();
        switch(pLegacy->GetWhich())
        {
            case RES_FMT_CHG:
                bClear |= pLegacy->m_pOld->StaticWhichCast(RES_FMT_CHG).pChangedFormat == PTR.pFormat;
                break;
            case RES_REMOVE_UNO_OBJECT:
                bClear |= pLegacy->m_pOld->StaticWhichCast(RES_REMOVE_UNO_OBJECT).pObject == PTR.pFormat;
        }
        if(bClear)
            Clear();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
