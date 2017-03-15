/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_XMLSECURITY_INC_XSECGPGDLLAPI_H
#define INCLUDED_XMLSECURITY_INC_XSECGPGDLLAPI_H

#include <sal/types.h>

#if defined(XSECGPG_DLLIMPLEMENTATION)
#define XSECGPG_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define XSECGPG_DLLPUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#endif // INCLUDED_XMLSECURITY_INC_XSECGPGDLLAPI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
