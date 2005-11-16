/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:03 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef WW_TYPES
#define WW_TYPES

#include <vector>

//if part of OOo
#ifndef WW_TYPESSW
#include "typessw.hxx"
#endif
//else
//include standalonetypes.hxx
//endif

namespace ww
{
    typedef std::vector<sal_uInt8> bytes;

    enum WordVersion {eWW6 = 6, eWW7 = 7, eWW8 = 8};
    inline bool IsSevenMinus(WordVersion eVer) { return eVer <= eWW7; }
    inline bool IsEightPlus(WordVersion eVer) { return eVer >= eWW8; }

    /** For custom wrapping

        When you edit the wrap points of a contour in word, word uses a relative
        scale of 0 to 21600 where 21600 is apparently 100% of the graphic width
    */
    const int nWrap100Percent = 21600;
}

#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
