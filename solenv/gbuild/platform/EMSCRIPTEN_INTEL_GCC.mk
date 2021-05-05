# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb_UnoApiHeadersTarget_select_variant = $(if $(filter udkapi,$(1)),comprehensive,$(2))

include $(GBUILDDIR)/platform/unxgcc.mk

gb_RUN_CONFIGURE := $(SRCDIR)/solenv/bin/run-configure
# avoid -s SAFE_HEAP=1 - c.f. gh#8584 this breaks source maps
gb_EMSCRIPTEN_CPPFLAGS := -pthread -s TOTAL_MEMORY=1GB -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4
gb_EMSCRIPTEN_LDFLAGS := $(gb_EMSCRIPTEN_CPPFLAGS) --bind -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s FETCH=1 -s ASSERTIONS=2 -s EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS=["UTF16ToString","stringToUTF16"]
gb_EMSCRIPTEN_QTDEFS := -DQT_NO_LINKED_LIST -DQT_NO_JAVA_STYLE_ITERATORS -DQT_NO_EXCEPTIONS -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB

gb_Executable_EXT := .html
gb_EMSCRIPTEN_EXCEPT := -s DISABLE_EXCEPTION_CATCHING=0

gb_LinkTarget_CFLAGS += $(gb_EMSCRIPTEN_CPPFLAGS) $(gb_EMSCRIPTEN_QTDEFS)
gb_LinkTarget_CXXFLAGS += $(gb_EMSCRIPTEN_CPPFLAGS) $(gb_EMSCRIPTEN_QTDEFS) $(gb_EMSCRIPTEN_EXCEPT)
gb_LinkTarget_LDFLAGS += $(gb_EMSCRIPTEN_LDFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS) $(gb_EMSCRIPTEN_EXCEPT)

# Linker and compiler optimize + debug flags are handled in LinkTarget.mk
gb_LINKEROPTFLAGS :=
gb_LINKERSTRIPDEBUGFLAGS :=
# This maps to g4, AKA source maps. The LO default would otherwise be g2!
gb_DEBUGINFO_FLAGS = -g
# We need at least code elimination, otherwise linking OOMs even with 64GB.
# So we "fake" -Og support to mean -O1 for Emscripten and always enable it for debug in configure.
gb_COMPILERDEBUGOPTFLAGS := -O1
gb_COMPILERNOOPTFLAGS := -O1 -fstrict-aliasing -fstrict-overflow

# cleanup addition JS and wasm files for binaries
define gb_Executable_Executable_platform
$(call gb_LinkTarget_add_auxtargets,$(2),\
        $(patsubst %.lib,%.wasm,$(3)) \
        $(patsubst %.lib,%.js,$(3)) \
        $(patsubst %.lib,%.worker.js,$(3)) \
)

endef

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_add_auxtargets,$(2),\
        $(patsubst %.lib,%.wasm,$(3)) \
        $(patsubst %.lib,%.js,$(3)) \
        $(patsubst %.lib,%.worker.js,$(3)) \
)

endef

gb_SUPPRESS_TESTS := $(true)

define gb_Library_get_rpath
endef

define gb_Executable_get_rpath
endef

# vim: set noet sw=4 ts=4
