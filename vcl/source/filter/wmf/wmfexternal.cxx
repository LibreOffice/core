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

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>
#include <vcl/wmfexternal.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

// formally known as WMF_EXTERNALHEADER
WmfExternal::WmfExternal()
    : xExt(0)
    , yExt(0)
    , mapMode(0)
{
}

css::uno::Sequence<css::beans::PropertyValue> WmfExternal::getSequence() const
{
    if (0 != xExt || 0 != yExt || 0 != mapMode)
    {
        return { comphelper::makePropertyValue("Width", static_cast<sal_Int16>(xExt)),
                 comphelper::makePropertyValue("Height", static_cast<sal_Int16>(yExt)),
                 comphelper::makePropertyValue("MapMode", static_cast<sal_Int16>(mapMode)) };
    }

    return {};
}

bool WmfExternal::setSequence(const css::uno::Sequence<css::beans::PropertyValue>& rSequence)
{
    bool bRetval(false);

    for (const auto& rPropVal : rSequence)
    {
        const OUString aName(rPropVal.Name);

        if (aName == "Width")
        {
            rPropVal.Value >>= xExt;
            bRetval = true;
        }
        else if (aName == "Height")
        {
            rPropVal.Value >>= yExt;
            bRetval = true;
        }
        else if (aName == "MapMode")
        {
            rPropVal.Value >>= mapMode;
            bRetval = true;
        }
    }

    return bRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
