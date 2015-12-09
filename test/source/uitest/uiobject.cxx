/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/uiobject.hxx>

UIObject::~UIObject()
{
}

std::map<const OUString, OUString> UIObject::get_state()
{
    std::map<const OUString, OUString> aMap;
    aMap["NotImplemented"] = "NotImplemented";
    return aMap;
}

void UIObject::execute(const OUString& /*rAction*/,
        const std::map<const OUString, OUString>& /*rParameters*/)
{
    // should never be called
    throw std::exception();
}

UIObjectType UIObject::getType()
{
    return UIObjectType::UNKNOWN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
