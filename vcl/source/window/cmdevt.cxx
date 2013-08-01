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

#include <vcl/cmdevt.hxx>

CommandExtTextInputData::CommandExtTextInputData( const OUString& rText,
                                                  const sal_uInt16* pTextAttr,
                                                  sal_Int32 nCursorPos,
                                                  sal_uInt16 nCursorFlags,
                                                  sal_Int32 nDeltaStart,
                                                  sal_Int32 nOldTextLen,
                                                  sal_Bool bOnlyCursor ) :
    maText( rText )
{
    if ( pTextAttr && !maText.isEmpty() )
    {
        mpTextAttr = new sal_uInt16[maText.getLength()];
        memcpy( mpTextAttr, pTextAttr, maText.getLength()*sizeof(sal_uInt16) );
    }
    else
        mpTextAttr = NULL;
    mnCursorPos     = nCursorPos;
    mnDeltaStart    = nDeltaStart;
    mnOldTextLen    = nOldTextLen;
    mnCursorFlags   = nCursorFlags;
    mbOnlyCursor    = bOnlyCursor;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::CommandExtTextInputData( const CommandExtTextInputData& rData ) :
    maText( rData.maText )
{
    if ( rData.mpTextAttr && !maText.isEmpty() )
    {
        mpTextAttr = new sal_uInt16[maText.getLength()];
        memcpy( mpTextAttr, rData.mpTextAttr, maText.getLength()*sizeof(sal_uInt16) );
    }
    else
        mpTextAttr = NULL;
    mnCursorPos     = rData.mnCursorPos;
    mnDeltaStart    = rData.mnDeltaStart;
    mnOldTextLen    = rData.mnOldTextLen;
    mnCursorFlags   = rData.mnCursorFlags;
    mbOnlyCursor    = rData.mbOnlyCursor;
}

// -----------------------------------------------------------------------

CommandExtTextInputData::~CommandExtTextInputData()
{
    if ( mpTextAttr )
        delete [] mpTextAttr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
