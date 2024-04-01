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

#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/MeasureKind.hpp>
#include <com/sun/star/drawing/MeasureTextHorzPos.hpp>
#include <com/sun/star/drawing/MeasureTextVertPos.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>

#include <docmodel/theme/FormatScheme.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <osl/diagnose.h>
#include <i18nutil/unicode.hxx>
#include <tools/bigint.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svx/xflbckit.hxx>
#include <xftshtit.hxx>
#include <svx/xflboxy.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xsflclit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xgrscit.hxx>
#include <svx/xflasit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbmpit.hxx>
#include <svx/xflbmsxy.hxx>
#include <svx/xflbmsli.hxx>
#include <svx/xflbtoxy.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xfilluseslidebackgrounditem.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xlnasit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/svddef.hxx>
#include <svl/itemset.hxx>
#include <svx/xftadit.hxx>
#include <svx/xftdiit.hxx>
#include <svx/xftstit.hxx>
#include <svx/xftmrit.hxx>
#include <svx/xftouit.hxx>
#include <svx/xftshit.hxx>
#include <svx/xftshcit.hxx>
#include <svx/xftshxy.hxx>

#include <svl/grabbagitem.hxx>
#include <svl/voiditem.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/sdasitm.hxx>
#include <sdgcoitm.hxx>
#include <svx/sdggaitm.hxx>
#include <sdginitm.hxx>
#include <svx/sdgluitm.hxx>
#include <svx/sdgmoitm.hxx>
#include <sdgtritm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/sdtaditm.hxx>
#include <svx/sdtaiitm.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/sdtakitm.hxx>
#include <svx/sdtayitm.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/svxids.hrc>
#include <sxallitm.hxx>
#include <sxcaitm.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/sxcgitm.hxx>
#include <sxcikitm.hxx>
#include <svx/sxcllitm.hxx>
#include <svx/sxctitm.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxenditm.hxx>
#include <sxfiitm.hxx>
#include <sxlayitm.hxx>
#include <sxlogitm.hxx>
#include <svx/sxmbritm.hxx>
#include <sxmfsitm.hxx>
#include <sxmkitm.hxx>
#include <sxmoitm.hxx>
#include <sxmovitm.hxx>
#include <sxmsitm.hxx>
#include <sxmtaitm.hxx>
#include <svx/sxmtfitm.hxx>
#include <svx/sxmtpitm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/sxmuitm.hxx>
#include <svx/xcolit.hxx>
#include <svx/RectangleAlignmentItem.hxx>
#include <sxoneitm.hxx>
#include <sxopitm.hxx>
#include <sxreaitm.hxx>
#include <sxreoitm.hxx>
#include <sxroaitm.hxx>
#include <sxrooitm.hxx>
#include <sxsaitm.hxx>
#include <sxsalitm.hxx>
#include <sxsiitm.hxx>
#include <sxsoitm.hxx>
#include <sxtraitm.hxx>
#include <editeng/frmdiritem.hxx>
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

