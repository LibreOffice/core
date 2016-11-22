/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#if defined OSL_BIGENDIAN || defined OSL_LITENDIAN // expected-error {{definition of macro 'OSL_BIGENDIAN' checked but 'osl/endian.h' is not included [loplugin:oslendian]}} // expected-error {{definition of macro 'OSL_LITENDIAN' checked but 'osl/endian.h' is not included [loplugin:oslendian]}}
#endif
#ifdef OSL_BIGENDIAN // expected-error {{definition of macro 'OSL_BIGENDIAN' checked but 'osl/endian.h' is not included [loplugin:oslendian]}}
#endif
#ifdef OSL_LITENDIAN // expected-error {{definition of macro 'OSL_LITENDIAN' checked but 'osl/endian.h' is not included [loplugin:oslendian]}}
#endif
#ifndef OSL_BIGENDIAN // expected-error {{definition of macro 'OSL_BIGENDIAN' checked but 'osl/endian.h' is not included [loplugin:oslendian]}}
#endif
#ifndef OSL_LITENDIAN // expected-error {{definition of macro 'OSL_LITENDIAN' checked but 'osl/endian.h' is not included [loplugin:oslendian]}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
