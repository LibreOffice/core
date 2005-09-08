/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: broadcast.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:10:34 $
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
#ifndef _SVT_BROADCAST_HXX
#define _SVT_BROADCAST_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif

class SvtListener;
class SfxHint;
class SvtListenerBase;

//-------------------------------------------------------------------------

class SVL_DLLPUBLIC SvtBroadcaster
{
friend class SvtListener;
friend class SvtListenerBase;
friend class SvtListenerIter;
    SvtListenerBase* pRoot;

    const SvtBroadcaster&   operator=(const SvtBroadcaster &); // verboten

protected:
    void                    Forward( SvtBroadcaster& rBC,
                                     const SfxHint& rHint );
    virtual void            ListenersGone();

public:
                            TYPEINFO();

                            SvtBroadcaster();
                            SvtBroadcaster( const SvtBroadcaster &rBC );
    virtual                 ~SvtBroadcaster();

    void                    Broadcast( const SfxHint &rHint );

    BOOL                    HasListeners() const { return 0 != pRoot; }
};


#endif

