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

#include <svx/svxids.hrc>
#include "sc.hrc"
#include "scitems.hxx"
#include <editeng/memberids.hrc>

#define ITEMID_DBTYPE           0

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
#include <editeng/brshitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/bolnitem.hxx>
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
#include <svx/xftsfit.hxx>
#include <editeng/langitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/grafctrl.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <svx/rotmodit.hxx>
#include <svx/drawitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/globalnameitem.hxx>
#include <svx/chrtitem.hxx>
#include <svx/zoomslideritem.hxx>

// #i25616#
#include <svx/sdshitm.hxx>

#include <svl/aeitem.hxx>
#include <avmedia/mediaitem.hxx>
#include <sfx2/frame.hxx>
#include "attrib.hxx"

#define SvxDrawToolItem             SfxAllEnumItem
#define SvxDrawAlignItem            SfxAllEnumItem
#define SvxChooseControlItem        SfxEnumItem
#define avmedia_MediaItem           ::avmedia::MediaItem

#ifdef DISABLE_DYNLOADING
/* Avoid clash with the ones from svx/source/form/typemap.cxx */
#define aSfxBoolItem_Impl sc_source_ui_appl_typemap_aSfxBoolItem_Impl
#define aSfxByteItem_Impl sc_source_ui_appl_typemap_aSfxByteItem_Impl
#define aSfxGlobalNameItem_Impl sc_source_ui_appl_typemap_aSfxGlobalNameItem_Impl
#define aSfxInt16Item_Impl sc_source_ui_appl_typemap_aSfxInt16Item_Impl
#define aSfxInt32Item_Impl sc_source_ui_appl_typemap_aSfxInt32Item_Impl
#define aSfxObjectItem_Impl sc_source_ui_appl_typemap_aSfxObjectItem_Impl
#define aSfxPointItem_Impl sc_source_ui_appl_typemap_aSfxPointItem_Impl
#define aSfxStringItem_Impl sc_source_ui_appl_typemap_aSfxStringItem_Impl
#define aSfxStringListItem_Impl sc_source_ui_appl_typemap_aSfxStringListItem_Impl
#define aSfxTemplateItem_Impl sc_source_ui_appl_typemap_aSfxTemplateItem_Impl
#define aSfxUInt16Item_Impl sc_source_ui_appl_typemap_aSfxUInt16Item_Impl
#define aSfxUInt32Item_Impl sc_source_ui_appl_typemap_aSfxUInt32Item_Impl
#define aSfxVoidItem_Impl sc_source_ui_appl_typemap_aSfxVoidItem_Impl
#define aSvxBitmapListItem_Impl sc_source_ui_appl_typemap_aSvxBitmapListItem_Impl
#define aSvxBoxInfoItem_Impl sc_source_ui_appl_typemap_aSvxBoxInfoItem_Impl
#define aSvxBoxItem_Impl sc_source_ui_appl_typemap_aSvxBoxItem_Impl
#define aSvxBrushItem_Impl sc_source_ui_appl_typemap_aSvxBrushItem_Impl
#define aSvxCharReliefItem_Impl sc_source_ui_appl_typemap_aSvxCharReliefItem_Impl
#define aSvxClipboardFmtItem_Impl sc_source_ui_appl_typemap_aSvxClipboardFmtItem_Impl
#define aSvxColorItem_Impl sc_source_ui_appl_typemap_aSvxColorItem_Impl
#define aSvxColorListItem_Impl sc_source_ui_appl_typemap_aSvxColorListItem_Impl
#define aSvxContourItem_Impl sc_source_ui_appl_typemap_aSvxContourItem_Impl
#define aSvxCrossedOutItem_Impl sc_source_ui_appl_typemap_aSvxCrossedOutItem_Impl
#define aSvxDashListItem_Impl sc_source_ui_appl_typemap_aSvxDashListItem_Impl
#define aSvxDoubleItem_Impl sc_source_ui_appl_typemap_aSvxDoubleItem_Impl
#define aSvxDrawAlignItem_Impl sc_source_ui_appl_typemap_aSvxDrawAlignItem_Impl
#define aSvxDrawToolItem_Impl sc_source_ui_appl_typemap_aSvxDrawToolItem_Impl
#define aSvxFontHeightItem_Impl sc_source_ui_appl_typemap_aSvxFontHeightItem_Impl
#define aSvxFontItem_Impl sc_source_ui_appl_typemap_aSvxFontItem_Impl
#define aSvxGradientListItem_Impl sc_source_ui_appl_typemap_aSvxGradientListItem_Impl
#define aSvxHatchListItem_Impl sc_source_ui_appl_typemap_aSvxHatchListItem_Impl
#define aSvxHyperlinkItem_Impl sc_source_ui_appl_typemap_aSvxHyperlinkItem_Impl
#define aSvxLanguageItem_Impl sc_source_ui_appl_typemap_aSvxLanguageItem_Impl
#define aSvxLineEndListItem_Impl sc_source_ui_appl_typemap_aSvxLineEndListItem_Impl
#define aSvxLineItem_Impl sc_source_ui_appl_typemap_aSvxLineItem_Impl
#define aSvxPostItAuthorItem_Impl sc_source_ui_appl_typemap_aSvxPostItAuthorItem_Impl
#define aSvxPostItDateItem_Impl sc_source_ui_appl_typemap_aSvxPostItDateItem_Impl
#define aSvxPostItTextItem_Impl sc_source_ui_appl_typemap_aSvxPostItTextItem_Impl
#define aSvxPostureItem_Impl sc_source_ui_appl_typemap_aSvxPostureItem_Impl
#define aSvxSearchItem_Impl sc_source_ui_appl_typemap_aSvxSearchItem_Impl
#define aSvxShadowItem_Impl sc_source_ui_appl_typemap_aSvxShadowItem_Impl
#define aSvxShadowedItem_Impl sc_source_ui_appl_typemap_aSvxShadowedItem_Impl
#define aSvxSizeItem_Impl sc_source_ui_appl_typemap_aSvxSizeItem_Impl
#define aSvxTextLineItem_Impl sc_source_ui_appl_typemap_aSvxTextLineItem_Impl
#define aSvxWeightItem_Impl sc_source_ui_appl_typemap_aSvxWeightItem_Impl
#define aSvxZoomItem_Impl sc_source_ui_appl_typemap_aSvxZoomItem_Impl
#define aSvxZoomSliderItem_Impl sc_source_ui_appl_typemap_aSvxZoomSliderItem_Impl
#define aTbxImageItem_Impl sc_source_ui_appl_typemap_aTbxImageItem_Impl
#define aXFillBitmapItem_Impl sc_source_ui_appl_typemap_aXFillBitmapItem_Impl
#define aXFillColorItem_Impl sc_source_ui_appl_typemap_aXFillColorItem_Impl
#define aXFillGradientItem_Impl sc_source_ui_appl_typemap_aXFillGradientItem_Impl
#define aXFillHatchItem_Impl sc_source_ui_appl_typemap_aXFillHatchItem_Impl
#define aXFillStyleItem_Impl sc_source_ui_appl_typemap_aXFillStyleItem_Impl
#define aXFormTextAdjustItem_Impl sc_source_ui_appl_typemap_aXFormTextAdjustItem_Impl
#define aXFormTextDistanceItem_Impl sc_source_ui_appl_typemap_aXFormTextDistanceItem_Impl
#define aXFormTextHideFormItem_Impl sc_source_ui_appl_typemap_aXFormTextHideFormItem_Impl
#define aXFormTextMirrorItem_Impl sc_source_ui_appl_typemap_aXFormTextMirrorItem_Impl
#define aXFormTextOutlineItem_Impl sc_source_ui_appl_typemap_aXFormTextOutlineItem_Impl
#define aXFormTextShadowColorItem_Impl sc_source_ui_appl_typemap_aXFormTextShadowColorItem_Impl
#define aXFormTextShadowItem_Impl sc_source_ui_appl_typemap_aXFormTextShadowItem_Impl
#define aXFormTextShadowXValItem_Impl sc_source_ui_appl_typemap_aXFormTextShadowXValItem_Impl
#define aXFormTextShadowYValItem_Impl sc_source_ui_appl_typemap_aXFormTextShadowYValItem_Impl
#define aXFormTextStartItem_Impl sc_source_ui_appl_typemap_aXFormTextStartItem_Impl
#define aXFormTextStdFormItem_Impl sc_source_ui_appl_typemap_aXFormTextStdFormItem_Impl
#define aXFormTextStyleItem_Impl sc_source_ui_appl_typemap_aXFormTextStyleItem_Impl
#define aXLineColorItem_Impl sc_source_ui_appl_typemap_aXLineColorItem_Impl
#define aXLineDashItem_Impl sc_source_ui_appl_typemap_aXLineDashItem_Impl
#define aXLineEndItem_Impl sc_source_ui_appl_typemap_aXLineEndItem_Impl
#define aXLineStartItem_Impl sc_source_ui_appl_typemap_aXLineStartItem_Impl
#define aXLineStyleItem_Impl sc_source_ui_appl_typemap_aXLineStyleItem_Impl
#define aXLineWidthItem_Impl sc_source_ui_appl_typemap_aXLineWidthItem_Impl
#define aavmedia_MediaItem_Impl sc_source_ui_appl_typemap_aavmedia_MediaItem_Impl
#endif

