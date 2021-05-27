/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "XMLTextColumnsPropHdl.hxx"

XMLTextColumnsPropHdl::XMLTextColumnsPropHdl() = default;

bool XMLTextColumnsPropHdl::equals(const css::uno::Any& r1, const css::uno::Any& r2) const
{
    return XMLPropertyHandler::equals(r1, r2);
}

bool XMLTextColumnsPropHdl::importXML(const OUString& /*rStrImpValue*/, css::uno::Any& /*rValue*/,
                                      const SvXMLUnitConverter& /*rUnitConverter*/) const
{
    return false;
}

bool XMLTextColumnsPropHdl::exportXML(OUString& /*rStrExpValue*/, const css::uno::Any& /*rValue*/,
                                      const SvXMLUnitConverter& /*rUnitConverter*/) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
