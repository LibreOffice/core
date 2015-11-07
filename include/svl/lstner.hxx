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
#ifndef INCLUDED_SVL_LSTNER_HXX
#define INCLUDED_SVL_LSTNER_HXX

#include <svl/svldllapi.h>

class SfxBroadcaster;
class SfxHint;

class SVL_DLLPUBLIC SfxListener
{
    struct Impl;
    Impl* mpImpl;

private:
    const SfxListener&  operator=(const SfxListener &) = delete;

public:

                        SfxListener();
                        SfxListener( const SfxListener &rCopy );
    virtual             ~SfxListener();

    void                StartListening( SfxBroadcaster& rBroadcaster, bool bPreventDups = false );
    void                EndListening( SfxBroadcaster& rBroadcaster, bool bAllDups = false );
    void                EndListeningAll();
    bool                IsListening( SfxBroadcaster& rBroadcaster ) const;

    sal_uInt16          GetBroadcasterCount() const;
    SfxBroadcaster*     GetBroadcasterJOE( sal_uInt16 nNo ) const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void                RemoveBroadcaster_Impl( SfxBroadcaster& rBC );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
