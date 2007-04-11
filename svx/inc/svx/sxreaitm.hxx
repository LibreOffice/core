/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxreaitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:39:11 $
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
#ifndef _SXREAITM_HXX
#define _SXREAITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SXFIITM_HXX
#include <svx/sxfiitm.hxx>
#endif

//------------------------------
// class SdrResizeXAllItem
//------------------------------
class SdrResizeXAllItem: public SdrFractionItem {
public:
    SdrResizeXAllItem(): SdrFractionItem(SDRATTR_RESIZEXALL,Fraction(1,1)) {}
    SdrResizeXAllItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEXALL,rFact) {}
    SdrResizeXAllItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEXALL,rIn)    {}
};

//------------------------------
// class SdrResizeYAllItem
//------------------------------
class SdrResizeYAllItem: public SdrFractionItem {
public:
    SdrResizeYAllItem(): SdrFractionItem(SDRATTR_RESIZEYALL,Fraction(1,1)) {}
    SdrResizeYAllItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEYALL,rFact) {}
    SdrResizeYAllItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEYALL,rIn)    {}
};

#endif
