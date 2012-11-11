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

#ifndef _SV_ACCMGR_HXX
#define _SV_ACCMGR_HXX

#include <tools/solar.h>
#include <vector>

class Accelerator;
class KeyCode;

typedef ::std::vector< Accelerator* > ImplAccelList;

// --------------------
// - ImplAccelManager -
// --------------------

class ImplAccelManager
{
private:
    ImplAccelList*      mpAccelList;
    ImplAccelList*      mpSequenceList;

public:
                        ImplAccelManager()
                        {
                            mpAccelList    = NULL;
                            mpSequenceList = NULL;
                        }
                        ~ImplAccelManager();

    sal_Bool                InsertAccel( Accelerator* pAccel );
    void                RemoveAccel( Accelerator* pAccel );

    void                EndSequence( sal_Bool bCancel = sal_False );
    void                FlushAccel() { EndSequence( sal_True ); }

    sal_Bool                IsAccelKey( const KeyCode& rKeyCode, sal_uInt16 nRepeat );
};

#endif  // _SV_ACCMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
