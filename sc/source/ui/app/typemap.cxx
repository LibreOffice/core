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

#include <svx/svxids.hrc>
#include "sc.hrc"
#include "scitems.hxx"
#include <editeng/memberids.hrc>

#include <sfx2/msg.hxx>
#include <svl/stritem.hxx>
#include <svl/slstitm.hxx>
#include <sfx2/objitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svl/srchitem.hxx>
#include <svx/postattr.hxx>
#include <editeng/postitem.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/zoomitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <svl/ptitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/algitem.hxx>
#include <svx/clipfmtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xftadit.hxx>
#include <svx/xftdiit.hxx>
#include <svx/xftstit.hxx>
#include <svx/xftmrit.hxx>
#include <svx/xftouit.hxx>
#include <svx/xftshit.hxx>
#include <svx/xftshcit.hxx>
#include <svx/xftshxy.hxx>
#include <editeng/langitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/grafctrl.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/kernitem.hxx>
#include <svx/rotmodit.hxx>
#include <svx/drawitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/globalnameitem.hxx>
#include <svx/chrtitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/AffineMatrixItem.hxx>
#include <svx/galleryitem.hxx>
#include <svx/sdshitm.hxx>
#include <svl/aeitem.hxx>
#include <avmedia/mediaitem.hxx>
#include <sfx2/frame.hxx>
#include "attrib.hxx"

#define SvxDrawToolItem             SfxAllEnumItem
#define SvxDrawAlignItem            SfxAllEnumItem
#define avmedia_MediaItem           ::avmedia::MediaItem

#if defined(IOS)
/* Avoid clash with the ones from svx/source/form/typemap.cxx */
#define aSfxBoolItem_Impl sc_source_ui_appl_typemap_aSfxBoolItem_Impl
#define aSfxInt32Item_Impl sc_source_ui_appl_typemap_aSfxInt32Item_Impl
#define aSfxObjectItem_Impl sc_source_ui_appl_typemap_aSfxObjectItem_Impl
#define aSfxStringItem_Impl sc_source_ui_appl_typemap_aSfxStringItem_Impl
#define aSfxUInt16Item_Impl sc_source_ui_appl_typemap_aSfxUInt16Item_Impl
#define aSfxUInt32Item_Impl sc_source_ui_appl_typemap_aSfxUInt32Item_Impl
#define aSfxVoidItem_Impl sc_source_ui_appl_typemap_aSfxVoidItem_Impl
#define aSvxCharReliefItem_Impl sc_source_ui_appl_typemap_aSvxCharReliefItem_Impl
#define aSvxClipboardFormatItem_Impl sc_source_ui_appl_typemap_aSvxClipboardFormatItem_Impl
#define aSvxColorItem_Impl sc_source_ui_appl_typemap_aSvxColorItem_Impl
#define aSvxContourItem_Impl sc_source_ui_appl_typemap_aSvxContourItem_Impl
#define aSvxCrossedOutItem_Impl sc_source_ui_appl_typemap_aSvxCrossedOutItem_Impl
#define aSvxFontHeightItem_Impl sc_source_ui_appl_typemap_aSvxFontHeightItem_Impl
#define aSvxFontItem_Impl sc_source_ui_appl_typemap_aSvxFontItem_Impl
#define aSvxLanguageItem_Impl sc_source_ui_appl_typemap_aSvxLanguageItem_Impl
#define aSvxPostureItem_Impl sc_source_ui_appl_typemap_aSvxPostureItem_Impl
#define aSvxShadowedItem_Impl sc_source_ui_appl_typemap_aSvxShadowedItem_Impl
#define aSvxTextLineItem_Impl sc_source_ui_appl_typemap_aSvxTextLineItem_Impl
#define aSvxWeightItem_Impl sc_source_ui_appl_typemap_aSvxWeightItem_Impl
#endif

#define SFX_TYPEMAP
#include "scslots.hxx"

#if defined(IOS)
#undef aSfxBoolItem_Impl
#undef aSfxInt32Item_Impl
#undef aSfxObjectItem_Impl
#undef aSfxStringItem_Impl
#undef aSfxUInt16Item_Impl
#undef aSfxUInt32Item_Impl
#undef aSfxVoidItem_Impl
#undef aSvxCharReliefItem_Impl
#undef aSvxClipboardFormatItem_Impl
#undef aSvxColorItem_Impl
#undef aSvxContourItem_Impl
#undef aSvxCrossedOutItem_Impl
#undef aSvxFontHeightItem_Impl
#undef aSvxFontItem_Impl
#undef aSvxLanguageItem_Impl
#undef aSvxPostureItem_Impl
#undef aSvxShadowedItem_Impl
#undef aSvxTextLineItem_Impl
#undef aSvxWeightItem_Impl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
