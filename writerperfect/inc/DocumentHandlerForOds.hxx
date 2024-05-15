/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <libodfgen/libodfgen.hxx>

#include "DocumentHandlerFor.hxx"

#include <rtl/ustring.hxx>

namespace writerperfect
{
template <> struct DocumentHandlerFor<OdsGenerator>
{
    static OUString name() { return u"com.sun.star.comp.Calc.XMLOasisImporter"_ustr; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
