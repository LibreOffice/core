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

#include <sal/log.hxx>
#include <svx/svdmodel.hxx>

#include <svx/annotation/AnnotationObject.hxx>
#include <svx/annotation/ObjectAnnotationData.hxx>
#include <sdr/properties/rectangleproperties.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdasitm.hxx>
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
        constexpr const auto constArrayLight
            = std::to_array<Color>({ COL_AUTHOR1_LIGHT, COL_AUTHOR2_LIGHT, COL_AUTHOR3_LIGHT,
                                     COL_AUTHOR4_LIGHT, COL_AUTHOR5_LIGHT, COL_AUTHOR6_LIGHT,
                                     COL_AUTHOR7_LIGHT, COL_AUTHOR8_LIGHT, COL_AUTHOR9_LIGHT });
        return constArrayLight[aAuthorIndex % constArrayLight.size()];
    }

    return COL_WHITE;
}
}

AnnotationObject::AnnotationObject(SdrModel& rSdrModel)
    : SdrRectObj(rSdrModel)
{
    setAsAnnotationObject(true);
}

AnnotationObject::AnnotationObject(SdrModel& rSdrModel, AnnotationObject const& rSource)
    : SdrRectObj(rSdrModel, rSource)
{
    setAsAnnotationObject(true);
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
    setAsAnnotationObject(true);
}

void AnnotationObject::ApplyAnnotationName()
{
    if (mpAnnotationData->mxAnnotation)
    {
        OUString sInitials(mpAnnotationData->mxAnnotation->getInitials());
        if (sInitials.isEmpty())
            sInitials = createInitials(mpAnnotationData->mxAnnotation->getAuthor());
        SetText(sInitials + " " + OUString::number(maViewData.nIndex));
    }
    else
    {
        SetText(u"Empty"_ustr);
    }

    Color aColor(getColorLight(maViewData.nAuthorIndex));

    SetMergedItem(XFillStyleItem(drawing::FillStyle_SOLID));
    SetMergedItem(XFillColorItem(OUString(), aColor));

    Color aBorderColor(aColor);
    if (aColor.IsDark())
        aBorderColor.IncreaseLuminance(32);
    else
        aBorderColor.DecreaseLuminance(32);

    SetMergedItem(XLineStyleItem(drawing::LineStyle_SOLID));
    SetMergedItem(XLineColorItem(OUString(), aBorderColor));
    SetMergedItem(XLineWidthItem(o3tl::convert(0, o3tl::Length::pt, o3tl::Length::mm100)));

    SetMergedItem(SvxFontHeightItem(o3tl::convert(10, o3tl::Length::pt, o3tl::Length::mm100), 100,
                                    EE_CHAR_FONTHEIGHT));

    SetMergedItem(SvxColorItem(aColor.IsDark() ? COL_WHITE : COL_BLACK, EE_CHAR_COLOR));

    SetMergedItem(SdrTextFitToSizeTypeItem(drawing::TextFitToSizeType_NONE));

    SetMergedItem(makeSdrTextWordWrapItem(false));
    SetMergedItem(makeSdrTextAutoGrowWidthItem(true));
    SetMergedItem(makeSdrTextAutoGrowHeightItem(true));
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
    OUString sName(u"Annotation"_ustr);
    OUString aName(GetName());

    if (!aName.isEmpty())
        sName += " '" + aName + "'";

    return sName;
}

OUString AnnotationObject::TakeObjNamePlural() const { return u"Annotations"_ustr; }

rtl::Reference<SdrObject> AnnotationObject::CloneSdrObject(SdrModel& rTargetModel) const
{
    return new AnnotationObject(rTargetModel, *this);
}

bool AnnotationObject::HasTextEdit() const { return false; }

} // end sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
