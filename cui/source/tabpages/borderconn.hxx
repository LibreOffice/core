/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CUI_SOURCE_TABPAGES_BORDERCONN_HXX
#define INCLUDED_CUI_SOURCE_TABPAGES_BORDERCONN_HXX

#include <sfx2/itemconnect.hxx>
#include <svx/framebordertype.hxx>

class SfxItemSet;
class MetricField;
class ValueSet;
class ColorListBox;

namespace svx {

class FrameSelector;



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



}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
