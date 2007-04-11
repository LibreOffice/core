/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxlayitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:34:28 $
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
#ifndef _SYLAYITM_HXX
#define _SYLAYITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef _SVDTYPES_HXX //autogen
#include <svx/svdtypes.hxx>
#endif

//------------------------------
// class SdrLayerIdItem
//------------------------------
class SdrLayerIdItem: public SfxUInt16Item {
public:
    SdrLayerIdItem(USHORT nId=0): SfxUInt16Item(SDRATTR_LAYERID,nId)  {}
    SdrLayerIdItem(SvStream& rIn): SfxUInt16Item(SDRATTR_LAYERID,rIn) {}
    SdrLayerID GetValue() const { return (SdrLayerID)SfxUInt16Item::GetValue(); }
};

//------------------------------
// class SdrLayerNameItem
//------------------------------

class SdrLayerNameItem: public SfxStringItem {
public:
    SdrLayerNameItem()                   : SfxStringItem() { SetWhich(SDRATTR_LAYERNAME); }
    SdrLayerNameItem(const String& rStr) : SfxStringItem(SDRATTR_LAYERNAME,rStr) {}
    SdrLayerNameItem(SvStream& rIn)      : SfxStringItem(SDRATTR_LAYERNAME,rIn)  {}
};

#endif
