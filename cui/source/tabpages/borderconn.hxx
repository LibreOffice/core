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

#ifndef SVX_BORDERCONN_HXX
#define SVX_BORDERCONN_HXX

#include <sfx2/itemconnect.hxx>
#include <svx/framebordertype.hxx>

class SfxItemSet;
class MetricField;
class ValueSet;
class ColorListBox;

namespace svx {

class FrameSelector;

// ============================================================================

/** Creates an item connection object that connects an SvxLineItem with an
    svx::FrameSelector control. */
sfx::ItemConnectionBase* CreateFrameLineConnection( sal_uInt16 nSlot,
        FrameSelector& rFrameSel, FrameBorderType eBorder,
        sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );

/** Creates an item connection object that connects an SvxMarginItem with the
    controls of the SvxBorderTabPage. */
sfx::ItemConnectionBase* CreateMarginConnection( const SfxItemSet& rItemSet,
        MetricField& rMfLeft, MetricField& rMfRight,
        MetricField& rMfTop, MetricField& rMfBottom,
        sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );

/** Creates an item connection object that connects an SvxShadowItem with the
    controls of the SvxBorderTabPage. */
sfx::ItemConnectionBase* CreateShadowConnection( const SfxItemSet& rItemSet,
        ValueSet& rVsPos, MetricField& rMfSize, ColorListBox& rLbColor,
        sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );

// ============================================================================

} // namespace svx

#endif

