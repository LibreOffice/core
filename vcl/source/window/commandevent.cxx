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

#include <string.h>

#include <vcl/commandevent.hxx>

CommandExtTextInputData::CommandExtTextInputData( const OUString& rText,
    const ExtTextInputAttr* pTextAttr, sal_Int32 nCursorPos, sal_uInt16 nCursorFlags,
    bool bOnlyCursor)
    : maText(rText)
{
    if ( pTextAttr && !maText.isEmpty() )
    {
        mpTextAttr.reset( new ExtTextInputAttr[maText.getLength()] );
        memcpy( mpTextAttr.get(), pTextAttr, maText.getLength()*sizeof(ExtTextInputAttr) );
    }

    mnCursorPos     = nCursorPos;
    mnCursorFlags   = nCursorFlags;
    mbOnlyCursor    = bOnlyCursor;
}

CommandExtTextInputData::CommandExtTextInputData( const CommandExtTextInputData& rData ) :
    maText( rData.maText )
{
    if ( rData.mpTextAttr && !maText.isEmpty() )
    {
        mpTextAttr.reset( new ExtTextInputAttr[maText.getLength()] );
        memcpy( mpTextAttr.get(), rData.mpTextAttr.get(), maText.getLength()*sizeof(ExtTextInputAttr) );
    }

    mnCursorPos     = rData.mnCursorPos;
    mnCursorFlags   = rData.mnCursorFlags;
    mbOnlyCursor    = rData.mbOnlyCursor;
}

CommandExtTextInputData::~CommandExtTextInputData()
{
}

CommandWheelData::CommandWheelData()
{
    mnDelta         = 0;
    mnNotchDelta    = 0;
    mnLines         = 0.0;
    mnWheelMode     = CommandWheelMode::NONE;
    mnCode          = 0;
    mbHorz          = false;
    mbDeltaIsPixel  = false;
}

CommandWheelData::CommandWheelData( tools::Long nWheelDelta, tools::Long nWheelNotchDelta,
                                    double nScrollLines,
                                    CommandWheelMode nWheelMode, sal_uInt16 nKeyModifier,
                                    bool bHorz, bool bDeltaIsPixel )
{
    mnDelta         = nWheelDelta;
    mnNotchDelta    = nWheelNotchDelta;
    mnLines         = nScrollLines;
    mnWheelMode     = nWheelMode;
    mnCode          = nKeyModifier;
    mbHorz          = bHorz;
    mbDeltaIsPixel  = bDeltaIsPixel;
}

CommandScrollData::CommandScrollData( tools::Long nDeltaX, tools::Long nDeltaY )
{
    mnDeltaX    = nDeltaX;
    mnDeltaY    = nDeltaY;
}

CommandModKeyData::CommandModKeyData( ModKeyFlags nCode, bool bDown )
{
    mbDown = bDown;
    mnCode = nCode;
}

CommandSelectionChangeData::CommandSelectionChangeData( sal_uLong nStart, sal_uLong nEnd )
{
    mnStart = nStart;
    mnEnd = nEnd;
}

CommandEvent::CommandEvent()
{
    mpData          = nullptr;
    mnCommand       = CommandEventId::NONE;
    mbMouseEvent    = false;
}

CommandEvent::CommandEvent( const Point& rMousePos,
                                   CommandEventId nCmd, bool bMEvt, const void* pCmdData ) :
            maPos( rMousePos )
{
    mpData          = const_cast<void*>(pCmdData);
    mnCommand       = nCmd;
    mbMouseEvent    = bMEvt;
}

const CommandExtTextInputData* CommandEvent::GetExtTextInputData() const
{
    if ( mnCommand == CommandEventId::ExtTextInput )
        return static_cast<const CommandExtTextInputData*>(mpData);
    else
        return nullptr;
}

const CommandWheelData* CommandEvent::GetWheelData() const
{
    if ( mnCommand == CommandEventId::Wheel )
        return static_cast<const CommandWheelData*>(mpData);
    else
        return nullptr;
}

const CommandScrollData* CommandEvent::GetAutoScrollData() const
{
    if ( mnCommand == CommandEventId::AutoScroll )
        return static_cast<const CommandScrollData*>(mpData);
    else
        return nullptr;
}

const CommandModKeyData* CommandEvent::GetModKeyData() const
{
    if( mnCommand == CommandEventId::ModKeyChange )
        return static_cast<const CommandModKeyData*>(mpData);
    else
        return nullptr;
}

const CommandDialogData* CommandEvent::GetDialogData() const
{
    if( mnCommand == CommandEventId::ShowDialog )
        return static_cast<const CommandDialogData*>(mpData);
    else
        return nullptr;
}

CommandMediaData* CommandEvent::GetMediaData() const
{
    if( mnCommand == CommandEventId::Media )
        return static_cast<CommandMediaData*>(mpData);
    else
        return nullptr;
}

const CommandSelectionChangeData* CommandEvent::GetSelectionChangeData() const
{
    if( mnCommand == CommandEventId::SelectionChange )
        return static_cast<const CommandSelectionChangeData*>(mpData);
    else
        return nullptr;
}

const CommandSwipeData* CommandEvent::GetSwipeData() const
{
    if( mnCommand == CommandEventId::Swipe )
        return static_cast<const CommandSwipeData*>(mpData);
    else
        return nullptr;
}

const CommandLongPressData* CommandEvent::GetLongPressData() const
{
    if( mnCommand == CommandEventId::LongPress )
        return static_cast<const CommandLongPressData*>(mpData);
    else
        return nullptr;
}

const CommandGestureData* CommandEvent::GetGestureData() const
{
    if (mnCommand == CommandEventId::Gesture)
        return static_cast<const CommandGestureData*>(mpData);
    else
        return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