#define SFX_TYPEMAP
#include "scslots.hxx"

#ifdef DISABLE_DYNLOADING
#undef aSfxBoolItem_Impl
#undef aSfxByteItem_Impl
#undef aSfxGlobalNameItem_Impl
#undef aSfxInt16Item_Impl
#undef aSfxInt32Item_Impl
#undef aSfxObjectItem_Impl
#undef aSfxPointItem_Impl
#undef aSfxStringItem_Impl
#undef aSfxStringListItem_Impl
#undef aSfxTemplateItem_Impl
#undef aSfxUInt16Item_Impl
#undef aSfxUInt32Item_Impl
#undef aSfxVoidItem_Impl
#undef aSvxBitmapListItem_Impl
#undef aSvxBoxInfoItem_Impl
#undef aSvxBox_Impl
#undef aSvxBrushItem_Impl
#undef aSvxCharReliefItem_Impl
#undef aSvxClipboardFmtItem_Impl
#undef aSvxColorItem_Impl
#undef aSvxColorListItem_Impl
#undef aSvxContourItem_Impl
#undef aSvxCrossedOutItem_Impl
#undef aSvxDashListItem_Impl
#undef aSvxDoubleItem_Impl
#undef aSvxDrawAlignItem_Impl
#undef aSvxDrawToolItem_Impl
#undef aSvxFontHeightItem_Impl
#undef aSvxFontItem_Impl
#undef aSvxGradientListItem_Impl
#undef aSvxHatchListItem_Impl
#undef aSvxHyperlinkItem_Impl
#undef aSvxLanguageItem_Impl
#undef aSvxLineEndListItem_Impl
#undef aSvxLineItem_Impl
#undef aSvxPostItAuthorItem_Impl
#undef aSvxPostItDateItem_Impl
#undef aSvxPostItTextItem_Impl
#undef aSvxPostureItem_Impl
#undef aSvxSearchItem_Impl
#undef aSvxShadowItem_Impl
#undef aSvxShadowedItem_Impl
#undef aSvxSizeItem_Impl
#undef aSvxTextLineItem_Impl
#undef aSvxWeightItem_Impl
#undef aSvxZoomItem_Impl
#undef aSvxZoomSliderItem_Impl
#undef aTbxImageItem_Impl
#undef aXFillBitmapItem_Impl
#undef aXFillColorItem_Impl
#undef aXFillGradientItem_Impl
#undef aXFillHatchItem_Impl
#undef aXFillStyleItem_Impl
#undef aXFormTextAdjustItem_Impl
#undef aXFormTextDistanceItem_Impl
#undef aXFormTextHideFormItem_Impl
#undef aXFormTextMirrorItem_Impl
#undef aXFormTextOutlineItem_Impl
#undef aXFormTextShadowColorItem_Impl
#undef aXFormTextShadowItem_Impl
#undef aXFormTextShadowXValItem_Impl
#undef aXFormTextShadowYValItem_Impl
#undef aXFormTextStartItem_Impl
#undef aXFormTextStdFormItem_Impl
#undef aXFormTextStyleItem_Impl
#undef aXLineColorItem_Impl
#undef aXLineDashItem_Impl
#undef aXLineEndItem_Impl
#undef aXLineStartItem_Impl
#undef aXLineStyleItem_Impl
#undef aXLineWidthItem_Impl
#undef aavmedia_MediaItem_Impl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
