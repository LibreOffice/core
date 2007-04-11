/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdtditm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:14:31 $
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
#ifndef SDTMITM_HXX
#define SDTMITM_HXX

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

//----------------------------------------------
// class SdrTextLeft/Right/Upper/LowerDistItem
//----------------------------------------------
class SdrTextLeftDistItem: public SdrMetricItem {
public:
    SdrTextLeftDistItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_LEFTDIST,nHgt)  {}
    SdrTextLeftDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_LEFTDIST,rIn) {}
};

class SdrTextRightDistItem: public SdrMetricItem {
public:
    SdrTextRightDistItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_RIGHTDIST,nHgt)  {}
    SdrTextRightDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_RIGHTDIST,rIn) {}
};

class SdrTextUpperDistItem: public SdrMetricItem {
public:
    SdrTextUpperDistItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_UPPERDIST,nHgt)  {}
    SdrTextUpperDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_UPPERDIST,rIn) {}
};

class SdrTextLowerDistItem: public SdrMetricItem {
public:
    SdrTextLowerDistItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_LOWERDIST,nHgt)  {}
    SdrTextLowerDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_LOWERDIST,rIn) {}
};

#endif
