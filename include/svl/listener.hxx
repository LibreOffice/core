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
#ifndef INCLUDED_SVL_LISTENER_HXX
#define INCLUDED_SVL_LISTENER_HXX

#include <svl/svldllapi.h>

#include <o3tl/sorted_vector.hxx>

class SvtBroadcaster;
class SfxHint;

class SVL_DLLPUBLIC SvtListener
{
    friend class SvtBroadcaster;
    typedef o3tl::sorted_vector<SvtBroadcaster*> BroadcastersType;
    BroadcastersType maBroadcasters;

    const SvtListener&  operator=(const SvtListener &) = delete;
    // called from the SvtBroadcaster destructor
    void BroadcasterDying( SvtBroadcaster& rBroadcaster );

public:
    class SVL_DLLPUBLIC QueryBase
    {
        sal_uInt16 mnId;
    public:
        QueryBase( sal_uInt16 nId );
        virtual ~QueryBase();

        sal_uInt16 getId() const;
    };

    SvtListener();
    SvtListener( const SvtListener &r );
    virtual ~SvtListener() COVERITY_NOEXCEPT_FALSE;

    bool StartListening( SvtBroadcaster& rBroadcaster );
    bool EndListening( SvtBroadcaster& rBroadcaster );
    void EndListeningAll();

    /// Overwrites existing broadcasters with the ones from the specified listener
    void CopyAllBroadcasters( const SvtListener& r );
    bool HasBroadcaster() const;

    virtual void Notify( const SfxHint& rHint );
    virtual void Query( QueryBase& rQuery ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