static ItemInfoPackage& getItemInfoPackageSdr()
{
    class ItemInfoPackageSdr : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, SDRATTR_END - SDRATTR_START + 1> ItemInfoArraySdr;
        ItemInfoArraySdr maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { XATTR_LINESTYLE, new XLineStyleItem, SID_ATTR_LINE_STYLE, SFX_ITEMINFOFLAG_NONE },
            { XATTR_LINEDASH, new XLineDashItem(XDash()), SID_ATTR_LINE_DASH, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_LINEWIDTH, new XLineWidthItem, SID_ATTR_LINE_WIDTH, SFX_ITEMINFOFLAG_NONE },
            { XATTR_LINECOLOR, new XLineColorItem(OUString(), COL_DEFAULT_SHAPE_STROKE), SID_ATTR_LINE_COLOR, SFX_ITEMINFOFLAG_NONE },
            { XATTR_LINESTART, new XLineStartItem(basegfx::B2DPolyPolygon()), SID_ATTR_LINE_START, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_LINEEND, new XLineEndItem  (basegfx::B2DPolyPolygon()), SID_ATTR_LINE_END, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_LINESTARTWIDTH, new XLineStartWidthItem, SID_ATTR_LINE_STARTWIDTH, SFX_ITEMINFOFLAG_NONE },
            { XATTR_LINEENDWIDTH, new XLineEndWidthItem, SID_ATTR_LINE_ENDWIDTH, SFX_ITEMINFOFLAG_NONE },
            { XATTR_LINESTARTCENTER, new XLineStartCenterItem, SID_ATTR_LINE_STARTCENTER, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_LINEENDCENTER, new XLineEndCenterItem, SID_ATTR_LINE_ENDCENTER, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_LINETRANSPARENCE, new XLineTransparenceItem, SID_ATTR_LINE_TRANSPARENCE, SFX_ITEMINFOFLAG_NONE },
            { XATTR_LINEJOINT, new XLineJointItem, SID_ATTR_LINE_JOINT, SFX_ITEMINFOFLAG_NONE },
            { XATTR_LINECAP, new XLineCapItem, SID_ATTR_LINE_CAP, SFX_ITEMINFOFLAG_NONE },
            { XATTRSET_LINE, nullptr, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLSTYLE, new XFillStyleItem, SID_ATTR_FILL_STYLE, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLCOLOR, new XFillColorItem   (OUString(), COL_DEFAULT_SHAPE_FILLING), SID_ATTR_FILL_COLOR, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_FILLGRADIENT, new XFillGradientItem(basegfx::BGradient()), SID_ATTR_FILL_GRADIENT, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_FILLHATCH, new XFillHatchItem   (XHatch(COL_DEFAULT_SHAPE_STROKE)), SID_ATTR_FILL_HATCH, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_FILLBITMAP, nullptr, SID_ATTR_FILL_BITMAP, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_FILLTRANSPARENCE, new XFillTransparenceItem, SID_ATTR_FILL_TRANSPARENCE, SFX_ITEMINFOFLAG_NONE },
            { XATTR_GRADIENTSTEPCOUNT, new XGradientStepCountItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_TILE, new XFillBmpTileItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_POS, new XFillBmpPosItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_SIZEX, new XFillBmpSizeXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_SIZEY, new XFillBmpSizeYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLFLOATTRANSPARENCE, new XFillFloatTransparenceItem(basegfx::BGradient(basegfx::BColorStops(COL_BLACK.getBColor(), COL_BLACK.getBColor())), false), SID_ATTR_FILL_FLOATTRANSPARENCE, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { XATTR_SECONDARYFILLCOLOR, new XSecondaryFillColorItem(OUString(), COL_DEFAULT_SHAPE_FILLING), 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_SIZELOG, new XFillBmpSizeLogItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_TILEOFFSETX, new XFillBmpTileOffsetXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_TILEOFFSETY, new XFillBmpTileOffsetYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_STRETCH, new XFillBmpStretchItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_POSOFFSETX, new XFillBmpPosOffsetXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBMP_POSOFFSETY, new XFillBmpPosOffsetYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLBACKGROUND, new XFillBackgroundItem, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FILLUSESLIDEBACKGROUND, new XFillUseSlideBackgroundItem, SID_ATTR_FILL_USE_SLIDE_BACKGROUND, SFX_ITEMINFOFLAG_NONE },
            { XATTRSET_FILL, nullptr, 0, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTSTYLE, new XFormTextStyleItem, SID_FORMTEXT_STYLE, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTADJUST, new XFormTextAdjustItem, SID_FORMTEXT_ADJUST, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTDISTANCE, new XFormTextDistanceItem, SID_FORMTEXT_DISTANCE, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTSTART, new XFormTextStartItem, SID_FORMTEXT_START, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTMIRROR, new XFormTextMirrorItem, SID_FORMTEXT_MIRROR, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTOUTLINE, new XFormTextOutlineItem, SID_FORMTEXT_OUTLINE, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTSHADOW, new XFormTextShadowItem, SID_FORMTEXT_SHADOW, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTSHDWCOLOR, new XFormTextShadowColorItem(OUString(),COL_LIGHTGRAY), SID_FORMTEXT_SHDWCOLOR, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTSHDWXVAL, new XFormTextShadowXValItem, SID_FORMTEXT_SHDWXVAL, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTSHDWYVAL, new XFormTextShadowYValItem, SID_FORMTEXT_SHDWYVAL, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTHIDEFORM, new XFormTextHideFormItem, SID_FORMTEXT_HIDEFORM, SFX_ITEMINFOFLAG_NONE },
            { XATTR_FORMTXTSHDWTRANSP, new XFormTextShadowTranspItem, 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_SHADOW, new SdrOnOffItem(SDRATTR_SHADOW, false), SID_ATTR_FILL_SHADOW, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWCOLOR, new XColorItem(SDRATTR_SHADOWCOLOR, COL_BLACK), SID_ATTR_SHADOW_COLOR, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWXDIST, new SdrMetricItem(SDRATTR_SHADOWXDIST, 0), SID_ATTR_SHADOW_XDISTANCE, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWYDIST, new SdrMetricItem(SDRATTR_SHADOWYDIST, 0), SID_ATTR_SHADOW_YDISTANCE, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWTRANSPARENCE, new SdrPercentItem(SDRATTR_SHADOWTRANSPARENCE, 0), SID_ATTR_SHADOW_TRANSPARENCE, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOW3D, new SfxVoidItem(SDRATTR_SHADOW3D), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWPERSP, new SfxVoidItem(SDRATTR_SHADOWPERSP), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWSIZEX, new SdrMetricItem(SDRATTR_SHADOWSIZEX, 100000), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWSIZEY, new SdrMetricItem(SDRATTR_SHADOWSIZEY, 100000), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWBLUR, new SdrMetricItem(SDRATTR_SHADOWBLUR, 0), SID_ATTR_SHADOW_BLUR, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHADOWALIGNMENT, new SvxRectangleAlignmentItem(SDRATTR_SHADOWALIGNMENT, model::RectangleAlignment::Unset), 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_CAPTIONTYPE, new SdrCaptionTypeItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONFIXEDANGLE, new SdrOnOffItem(SDRATTR_CAPTIONFIXEDANGLE, true), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONANGLE, new SdrCaptionAngleItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONGAP, new SdrCaptionGapItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONESCDIR, new SdrCaptionEscDirItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONESCISREL, new SdrCaptionEscIsRelItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONESCREL, new SdrCaptionEscRelItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONESCABS, new SdrCaptionEscAbsItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONLINELEN, new SdrCaptionLineLenItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CAPTIONFITLINELEN, new SdrCaptionFitLineLenItem, 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_CORNER_RADIUS, new SdrMetricItem(SDRATTR_CORNER_RADIUS, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_MINFRAMEHEIGHT, new SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_AUTOGROWHEIGHT, new SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, true), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_FITTOSIZE, new SdrTextFitToSizeTypeItem, SID_ATTR_TEXT_FITTOSIZE, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_LEFTDIST, new SdrMetricItem(SDRATTR_TEXT_LEFTDIST, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_RIGHTDIST, new SdrMetricItem(SDRATTR_TEXT_RIGHTDIST, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_UPPERDIST, new SdrMetricItem(SDRATTR_TEXT_UPPERDIST, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_LOWERDIST, new SdrMetricItem(SDRATTR_TEXT_LOWERDIST, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_VERTADJUST, new SdrTextVertAdjustItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_MAXFRAMEHEIGHT, new SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_MINFRAMEWIDTH, new SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_MAXFRAMEWIDTH, new SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_AUTOGROWWIDTH, new SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_HORZADJUST, new SdrTextHorzAdjustItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_ANIKIND, new SdrTextAniKindItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_ANIDIRECTION, new SdrTextAniDirectionItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_ANISTARTINSIDE, new SdrTextAniStartInsideItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_ANISTOPINSIDE, new SdrTextAniStopInsideItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_ANICOUNT, new SdrTextAniCountItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_ANIDELAY, new SdrTextAniDelayItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_ANIAMOUNT, new SdrTextAniAmountItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_CONTOURFRAME, new SdrOnOffItem(SDRATTR_TEXT_CONTOURFRAME, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_XMLATTRIBUTES, new SvXMLAttrContainerItem( SDRATTR_XMLATTRIBUTES ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { SDRATTR_TEXT_USEFIXEDCELLHEIGHT, new SdrTextFixedCellHeightItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_WORDWRAP, new SdrOnOffItem(SDRATTR_TEXT_WORDWRAP, true), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_CHAINNEXTNAME, new SfxStringItem(SDRATTR_TEXT_CHAINNEXTNAME, ""), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXT_CLIPVERTOVERFLOW, new SdrOnOffItem(SDRATTR_TEXT_CLIPVERTOVERFLOW, false), 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_EDGEKIND, new SdrEdgeKindItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGENODE1HORZDIST, new SdrEdgeNode1HorzDistItem(500), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGENODE1VERTDIST, new SdrEdgeNode1VertDistItem(500), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGENODE2HORZDIST, new SdrEdgeNode2HorzDistItem(500), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGENODE2VERTDIST, new SdrEdgeNode2VertDistItem(500), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGENODE1GLUEDIST, new SdrEdgeNode1GlueDistItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGENODE2GLUEDIST, new SdrEdgeNode2GlueDistItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGELINEDELTACOUNT, new SdrEdgeLineDeltaCountItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGELINE1DELTA, new SdrMetricItem(SDRATTR_EDGELINE1DELTA, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGELINE2DELTA, new SdrMetricItem(SDRATTR_EDGELINE2DELTA, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_EDGELINE3DELTA, new SdrMetricItem(SDRATTR_EDGELINE3DELTA, 0), 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_MEASUREKIND, new SdrMeasureKindItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURETEXTHPOS, new SdrMeasureTextHPosItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURETEXTVPOS, new SdrMeasureTextVPosItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURELINEDIST, new SdrMetricItem(SDRATTR_MEASURELINEDIST, 800), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREHELPLINEOVERHANG, new SdrMetricItem(SDRATTR_MEASUREHELPLINEOVERHANG, 200), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREHELPLINEDIST, new SdrMetricItem(SDRATTR_MEASUREHELPLINEDIST, 100), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREHELPLINE1LEN, new SdrMetricItem(SDRATTR_MEASUREHELPLINE1LEN, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREHELPLINE2LEN, new SdrMetricItem(SDRATTR_MEASUREHELPLINE2LEN, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREBELOWREFEDGE, new SdrMeasureBelowRefEdgeItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURETEXTROTA90, new SdrMeasureTextRota90Item, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURETEXTUPSIDEDOWN, new SdrMeasureTextUpsideDownItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREOVERHANG, new SdrMeasureOverhangItem(600), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREUNIT, new SdrMeasureUnitItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURESCALE, new SdrMeasureScaleItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURESHOWUNIT, new SdrYesNoItem(SDRATTR_MEASURESHOWUNIT, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREFORMATSTRING, new SdrMeasureFormatStringItem(), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURETEXTAUTOANGLE, new SdrMeasureTextAutoAngleItem(), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURETEXTAUTOANGLEVIEW, new SdrMeasureTextAutoAngleViewItem(), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURETEXTISFIXEDANGLE, new SdrMeasureTextIsFixedAngleItem(), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASURETEXTFIXEDANGLE, new SdrMeasureTextFixedAngleItem(), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MEASUREDECIMALPLACES, new SdrMeasureDecimalPlacesItem(), 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_CIRCKIND, new SdrCircKindItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CIRCSTARTANGLE, new SdrAngleItem(SDRATTR_CIRCSTARTANGLE, 0_deg100), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CIRCENDANGLE, new SdrAngleItem(SDRATTR_CIRCENDANGLE, 36000_deg100), 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_OBJMOVEPROTECT, new SdrYesNoItem(SDRATTR_OBJMOVEPROTECT, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_OBJSIZEPROTECT, new SdrYesNoItem(SDRATTR_OBJSIZEPROTECT, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_OBJPRINTABLE, new SdrObjPrintableItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_LAYERID, new SdrLayerIdItem(SdrLayerID(0)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_LAYERNAME, new SdrLayerNameItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_OBJECTNAME, new SfxStringItem(SDRATTR_OBJECTNAME), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ALLPOSITIONX, new SdrAllPositionXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ALLPOSITIONY, new SdrAllPositionYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ALLSIZEWIDTH, new SdrAllSizeWidthItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ALLSIZEHEIGHT, new SdrAllSizeHeightItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ONEPOSITIONX, new SdrOnePositionXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ONEPOSITIONY, new SdrOnePositionYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ONESIZEWIDTH, new SdrOneSizeWidthItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ONESIZEHEIGHT, new SdrOneSizeHeightItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_LOGICSIZEWIDTH, new SdrLogicSizeWidthItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_LOGICSIZEHEIGHT, new SdrLogicSizeHeightItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ROTATEANGLE, new SdrAngleItem(SDRATTR_ROTATEANGLE, 0_deg100), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_SHEARANGLE, new SdrShearAngleItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MOVEX, new SdrMoveXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_MOVEY, new SdrMoveYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_RESIZEXONE, new SdrResizeXOneItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_RESIZEYONE, new SdrResizeYOneItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ROTATEONE, new SdrRotateOneItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_HORZSHEARONE, new SdrHorzShearOneItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_VERTSHEARONE, new SdrVertShearOneItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_RESIZEXALL, new SdrResizeXAllItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_RESIZEYALL, new SdrResizeYAllItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_ROTATEALL, new SdrRotateAllItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_HORZSHEARALL, new SdrHorzShearAllItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_VERTSHEARALL, new SdrVertShearAllItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TRANSFORMREF1X, new SdrTransformRef1XItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TRANSFORMREF1Y, new SdrTransformRef1YItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TRANSFORMREF2X, new SdrTransformRef2XItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TRANSFORMREF2Y, new SdrTransformRef2YItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXTDIRECTION, new SvxWritingModeItem(css::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_OBJVISIBLE, new SdrObjVisibleItem, 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_GRAFRED, new SdrGrafRedItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFGREEN, new SdrGrafGreenItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFBLUE, new SdrGrafBlueItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFLUMINANCE, new SdrGrafLuminanceItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFCONTRAST, new SdrGrafContrastItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFGAMMA, new SdrGrafGamma100Item, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFTRANSPARENCE, new SdrGrafTransparenceItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFINVERT, new SdrGrafInvertItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFMODE, new SdrGrafModeItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GRAFCROP, new SdrGrafCropItem, SID_ATTR_GRAF_CROP, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_3DOBJ_PERCENT_DIAGONAL, new SfxUInt16Item(SDRATTR_3DOBJ_PERCENT_DIAGONAL, 10), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_BACKSCALE, new SfxUInt16Item(SDRATTR_3DOBJ_BACKSCALE, 100), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_DEPTH, new SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, 1000), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_HORZ_SEGS, new SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, 24), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_VERT_SEGS, new SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, 24), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_END_ANGLE, new SfxUInt32Item(SDRATTR_3DOBJ_END_ANGLE, 3600), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_DOUBLE_SIDED, new SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_NORMALS_KIND, new Svx3DNormalsKindItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_NORMALS_INVERT, new SfxBoolItem(SDRATTR_3DOBJ_NORMALS_INVERT, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_TEXTURE_PROJ_X, new Svx3DTextureProjectionXItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_TEXTURE_PROJ_Y, new Svx3DTextureProjectionYItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_SHADOW_3D, new SfxBoolItem(SDRATTR_3DOBJ_SHADOW_3D, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_MAT_COLOR, new SvxColorItem(Color(0x0000b8ff), SDRATTR_3DOBJ_MAT_COLOR), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_MAT_EMISSION, new SvxColorItem(Color(0x00000000), SDRATTR_3DOBJ_MAT_EMISSION), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_MAT_SPECULAR, new SvxColorItem(Color(0x00ffffff), SDRATTR_3DOBJ_MAT_SPECULAR), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, new SfxUInt16Item(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, 15), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_TEXTURE_KIND, new Svx3DTextureKindItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_TEXTURE_MODE, new Svx3DTextureModeItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_TEXTURE_FILTER, new SfxBoolItem(SDRATTR_3DOBJ_TEXTURE_FILTER, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_SMOOTH_NORMALS, new Svx3DSmoothNormalsItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_SMOOTH_LIDS, new Svx3DSmoothLidsItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_CHARACTER_MODE, new Svx3DCharacterModeItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_CLOSE_FRONT, new Svx3DCloseFrontItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_CLOSE_BACK, new Svx3DCloseBackItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY, new Svx3DReducedLineGeometryItem, 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_3DSCENE_PERSPECTIVE, new Svx3DPerspectiveItem, 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_DISTANCE, new SfxUInt32Item(SDRATTR_3DSCENE_DISTANCE, 100), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_FOCAL_LENGTH, new SfxUInt32Item(SDRATTR_3DSCENE_FOCAL_LENGTH, 100), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, new SfxBoolItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTCOLOR_1, new SvxColorItem(Color(ColorTransparency, 0xffcccccc), SDRATTR_3DSCENE_LIGHTCOLOR_1), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTCOLOR_2, new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_2), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTCOLOR_3, new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_3), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTCOLOR_4, new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_4), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTCOLOR_5, new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_5), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTCOLOR_6, new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_6), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTCOLOR_7, new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_7), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTCOLOR_8, new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_8), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_AMBIENTCOLOR, new SvxColorItem(Color(0x00666666), SDRATTR_3DSCENE_AMBIENTCOLOR), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTON_1, new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_1, true), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTON_2, new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_2, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTON_3, new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_3, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTON_4, new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_4, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTON_5, new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_5, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTON_6, new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_6, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTON_7, new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_7, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTON_8, new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_8, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTDIRECTION_1, new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_1, basegfx::B3DVector(0.57735026918963, 0.57735026918963, 0.57735026918963)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTDIRECTION_2, new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_2, basegfx::B3DVector(0.0,0.0,1.0)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTDIRECTION_3, new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_3, basegfx::B3DVector(0.0,0.0,1.0)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTDIRECTION_4, new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_4, basegfx::B3DVector(0.0,0.0,1.0)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTDIRECTION_5, new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_5, basegfx::B3DVector(0.0,0.0,1.0)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTDIRECTION_6, new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_6, basegfx::B3DVector(0.0,0.0,1.0)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTDIRECTION_7, new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_7, basegfx::B3DVector(0.0,0.0,1.0)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_LIGHTDIRECTION_8, new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_8, basegfx::B3DVector(0.0,0.0,1.0)), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_SHADOW_SLANT, new SfxUInt16Item(SDRATTR_3DSCENE_SHADOW_SLANT, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_3DSCENE_SHADE_MODE, new Svx3DShadeModeItem, 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_CUSTOMSHAPE_ENGINE, new SfxStringItem(SDRATTR_CUSTOMSHAPE_ENGINE, ""), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CUSTOMSHAPE_DATA, new SfxStringItem(SDRATTR_CUSTOMSHAPE_DATA, ""), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_CUSTOMSHAPE_GEOMETRY, new SdrCustomShapeGeometryItem, 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_TABLE_BORDER, nullptr, SID_ATTR_BORDER_OUTER, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TABLE_BORDER_INNER, nullptr, SID_ATTR_BORDER_INNER, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TABLE_BORDER_TLBR, new SvxLineItem( SDRATTR_TABLE_BORDER_TLBR ), SID_ATTR_BORDER_DIAG_TLBR, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TABLE_BORDER_BLTR, new SvxLineItem( SDRATTR_TABLE_BORDER_BLTR ), SID_ATTR_BORDER_DIAG_BLTR, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TABLE_TEXT_ROTATION, new SvxTextRotateItem(0_deg10, SDRATTR_TABLE_TEXT_ROTATION), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TABLE_CELL_GRABBAG, new SfxGrabBagItem(SDRATTR_TABLE_CELL_GRABBAG), 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_GLOW_RADIUS, new SdrMetricItem(SDRATTR_GLOW_RADIUS, 0), SID_ATTR_GLOW_RADIUS, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GLOW_COLOR, new XColorItem(SDRATTR_GLOW_COLOR, COL_BLACK), SID_ATTR_GLOW_COLOR, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_GLOW_TRANSPARENCY, new SdrPercentItem(SDRATTR_GLOW_TRANSPARENCY, 0), SID_ATTR_GLOW_TRANSPARENCY, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_SOFTEDGE_RADIUS, new SdrMetricItem(SDRATTR_SOFTEDGE_RADIUS, 0), SID_ATTR_SOFTEDGE_RADIUS, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_TEXTCOLUMNS_NUMBER, new SfxInt16Item(SDRATTR_TEXTCOLUMNS_NUMBER, 1), 0, SFX_ITEMINFOFLAG_NONE },
            { SDRATTR_TEXTCOLUMNS_SPACING, new SdrMetricItem(SDRATTR_TEXTCOLUMNS_SPACING, 0), 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_WRITINGMODE2, new SvxFrameDirectionItem(SvxFrameDirection::Horizontal_LR_TB, SDRATTR_WRITINGMODE2), 0, SFX_ITEMINFOFLAG_NONE },

            { SDRATTR_EDGEOOXMLCURVE, new SfxBoolItem(SDRATTR_EDGEOOXMLCURVE, false), 0, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        ItemInfoPackageSdr()
        {
            SvxBoxItem* pboxItem(new SvxBoxItem(SDRATTR_TABLE_BORDER));
            pboxItem->SetAllDistances(100);
            setItemAtItemInfoStatic(pboxItem, maItemInfos[SDRATTR_TABLE_BORDER - SDRATTR_START]);

            SvxBoxInfoItem* pBoxInfoItem = new SvxBoxInfoItem(SDRATTR_TABLE_BORDER_INNER);
            pBoxInfoItem->SetTable(true);
            pBoxInfoItem->SetDist(true); // always show margin field
            // some lines may have DontCare state only in tables
            pBoxInfoItem->SetValid(SvxBoxInfoItemValidFlags::DISABLE);
            setItemAtItemInfoStatic(pBoxInfoItem, maItemInfos[SDRATTR_TABLE_BORDER_INNER - SDRATTR_START]);
        }

        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& rPool) override
        {
            const ItemInfo& rRetval(maItemInfos[nIndex]);

            // return immediately if we have the static entry and Item
            if (nullptr != rRetval.getItem())
                return rRetval;

            if (XATTRSET_LINE == rRetval.getWhich())
                return *new ItemInfoDynamic(rRetval, new XLineAttrSetItem(SfxItemSetFixed<XATTR_LINE_FIRST, XATTR_LINE_LAST>(rPool)));

            if (XATTRSET_FILL == rRetval.getWhich())
                return *new ItemInfoDynamic(rRetval, new XFillAttrSetItem(SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST>(rPool)));

            if (XATTR_FILLBITMAP == rRetval.getWhich())
                return *new ItemInfoDynamic(rRetval, new XFillBitmapItem(Graphic()));

            // return in any case
            return rRetval;
        }
    };

    static std::unique_ptr<ItemInfoPackageSdr> g_aItemInfoPackageSdr;
    if (!g_aItemInfoPackageSdr)
        g_aItemInfoPackageSdr.reset(new ItemInfoPackageSdr);
    return *g_aItemInfoPackageSdr;
}

SdrItemPool::SdrItemPool(SfxItemPool* _pMaster)
: SfxItemPool("SdrItemPool")
{
    // registerItemInfoPackage(getItemInfoPackageXOutdev());
    registerItemInfoPackage(getItemInfoPackageSdr());

    // get master pointer, evtl. add myself to the end of the pools
    if(nullptr != _pMaster)
    {
        _pMaster->GetLastPoolInChain()->SetSecondaryPool(this);
    }
}

SdrItemPool::SdrItemPool(const SdrItemPool& rPool)
: SfxItemPool(rPool)
{
}

rtl::Reference<SfxItemPool> SdrItemPool::Clone() const
{
    return new SdrItemPool(*this);
}

SdrItemPool::~SdrItemPool()
{
    // split pools before destroying
    SetSecondaryPool(nullptr);
    sendShutdownHint();
}

bool SdrItemPool::GetPresentation(
              const SfxPoolItem& rItem,
              MapUnit ePresentationMetric, OUString& rText,
              const IntlWrapper& rIntlWrapper) const
{
    if (!IsInvalidItem(&rItem)) {
        sal_uInt16 nWhich=rItem.Which();
        if (nWhich>=SDRATTR_SHADOW_FIRST && nWhich<=SDRATTR_END) {
            rItem.GetPresentation(SfxItemPresentation::Nameless,
                        GetMetric(nWhich),ePresentationMetric,rText,
                        rIntlWrapper);
            rText = GetItemName(nWhich) + " " + rText;

            return true;
        }
    }

    return SfxItemPool::GetPresentation(rItem,ePresentationMetric,rText,rIntlWrapper);
}

OUString SdrItemPool::GetItemName(sal_uInt16 nWhich)
{
    TranslateId pResId = SIP_UNKNOWN_ATTR;

    switch (nWhich)
    {
        case XATTR_LINESTYLE        : pResId = SIP_XA_LINESTYLE;break;
        case XATTR_LINEDASH         : pResId = SIP_XA_LINEDASH;break;
        case XATTR_LINEWIDTH        : pResId = SIP_XA_LINEWIDTH;break;
        case XATTR_LINECOLOR        : pResId = SIP_XA_LINECOLOR;break;
        case XATTR_LINESTART        : pResId = SIP_XA_LINESTART;break;
        case XATTR_LINEEND          : pResId = SIP_XA_LINEEND;break;
        case XATTR_LINESTARTWIDTH   : pResId = SIP_XA_LINESTARTWIDTH;break;
        case XATTR_LINEENDWIDTH     : pResId = SIP_XA_LINEENDWIDTH;break;
        case XATTR_LINESTARTCENTER  : pResId = SIP_XA_LINESTARTCENTER;break;
        case XATTR_LINEENDCENTER    : pResId = SIP_XA_LINEENDCENTER;break;
        case XATTR_LINETRANSPARENCE : pResId = SIP_XA_LINETRANSPARENCE;break;
        case XATTR_LINEJOINT        : pResId = SIP_XA_LINEJOINT;break;
        case XATTRSET_LINE          : pResId = SIP_XATTRSET_LINE;break;

        case XATTR_FILLSTYLE            : pResId = SIP_XA_FILLSTYLE;break;
        case XATTR_FILLCOLOR            : pResId = SIP_XA_FILLCOLOR;break;
        case XATTR_FILLGRADIENT         : pResId = SIP_XA_FILLGRADIENT;break;
        case XATTR_FILLHATCH            : pResId = SIP_XA_FILLHATCH;break;
        case XATTR_FILLBITMAP           : pResId = SIP_XA_FILLBITMAP;break;
        case XATTR_FILLTRANSPARENCE     : pResId = SIP_XA_FILLTRANSPARENCE;break;
        case XATTR_GRADIENTSTEPCOUNT    : pResId = SIP_XA_GRADIENTSTEPCOUNT;break;
        case XATTR_FILLBMP_TILE         : pResId = SIP_XA_FILLBMP_TILE;break;
        case XATTR_FILLBMP_POS          : pResId = SIP_XA_FILLBMP_POS;break;
        case XATTR_FILLBMP_SIZEX        : pResId = SIP_XA_FILLBMP_SIZEX;break;
        case XATTR_FILLBMP_SIZEY        : pResId = SIP_XA_FILLBMP_SIZEY;break;
        case XATTR_FILLFLOATTRANSPARENCE: pResId = SIP_XA_FILLFLOATTRANSPARENCE;break;
        case XATTR_SECONDARYFILLCOLOR   : pResId = SIP_XA_SECONDARYFILLCOLOR;break;
        case XATTR_FILLBMP_SIZELOG      : pResId = SIP_XA_FILLBMP_SIZELOG;break;
        case XATTR_FILLBMP_TILEOFFSETX  : pResId = SIP_XA_FILLBMP_TILEOFFSETX;break;
        case XATTR_FILLBMP_TILEOFFSETY  : pResId = SIP_XA_FILLBMP_TILEOFFSETY;break;
        case XATTR_FILLBMP_STRETCH      : pResId = SIP_XA_FILLBMP_STRETCH;break;
        case XATTR_FILLBMP_POSOFFSETX   : pResId = SIP_XA_FILLBMP_POSOFFSETX;break;
        case XATTR_FILLBMP_POSOFFSETY   : pResId = SIP_XA_FILLBMP_POSOFFSETY;break;
        case XATTR_FILLBACKGROUND       : pResId = SIP_XA_FILLBACKGROUND;break;
        case XATTR_FILLUSESLIDEBACKGROUND: pResId = SIP_XA_FILLUSESLIDEBACKGROUND;break;

        case XATTRSET_FILL             : pResId = SIP_XATTRSET_FILL;break;

        case XATTR_FORMTXTSTYLE     : pResId = SIP_XA_FORMTXTSTYLE;break;
        case XATTR_FORMTXTADJUST    : pResId = SIP_XA_FORMTXTADJUST;break;
        case XATTR_FORMTXTDISTANCE  : pResId = SIP_XA_FORMTXTDISTANCE;break;
        case XATTR_FORMTXTSTART     : pResId = SIP_XA_FORMTXTSTART;break;
        case XATTR_FORMTXTMIRROR    : pResId = SIP_XA_FORMTXTMIRROR;break;
        case XATTR_FORMTXTOUTLINE   : pResId = SIP_XA_FORMTXTOUTLINE;break;
        case XATTR_FORMTXTSHADOW    : pResId = SIP_XA_FORMTXTSHADOW;break;
        case XATTR_FORMTXTSHDWCOLOR : pResId = SIP_XA_FORMTXTSHDWCOLOR;break;
        case XATTR_FORMTXTSHDWXVAL  : pResId = SIP_XA_FORMTXTSHDWXVAL;break;
        case XATTR_FORMTXTSHDWYVAL  : pResId = SIP_XA_FORMTXTSHDWYVAL;break;
        case XATTR_FORMTXTHIDEFORM  : pResId = SIP_XA_FORMTXTHIDEFORM;break;
        case XATTR_FORMTXTSHDWTRANSP: pResId = SIP_XA_FORMTXTSHDWTRANSP;break;

        case SDRATTR_SHADOW            : pResId = SIP_SA_SHADOW;break;
        case SDRATTR_SHADOWCOLOR       : pResId = SIP_SA_SHADOWCOLOR;break;
        case SDRATTR_SHADOWXDIST       : pResId = SIP_SA_SHADOWXDIST;break;
        case SDRATTR_SHADOWYDIST       : pResId = SIP_SA_SHADOWYDIST;break;
        case SDRATTR_SHADOWTRANSPARENCE: pResId = SIP_SA_SHADOWTRANSPARENCE;break;
        case SDRATTR_SHADOWBLUR        : pResId = SIP_SA_SHADOWBLUR;break;
        case SDRATTR_SHADOW3D          : pResId = SIP_SA_SHADOW3D;break;
        case SDRATTR_SHADOWPERSP       : pResId = SIP_SA_SHADOWPERSP;break;

        case SDRATTR_GLOW_RADIUS       : pResId = SIP_SA_GLOW_RADIUS;break;
        case SDRATTR_GLOW_COLOR        : pResId = SIP_SA_GLOW_COLOR;break;
        case SDRATTR_GLOW_TRANSPARENCY : pResId = SIP_SA_GLOW_TRANSPARENCY;break;

        case SDRATTR_SOFTEDGE_RADIUS   : pResId = SIP_SA_SOFTEDGE_RADIUS; break;

        case SDRATTR_CAPTIONTYPE      : pResId = SIP_SA_CAPTIONTYPE;break;
        case SDRATTR_CAPTIONFIXEDANGLE: pResId = SIP_SA_CAPTIONFIXEDANGLE;break;
        case SDRATTR_CAPTIONANGLE     : pResId = SIP_SA_CAPTIONANGLE;break;
        case SDRATTR_CAPTIONGAP       : pResId = SIP_SA_CAPTIONGAP;break;
        case SDRATTR_CAPTIONESCDIR    : pResId = SIP_SA_CAPTIONESCDIR;break;
        case SDRATTR_CAPTIONESCISREL  : pResId = SIP_SA_CAPTIONESCISREL;break;
        case SDRATTR_CAPTIONESCREL    : pResId = SIP_SA_CAPTIONESCREL;break;
        case SDRATTR_CAPTIONESCABS    : pResId = SIP_SA_CAPTIONESCABS;break;
        case SDRATTR_CAPTIONLINELEN   : pResId = SIP_SA_CAPTIONLINELEN;break;
        case SDRATTR_CAPTIONFITLINELEN: pResId = SIP_SA_CAPTIONFITLINELEN;break;

        case SDRATTR_CORNER_RADIUS          : pResId = SIP_SA_CORNER_RADIUS;break;
        case SDRATTR_TEXT_MINFRAMEHEIGHT    : pResId = SIP_SA_TEXT_MINFRAMEHEIGHT;break;
        case SDRATTR_TEXT_AUTOGROWHEIGHT    : pResId = SIP_SA_TEXT_AUTOGROWHEIGHT;break;
        case SDRATTR_TEXT_FITTOSIZE         : pResId = SIP_SA_TEXT_FITTOSIZE;break;
        case SDRATTR_TEXT_LEFTDIST          : pResId = SIP_SA_TEXT_LEFTDIST;break;
        case SDRATTR_TEXT_RIGHTDIST         : pResId = SIP_SA_TEXT_RIGHTDIST;break;
        case SDRATTR_TEXT_UPPERDIST         : pResId = SIP_SA_TEXT_UPPERDIST;break;
        case SDRATTR_TEXT_LOWERDIST         : pResId = SIP_SA_TEXT_LOWERDIST;break;
        case SDRATTR_TEXT_VERTADJUST        : pResId = SIP_SA_TEXT_VERTADJUST;break;
        case SDRATTR_TEXT_MAXFRAMEHEIGHT    : pResId = SIP_SA_TEXT_MAXFRAMEHEIGHT;break;
        case SDRATTR_TEXT_MINFRAMEWIDTH     : pResId = SIP_SA_TEXT_MINFRAMEWIDTH;break;
        case SDRATTR_TEXT_MAXFRAMEWIDTH     : pResId = SIP_SA_TEXT_MAXFRAMEWIDTH;break;
        case SDRATTR_TEXT_AUTOGROWWIDTH     : pResId = SIP_SA_TEXT_AUTOGROWWIDTH;break;
        case SDRATTR_TEXT_HORZADJUST        : pResId = SIP_SA_TEXT_HORZADJUST;break;
        case SDRATTR_TEXT_ANIKIND           : pResId = SIP_SA_TEXT_ANIKIND;break;
        case SDRATTR_TEXT_ANIDIRECTION      : pResId = SIP_SA_TEXT_ANIDIRECTION;break;
        case SDRATTR_TEXT_ANISTARTINSIDE    : pResId = SIP_SA_TEXT_ANISTARTINSIDE;break;
        case SDRATTR_TEXT_ANISTOPINSIDE     : pResId = SIP_SA_TEXT_ANISTOPINSIDE;break;
        case SDRATTR_TEXT_ANICOUNT          : pResId = SIP_SA_TEXT_ANICOUNT;break;
        case SDRATTR_TEXT_ANIDELAY          : pResId = SIP_SA_TEXT_ANIDELAY;break;
        case SDRATTR_TEXT_ANIAMOUNT         : pResId = SIP_SA_TEXT_ANIAMOUNT;break;
        case SDRATTR_TEXT_CONTOURFRAME      : pResId = SIP_SA_TEXT_CONTOURFRAME;break;
        case SDRATTR_XMLATTRIBUTES          : pResId = SIP_SA_XMLATTRIBUTES;break;
        case SDRATTR_TEXT_USEFIXEDCELLHEIGHT: pResId = SIP_SA_TEXT_USEFIXEDCELLHEIGHT;break;
        case SDRATTR_TEXT_WORDWRAP          : pResId = SIP_SA_WORDWRAP;break;
        case SDRATTR_TEXT_CHAINNEXTNAME     : pResId = SIP_SA_CHAINNEXTNAME;break;

        case SDRATTR_EDGEKIND           : pResId = SIP_SA_EDGEKIND;break;
        case SDRATTR_EDGENODE1HORZDIST  : pResId = SIP_SA_EDGENODE1HORZDIST;break;
        case SDRATTR_EDGENODE1VERTDIST  : pResId = SIP_SA_EDGENODE1VERTDIST;break;
        case SDRATTR_EDGENODE2HORZDIST  : pResId = SIP_SA_EDGENODE2HORZDIST;break;
        case SDRATTR_EDGENODE2VERTDIST  : pResId = SIP_SA_EDGENODE2VERTDIST;break;
        case SDRATTR_EDGENODE1GLUEDIST  : pResId = SIP_SA_EDGENODE1GLUEDIST;break;
        case SDRATTR_EDGENODE2GLUEDIST  : pResId = SIP_SA_EDGENODE2GLUEDIST;break;
        case SDRATTR_EDGELINEDELTACOUNT : pResId = SIP_SA_EDGELINEDELTACOUNT;break;
        case SDRATTR_EDGELINE1DELTA     : pResId = SIP_SA_EDGELINE1DELTA;break;
        case SDRATTR_EDGELINE2DELTA     : pResId = SIP_SA_EDGELINE2DELTA;break;
        case SDRATTR_EDGELINE3DELTA     : pResId = SIP_SA_EDGELINE3DELTA;break;

        case SDRATTR_MEASUREKIND             : pResId = SIP_SA_MEASUREKIND;break;
        case SDRATTR_MEASURETEXTHPOS         : pResId = SIP_SA_MEASURETEXTHPOS;break;
        case SDRATTR_MEASURETEXTVPOS         : pResId = SIP_SA_MEASURETEXTVPOS;break;
        case SDRATTR_MEASURELINEDIST         : pResId = SIP_SA_MEASURELINEDIST;break;
        case SDRATTR_MEASUREHELPLINEOVERHANG : pResId = SIP_SA_MEASUREHELPLINEOVERHANG;break;
        case SDRATTR_MEASUREHELPLINEDIST     : pResId = SIP_SA_MEASUREHELPLINEDIST;break;
        case SDRATTR_MEASUREHELPLINE1LEN     : pResId = SIP_SA_MEASUREHELPLINE1LEN;break;
        case SDRATTR_MEASUREHELPLINE2LEN     : pResId = SIP_SA_MEASUREHELPLINE2LEN;break;
        case SDRATTR_MEASUREBELOWREFEDGE     : pResId = SIP_SA_MEASUREBELOWREFEDGE;break;
        case SDRATTR_MEASURETEXTROTA90       : pResId = SIP_SA_MEASURETEXTROTA90;break;
        case SDRATTR_MEASURETEXTUPSIDEDOWN   : pResId = SIP_SA_MEASURETEXTUPSIDEDOWN;break;
        case SDRATTR_MEASUREOVERHANG         : pResId = SIP_SA_MEASUREOVERHANG;break;
        case SDRATTR_MEASUREUNIT             : pResId = SIP_SA_MEASUREUNIT;break;
        case SDRATTR_MEASURESCALE            : pResId = SIP_SA_MEASURESCALE;break;
        case SDRATTR_MEASURESHOWUNIT         : pResId = SIP_SA_MEASURESHOWUNIT;break;
        case SDRATTR_MEASUREFORMATSTRING     : pResId = SIP_SA_MEASUREFORMATSTRING;break;
        case SDRATTR_MEASURETEXTAUTOANGLE    : pResId = SIP_SA_MEASURETEXTAUTOANGLE;break;
        case SDRATTR_MEASURETEXTAUTOANGLEVIEW: pResId = SIP_SA_MEASURETEXTAUTOANGLEVIEW;break;
        case SDRATTR_MEASURETEXTISFIXEDANGLE : pResId = SIP_SA_MEASURETEXTISFIXEDANGLE;break;
        case SDRATTR_MEASURETEXTFIXEDANGLE   : pResId = SIP_SA_MEASURETEXTFIXEDANGLE;break;
        case SDRATTR_MEASUREDECIMALPLACES    : pResId = SIP_SA_MEASUREDECIMALPLACES;break;

        case SDRATTR_CIRCKIND      : pResId = SIP_SA_CIRCKIND;break;
        case SDRATTR_CIRCSTARTANGLE: pResId = SIP_SA_CIRCSTARTANGLE;break;
        case SDRATTR_CIRCENDANGLE  : pResId = SIP_SA_CIRCENDANGLE;break;

        case SDRATTR_OBJMOVEPROTECT : pResId = SIP_SA_OBJMOVEPROTECT;break;
        case SDRATTR_OBJSIZEPROTECT : pResId = SIP_SA_OBJSIZEPROTECT;break;
        case SDRATTR_OBJPRINTABLE   : pResId = SIP_SA_OBJPRINTABLE;break;
        case SDRATTR_OBJVISIBLE     : pResId = SIP_SA_OBJVISIBLE;break;
        case SDRATTR_LAYERID        : pResId = SIP_SA_LAYERID;break;
        case SDRATTR_LAYERNAME      : pResId = SIP_SA_LAYERNAME;break;
        case SDRATTR_OBJECTNAME     : pResId = SIP_SA_OBJECTNAME;break;
        case SDRATTR_ALLPOSITIONX   : pResId = SIP_SA_ALLPOSITIONX;break;
        case SDRATTR_ALLPOSITIONY   : pResId = SIP_SA_ALLPOSITIONY;break;
        case SDRATTR_ALLSIZEWIDTH   : pResId = SIP_SA_ALLSIZEWIDTH;break;
        case SDRATTR_ALLSIZEHEIGHT  : pResId = SIP_SA_ALLSIZEHEIGHT;break;
        case SDRATTR_ONEPOSITIONX   : pResId = SIP_SA_ONEPOSITIONX;break;
        case SDRATTR_ONEPOSITIONY   : pResId = SIP_SA_ONEPOSITIONY;break;
        case SDRATTR_ONESIZEWIDTH   : pResId = SIP_SA_ONESIZEWIDTH;break;
        case SDRATTR_ONESIZEHEIGHT  : pResId = SIP_SA_ONESIZEHEIGHT;break;
        case SDRATTR_LOGICSIZEWIDTH : pResId = SIP_SA_LOGICSIZEWIDTH;break;
        case SDRATTR_LOGICSIZEHEIGHT: pResId = SIP_SA_LOGICSIZEHEIGHT;break;
        case SDRATTR_ROTATEANGLE    : pResId = SIP_SA_ROTATEANGLE;break;
        case SDRATTR_SHEARANGLE     : pResId = SIP_SA_SHEARANGLE;break;
        case SDRATTR_MOVEX          : pResId = SIP_SA_MOVEX;break;
        case SDRATTR_MOVEY          : pResId = SIP_SA_MOVEY;break;
        case SDRATTR_RESIZEXONE     : pResId = SIP_SA_RESIZEXONE;break;
        case SDRATTR_RESIZEYONE     : pResId = SIP_SA_RESIZEYONE;break;
        case SDRATTR_ROTATEONE      : pResId = SIP_SA_ROTATEONE;break;
        case SDRATTR_HORZSHEARONE   : pResId = SIP_SA_HORZSHEARONE;break;
        case SDRATTR_VERTSHEARONE   : pResId = SIP_SA_VERTSHEARONE;break;
        case SDRATTR_RESIZEXALL     : pResId = SIP_SA_RESIZEXALL;break;
        case SDRATTR_RESIZEYALL     : pResId = SIP_SA_RESIZEYALL;break;
        case SDRATTR_ROTATEALL      : pResId = SIP_SA_ROTATEALL;break;
        case SDRATTR_HORZSHEARALL   : pResId = SIP_SA_HORZSHEARALL;break;
        case SDRATTR_VERTSHEARALL   : pResId = SIP_SA_VERTSHEARALL;break;
        case SDRATTR_TRANSFORMREF1X : pResId = SIP_SA_TRANSFORMREF1X;break;
        case SDRATTR_TRANSFORMREF1Y : pResId = SIP_SA_TRANSFORMREF1Y;break;
        case SDRATTR_TRANSFORMREF2X : pResId = SIP_SA_TRANSFORMREF2X;break;
        case SDRATTR_TRANSFORMREF2Y : pResId = SIP_SA_TRANSFORMREF2Y;break;

        case SDRATTR_GRAFRED            : pResId = SIP_SA_GRAFRED;break;
        case SDRATTR_GRAFGREEN          : pResId = SIP_SA_GRAFGREEN;break;
        case SDRATTR_GRAFBLUE           : pResId = SIP_SA_GRAFBLUE;break;
        case SDRATTR_GRAFLUMINANCE      : pResId = SIP_SA_GRAFLUMINANCE;break;
        case SDRATTR_GRAFCONTRAST       : pResId = SIP_SA_GRAFCONTRAST;break;
        case SDRATTR_GRAFGAMMA          : pResId = SIP_SA_GRAFGAMMA;break;
        case SDRATTR_GRAFTRANSPARENCE   : pResId = SIP_SA_GRAFTRANSPARENCE;break;
        case SDRATTR_GRAFINVERT         : pResId = SIP_SA_GRAFINVERT;break;
        case SDRATTR_GRAFMODE           : pResId = SIP_SA_GRAFMODE;break;
        case SDRATTR_GRAFCROP           : pResId = SIP_SA_GRAFCROP;break;

        case EE_PARA_HYPHENATE  : pResId = SIP_EE_PARA_HYPHENATE;break;
        case EE_PARA_BULLETSTATE: pResId = SIP_EE_PARA_BULLETSTATE;break;
        case EE_PARA_OUTLLRSPACE: pResId = SIP_EE_PARA_OUTLLRSPACE;break;
        case EE_PARA_OUTLLEVEL  : pResId = SIP_EE_PARA_OUTLLEVEL;break;
        case EE_PARA_BULLET     : pResId = SIP_EE_PARA_BULLET;break;
        case EE_PARA_LRSPACE    : pResId = SIP_EE_PARA_LRSPACE;break;
        case EE_PARA_ULSPACE    : pResId = SIP_EE_PARA_ULSPACE;break;
        case EE_PARA_SBL        : pResId = SIP_EE_PARA_SBL;break;
        case EE_PARA_JUST       : pResId = SIP_EE_PARA_JUST;break;
        case EE_PARA_TABS       : pResId = SIP_EE_PARA_TABS;break;

        case EE_CHAR_COLOR      : pResId = SIP_EE_CHAR_COLOR;break;
        case EE_CHAR_FONTINFO   : pResId = SIP_EE_CHAR_FONTINFO;break;
        case EE_CHAR_FONTHEIGHT : pResId = SIP_EE_CHAR_FONTHEIGHT;break;
        case EE_CHAR_FONTWIDTH  : pResId = SIP_EE_CHAR_FONTWIDTH;break;
        case EE_CHAR_WEIGHT     : pResId = SIP_EE_CHAR_WEIGHT;break;
        case EE_CHAR_UNDERLINE  : pResId = SIP_EE_CHAR_UNDERLINE;break;
        case EE_CHAR_OVERLINE   : pResId = SIP_EE_CHAR_OVERLINE;break;
        case EE_CHAR_STRIKEOUT  : pResId = SIP_EE_CHAR_STRIKEOUT;break;
        case EE_CHAR_ITALIC     : pResId = SIP_EE_CHAR_ITALIC;break;
        case EE_CHAR_OUTLINE    : pResId = SIP_EE_CHAR_OUTLINE;break;
        case EE_CHAR_SHADOW     : pResId = SIP_EE_CHAR_SHADOW;break;
        case EE_CHAR_ESCAPEMENT : pResId = SIP_EE_CHAR_ESCAPEMENT;break;
        case EE_CHAR_PAIRKERNING: pResId = SIP_EE_CHAR_PAIRKERNING;break;
        case EE_CHAR_KERNING    : pResId = SIP_EE_CHAR_KERNING;break;
        case EE_CHAR_WLM        : pResId = SIP_EE_CHAR_WLM;break;
        case EE_FEATURE_TAB     : pResId = SIP_EE_FEATURE_TAB;break;
        case EE_FEATURE_LINEBR  : pResId = SIP_EE_FEATURE_LINEBR;break;
        case EE_FEATURE_NOTCONV : pResId = SIP_EE_FEATURE_NOTCONV;break;
        case EE_FEATURE_FIELD   : pResId = SIP_EE_FEATURE_FIELD;break;

        case SDRATTR_TEXTCOLUMNS_NUMBER: pResId = SIP_SA_TEXTCOLUMNS_NUMBER; break;
        case SDRATTR_TEXTCOLUMNS_SPACING: pResId = SIP_SA_TEXTCOLUMNS_SPACING; break;
    } // switch

    return SvxResId(pResId);
}


// FractionItem


bool SdrFractionItem::operator==(const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
           static_cast<const SdrFractionItem&>(rCmp).GetValue()==nValue;
}

bool SdrFractionItem::GetPresentation(
    SfxItemPresentation ePresentation, MapUnit /*eCoreMetric*/,
    MapUnit /*ePresentationMetric*/, OUString &rText, const IntlWrapper&) const
{
    if(nValue.IsValid())
    {
        sal_Int32 nDiv = nValue.GetDenominator();
        rText = OUString::number(nValue.GetNumerator());

        if(nDiv != 1)
        {
            rText += "/" + OUString::number(nDiv);
        }
    }
    else
    {
        rText = "?";
    }

    if(ePresentation == SfxItemPresentation::Complete)
    {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
        return true;
    }
    else if(ePresentation == SfxItemPresentation::Nameless)
        return true;

    return false;
}

SdrFractionItem* SdrFractionItem::Clone(SfxItemPool * /*pPool*/) const
{
    return new SdrFractionItem(Which(),GetValue());
}


// ScaleItem


bool SdrScaleItem::GetPresentation(
    SfxItemPresentation ePresentation, MapUnit /*eCoreMetric*/,
    MapUnit /*ePresentationMetric*/, OUString &rText, const IntlWrapper&) const
{
    if(GetValue().IsValid())
    {
        sal_Int32 nDiv = GetValue().GetDenominator();

        rText = OUString::number(GetValue().GetNumerator()) + ":" + OUString::number(nDiv);
    }
    else
    {
        rText = "?";
    }

    if(ePresentation == SfxItemPresentation::Complete)
    {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }

    return true;
}

SdrScaleItem* SdrScaleItem::Clone(SfxItemPool * /*pPool*/) const
{
    return new SdrScaleItem(Which(),GetValue());
}


// OnOffItem


SdrOnOffItem* SdrOnOffItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrOnOffItem(TypedWhichId<SdrOnOffItem>(Which()),GetValue());
}

OUString SdrOnOffItem::GetValueTextByVal(bool bVal) const
{
    if (bVal)
        return SvxResId(STR_ItemValON);
    return SvxResId(STR_ItemValOFF);
}

bool SdrOnOffItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByVal(GetValue());
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

void SdrOnOffItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrOnOffItem"));
    if (Which() == SDRATTR_SHADOW)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST("SDRATTR_SHADOW"));
    }

    SfxBoolItem::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

SdrYesNoItem* SdrYesNoItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrYesNoItem(TypedWhichId<SdrYesNoItem>(Which()),GetValue());
}

OUString SdrYesNoItem::GetValueTextByVal(bool bVal) const
{
    if (bVal)
        return SvxResId(STR_ItemValYES);
    return SvxResId(STR_ItemValNO);
}

bool SdrYesNoItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByVal(GetValue());
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

SdrPercentItem* SdrPercentItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrPercentItem(TypedWhichId<SdrPercentItem>(Which()),GetValue());
}

bool SdrPercentItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreMetric*/,
    MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText = unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());

    if(ePres == SfxItemPresentation::Complete)
    {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }

    return true;
}

void SdrPercentItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrPercentItem"));
    if (Which() == SDRATTR_SHADOWTRANSPARENCE)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"),
                                    BAD_CAST("SDRATTR_SHADOWTRANSPARENCE"));
    }

    SfxUInt16Item::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

