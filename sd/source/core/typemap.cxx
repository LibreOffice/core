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

#include <config_options.h>

#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fontitem.hxx>
#include <svl/poolitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/protitem.hxx>
#include <svx/chrtitem.hxx>
#include <sfx2/msg.hxx>
#include <svl/globalnameitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/postattr.hxx>
#include <editeng/editdata.hxx>
#include <svx/srchdlg.hxx>
#include <svx/rulritem.hxx>
#include <svx/clipfmtitem.hxx>
#include <svl/srchitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/svxenum.hxx>
#include <svx/algitem.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/svdattr.hxx>
#include <svx/grafctrl.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include "sdattr.hxx"
#include <svx/zoomslideritem.hxx>
#include <editeng/memberids.hrc>
#include <svx/xftstit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xftadit.hxx>
#include <svx/xftdiit.hxx>
#include <svx/xftmrit.hxx>
#include <svx/xftouit.hxx>
#include <svx/xftshit.hxx>
#include <svx/xftshcit.hxx>
#include <svx/xftshxy.hxx>
#include <avmedia/mediaitem.hxx>
#include <svx/drawitem.hxx>
#include <svl/aeitem.hxx>

// #UndoRedo#
#include <svl/slstitm.hxx>

#include <svl/lckbitem.hxx>

#define avmedia_MediaItem           ::avmedia::MediaItem
#include <sfx2/tplpitem.hxx>
#include <svl/ptitem.hxx>
#include <svl/rectitem.hxx>

#include <sfx2/frame.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/AffineMatrixItem.hxx>
#include <svx/galleryitem.hxx>

#ifdef DISABLE_DYNLOADING
/* Avoid clash with the ones from svx/source/form/typemap.cxx */
#define aSfxInt16Item_Impl sd_source_core_typemap_aSfxInt16Item_Impl
#define aSfxInt32Item_Impl sd_source_core_typemap_aSfxInt32Item_Impl
#define aSfxUnoFrameItem_Impl sd_source_core_typemap_aSfxUnoFrameItem_Impl
#define aSvxClipboardFormatItem_Impl sd_source_core_typemap_aSvxClipboardFormatItem_Impl
#define aSvxColorItem_Impl sd_source_core_typemap_aSvxColorItem_Impl
#define aSvxContourItem_Impl sd_source_core_typemap_aSvxContourItem_Impl
#define aSvxCrossedOutItem_Impl sd_source_core_typemap_aSvxCrossedOutItem_Impl
#define aSvxFontHeightItem_Impl sd_source_core_typemap_aSvxFontHeightItem_Impl
#define aSvxFontItem_Impl sd_source_core_typemap_aSvxFontItem_Impl
#define aSvxKerningItem_Impl sd_source_core_typemap_aSvxKerningItem_Impl
#define aSvxLRSpaceItem_Impl sd_source_core_typemap_aSvxLRSpaceItem_Impl
#define aSvxLanguageItem_Impl sd_source_core_typemap_aSvxLanguageItem_Impl
#define aSvxLineSpacingItem_Impl sd_source_core_typemap_aSvxLineSpacingItem_Impl
#define aSvxPostureItem_Impl sd_source_core_typemap_aSvxPostureItem_Impl
#define aSvxShadowedItem_Impl sd_source_core_typemap_aSvxShadowedItem_Impl
#define aSvxTextLineItem_Impl sd_source_core_typemap_aSvxTextLineItem_Impl
#define aSvxULSpaceItem_Impl sd_source_core_typemap_aSvxULSpaceItem_Impl
#define aSvxWeightItem_Impl sd_source_core_typemap_aSvxWeightItem_Impl
#define aSvxSearchItem_Impl sd_source_core_typemap_aSvxSearchItem_Impl
#define aSvxSizeItem_Impl sd_source_core_typemap_aSvxSizeItem_Impl
#endif

#define SFX_TYPEMAP
#include "sdslots.hxx"

#ifdef DISABLE_DYNLOADING
#undef aSfxInt16Item_Impl
#undef aSfxInt32Item_Impl
#undef aSfxUnoFrameItem_Impl
#undef aSvxClipboardFormatItem_Impl
#undef aSvxColorItem_Impl
#undef aSvxContourItem_Impl
#undef aSvxCrossedOutItem_Impl
#undef aSvxFontHeightItem_Impl
#undef aSvxFontItem_Impl
#undef aSvxKerningItem_Impl
#undef aSvxLRSpaceItem_Impl
#undef aSvxLanguageItem_Impl
#undef aSvxLineSpacingItem_Impl
#undef aSvxPostureItem_Impl
#undef aSvxShadowedItem_Impl
#undef aSvxTextLineItem_Impl
#undef aSvxULSpaceItem_Impl
#undef aSvxWeightItem_Impl
#undef aSvxSearchItem_Impl
#undef aSvxSizeItem_Impl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
