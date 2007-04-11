/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxmovitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:36:18 $
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
#ifndef _SXMOVITM_HXX
#define _SXMOVITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

//------------------------------
// class SdrMoveXItem
//------------------------------
class SdrMoveXItem: public SdrMetricItem {
public:
    SdrMoveXItem(long n=0): SdrMetricItem(SDRATTR_MOVEX,n)        {}
    SdrMoveXItem(SvStream& rIn): SdrMetricItem(SDRATTR_MOVEX,rIn) {}
};

//------------------------------
// class SdrMoveYItem
//------------------------------
class SdrMoveYItem: public SdrMetricItem {
public:
    SdrMoveYItem(long n=0): SdrMetricItem(SDRATTR_MOVEY,n)        {}
    SdrMoveYItem(SvStream& rIn): SdrMetricItem(SDRATTR_MOVEY,rIn) {}
};

#endif