SdrAngleItem* SdrAngleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrAngleItem(TypedWhichId<SdrAngleItem>(Which()),GetValue());
}

bool SdrAngleItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper& rIntlWrapper) const
{
    sal_Int32 nValue(GetValue());
    bool bNeg(nValue < 0);

    if(bNeg)
        nValue = -nValue;

    OUStringBuffer aText = OUString::number(nValue);

    if(nValue)
    {
        sal_Unicode aUnicodeNull('0');
        sal_Int32 nCount(2);

        if(LocaleDataWrapper::isNumLeadingZero())
            nCount++;

        while(aText.getLength() < nCount)
            aText.insert(0, aUnicodeNull);

        sal_Int32 nLen = aText.getLength();
        bool bNull1(aText[nLen-1] == aUnicodeNull);
        bool bNull2(bNull1 && aText[nLen-2] == aUnicodeNull);

        if(bNull2)
        {
            // no decimal place(s)
            sal_Int32 idx = nLen-2;
            aText.remove(idx, aText.getLength()-idx);
        }
        else
        {
            sal_Unicode cDec =
                rIntlWrapper.getLocaleData()->getNumDecimalSep()[0];
            aText.insert(nLen-2, cDec);

            if(bNull1)
                aText.remove(nLen, aText.getLength()-nLen);
        }

        if(bNeg)
            aText.insert(0, '-');
    }

    aText.append(sal_Unicode(DEGREE_CHAR));

    if(ePres == SfxItemPresentation::Complete)
    {
        OUString aStr = SdrItemPool::GetItemName(Which());
        aText.insert(0, aStr + " ");
    }

    rText = aText.makeStringAndClear();
    return true;
}

