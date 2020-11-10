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
enum class PDFPageObjectType
{
    Unknown = 0,
    Text = 1,
    Path = 2,
    Image = 3,
    Shading = 4,
    Form = 5
};

} // namespace vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
