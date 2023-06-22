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

#include <XMLRtlGutterPropertyHandler.hxx>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_set.h>
#include <com/sun/star/uno/Any.hxx>
#include <xmloff/xmluconv.hxx>

using namespace com::sun::star;

XMLRtlGutterPropertyHandler::XMLRtlGutterPropertyHandler() = default;

XMLRtlGutterPropertyHandler::~XMLRtlGutterPropertyHandler() = default;

namespace
{
constexpr frozen::unordered_set<std::u16string_view, 4> constRtlModes{ u"rl-tb", u"tb-rl", u"rl",
                                                                       u"tb" };
} // end anonymous ns

bool XMLRtlGutterPropertyHandler::importXML(const OUString& rStrImpValue, uno::Any& rValue,
                                            const SvXMLUnitConverter&) const
{
    // Infer RtlGutter from WritingMode.
    auto it = constRtlModes.find(rStrImpValue);
    rValue <<= (it != constRtlModes.end());
    return true;
}

bool XMLRtlGutterPropertyHandler::exportXML(OUString&, const uno::Any&,
                                            const SvXMLUnitConverter&) const
{
    // No need to export RtlGutter.
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
