/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_GMH_HXX
#define INCLUDED_COMPHELPER_GMH_HXX

#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>

namespace comphelper
{
class COMPHELPER_DLLPUBLIC GraphicMimeTypeHelper
{
public:
    static OUString GetMimeTypeForExtension(const OString& rExt);
};
}

#endif // INCLUDED_COMPHELPER_GMH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
