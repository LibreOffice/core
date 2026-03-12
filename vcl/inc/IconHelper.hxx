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
#include <vcl/dllapi.h>

class VCL_DLLPUBLIC IconHelper
{
public:
    /** Get icon name as used for app icons by (externally provided) icon themes. */
    static OUString GetAppIconName(sal_uInt16 nIcon);

    /** Get icon name used for app icons in LibreOffice's own icon themes. */
    static OUString GetInternalAppIconName(sal_uInt16 nIcon);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
