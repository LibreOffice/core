/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * Some of our templating stuff clashes with EM_ASM / MAIN_THREAD_EM_ASM:
 *
 *     shellexec.cxx:250:5: error: called object type 'const char *' is not a function or function pointer
 *     MAIN_THREAD_EM_ASM(
 *     ^
 *     git_emsdk/upstream/emscripten/cache/sysroot/include/emscripten/em_asm.h:208:39: note: expanded from macro 'MAIN_THREAD_EM_ASM'
 *     #define MAIN_THREAD_EM_ASM(code, ...) ((void)emscripten_asm_const_int_sync_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__)))
 *                                           ^
 *     1 error generated.
 *
 *  so as a workaround the EM_ASM call is now in an extra file.
 */

#include <emscripten.h>

void execute_browser(const char* sUrl) { EM_ASM("window.open(UTF8ToString($0));", sUrl); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
