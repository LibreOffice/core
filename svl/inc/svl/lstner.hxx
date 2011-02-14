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

#include "svl/svldllapi.h"
#include <tools/rtti.hxx>
#include <svl/svarray.hxx>

class SfxBroadcaster;
class SfxHint;

#ifndef _SFX_LSTNER_CXX
typedef SvPtrarr SfxBroadcasterArr_Impl;
#endif

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
    void                EndListening( sal_uInt16 nNo );
    void                EndListeningAll();
    sal_Bool                IsListening( SfxBroadcaster& rBroadcaster ) const;

    sal_uInt16              GetBroadcasterCount() const
                        { return aBCs.Count(); }
    SfxBroadcaster*     GetBroadcasterJOE( sal_uInt16 nNo ) const
                        { return (SfxBroadcaster*) aBCs.GetObject(nNo); }

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

#ifndef _NOTIFY_HXX
    void RemoveBroadcaster_Impl( SfxBroadcaster& rBC );
#endif
};

#endif
