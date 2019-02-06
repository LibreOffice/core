/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define DISABLE_DYNLOADING
#define NO_CHILD_PROCESSES
#undef SAL_LOG_INFO
#undef SAL_LOG_WARN

#include <sal/config.h>

#include <cstdlib>

#include <sal/rtl/string.cxx>
#include <sal/rtl/ustring.cxx>

#include <sal/osl/unx/conditn.cxx>
#include <sal/osl/unx/file.cxx>
#include <sal/osl/unx/file_error_transl.cxx>
#include <sal/osl/unx/file_misc.cxx>
#include <sal/osl/unx/file_path_helper.cxx>
#include <sal/osl/unx/file_stat.cxx>
#include <sal/osl/unx/file_url.cxx>
#include <sal/osl/unx/memory.cxx>
#include <sal/osl/unx/module.cxx>
#include <sal/osl/unx/mutex.cxx>
#include <sal/osl/unx/nlsupport.cxx>
#include <sal/osl/unx/pipe.cxx>
#include <sal/osl/unx/process.cxx>
#include <sal/osl/unx/process_impl.cxx>
#include <sal/osl/unx/profile.cxx>
#include <sal/osl/unx/readwrite_helper.cxx>
#include <sal/osl/unx/security.cxx>
#include <sal/osl/unx/socket.cxx>
#include <sal/osl/unx/thread.cxx>
#include <sal/osl/unx/uunxapi.cxx>
#include <sal/rtl/alloc_arena.cxx>
#include <sal/rtl/alloc_cache.cxx>
#include <sal/rtl/alloc_fini.cxx>
#include <sal/rtl/alloc_global.cxx>
#include <sal/rtl/bootstrap.cxx>
#include <sal/rtl/byteseq.cxx>
#include <sal/rtl/hash.cxx>
#include <sal/rtl/locale.cxx>
#include <sal/rtl/math.cxx>
#include <sal/rtl/strbuf.cxx>
#include <sal/rtl/strimp.cxx>
#include <sal/rtl/uri.cxx>
#include <sal/rtl/ustrbuf.cxx>
#include <sal/textenc/converter.cxx>
#include <sal/textenc/convertsimple.cxx>
#include <sal/textenc/handleundefinedunicodetotextchar.cxx>
#include <sal/textenc/tcvtutf8.cxx>
#include <sal/textenc/tencinfo.cxx>
#include <sal/textenc/textcvt.cxx>
#include <sal/textenc/textenc.cxx>
#include <sal/textenc/unichars.cxx>

#if defined MACOSX
#include <sal/osl/unx/osxlocale.cxx>
#include <sal/osl/unx/system.cxx>
#endif

// Called from FullTextEncodingData::get in sal/textenc/textenc.cxx, but only
// defined for ANDROID (in ANDROID-specific sal/textenc/tables.cxx); would even
// work to leave it undefined for LINUX due to no '-z defs' under -fsanitize=*
// (solenv/gbuild/platform/linux.mk), but not for MACOSX:
extern "C" ImplTextEncodingData const * sal_getFullTextEncodingData(
    rtl_TextEncoding)
{
    std::abort();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
