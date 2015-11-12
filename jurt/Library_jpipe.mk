# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,jpipe))

$(eval $(call gb_Library_use_libraries,jpipe,\
	$(gb_UWINAPI) \
))

ifeq ($(OS),WNT)

# The real library is called jpipx on Windows. We build only a wrapper
# for it here.
$(eval $(call gb_Library_add_cobjects,jpipe,\
    jurt/source/pipe/wrapper/wrapper \
))

else

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(gb_CC)),)
Library_jpipe__staticsalhack = TRUE
endif
endif

ifeq ($(Library_jpipe__staticsalhack),)

$(eval $(call gb_Library_use_libraries,jpipe,\
    sal \
))

$(eval $(call gb_Library_add_cobjects,jpipe,\
    jurt/source/pipe/com_sun_star_lib_connections_pipe_PipeConnection \
))

else

# HACK for Clang -fsanitize=address etc. to build libjpipe.so (dlopen'ed from
# java executable) without dependence on __asan_* etc. symbols (expected to be
# provided by the executable), incl. statically linking in the relevant parts of
# sal:

$(eval $(call gb_Library_add_cobjects,jpipe, \
    jurt/source/pipe/com_sun_star_lib_connections_pipe_PipeConnection \
))

$(eval $(call gb_Library_add_exception_objects,jpipe, \
    jurt/source/pipe/staticsalhack \
))

$(eval $(call gb_Library_add_defs,jpipe, \
    -DRTL_OS="\"$(RTL_OS)"\" \
    -DRTL_ARCH="\"$(RTL_ARCH)"\" \
))

$(eval $(call gb_Library_set_include,jpipe, \
    $$(INCLUDE) \
    -I$(SRCDIR) \
    -I$(SRCDIR)/sal/inc \
    -I$(SRCDIR)/sal/osl/unx \
    -I$(SRCDIR)/sal/rtl \
    -I$(SRCDIR)/sal/textenc \
))

$(eval $(call gb_Library_use_externals,jpipe, \
    boost_headers \
))

$(eval $(call gb_Library_add_libs,jpipe, \
    $(if $(filter-out $(OS),ANDROID),-lpthread) \
))

$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,jpipe)): \
    gb_CC := $(filter-out -fsanitize=%,$(gb_CC))
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,jpipe)): \
    gb_CXX := $(filter-out -fsanitize=%,$(gb_CXX))

endif
endif

# vim:set noet sw=4 ts=4:
