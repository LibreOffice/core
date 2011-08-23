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
#ifndef _SXCECITM_HXX
#define _SXCECITM_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDYNITM_HXX
#include <bf_svx/sdynitm.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <bf_svx/sdmetitm.hxx>
#endif
namespace binfilter {

enum SdrCaptionEscDir {SDRCAPT_ESCHORIZONTAL,SDRCAPT_ESCVERTICAL,SDRCAPT_ESCBESTFIT};

//------------------------------
// class SdrCaptionEscDirItem
//------------------------------
class SdrCaptionEscDirItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrCaptionEscDirItem(SdrCaptionEscDir eDir=SDRCAPT_ESCHORIZONTAL): SfxEnumItem(SDRATTR_CAPTIONESCDIR,eDir) {}
    SdrCaptionEscDirItem(SvStream& rIn)                              : SfxEnumItem(SDRATTR_CAPTIONESCDIR,rIn)  {}
    virtual SfxPoolItem*     Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*     Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT           GetValueCount() const; // { return 3; }
    SdrCaptionEscDir GetValue() const      { return (SdrCaptionEscDir)SfxEnumItem::GetValue(); }


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

}//end of namespace binfilter
#endif
