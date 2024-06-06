# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(GBUILDDIR)/platform/unxgcc.mk

gb_RUN_CONFIGURE := $(SRCDIR)/solenv/bin/run-configure
# avoid -s SAFE_HEAP=1 - c.f. gh#8584 this breaks source maps
gb_EMSCRIPTEN_CPPFLAGS := -pthread -s USE_PTHREADS=1 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -s SUPPORT_LONGJMP=wasm
gb_EMSCRIPTEN_LDFLAGS := $(gb_EMSCRIPTEN_CPPFLAGS)

# Initial memory size and worker thread pool
gb_EMSCRIPTEN_LDFLAGS += -s TOTAL_MEMORY=1GB -s PTHREAD_POOL_SIZE=4

# To keep the link time (and memory) down, prevent all rewriting options from wasm-emscripten-finalize
# See emscripten.py, finalize_wasm, modify_wasm = True
# So we need WASM_BIGINT=1 and ASSERTIONS=1 (2 implies STACK_OVERFLOW_CHECK)
gb_EMSCRIPTEN_LDFLAGS += --bind -s FORCE_FILESYSTEM=1 -s WASM_BIGINT=1 -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s FETCH=1 -s ASSERTIONS=1 -s EXIT_RUNTIME=0 -s EXPORTED_RUNTIME_METHODS=["UTF16ToString","stringToUTF16","UTF8ToString","ccall","cwrap","addOnPreMain","addOnPostRun","registerType","throwBindingError"$(if $(ENABLE_QT6),$(COMMA)"callMain"$(COMMA)"specialHTMLTargets")]
gb_EMSCRIPTEN_QTDEFS := -DQT_NO_LINKED_LIST -DQT_NO_JAVA_STYLE_ITERATORS -DQT_NO_EXCEPTIONS -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB

gb_Executable_EXT := .html
gb_EMSCRIPTEN_EXCEPT = -fwasm-exceptions -s SUPPORT_LONGJMP=wasm

gb_CXXFLAGS += $(gb_EMSCRIPTEN_CPPFLAGS)

# Here we don't use += because gb_LinkTarget_EXCEPTIONFLAGS from com_GCC_defs.mk contains -fexceptions and
# gb_EMSCRIPTEN_EXCEPT already has -fwasm-exceptions
gb_LinkTarget_EXCEPTIONFLAGS = $(gb_EMSCRIPTEN_EXCEPT)

gb_LinkTarget_CFLAGS += $(gb_EMSCRIPTEN_CPPFLAGS)
gb_LinkTarget_CXXFLAGS += $(gb_EMSCRIPTEN_CPPFLAGS) $(gb_EMSCRIPTEN_EXCEPT)
ifeq ($(ENABLE_QT5),TRUE)
gb_LinkTarget_CFLAGS += $(gb_EMSCRIPTEN_QTDEFS)
gb_LinkTarget_CXXFLAGS += $(gb_EMSCRIPTEN_QTDEFS)
endif
gb_LinkTarget_LDFLAGS += $(gb_EMSCRIPTEN_LDFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS) $(gb_EMSCRIPTEN_EXCEPT)

# Linker and compiler optimize + debug flags are handled in LinkTarget.mk
gb_LINKEROPTFLAGS :=
gb_LINKERSTRIPDEBUGFLAGS :=
# This maps to g3, no source maps, but DWARF with current emscripten!
# https://developer.chrome.com/blog/wasm-debugging-2020/
gb_DEBUGINFO_FLAGS = -g

ifeq ($(HAVE_EXTERNAL_DWARF),TRUE)
gb_DEBUGINFO_FLAGS += -gsplit-dwarf -gpubnames
endif

gb_COMPILEROPTFLAGS := -O3

# We need at least code elimination, otherwise linking OOMs even with 64GB.
# So we "fake" -Og support to mean -O1 for Emscripten and always enable it for debug in configure.
gb_COMPILERDEBUGOPTFLAGS := -O1
gb_COMPILERNOOPTFLAGS := -O1 -fstrict-aliasing -fstrict-overflow

# cleanup addition JS and wasm files for binaries
define gb_Executable_Executable_platform
$(call gb_LinkTarget_add_auxtargets,$(2),\
        $(patsubst %.lib,%.linkdeps,$(3)) \
        $(patsubst %.lib,%.wasm,$(3)) \
        $(patsubst %.lib,%.js,$(3)) \
        $(patsubst %.lib,%.worker.js,$(3)) \
        $(patsubst %.lib,%.wasm.dwp,$(3)) \
)

endef

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_add_auxtargets,$(2),\
        $(patsubst %.lib,%.linkdeps,$(3)) \
        $(patsubst %.lib,%.wasm,$(3)) \
        $(patsubst %.lib,%.js,$(3)) \
        $(patsubst %.lib,%.worker.js,$(3)) \
        $(patsubst %.lib,%.wasm.dwp,$(3)) \
)

endef

define gb_Library_get_rpath
endef

define gb_Executable_get_rpath
endef

# vim: set noet sw=4 ts=4
