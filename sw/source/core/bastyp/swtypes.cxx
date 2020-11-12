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

#include <swtypes.hxx>

#include <editeng/unolingu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>

using namespace com::sun::star;

Size GetGraphicSizeTwip(const Graphic& rGraphic, vcl::RenderContext* pOutDev)
{
    const MapMode aMapTwip(MapUnit::MapTwip);
    Size aSize(rGraphic.GetPrefSize());

    if (!aSize.getWidth() && !aSize.getHeight())
    {
        const_cast<Graphic&>(rGraphic).makeAvailable();
        aSize = rGraphic.GetPrefSize();
    }

    if (MapUnit::MapPixel == rGraphic.GetPrefMapMode().GetMapUnit())
    {
        if (!pOutDev)
            pOutDev = Application::GetDefaultDevice();
        aSize = pOutDev->PixelToLogic(aSize, aMapTwip);
    }
    else
    {
        aSize = OutputDevice::LogicToLogic(aSize, rGraphic.GetPrefMapMode(), aMapTwip);
    }
    return aSize;
}

uno::Reference<linguistic2::XSpellChecker1> GetSpellChecker()
{
    return LinguMgr::GetSpellChecker();
}

uno::Reference<linguistic2::XHyphenator> GetHyphenator() { return LinguMgr::GetHyphenator(); }

uno::Reference<linguistic2::XThesaurus> GetThesaurus() { return LinguMgr::GetThesaurus(); }

uno::Reference<linguistic2::XLinguProperties> GetLinguPropertySet()
{
    return LinguMgr::GetLinguPropertySet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