SdrMetricItem* SdrMetricItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrMetricItem(TypedWhichId<SdrMetricItem>(Which()),GetValue());
}

bool SdrMetricItem::HasMetrics() const
{
    return true;
}

void SdrMetricItem::ScaleMetrics(tools::Long nMul, tools::Long nDiv)
{
    if (GetValue()!=0) {
        SetValue(BigInt::Scale(GetValue(), nMul, nDiv));
    }
}

bool SdrMetricItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const
{
    tools::Long nValue=GetValue();
    SdrFormatter aFmt(eCoreMetric,ePresMetric);
    rText = aFmt.GetStr(nValue);
    rText += " " + SdrFormatter::GetUnitStr(ePresMetric);
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

void SdrMetricItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrMetricItem"));
    if (Which() == SDRATTR_SHADOWXDIST)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST("SDRATTR_SHADOWXDIST"));
    }
    else if (Which() == SDRATTR_SHADOWYDIST)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST("SDRATTR_SHADOWYDIST"));
    }
    else if (Which() == SDRATTR_SHADOWSIZEX)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST("SDRATTR_SHADOWSIZEX"));
    }
    else if (Which() == SDRATTR_SHADOWSIZEY)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST("SDRATTR_SHADOWSIZEY"));
    }
    else if (Which() == SDRATTR_SHADOWBLUR)
    {
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST("SDRATTR_SHADOWBLUR"));
    }

    SfxInt32Item::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

