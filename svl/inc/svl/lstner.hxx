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
#ifndef _SFXLSTNER_HXX
#define _SFXLSTNER_HXX

#include "svl/svldllapi.h"
#include <tools/rtti.hxx>
#include <deque>

class SfxBroadcaster;
class SfxHint;

typedef std::deque<SfxBroadcaster*> SfxBroadcasterArr_Impl;

#define SFX_NOTIFY( rBC, rBCT, rHint, rHintT ) \
        Notify( rBC, rHint )

//-------------------------------------------------------------------------

class SVL_DLLPUBLIC SfxListener
{
    SfxBroadcasterArr_Impl aBCs;

private:
    const SfxListener&  operator=(const SfxListener &); // n.i., ist verboten

public:
    TYPEINFO();

                        SfxListener();
                        SfxListener( const SfxListener &rCopy );
    virtual             ~SfxListener();

    sal_Bool                StartListening( SfxBroadcaster& rBroadcaster, sal_Bool bPreventDups = sal_False );
    sal_Bool                EndListening( SfxBroadcaster& rBroadcaster, sal_Bool bAllDups = sal_False );
    void                EndListeningAll();
    sal_Bool                IsListening( SfxBroadcaster& rBroadcaster ) const;

    sal_uInt16              GetBroadcasterCount() const
                        { return aBCs.size(); }
    SfxBroadcaster*     GetBroadcasterJOE( sal_uInt16 nNo ) const
                        { return aBCs[nNo]; }

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

#ifndef _NOTIFY_HXX
    void RemoveBroadcaster_Impl( SfxBroadcaster& rBC );
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
