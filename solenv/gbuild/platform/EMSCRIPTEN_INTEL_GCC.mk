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

# WASM is also optimized at link time, but ignores linker flags, so wants $(gb_COMPILEROPTFLAGS)
gb_LINKEROPTFLAGS :=
gb_LINKERSTRIPDEBUGFLAGS :=
gb_LinkTarget_LDFLAGS += $(gb_EMSCRIPTEN_LDFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS) $(gb_EMSCRIPTEN_EXCEPT) \
  $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS), \
  $(if $(ENABLE_OPTIMIZED_DEBUG),$(gb_COMPILERDEBUGOPTFLAGS), \
  $(gb_COMPILERNOOPTFLAGS)))

define gb_Library_get_rpath
endef

define gb_Executable_get_rpath
endef

# vim: set noet sw=4 ts=4
