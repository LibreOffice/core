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
#ifndef _SFXHINTPOST_HXX
#define _SFXHINTPOST_HXX

#include <sfx2/genlink.hxx>
#include <tools/ref.hxx>

//===================================================================

class SfxHint;

//-------------------------------------------------------------------

class SfxHintPoster: public SvRefBase

/*  [Description]

    With instances of this class unique events per PostUserEvent can be sent
    using the StarView-application. If the User-Event is triggered often,
    the handler <Event()> is called, the base implementation with the
    <SetEventHdl()> line is making these link calls.

    The instance are held via Ref-Count at least as long as a possible sent
    event has not arrived yet. Should be killed before the goal, before the
    connection is SetEventHdl (GenLink ()) .
*/

{
    GenLink         aLink;

private:
                    DECL_LINK( DoEvent_Impl, SfxHint * );

protected:
    virtual         ~SfxHintPoster();
    virtual void    Event( SfxHint* pPostedHint );

public:
                    SfxHintPoster( const GenLink& rLink );

    void            Post( SfxHint* pHint = 0 );
    void            SetEventHdl( const GenLink& rLink );
};

//-------------------------------------------------------------------

SV_DECL_IMPL_REF(SfxHintPoster);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