// items of the legend object


SdrCaptionTypeItem* SdrCaptionTypeItem::Clone(SfxItemPool* /*pPool*/) const                { return new SdrCaptionTypeItem(*this); }

sal_uInt16 SdrCaptionTypeItem::GetValueCount() const { return 4; }

OUString SdrCaptionTypeItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALCAPTIONTYPES[] =
    {
        STR_ItemValCAPTIONTYPE1,
        STR_ItemValCAPTIONTYPE2,
        STR_ItemValCAPTIONTYPE3,
        STR_ItemValCAPTIONTYPE4
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALCAPTIONTYPES) && "wrong pos!");
    return SvxResId(ITEMVALCAPTIONTYPES[nPos]);
}

bool SdrCaptionTypeItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}


SdrCaptionEscDirItem* SdrCaptionEscDirItem::Clone(SfxItemPool* /*pPool*/) const              { return new SdrCaptionEscDirItem(*this); }

sal_uInt16 SdrCaptionEscDirItem::GetValueCount() const { return 3; }

OUString SdrCaptionEscDirItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALCAPTIONTYPES[] =
    {
        STR_ItemValCAPTIONESCHORI,
        STR_ItemValCAPTIONESCVERT,
        STR_ItemValCAPTIONESCBESTFIT
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALCAPTIONTYPES) && "wrong pos!");
    return SvxResId(ITEMVALCAPTIONTYPES[nPos]);
}

