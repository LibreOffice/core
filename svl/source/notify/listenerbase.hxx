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
#ifndef _SVT_LISTENERBASE_HXX
#define _SVT_LISTENERBASE_HXX

class SvtBroadcaster;
class SvtListener;

class SvtListenerBase
{
    SvtListenerBase *pNext;
    SvtListenerBase *pLeft, *pRight;
    SvtBroadcaster *pBroadcaster;
    SvtListener *pListener;

public:

    SvtListenerBase( SvtListener& rLst, SvtBroadcaster& rBroadcaster );
    ~SvtListenerBase();

    SvtListenerBase* GetNext() const        { return pNext; }
    void SetNext( SvtListenerBase* p )      { pNext = p; }

    SvtBroadcaster* GetBroadcaster() const  { return pBroadcaster; }
    SvtListener* GetListener() const        { return pListener; }

    SvtListenerBase* GetLeft() const        { return pLeft; }
    SvtListenerBase* GetRight() const       { return pRight; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
