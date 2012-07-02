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
#ifndef _SVT_LISTENERITER_HXX
#define _SVT_LISTENERITER_HXX

#include "svl/svldllapi.h"
#include <tools/rtti.hxx>

class SvtListener;
class SvtListenerBase;
class SvtBroadcaster;

//-------------------------------------------------------------------------

class SVL_DLLPUBLIC SvtListenerIter
{
    friend class SvtListenerBase;

    SvtBroadcaster& rRoot;
    SvtListenerBase *pAkt, *pDelNext;

    // for the update of all iterator's, if a listener is added or removed
    // at the same time.
    static SvtListenerIter *pListenerIters;
    SvtListenerIter *pNxtIter;

    SVL_DLLPRIVATE static void RemoveListener( SvtListenerBase& rDel,
                                               SvtListenerBase* pNext );

public:
    SvtListenerIter( SvtBroadcaster& );
    ~SvtListenerIter();

    const SvtBroadcaster& GetBroadcaster() const    { return rRoot; }
          SvtBroadcaster& GetBroadcaster()          { return rRoot; }

    SvtListener* GoNext();          // to the next

    SvtListener* GoStart();         // to the start of the list

    SvtListener* GetCurr() const;   // returns the current

    int IsChanged() const       { return pDelNext != pAkt; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
