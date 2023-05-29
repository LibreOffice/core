/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef __clang__
// This is a bit lame, but when using -DMACRO that wasn't present when the PCH was built,
// Clang suddenly subjects it to the -Wunused-macros check, which isn't the case otherwise.
// And many of these macros are not actually used. So use them here to silence the warnings.
// See gb_PrecompiledHeader_ignore_flags_system in solenv/gbuild/PrecompiledHeaders.mk .

#ifdef SAX_DLLIMPLEMENTATION
#endif
#ifdef SCQAHELPER_DLLIMPLEMENTATION
#endif
#ifdef SYSTEM_EXPAT
#endif
#ifdef SYSTEM_LIBXML
#endif
#ifdef SYSTEM_ZLIB
#endif

#endif // __clang__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
