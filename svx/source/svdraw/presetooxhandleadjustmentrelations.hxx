/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SOURCE_SVDRAW_PRESETOOXHANDLEADJUSTMENTRELATIONS_HXX
#define INCLUDED_SVX_SOURCE_SVDRAW_PRESETOOXHANDLEADJUSTMENTRELATIONS_HXX
#include <rtl/ustring.hxx>

namespace PresetOOXHandleAdj
{
/* This method is used in SdrObjCustomShape::MergeDefaultAttributes() */
void GetOOXHandleAdjRelation(
    const OUString sFullOOXShapeName, /* e.g. "ooxml-circularArrow" */
    const sal_Int32 nHandleIndex, /* index in sequence from property "Handles" */
    OUString& rFirstRefType, /* Propertyname, same as by pptx import, e.g. "RefX" */
    sal_Int32& rFirstAdjValueIndex, /* index in sequence from property "AdjustmentValues" */
    OUString& rSecondRefType, /* Propertyname, same as by pptx import, e.g. "RefY" */
    sal_Int32& rSecondAdjValueIndex /* index in sequence from property "AdjustmentValues" */
);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
