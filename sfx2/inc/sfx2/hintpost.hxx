/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
    sal_uIntPtr         nId;
    GenLink         aLink;

private:
    void            RegisterEvent();
                    DECL_LINK( DoEvent_Impl, SfxHint * );

protected:
    virtual         ~SfxHintPoster();
    virtual void    Event( SfxHint* pPostedHint );

public:
                    SfxHintPoster();
                    SfxHintPoster( const GenLink& rLink );

    void            Post( SfxHint* pHint = 0 );
    void            SetEventHdl( const GenLink& rLink );
};

//-------------------------------------------------------------------

SV_DECL_IMPL_REF(SfxHintPoster);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
