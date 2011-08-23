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
#ifndef _SFXLSTNER_HXX
#define _SFXLSTNER_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif

namespace binfilter
{

class SfxBroadcaster;
class SfxHint;

SV_DECL_PTRARR( SfxBroadcasterArr_Impl, SfxBroadcaster*, 0, 2 )

#define SFX_NOTIFY( rBC, rBCT, rHint, rHintT ) \
        Notify( rBC, rHint )

//-------------------------------------------------------------------------

class  SfxListener
{
    ::binfilter::SfxBroadcasterArr_Impl aBCs;

private:
    const SfxListener&	operator=(const SfxListener &); // n.i., ist verboten

public:
    TYPEINFO();

                        SfxListener();
                        SfxListener( const SfxListener &rCopy );
    virtual 			~SfxListener();

    BOOL				StartListening( SfxBroadcaster& rBroadcaster, BOOL bPreventDups = FALSE );
    BOOL				EndListening( SfxBroadcaster& rBroadcaster, BOOL bAllDups = FALSE );
    void				EndListeningAll();
    BOOL				IsListening( SfxBroadcaster& rBroadcaster ) const;

    USHORT				GetBroadcasterCount() const
                        { return aBCs.Count(); }
    SfxBroadcaster* 	GetBroadcasterJOE( USHORT nNo ) const
                        { return (SfxBroadcaster*) aBCs.GetObject(nNo); }

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

#ifndef _NOTIFY_HXX
    void RemoveBroadcaster_Impl( SfxBroadcaster& rBC );
#endif
};

}

#endif
