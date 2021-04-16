/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

#include <unordered_map>

namespace writerfilter
{
struct CommentProperties
{
    bool bDone;
    // TODO: a reference to a parent comment (paraIdParent: [MS-DOCX] sect. 2.5.3.1 CT_CommentEx)
};

using CommentPropertiesMap = std::unordered_map<OUString, CommentProperties>;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
