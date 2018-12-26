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

#include <vcl/wmfexternal.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

// formally known as WMF_EXTERNALHEADER
WmfExternal::WmfExternal() :
    xExt(0),
    yExt(0),
    mapMode(0)
{
}

css::uno::Sequence< css::beans::PropertyValue > WmfExternal::getSequence() const
{
    css::uno::Sequence< css::beans::PropertyValue > aSequence;

    if (0 != xExt || 0 != yExt || 0 != mapMode)
    {
        aSequence.realloc(3);
        aSequence[0].Name = "Width";
        aSequence[0].Value <<= static_cast<sal_Int16>(xExt);
        aSequence[1].Name = "Height";
        aSequence[1].Value <<= static_cast<sal_Int16>(yExt);
        aSequence[2].Name = "MapMode";
        aSequence[2].Value <<= static_cast<sal_Int16>(mapMode);
    }

    return aSequence;
}

bool WmfExternal::setSequence(const css::uno::Sequence< css::beans::PropertyValue >& rSequence)
{
    bool bRetval(false);

    for (sal_Int32 i = 0; i < rSequence.getLength(); ++i)
    {
        const OUString aName(rSequence[i].Name);

        if (aName == "Width")
        {
            rSequence[i].Value >>= xExt;
            bRetval = true;
        }
        else if (aName == "Height")
        {
            rSequence[i].Value >>= yExt;
            bRetval = true;
        }
        else if (aName == "MapMode")
        {
            rSequence[i].Value >>= mapMode;
            bRetval = true;
        }
    }

    return bRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