bool SdrCaptionEscDirItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}


// MiscItems


// FitToSize

SfxPoolItem* SdrTextFitToSizeTypeItem::CreateDefault() { return new SdrTextFitToSizeTypeItem; }

SdrTextFitToSizeTypeItem* SdrTextFitToSizeTypeItem::Clone(SfxItemPool* /*pPool*/) const         { return new SdrTextFitToSizeTypeItem(*this); }

bool SdrTextFitToSizeTypeItem::operator==(const SfxPoolItem& rItem) const
{
    if (!SfxEnumItem<css::drawing::TextFitToSizeType>::operator==(rItem))
    {
        return false;
    }
    auto& rTextFitToSizeTypeItem = static_cast<const SdrTextFitToSizeTypeItem&>(rItem);
    return mfFontScale == rTextFitToSizeTypeItem.mfFontScale
        && mfSpacingScale == rTextFitToSizeTypeItem.mfSpacingScale;
}

sal_uInt16 SdrTextFitToSizeTypeItem::GetValueCount() const { return 4; }

OUString SdrTextFitToSizeTypeItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALFITTISIZETYPES[] =
    {
        STR_ItemValFITTOSIZENONE,
        STR_ItemValFITTOSIZEPROP,
        STR_ItemValFITTOSIZEALLLINES,
        STR_ItemValFITTOSIZERESIZEAT
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALFITTISIZETYPES) && "wrong pos!");
    return SvxResId(ITEMVALFITTISIZETYPES[nPos]);
}

bool SdrTextFitToSizeTypeItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrTextFitToSizeTypeItem::HasBoolValue() const { return true; }

bool SdrTextFitToSizeTypeItem::GetBoolValue() const { return GetValue() != drawing::TextFitToSizeType_NONE; }

void SdrTextFitToSizeTypeItem::SetBoolValue(bool bVal)
{
    SetValue(bVal ? drawing::TextFitToSizeType_PROPORTIONAL : drawing::TextFitToSizeType_NONE);
}

bool SdrTextFitToSizeTypeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    drawing::TextFitToSizeType eFS = GetValue();
    rVal <<= eFS;

    return true;
}

bool SdrTextFitToSizeTypeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextFitToSizeType eFS;
    if(!(rVal >>= eFS))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eFS = static_cast<drawing::TextFitToSizeType>(nEnum);
    }

    SetValue(eFS);

    return true;
}


SdrTextVertAdjustItem* SdrTextVertAdjustItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextVertAdjustItem(*this); }

sal_uInt16 SdrTextVertAdjustItem::GetValueCount() const { return 5; }

OUString SdrTextVertAdjustItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALTEXTVADJTYPES[] =
    {
        STR_ItemValTEXTVADJTOP,
        STR_ItemValTEXTVADJCENTER,
        STR_ItemValTEXTVADJBOTTOM,
        STR_ItemValTEXTVADJBLOCK,
        STR_ItemValTEXTVADJSTRETCH
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALTEXTVADJTYPES) && "wrong pos!");
    return SvxResId(ITEMVALTEXTVADJTYPES[nPos]);
}

bool SdrTextVertAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrTextVertAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextVerticalAdjust>(GetValue());
    return true;
}

bool SdrTextVertAdjustItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextVerticalAdjust eAdj;
    if(!(rVal >>= eAdj))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eAdj = static_cast<drawing::TextVerticalAdjust>(nEnum);
    }

    SetValue( static_cast<SdrTextVertAdjust>(eAdj) );

    return true;
}

void SdrTextVertAdjustItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrTextVertAdjustItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

SdrTextHorzAdjustItem* SdrTextHorzAdjustItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextHorzAdjustItem(*this); }

sal_uInt16 SdrTextHorzAdjustItem::GetValueCount() const { return 5; }

OUString SdrTextHorzAdjustItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALTEXTHADJTYPES[] =
    {
        STR_ItemValTEXTHADJLEFT,
        STR_ItemValTEXTHADJCENTER,
        STR_ItemValTEXTHADJRIGHT,
        STR_ItemValTEXTHADJBLOCK,
        STR_ItemValTEXTHADJSTRETCH
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALTEXTHADJTYPES) && "wrong pos!");
    return SvxResId(ITEMVALTEXTHADJTYPES[nPos]);
}

bool SdrTextHorzAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrTextHorzAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextHorizontalAdjust>(GetValue());
    return true;
}

bool SdrTextHorzAdjustItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextHorizontalAdjust eAdj;
    if(!(rVal >>= eAdj))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eAdj = static_cast<drawing::TextHorizontalAdjust>(nEnum);
    }

    SetValue( static_cast<SdrTextHorzAdjust>(eAdj) );

    return true;
}


SdrTextAniKindItem* SdrTextAniKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniKindItem(*this); }

sal_uInt16 SdrTextAniKindItem::GetValueCount() const { return 5; }

OUString SdrTextAniKindItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALTEXTANITYPES[] =
    {
        STR_ItemValTEXTANI_NONE,
        STR_ItemValTEXTANI_BLINK,
        STR_ItemValTEXTANI_SCROLL,
        STR_ItemValTEXTANI_ALTERNATE,
        STR_ItemValTEXTANI_SLIDE
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALTEXTANITYPES) && "wrong pos!");
    return SvxResId(ITEMVALTEXTANITYPES[nPos]);
}

bool SdrTextAniKindItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrTextAniKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextAnimationKind>(GetValue());
    return true;
}

bool SdrTextAniKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextAnimationKind eKind;
    if(!(rVal >>= eKind))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;
        eKind = static_cast<drawing::TextAnimationKind>(nEnum);
    }

    SetValue( static_cast<SdrTextAniKind>(eKind) );

    return true;
}


SdrTextAniDirectionItem* SdrTextAniDirectionItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniDirectionItem(*this); }

sal_uInt16 SdrTextAniDirectionItem::GetValueCount() const { return 4; }

OUString SdrTextAniDirectionItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALTEXTANITYPES[] =
    {
        STR_ItemValTEXTANI_LEFT,
        STR_ItemValTEXTANI_UP,
        STR_ItemValTEXTANI_RIGHT,
        STR_ItemValTEXTANI_DOWN
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALTEXTANITYPES) && "wrong pos!");
    return SvxResId(ITEMVALTEXTANITYPES[nPos]);
}

bool SdrTextAniDirectionItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrTextAniDirectionItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextAnimationDirection>(GetValue());
    return true;
}

bool SdrTextAniDirectionItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextAnimationDirection eDir;
    if(!(rVal >>= eDir))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eDir = static_cast<drawing::TextAnimationDirection>(nEnum);
    }

    SetValue( static_cast<SdrTextAniDirection>(eDir) );

    return true;
}


SdrTextAniDelayItem* SdrTextAniDelayItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniDelayItem(*this); }

bool SdrTextAniDelayItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper&) const
{
    rText = OUString::number(GetValue()) + "ms";

    if(ePres == SfxItemPresentation::Complete)
    {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }

    return true;
}


SdrTextAniAmountItem* SdrTextAniAmountItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniAmountItem(*this); }

bool SdrTextAniAmountItem::HasMetrics() const
{
    return GetValue()>0;
}

void SdrTextAniAmountItem::ScaleMetrics(tools::Long nMul, tools::Long nDiv)
{
    if (GetValue()>0) {
        BigInt aVal(GetValue());
        aVal*=nMul;
        aVal+=nDiv/2; // to round accurately
        aVal/=nDiv;
        SetValue(short(aVal));
    }
}

bool SdrTextAniAmountItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric,
    OUString& rText, const IntlWrapper&) const
{
    sal_Int32 nValue(GetValue());

    if(!nValue)
        nValue = -1;

    if(nValue < 0)
    {
        rText = OUString::number(-nValue) + "pixel";
    }
    else
    {
        SdrFormatter aFmt(eCoreMetric, ePresMetric);
        rText = aFmt.GetStr(nValue) +
            SdrFormatter::GetUnitStr(ePresMetric);
    }

    if(ePres == SfxItemPresentation::Complete)
    {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }

    return true;
}


SdrTextFixedCellHeightItem::SdrTextFixedCellHeightItem( bool bUseFixedCellHeight )
    : SfxBoolItem( SDRATTR_TEXT_USEFIXEDCELLHEIGHT, bUseFixedCellHeight )
{
}
bool SdrTextFixedCellHeightItem::GetPresentation( SfxItemPresentation ePres,
                                    MapUnit /*eCoreMetric*/, MapUnit /*ePresentationMetric*/,
                                    OUString &rText, const IntlWrapper& ) const
{
    rText = GetValueTextByVal( GetValue() );
    if (ePres==SfxItemPresentation::Complete)
    {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

SdrTextFixedCellHeightItem* SdrTextFixedCellHeightItem::Clone( SfxItemPool * /*pPool*/) const
{
    return new SdrTextFixedCellHeightItem( GetValue() );
}

bool SdrTextFixedCellHeightItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    bool bValue = GetValue();
    rVal <<= bValue;
    return true;
}
bool SdrTextFixedCellHeightItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    bool bValue;
    if( !( rVal >>= bValue ) )
        return false;
    SetValue( bValue );
    return true;
}

// EdgeKind

SdrEdgeKindItem* SdrEdgeKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrEdgeKindItem(*this); }

sal_uInt16 SdrEdgeKindItem::GetValueCount() const { return 4; }

OUString SdrEdgeKindItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALEDGES[] =
    {
        STR_ItemValEDGE_ORTHOLINES,
        STR_ItemValEDGE_THREELINES,
        STR_ItemValEDGE_ONELINE,
        STR_ItemValEDGE_BEZIER
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALEDGES) && "wrong pos!");
    return SvxResId(ITEMVALEDGES[nPos]);
}

bool SdrEdgeKindItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrEdgeKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    drawing::ConnectorType eCT = drawing::ConnectorType_STANDARD;

    switch( GetValue() )
    {
        case SdrEdgeKind::OrthoLines :   eCT = drawing::ConnectorType_STANDARD; break;
        case SdrEdgeKind::ThreeLines :   eCT = drawing::ConnectorType_LINES; break;
        case SdrEdgeKind::OneLine :      eCT = drawing::ConnectorType_LINE;  break;
        case SdrEdgeKind::Bezier :       eCT = drawing::ConnectorType_CURVE; break;
        case SdrEdgeKind::Arc :          eCT = drawing::ConnectorType_CURVE; break;
        default:
            OSL_FAIL( "SdrEdgeKindItem::QueryValue : unknown enum" );
    }

    rVal <<= eCT;

    return true;
}

bool SdrEdgeKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::ConnectorType eCT;
    if(!(rVal >>= eCT))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eCT = static_cast<drawing::ConnectorType>(nEnum);
    }

    SdrEdgeKind eEK = SdrEdgeKind::OrthoLines;
    switch( eCT )
    {
        case drawing::ConnectorType_STANDARD :  eEK = SdrEdgeKind::OrthoLines;   break;
        case drawing::ConnectorType_CURVE :     eEK = SdrEdgeKind::Bezier;       break;
        case drawing::ConnectorType_LINE :       eEK = SdrEdgeKind::OneLine;     break;
        case drawing::ConnectorType_LINES :      eEK = SdrEdgeKind::ThreeLines;   break;
        default:
            OSL_FAIL( "SdrEdgeKindItem::PuValue : unknown enum" );
    }
    SetValue( eEK );

    return true;
}

bool SdrEdgeNode1HorzDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
    return true;
}

bool SdrEdgeNode1HorzDistItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

SdrEdgeNode1HorzDistItem* SdrEdgeNode1HorzDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode1HorzDistItem(*this);
}

bool SdrEdgeNode1VertDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
    return true;
}

bool SdrEdgeNode1VertDistItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

SdrEdgeNode1VertDistItem* SdrEdgeNode1VertDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode1VertDistItem(*this);
}

bool SdrEdgeNode2HorzDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
    return true;
}

bool SdrEdgeNode2HorzDistItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

SdrEdgeNode2HorzDistItem* SdrEdgeNode2HorzDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode2HorzDistItem(*this);
}

bool SdrEdgeNode2VertDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
    return true;
}

bool SdrEdgeNode2VertDistItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

SdrEdgeNode2VertDistItem* SdrEdgeNode2VertDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode2VertDistItem(*this);
}

SdrEdgeNode1GlueDistItem* SdrEdgeNode1GlueDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode1GlueDistItem(*this);
}

SdrEdgeNode2GlueDistItem* SdrEdgeNode2GlueDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode2GlueDistItem(*this);
}

