/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_CELLTEXTATTR_HXX
#define INCLUDED_SC_CELLTEXTATTR_HXX

#include <sal/types.h>
#include <svl/languageoptions.hxx>

namespace sc {

struct CellTextAttr
{
    sal_uInt16    mnTextWidth;
    SvtScriptType mnScriptType;

    CellTextAttr();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
