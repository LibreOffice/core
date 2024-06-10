/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/svxdllapi.h>

#include <svx/annotation/Annotation.hxx>
#include <svx/svdorect.hxx>

namespace sdr::annotation
{
/** Annotation data that contains the annotation unique ID and author's ID */
struct SVXCORE_DLLPUBLIC AnnotationViewData
{
    sal_Int32 nIndex = -1;
    sal_uInt16 nAuthorIndex = 0;
};

/** Annotation (sdr) object, which represents an annotation inside the document */
class SVXCORE_DLLPUBLIC AnnotationObject final : public SdrRectObj
{
private:
    virtual ~AnnotationObject() override;

    sdr::annotation::AnnotationViewData maViewData;

public:
    AnnotationObject(SdrModel& rSdrModel);
    AnnotationObject(SdrModel& rSdrModel, AnnotationObject const& rSource);
    AnnotationObject(SdrModel& rSdrModel, tools::Rectangle const& rRectangle,
                     sdr::annotation::AnnotationViewData const& aAnnotationViewData);

    rtl::Reference<SdrObject> CloneSdrObject(SdrModel& rTargetModel) const override;

    SdrObjKind GetObjIdentifier() const override;
    OUString TakeObjNameSingul() const override;
    OUString TakeObjNamePlural() const override;

    void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;

    void ApplyAnnotationName();

    bool HasTextEdit() const override;

    bool hasSpecialDrag() const override { return true; }

    bool beginSpecialDrag(SdrDragStat& /*rDrag*/) const override { return false; }
};

} // end sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
