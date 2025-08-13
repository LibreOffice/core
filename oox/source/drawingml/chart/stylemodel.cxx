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

#include <tools/gen.hxx>
#include <drawingml/chart/stylemodel.hxx>

namespace oox::drawingml::chart
{
model::StyleEntry StyleEntryModel::toStyleEntry(oox::core::XmlFilterBase& rFilter)
{
    std::shared_ptr<PropertyMap> aCharMap = std::make_shared<PropertyMap>();
    if (mrTextCharacterProperties)
    {
        mrTextCharacterProperties->pushToPropMap(*aCharMap, rFilter);
    }

    // Unlike TextCharacterPropsRef, handled above, TextBodyPropsRef does not
    // have a pushToPropMap() method. It does have a PropertyMap data member,
    // which can be set using pushTextDistances(). I'm not sure why the
    // interfaces are different, but that's why what's below is not parallel to
    // the above.
    std::shared_ptr<PropertyMap> aBodyMap = std::make_shared<PropertyMap>();
    if (mxBodyPr)
    {
        mxBodyPr->pushTextDistances(Size(0, 0));
        aBodyMap.reset(&mxBodyPr->maPropertyMap);
    }

    return model::StyleEntry(mxLnRef, mfLineWidthScale, mxFillRef, mxEffectRef, mxFontRef,
                             mxShapeProp, aCharMap, aBodyMap);
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
