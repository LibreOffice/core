/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

