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
#ifndef _SVT_LISTENER_HXX
#define _SVT_LISTENER_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

namespace binfilter
{

class SvtBroadcaster;
class SfxHint;
class SvtListenerBase;

//-------------------------------------------------------------------------

class  SvtListener
{
    friend class SvtListenerBase;
    SvtListenerBase *pBrdCastLst;

    const SvtListener&	operator=(const SvtListener &); // n.i., ist verboten

public:
    TYPEINFO();

                        SvtListener();
                        SvtListener( const SvtListener &rCopy );
    virtual 			~SvtListener();

    BOOL				StartListening( SvtBroadcaster& rBroadcaster );
    BOOL				EndListening( SvtBroadcaster& rBroadcaster );
    void				EndListeningAll();
#ifdef DBG_UTIL
    BOOL				IsListening( SvtBroadcaster& rBroadcaster ) const;
#endif
    BOOL 				HasBroadcaster() const { return 0 != pBrdCastLst; }

    virtual void		Notify( SvtBroadcaster& rBC, const SfxHint& rHint );
};

}

#endif
