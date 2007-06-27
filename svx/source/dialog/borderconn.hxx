/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: borderconn.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 16:50:25 $
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

#ifndef SVX_BORDERCONN_HXX
#define SVX_BORDERCONN_HXX

#ifndef SFX_ITEMCONNECT_HXX
#include <sfx2/itemconnect.hxx>
#endif
#ifndef SVX_FRAMEBORDERTYPE_HXX
#include <svx/framebordertype.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SfxItemSet;
class MetricField;
class ValueSet;
class ColorListBox;

namespace svx {

class FrameSelector;

// ============================================================================

/** Creates an item connection object that connects an SvxLineItem with an
    svx::FrameSelector control. */
SVX_DLLPUBLIC sfx::ItemConnectionBase* CreateFrameLineConnection( USHORT nSlot,
        FrameSelector& rFrameSel, FrameBorderType eBorder,
        sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );

/** Creates an item connection object that connects an SvxBoxItem and an
    SvxBoxInfoItem with an svx::FrameSelector control. */
sfx::ItemConnectionBase* CreateFrameBoxConnection(
        USHORT nBoxSlot, USHORT nBoxInfoSlot,
        FrameSelector& rFrameSel, FrameBorderType eBorder,
        sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );

/** Creates an item connection object that connects an SvxMarginItem with the
    controls of the SvxBorderTabPage. */
SVX_DLLPUBLIC sfx::ItemConnectionBase* CreateMarginConnection( const SfxItemSet& rItemSet,
        MetricField& rMfLeft, MetricField& rMfRight,
        MetricField& rMfTop, MetricField& rMfBottom,
        sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );

/** Creates an item connection object that connects an SvxShadowItem with the
    controls of the SvxBorderTabPage. */
SVX_DLLPUBLIC sfx::ItemConnectionBase* CreateShadowConnection( const SfxItemSet& rItemSet,
        ValueSet& rVsPos, MetricField& rMfSize, ColorListBox& rLbColor,
        sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );

// ============================================================================

} // namespace svx

#endif

