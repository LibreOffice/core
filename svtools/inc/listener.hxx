/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listener.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:53:22 $
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
#ifndef _SVT_LISTENER_HXX
#define _SVT_LISTENER_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

class SvtBroadcaster;
class SfxHint;
class SvtListenerBase;

//-------------------------------------------------------------------------

class SVL_DLLPUBLIC SvtListener
{
    friend class SvtListenerBase;
    SvtListenerBase *pBrdCastLst;

    const SvtListener&  operator=(const SvtListener &); // n.i., ist verboten

public:
    TYPEINFO();

                        SvtListener();
                        SvtListener( const SvtListener &rCopy );
    virtual             ~SvtListener();

    BOOL                StartListening( SvtBroadcaster& rBroadcaster );
    BOOL                EndListening( SvtBroadcaster& rBroadcaster );
    void                EndListeningAll();
    BOOL                IsListening( SvtBroadcaster& rBroadcaster ) const;

    BOOL                HasBroadcaster() const { return 0 != pBrdCastLst; }

    virtual void        Notify( SvtBroadcaster& rBC, const SfxHint& rHint );
};


#endif

