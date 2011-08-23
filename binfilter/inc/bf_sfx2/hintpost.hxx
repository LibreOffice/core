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

#include <bf_sfx2/genlink.hxx>

#include <tools/ref.hxx>
namespace binfilter {

class SfxHint;

//===================================================================


//-------------------------------------------------------------------

class SfxHintPoster: public SvRefBase

/*	[Beschreibung]

    Mit Instanzen dieser Klasse k"onnen eindeutige Events per PostUserEvent
    "uber die StarView-Application verschickt werden. Wenn das User-Event
    ausgel"ost wird, wird der Handler <Event()> gerufen, dessen
    Basisimplementierung den mit <SetEventHdl()> angegbenen Link ruft.

    Die Instanz wird via Ref-Count mindestens solange gehalten, wie
    ein ggf. abgeschicktes Event noch nicht angekommen ist. Sollte das
    Ziel vorher sterben, ist die Verbindung zuvor mit 'SetEventHdl(GenLink())'
    zu kappen.
*/

{
    ULONG			nId;
    GenLink 		aLink;

private:
#if _SOLAR__PRIVATE
                    DECL_LINK( DoEvent_Impl, SfxHint * );
#endif

protected:
    virtual         ~SfxHintPoster();
    virtual void	Event( SfxHint* pPostedHint );
};

//-------------------------------------------------------------------

SV_DECL_IMPL_REF(SfxHintPoster);

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
