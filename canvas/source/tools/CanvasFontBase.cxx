/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <CanvasFontBase.hxx>

css::rendering::FontMetrics SAL_CALL CanvasFontBase::getFontMetrics()
{
    // TODO
    return css::rendering::FontMetrics();
}

css::uno::Sequence<double> SAL_CALL CanvasFontBase::getAvailableSizes()
{
    // TODO
    return {};
}

css::uno::Sequence<css::beans::PropertyValue> SAL_CALL CanvasFontBase::getExtraFontProperties()
{
    // TODO
    return {};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
