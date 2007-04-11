/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxreoitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:39:24 $
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
#ifndef _SXREOITM_HXX
#define _SXREOITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SXFIITM_HXX
#include <svx/sxfiitm.hxx>
#endif

//------------------------------
// class SdrResizeXOneItem
//------------------------------
class SdrResizeXOneItem: public SdrFractionItem {
public:
    SdrResizeXOneItem(): SdrFractionItem(SDRATTR_RESIZEXONE,Fraction(1,1)) {}
    SdrResizeXOneItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEXONE,rFact) {}
    SdrResizeXOneItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEXONE,rIn)    {}
};

//------------------------------
// class SdrResizeYOneItem
//------------------------------
class SdrResizeYOneItem: public SdrFractionItem {
public:
    SdrResizeYOneItem(): SdrFractionItem(SDRATTR_RESIZEYONE,Fraction(1,1)) {}
    SdrResizeYOneItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEYONE,rFact) {}
    SdrResizeYOneItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEYONE,rIn)    {}
};

#endif
