/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_WRITERPERFECTDLLAPI_H
#define INCLUDED_WRITERPERFECT_WRITERPERFECTDLLAPI_H

#if defined WRITERPERFECT_DLLIMPLEMENTATION

#define WRITERPERFECT_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define WRITERPERFECT_DLLPUBLIC SAL_DLLPUBLIC_IMPORT
#endif
#define WRITERPERFECT_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_WRITERPERFECT_WRITERPERFECTDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
