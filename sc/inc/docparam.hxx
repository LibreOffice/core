/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_DOCPARAM_HXX
#define INCLUDED_SC_INC_DOCPARAM_HXX

#include "address.hxx"

// Let's put here misc structures that get passed to ScDocument's methods.

struct ScColWidthParam
{
    SCROW       mnMaxTextRow;
    sal_uInt32  mnMaxTextLen;
    bool        mbSimpleText;

    ScColWidthParam();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
