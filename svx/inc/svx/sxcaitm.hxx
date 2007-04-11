/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxcaitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:31:43 $
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
#ifndef _SXCAITM_HXX
#define _SXCAITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDRANGITM_HXX
#include <svx/sdangitm.hxx>
#endif

#ifndef _SDOOITM_HXX
#include <svx/sdooitm.hxx>
#endif

//------------------------------------
// class SdrCaptionFixedAngleItem
// TRUE=Winkel durch SdrCaptionAngleItem vorgegeben
// FALSE=Freier Winkel
//------------------------------------
class SdrCaptionFixedAngleItem: public SdrOnOffItem {
public:
    SdrCaptionFixedAngleItem(BOOL bFix=TRUE): SdrOnOffItem(SDRATTR_CAPTIONFIXEDANGLE,bFix) {}
    SdrCaptionFixedAngleItem(SvStream& rIn) : SdrOnOffItem(SDRATTR_CAPTIONFIXEDANGLE,rIn)  {}
};

//------------------------------
// class SdrCaptionAngleItem
// Winkelangabe in 1/100 Degree
// Nur bei Type2, Type3 und Type4
// und nur wenn SdrCaptionIsFixedAngleItem=TRUE
//------------------------------
class SdrCaptionAngleItem: public SdrAngleItem {
public:
    SdrCaptionAngleItem(long nAngle=0): SdrAngleItem(SDRATTR_CAPTIONANGLE,nAngle) {}
    SdrCaptionAngleItem(SvStream& rIn): SdrAngleItem(SDRATTR_CAPTIONANGLE,rIn)    {}
};

#endif
