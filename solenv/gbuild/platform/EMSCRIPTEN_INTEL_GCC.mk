# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(DISABLE_DYNLOADING),TRUE)
gb_UnoApiHeadersTarget_select_variant = $(if $(filter udkapi,$(1)),comprehensive,$(2))
else
gb_UnoApiHeadersTarget_select_variant = $(2)
endif

include $(GBUILDDIR)/platform/unxgcc.mk

emscripten_CPPFLAGS := -pthread -s TOTAL_MEMORY=1GB -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4

gb_Executable_EXT := .html

gb_CFLAGS += $(emscripten_CPPFLAGS)
gb_CXXFLAGS += $(emscripten_CPPFLAGS) -s DISABLE_EXCEPTION_CATCHING=0
gb_LinkTarget_LDFLAGS += $(emscripten_CPPFLAGS) --bind

define gb_Library_get_rpath
endef

define gb_Executable_get_rpath
endef

gb_LINKEROPTFLAGS :=
gb_LINKERSTRIPDEBUGFLAGS :=

#gb_CXX_LINKFLAGS += -pthread -s TOTAL_MEMORY=1GB -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4

# vim: set noet sw=4 ts=4
