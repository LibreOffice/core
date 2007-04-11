/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hintpost.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:21:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXHINTPOST_HXX
#define _SFXHINTPOST_HXX

#ifndef _SFXGENLINK_HXX
#include <sfx2/genlink.hxx>
#endif

#ifndef _REF_HXX //autogen
#include <tools/ref.hxx>
#endif

//===================================================================

class SfxHint;

//-------------------------------------------------------------------

class SfxHintPoster: public SvRefBase

/*  [Beschreibung]

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
    ULONG           nId;
    GenLink         aLink;

private:
    void            RegisterEvent();
//#if 0 // _SOLAR__PRIVATE
                    DECL_LINK( DoEvent_Impl, SfxHint * );
//#endif

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
