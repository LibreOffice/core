/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itabenum.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:05:41 $
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
#ifndef _ITABENUM_HXX
#define _ITABENUM_HXX

#include <tools/solar.h>

namespace tabopts
{
    const USHORT DEFAULT_BORDER     = 0x01;
    const USHORT HEADLINE           = 0x02;
//    const USHORT REPEAT             = 0x04;
//    const USHORT HEADLINE_REPEAT    = 0x06;  // Headline + Repeat
    const USHORT SPLIT_LAYOUT       = 0x08;
    const USHORT HEADLINE_NO_BORDER = HEADLINE | SPLIT_LAYOUT;
    const USHORT ALL_TBL_INS_ATTR   = DEFAULT_BORDER | HEADLINE | SPLIT_LAYOUT;
}

struct SwInsertTableOptions
{
    USHORT mnInsMode;
    USHORT mnRowsToRepeat;

    SwInsertTableOptions( USHORT nInsMode, USHORT nRowsToRepeat ) :
        mnInsMode( nInsMode ), mnRowsToRepeat( nRowsToRepeat ) {};
};


#endif

