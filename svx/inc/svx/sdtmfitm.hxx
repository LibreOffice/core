/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdtmfitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:15:16 $
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
#ifndef _SDTMFITM_HXX
#define _SDTMFITM_HXX

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

//------------------------------
// class SdrTextMinFrameHeighItem
//------------------------------
class SdrTextMinFrameHeightItem: public SdrMetricItem {
public:
    SdrTextMinFrameHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT,nHgt)  {}
    SdrTextMinFrameHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT,rIn) {}
};

//------------------------------
// class SdrTextMaxFrameHeightItem
//------------------------------
class SdrTextMaxFrameHeightItem: public SdrMetricItem {
public:
    SdrTextMaxFrameHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT,nHgt)  {}
    SdrTextMaxFrameHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT,rIn) {}
};

//------------------------------
// class SdrTextMinFrameWidthItem
//------------------------------
class SdrTextMinFrameWidthItem: public SdrMetricItem {
public:
    SdrTextMinFrameWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH,nWdt)  {}
    SdrTextMinFrameWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH,rIn) {}
};

//------------------------------
// class SdrTextMaxFrameWidthItem
//------------------------------
class SdrTextMaxFrameWidthItem: public SdrMetricItem {
public:
    SdrTextMaxFrameWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH,nWdt)  {}
    SdrTextMaxFrameWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH,rIn) {}
};

#endif
