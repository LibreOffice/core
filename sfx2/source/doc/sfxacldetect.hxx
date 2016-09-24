/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_DOC_SFXACLDETECT_HXX
#define INCLUDED_SFX2_SOURCE_DOC_SFXACLDETECT_HXX

// Let's check if this extra ACL check is still necessary...  If disabling it
// causes no regressions, then we can safely remove this code.
#define EXTRA_ACL_CHECK 0

#if EXTRA_ACL_CHECK
sal_Bool IsReadonlyAccordingACL( const sal_Unicode* pFilePath );
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
