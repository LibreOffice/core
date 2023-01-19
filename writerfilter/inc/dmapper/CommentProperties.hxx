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

namespace writerfilter
{
/**
 A container for the extended comment properties linked to the last paragraph of a comment

 Corresponds to the data available in w15:commentEx elements from commentsExtended stream
 ([MS-DOCX]): resolved state and parent (referring to comment that this one answers to).

 @since 7.2
*/
struct CommentProperties
{
    bool bDone;
    OUString sParaIdParent;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
