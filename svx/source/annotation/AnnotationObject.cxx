/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column:100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/lok.hxx>

#include <sal/log.hxx>
#include <svx/svdmodel.hxx>
#include <svx/annotation/AnnotationObject.hxx>
#include <svx/annotation/ObjectAnnotationData.hxx>
#include <sdr/properties/rectangleproperties.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdasitm.hxx>
#include <svtools/colorcfg.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <o3tl/unit_conversion.hxx>

using namespace css;

namespace sdr::annotation
{
namespace
{
OUString createInitials(OUString const& rName)
{
    OUStringBuffer sInitials;

    const sal_Unicode* pStr = rName.getStr();
    sal_Int32 nLength = rName.getLength();

    while (nLength)
    {
        // skip whitespace
        while (nLength && (*pStr <= ' '))
        {
            nLength--;
            pStr++;
        }

        // take letter
        if (nLength)
        {
            sInitials.append(*pStr);
            nLength--;
            pStr++;
        }

        // skip letters until whitespace
        while (nLength && (*pStr > ' '))
        {
            nLength--;
            pStr++;
        }
    }

    return sInitials.makeStringAndClear();
}

Color getColorLight(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        svtools::ColorConfig aColorConfig;
        switch (aAuthorIndex % 9)
        {
            case 0:
                return aColorConfig.GetColorValue(svtools::AUTHOR1).nColor;
            case 1:
                return aColorConfig.GetColorValue(svtools::AUTHOR2).nColor;
            case 2:
                return aColorConfig.GetColorValue(svtools::AUTHOR3).nColor;
            case 3:
                return aColorConfig.GetColorValue(svtools::AUTHOR4).nColor;
            case 4:
                return aColorConfig.GetColorValue(svtools::AUTHOR5).nColor;
            case 5:
                return aColorConfig.GetColorValue(svtools::AUTHOR6).nColor;
            case 6:
                return aColorConfig.GetColorValue(svtools::AUTHOR7).nColor;
            case 7:
                return aColorConfig.GetColorValue(svtools::AUTHOR8).nColor;
            case 8:
                return aColorConfig.GetColorValue(svtools::AUTHOR9).nColor;
        }
    }

    return COL_WHITE;
}
}

AnnotationObject::AnnotationObject(SdrModel& rSdrModel)
    : SdrRectObj(rSdrModel)
{
    setup();
}

AnnotationObject::AnnotationObject(SdrModel& rSdrModel, AnnotationObject const& rSource)
    : SdrRectObj(rSdrModel, rSource)
{
    setup();
}

AnnotationObject::AnnotationObject(SdrModel& rSdrModel, tools::Rectangle const& rRectangle,
                                   sdr::annotation::AnnotationViewData const& rAnnotationViewData)
    : SdrRectObj(rSdrModel, rRectangle)
    , maViewData(rAnnotationViewData)
{
    osl_atomic_increment(&m_refCount);

    const bool bUndo(rSdrModel.IsUndoEnabled());
    rSdrModel.EnableUndo(false);
    MakeNameUnique();
    rSdrModel.EnableUndo(bUndo);

    osl_atomic_decrement(&m_refCount);
    setup();
}

void AnnotationObject::setup()
{
    setAsAnnotationObject();
    mbTextFrame = true; // need this so the frame can be adjusted to the text
    bool bLOK = comphelper::LibreOfficeKit::isActive();
    SetVisible(getSdrModelFromSdrObject().IsPDFDocument() || !bLOK);
}

void AnnotationObject::ApplyAnnotationName()
{
    if (mpAnnotationData->mxAnnotation)
    {
        OUString sInitials(mpAnnotationData->mxAnnotation->getInitials());
        if (sInitials.isEmpty())
            sInitials = createInitials(mpAnnotationData->mxAnnotation->getAuthor());
        SetText(sInitials + u" "_ustr + OUString::number(maViewData.nIndex));
    }
    else
    {
        SetText(u"🗅"_ustr);
    }

    Color aColor(getColorLight(maViewData.nAuthorIndex));

    SfxItemSet aItemSet = GetMergedItemSet();
    aItemSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
    aItemSet.Put(XFillColorItem(OUString(), aColor));

    Color aBorderColor(aColor);
    if (aColor.IsDark())
        aBorderColor.IncreaseLuminance(32);
    else
        aBorderColor.DecreaseLuminance(32);

    aItemSet.Put(XLineStyleItem(drawing::LineStyle_SOLID));
    aItemSet.Put(XLineColorItem(OUString(), aBorderColor));
    aItemSet.Put(XLineWidthItem(o3tl::convert(0, o3tl::Length::pt, o3tl::Length::mm100)));

    aItemSet.Put(SvxFontHeightItem(o3tl::convert(10, o3tl::Length::pt, o3tl::Length::mm100), 100,
                                   EE_CHAR_FONTHEIGHT));

    aItemSet.Put(SvxColorItem(aColor.IsDark() ? COL_WHITE : COL_BLACK, EE_CHAR_COLOR));

    aItemSet.Put(SdrTextFitToSizeTypeItem(drawing::TextFitToSizeType_NONE));

    aItemSet.Put(makeSdrTextWordWrapItem(false));
    aItemSet.Put(makeSdrTextAutoGrowWidthItem(true));
    aItemSet.Put(makeSdrTextAutoGrowHeightItem(true));

    SetMergedItemSet(aItemSet);

    // Update the annotation size after the auto-sizing the frame to content does its magic
    auto& xAnnotationData = getAnnotationData();
    css::geometry::RealSize2D aRealSize2D{ GetLogicRect().GetWidth() / 100.0,
                                           GetLogicRect().GetHeight() / 100.0 };
    xAnnotationData->mxAnnotation->SetSize(aRealSize2D);
}

AnnotationObject::~AnnotationObject() {}

void AnnotationObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bMoveAllowed = true;
    rInfo.bResizeFreeAllowed = false;
    rInfo.bResizePropAllowed = false;
    rInfo.bRotateFreeAllowed = false;
    rInfo.bRotate90Allowed = false;
    rInfo.bMirrorFreeAllowed = false;
    rInfo.bMirror45Allowed = false;
    rInfo.bMirror90Allowed = false;
    rInfo.bTransparenceAllowed = false;
    rInfo.bShearAllowed = false;
    rInfo.bEdgeRadiusAllowed = false;
    rInfo.bNoOrthoDesired = false;
    rInfo.bNoContortion = false;
    rInfo.bCanConvToPath = false;
    rInfo.bCanConvToPoly = false;
    rInfo.bCanConvToContour = false;
    rInfo.bCanConvToPathLineToArea = false;
    rInfo.bCanConvToPolyLineToArea = false;
}

SdrObjKind AnnotationObject::GetObjIdentifier() const { return SdrObjKind::Annotation; }

OUString AnnotationObject::TakeObjNameSingul() const
{
    OUString sOutName(u"Annotation"_ustr);
    OUString aName(GetName());

    if (!aName.isEmpty())
        sOutName += u" '"_ustr + aName + u"'"_ustr;

    return sOutName;
}

OUString AnnotationObject::TakeObjNamePlural() const { return u"Annotations"_ustr; }

rtl::Reference<SdrObject> AnnotationObject::CloneSdrObject(SdrModel& rTargetModel) const
{
    return new AnnotationObject(rTargetModel, *this);
}

bool AnnotationObject::HasTextEdit() const { return false; }

} // end sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
