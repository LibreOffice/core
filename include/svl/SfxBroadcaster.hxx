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
#ifndef INCLUDED_SVL_BRDCST_HXX
#define INCLUDED_SVL_BRDCST_HXX

#include <svl/svldllapi.h>
#include <memory>

class SfxListener;
class SfxHint;
class SfxBroadcasterTest;

class SVL_DLLPUBLIC SfxBroadcaster
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

private:
    void                    AddListener( SfxListener& rListener );
    void                    RemoveListener( SfxListener& rListener );
    const SfxBroadcaster&   operator=(const SfxBroadcaster &) = delete;

protected:
    void                    Forward(SfxBroadcaster& rBC, const SfxHint& rHint);

public:

                            SfxBroadcaster();
                            SfxBroadcaster( const SfxBroadcaster &rBC );
    virtual                 ~SfxBroadcaster();

    void                    Broadcast( const SfxHint &rHint );
    bool                    HasListeners() const;

    /** Get the number of listeners which are registered at this broadcaster */
    size_t                  GetListenerCount() const;

    /** Get the size of the internally stored vector.
     * Use it to iterate over all listeners.
     */
    size_t GetSizeOfVector() const;

    /** Get a listener by its position in the internally stored vector.
     * Note that this method may return NULL
     */
    SfxListener* GetListener( size_t nNo ) const;

    friend class SfxListener;
    friend class ::SfxBroadcasterTest;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
