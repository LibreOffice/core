/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxcllitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:33:02 $
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
#ifndef _SXCLLITM_HXX
#define _SXCLLITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDYNITM_HXX
#include <svx/sdynitm.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

//------------------------------------
// class SdrCaptionLineLenItem
// Laenge der ersten Austrittslinie
// Nur bei Type3 und Type4
// Nur relevant, wenn SdrCaptionFitLineLenItem=FALSE
//------------------------------------
class SdrCaptionLineLenItem: public SdrMetricItem {
public:
    SdrCaptionLineLenItem(long nLineLen=0): SdrMetricItem(SDRATTR_CAPTIONLINELEN,nLineLen) {}
    SdrCaptionLineLenItem(SvStream& rIn)  : SdrMetricItem(SDRATTR_CAPTIONLINELEN,rIn)      {}
};

//------------------------------------
// class SdrCaptionFitLineLenItem
// Laenge der ersten Austrittslinie automatisch berechnen oder
// Vorgabe (SdrCaptionLineLenItem) verwenden.
// Nur bei Type3 und Type4
//------------------------------------
class SdrCaptionFitLineLenItem: public SdrYesNoItem {
public:
    SdrCaptionFitLineLenItem(BOOL bBestFit=TRUE): SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,bBestFit) {}
    SdrCaptionFitLineLenItem(SvStream& rIn)     : SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,rIn)      {}
};

#endif
