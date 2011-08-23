/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SXCAITM_HXX
#define _SXCAITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDRANGITM_HXX
#include <bf_svx/sdangitm.hxx>
#endif

#ifndef _SDOOITM_HXX
#include <bf_svx/sdooitm.hxx>
#endif
namespace binfilter {

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

}//end of namespace binfilter
#endif
