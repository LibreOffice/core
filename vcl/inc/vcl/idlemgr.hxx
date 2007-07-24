/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idlemgr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 09:59:12 $
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

#ifndef _SV_IDLEMGR_HXX
#define _SV_IDLEMGR_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

class ImplIdleList;

// ---------------
// - ImplIdleMgr -
// ---------------

class ImplIdleMgr
{
private:
    ImplIdleList*   mpIdleList;
    AutoTimer       maTimer;

public:
                    ImplIdleMgr();
                    ~ImplIdleMgr();

    BOOL            InsertIdleHdl( const Link& rLink, USHORT nPriority );
    void            RemoveIdleHdl( const Link& rLink );

    void            RestartIdler()
                        { if ( maTimer.IsActive() ) maTimer.Start(); }

                    // Timer* kann auch NULL sein
                    DECL_LINK( TimeoutHdl, Timer* );
};

#endif  // _SV_IDLEMGR_HXX
