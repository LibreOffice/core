/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

namespace vcl::pdf
{
enum class PDFObjectType
{
    Unknown = 0,
    Boolean = 1,
    Number = 2,
    String = 3,
    Name = 4,
    Array = 5,
    Dictionary = 6,
    Stream = 7,
    Nullobj = 8,
    Reference = 9
};

} // namespace vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
