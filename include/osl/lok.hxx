/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OSL_LOK_HXX
#define INCLUDED_OSL_LOK_HXX

#ifdef LIBO_INTERNAL_ONLY

#include <sal/saldllapi.h>

namespace osl
{

namespace LibreOfficeKit
{

SAL_DLLPUBLIC void setActive();

SAL_DLLPUBLIC bool isActive();

}
}

#endif // LIBO_INTERNAL_ONLY

#endif // INCLUDED_OSL_LOK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
