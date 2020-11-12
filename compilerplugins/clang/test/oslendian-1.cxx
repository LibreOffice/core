/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <osl/endian.h>

#if defined OSL_BIGENDIAN || defined OSL_LITENDIAN
#endif
#ifdef OSL_BIGENDIAN
#endif
#ifdef OSL_LITENDIAN
#endif
#ifndef OSL_BIGENDIAN
#endif
#ifndef OSL_LITENDIAN
#endif

#if !defined OSL_BIGENDIAN
#define OSL_BIGENDIAN
// expected-error@-1 {{macro 'OSL_BIGENDIAN' defined in addition to 'OSL_LITENDIAN' [loplugin:oslendian]}}
// expected-note@osl/endian.h:* {{conflicting macro definition is here [loplugin:oslendian]}}
#endif

#if !defined OSL_LITENDIAN
#define OSL_LITENDIAN
// expected-error@-1 {{macro 'OSL_LITENDIAN' defined in addition to 'OSL_BIGENDIAN' [loplugin:oslendian]}}
// expected-note@osl/endian.h:* {{conflicting macro definition is here [loplugin:oslendian]}}
#endif

#if defined OSL_BIGENDIAN
#undef OSL_BIGENDIAN
// expected-error@-1 {{macro 'OSL_BIGENDIAN' undefinition [loplugin:oslendian]}}
#endif

#if defined OSL_LITENDIAN
#undef OSL_LITENDIAN
// expected-error@-1 {{macro 'OSL_LITENDIAN' undefinition [loplugin:oslendian]}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
