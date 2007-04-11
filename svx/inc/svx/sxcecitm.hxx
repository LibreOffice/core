/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxcecitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:31:56 $
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
#ifndef _SXCECITM_HXX
#define _SXCECITM_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDYNITM_HXX
#include <svx/sdynitm.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

enum SdrCaptionEscDir {SDRCAPT_ESCHORIZONTAL,SDRCAPT_ESCVERTICAL,SDRCAPT_ESCBESTFIT};

//------------------------------
// class SdrCaptionEscDirItem
//------------------------------
class SVX_DLLPUBLIC SdrCaptionEscDirItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrCaptionEscDirItem(SdrCaptionEscDir eDir=SDRCAPT_ESCHORIZONTAL): SfxEnumItem(SDRATTR_CAPTIONESCDIR,sal::static_int_cast< USHORT >(eDir)) {}
    SdrCaptionEscDirItem(SvStream& rIn)                              : SfxEnumItem(SDRATTR_CAPTIONESCDIR,rIn)  {}
    virtual SfxPoolItem*     Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*     Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT           GetValueCount() const; // { return 3; }
    SdrCaptionEscDir GetValue() const      { return (SdrCaptionEscDir)SfxEnumItem::GetValue(); }

    virtual String  GetValueTextByPos(USHORT nPos) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

//------------------------------------
// class SdrCaptionEscIsRelItem
// TRUE=Linienaustrittsposition relativ
// FALSE=Linienaustrittsposition absolut
//------------------------------------
class SdrCaptionEscIsRelItem: public SdrYesNoItem {
public:
    SdrCaptionEscIsRelItem(BOOL bRel=TRUE): SdrYesNoItem(SDRATTR_CAPTIONESCISREL,bRel) {}
    SdrCaptionEscIsRelItem(SvStream& rIn) : SdrYesNoItem(SDRATTR_CAPTIONESCISREL,rIn)  {}
};

//------------------------------------
// class SdrCaptionEscRelItem
// Relativer Linienaustritt
//     0 =   0.00% = oben bzw. links,
// 10000 = 100.00% = rechts bzw. unten
// nur wenn SdrCaptionEscIsRelItem=TRUE
//------------------------------------
class SdrCaptionEscRelItem: public SfxInt32Item {
public:
    SdrCaptionEscRelItem(long nEscRel=5000): SfxInt32Item(SDRATTR_CAPTIONESCREL,nEscRel) {}
    SdrCaptionEscRelItem(SvStream& rIn)    : SfxInt32Item(SDRATTR_CAPTIONESCREL,rIn)     {}
};

//------------------------------------
// class SdrCaptionEscAbsItem
// Absoluter Linienaustritt
// 0  = oben bzw. links,
// >0 = in Richtung rechts bzw. unten
// nur wenn SdrCaptionEscIsRelItem=FALSE
//------------------------------------
class SdrCaptionEscAbsItem: public SdrMetricItem {
public:
    SdrCaptionEscAbsItem(long nEscAbs=0): SdrMetricItem(SDRATTR_CAPTIONESCABS,nEscAbs) {}
    SdrCaptionEscAbsItem(SvStream& rIn) : SdrMetricItem(SDRATTR_CAPTIONESCABS,rIn)     {}
};

#endif
