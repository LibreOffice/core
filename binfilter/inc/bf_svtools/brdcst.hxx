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
#ifndef _SFXBRDCST_HXX
#define _SFXBRDCST_HXX

#include <tools/rtti.hxx>

#include <bf_svtools/svarray.hxx>

namespace binfilter {

class SfxListener;
class SfxHint;

SV_DECL_PTRARR( SfxListenerArr_Impl, SfxListener*, 0, 2 )

#define SFX_FORWARD( rBC, rBCT, rHint, rHintT ) \
        Forward( rBC, rHint )

//-------------------------------------------------------------------------

class  SfxBroadcaster
{
friend class SfxListener;

    SfxListenerArr_Impl 	aListeners;

private:
    BOOL		 AddListener( SfxListener& rListener );
    void					RemoveListener( SfxListener& rListener );
    const SfxBroadcaster&	operator=(const SfxBroadcaster &); // verboten

protected:
    void					SFX_FORWARD(SfxBroadcaster& rBC, const TypeId& rBCType,
                                        const SfxHint& rHint, const TypeId& rHintType);
    virtual void			ListenersGone();

public:
                            TYPEINFO();

                            SfxBroadcaster();
                            SfxBroadcaster( const SfxBroadcaster &rBC );
    virtual 				~SfxBroadcaster();

    void					Broadcast( const SfxHint &rHint );

    BOOL					HasListeners() const;
    USHORT					GetListenerCount() const { return aListeners.Count(); }
    SfxListener*			GetListener( USHORT nNo ) const
                            { return (SfxListener*) aListeners[nNo]; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
