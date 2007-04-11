/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: genlink.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:21:25 $
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
#ifndef _SFXGENLINK_HXX
#define _SFXGENLINK_HXX


#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
typedef long (*CFuncPtr)(void*);

class GenLink
{
    Link        aLink;
    CFuncPtr    pFunc;

public:
    GenLink(): pFunc(0) {}
    GenLink( CFuncPtr pCFunc ): pFunc(pCFunc) {}
    GenLink( const Link& rLink ): aLink(rLink), pFunc(0) {}
    GenLink( const GenLink& rOrig ):
        aLink(rOrig.aLink), pFunc(rOrig.pFunc) {}

    GenLink& operator = ( const GenLink& rOrig )
        { pFunc = rOrig.pFunc; aLink = rOrig.aLink; return *this; }

    BOOL operator!() const { return !aLink && !pFunc; }
    BOOL IsSet() const { return aLink.IsSet() || pFunc; }

    long Call( void* pCaller )
         { return pFunc ? (*pFunc)(pCaller) : aLink.Call(pCaller); }
};

#endif