SdrMeasureKindItem* SdrMeasureKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureKindItem(*this); }

sal_uInt16 SdrMeasureKindItem::GetValueCount() const { return 2; }

OUString SdrMeasureKindItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALMEASURETYPES[] =
    {
        STR_ItemValMEASURE_STD,
        STR_ItemValMEASURE_RADIUS
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALMEASURETYPES) && "wrong pos!");
    return SvxResId(ITEMVALMEASURETYPES[nPos]);
}

bool SdrMeasureKindItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrMeasureKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::MeasureKind>(GetValue());
    return true;
}

bool SdrMeasureKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::MeasureKind eKind;
    if(!(rVal >>= eKind))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eKind = static_cast<drawing::MeasureKind>(nEnum);
    }

    SetValue( static_cast<SdrMeasureKind>(eKind) );
    return true;
}


SdrMeasureTextHPosItem* SdrMeasureTextHPosItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureTextHPosItem(*this); }

sal_uInt16 SdrMeasureTextHPosItem::GetValueCount() const { return 4; }

const OUString & SdrMeasureTextHPosItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static std::array<OUString, 4> aMeasureTextHPosItem
    {
        "automatic",
        "left outside",
        "inside (centered)",
        "right outside"
    };
    assert(nPos < aMeasureTextHPosItem.size() && "wrong pos!");
    return aMeasureTextHPosItem[nPos];
}

bool SdrMeasureTextHPosItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrMeasureTextHPosItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
    return true;
}

bool SdrMeasureTextHPosItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::MeasureTextHorzPos ePos;
    if(!(rVal >>= ePos))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        ePos = static_cast<drawing::MeasureTextHorzPos>(nEnum);
    }

    SetValue(ePos);
    return true;
}

SdrMeasureTextVPosItem* SdrMeasureTextVPosItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureTextVPosItem(*this); }

sal_uInt16 SdrMeasureTextVPosItem::GetValueCount() const { return 5; }

OUString SdrMeasureTextVPosItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALMEASURETEXTTYPES[] =
    {
        STR_ItemValMEASURE_TEXTVAUTO,
        STR_ItemValMEASURE_ABOVE,
        STR_ItemValMEASURETEXT_BREAKEDLINE,
        STR_ItemValMEASURE_BELOW,
        STR_ItemValMEASURETEXT_VERTICALCEN
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALMEASURETEXTTYPES) && "wrong pos!");
    return SvxResId(ITEMVALMEASURETEXTTYPES[nPos]);
}

bool SdrMeasureTextVPosItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrMeasureTextVPosItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
    return true;
}

bool SdrMeasureTextVPosItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::MeasureTextVertPos ePos;
    if(!(rVal >>= ePos))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        ePos = static_cast<drawing::MeasureTextVertPos>(nEnum);
    }

    SetValue(ePos);
    return true;
}

SdrMeasureUnitItem* SdrMeasureUnitItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureUnitItem(*this); }

sal_uInt16 SdrMeasureUnitItem::GetValueCount() const { return 14; }

OUString SdrMeasureUnitItem::GetValueTextByPos(sal_uInt16 nPos)
{
    if(static_cast<FieldUnit>(nPos) == FieldUnit::NONE)
        return "default";
    else
        return SdrFormatter::GetUnitStr(static_cast<FieldUnit>(nPos));
}

bool SdrMeasureUnitItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrMeasureUnitItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<sal_Int32>(GetValue());
    return true;
}

bool SdrMeasureUnitItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nMeasure = 0;
    if(!(rVal >>= nMeasure))
        return false;

    SetValue( static_cast<FieldUnit>(nMeasure) );
    return true;
}


SdrCircKindItem* SdrCircKindItem::Clone(SfxItemPool* /*pPool*/) const          { return new SdrCircKindItem(*this); }

sal_uInt16 SdrCircKindItem::GetValueCount() const { return 4; }

OUString SdrCircKindItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static TranslateId ITEMVALCIRCTYPES[] =
    {
        STR_ItemValCIRC_FULL,
        STR_ItemValCIRC_SECT,
        STR_ItemValCIRC_CUT,
        STR_ItemValCIRC_ARC
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALCIRCTYPES) && "wrong pos!");
    return SvxResId(ITEMVALCIRCTYPES[nPos]);
}

bool SdrCircKindItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }
    return true;
}

bool SdrCircKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::CircleKind>(GetValue());
    return true;
}

bool SdrCircKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::CircleKind eKind;
    if(!(rVal >>= eKind))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eKind = static_cast<drawing::CircleKind>(nEnum);
    }

    SetValue( static_cast<SdrCircKind>(eKind) );
    return true;
}

SdrSignedPercentItem* SdrSignedPercentItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrSignedPercentItem( Which(), GetValue() );
}

bool SdrSignedPercentItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper&) const
{
    rText = unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());

    if(ePres == SfxItemPresentation::Complete)
    {
        rText = SdrItemPool::GetItemName(Which()) + " " + rText;
    }

    return true;
}

SdrGrafRedItem* SdrGrafRedItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafRedItem( *this );
}

SdrGrafGreenItem* SdrGrafGreenItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafGreenItem( *this );
}

SdrGrafBlueItem* SdrGrafBlueItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafBlueItem( *this );
}

SdrGrafLuminanceItem* SdrGrafLuminanceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafLuminanceItem( *this );
}

SdrGrafContrastItem* SdrGrafContrastItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafContrastItem( *this );
}

SdrGrafGamma100Item* SdrGrafGamma100Item::Clone( SfxItemPool* /*pPool */) const
{
    return new SdrGrafGamma100Item( *this );
}

bool SdrGrafGamma100Item::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<double>(GetValue()) / 100.0;
    return true;
}

bool SdrGrafGamma100Item::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    double nGamma = 0;
    if(!(rVal >>= nGamma))
        return false;

    SetValue( static_cast<sal_uInt32>(nGamma * 100.0  ) );
    return true;
}

SdrGrafInvertItem* SdrGrafInvertItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafInvertItem( *this );
}

SdrGrafTransparenceItem* SdrGrafTransparenceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafTransparenceItem( *this );
}

SdrGrafModeItem* SdrGrafModeItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrGrafModeItem( *this );
}

sal_uInt16 SdrGrafModeItem::GetValueCount() const
{
    return 4;
}

OUString SdrGrafModeItem::GetValueTextByPos(sal_uInt16 nPos)
{
    OUString aStr;

    switch(nPos)
    {
        case 1:
        {
            aStr = "Greys";
            break;
        }
        case 2:
        {
            aStr = "Black/White";
            break;
        }
        case 3:
        {
            aStr = "Watermark";
            break;
        }
        default:
        {
            aStr = "Standard";
            break;
        }
    }

    return aStr;
}

bool SdrGrafModeItem::GetPresentation( SfxItemPresentation ePres,
                                                               MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
                                                               OUString& rText, const IntlWrapper&) const
{
    rText = GetValueTextByPos( sal::static_int_cast< sal_uInt16 >( GetValue() ) );

    if( ePres == SfxItemPresentation::Complete )
    {
        rText = SdrItemPool::GetItemName( Which() ) + " " + rText;
    }

    return true;
}

SdrGrafCropItem* SdrGrafCropItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafCropItem( *this );
}

SdrTextAniStartInsideItem::~SdrTextAniStartInsideItem()
{
}

SdrTextAniStartInsideItem* SdrTextAniStartInsideItem::Clone(SfxItemPool* ) const
{
    return new SdrTextAniStartInsideItem(*this);
}

SdrTextAniStopInsideItem::~SdrTextAniStopInsideItem()
{
}

SdrTextAniStopInsideItem* SdrTextAniStopInsideItem::Clone(SfxItemPool* ) const
{
    return new SdrTextAniStopInsideItem(*this);
}

SdrCaptionEscIsRelItem::~SdrCaptionEscIsRelItem()
{
}

SdrCaptionEscIsRelItem* SdrCaptionEscIsRelItem::Clone(SfxItemPool* ) const
{
    return new SdrCaptionEscIsRelItem(*this);
}

SdrCaptionEscRelItem::~SdrCaptionEscRelItem()
{
}

SdrCaptionEscRelItem* SdrCaptionEscRelItem::Clone(SfxItemPool*) const
{
    return new SdrCaptionEscRelItem(*this);
}

SdrCaptionFitLineLenItem::~SdrCaptionFitLineLenItem()
{
}

SdrCaptionFitLineLenItem* SdrCaptionFitLineLenItem::Clone(SfxItemPool* ) const
{
    return new SdrCaptionFitLineLenItem(*this);
}

SdrCaptionLineLenItem::~SdrCaptionLineLenItem()
{
}

SdrCaptionLineLenItem* SdrCaptionLineLenItem::Clone(SfxItemPool*) const
{
    return new SdrCaptionLineLenItem(*this);
}

SdrMeasureBelowRefEdgeItem::~SdrMeasureBelowRefEdgeItem()
{
}

SdrMeasureBelowRefEdgeItem* SdrMeasureBelowRefEdgeItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureBelowRefEdgeItem(*this);
}

SdrMeasureTextIsFixedAngleItem::~SdrMeasureTextIsFixedAngleItem()
{
}

SdrMeasureTextIsFixedAngleItem* SdrMeasureTextIsFixedAngleItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureTextIsFixedAngleItem(*this);
}

SdrMeasureTextFixedAngleItem::~SdrMeasureTextFixedAngleItem()
{
}

SdrMeasureTextFixedAngleItem* SdrMeasureTextFixedAngleItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureTextFixedAngleItem(*this);
}

SdrMeasureDecimalPlacesItem::~SdrMeasureDecimalPlacesItem()
{
}

SdrMeasureDecimalPlacesItem* SdrMeasureDecimalPlacesItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureDecimalPlacesItem(*this);
}

SdrMeasureTextRota90Item::~SdrMeasureTextRota90Item()
{
}

SdrMeasureTextRota90Item* SdrMeasureTextRota90Item::Clone(SfxItemPool* ) const
{
    return new SdrMeasureTextRota90Item(*this);
}

SdrMeasureTextUpsideDownItem::~SdrMeasureTextUpsideDownItem()
{
}

SdrMeasureTextUpsideDownItem* SdrMeasureTextUpsideDownItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureTextUpsideDownItem(*this);
}

SdrLayerIdItem* SdrLayerIdItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrLayerIdItem(*this);
}

SdrLayerNameItem* SdrLayerNameItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrLayerNameItem(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
