# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,epoxy))

$(eval $(call gb_Library_use_unpacked,epoxy,epoxy))

$(eval $(call gb_Library_set_warnings_disabled,epoxy))

$(eval $(call gb_Library_set_include,epoxy,\
	-I$(gb_UnpackedTarball_workdir)/epoxy/include \
	-I$(gb_UnpackedTarball_workdir)/epoxy/src \
	$$(INCLUDE) \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,epoxy,\
	-ldl \
))
endif

$(eval $(call gb_Library_add_generated_cobjects,epoxy,\
	UnpackedTarball/epoxy/src/dispatch_common \
	UnpackedTarball/epoxy/src/gl_generated_dispatch \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_generated_cobjects,epoxy,\
	UnpackedTarball/epoxy/src/dispatch_wgl \
	UnpackedTarball/epoxy/src/wgl_generated_dispatch \
))
else ifneq ($(filter iOS MACOSX,$(OS)),)
# nothing
else ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_add_generated_cobjects,epoxy,\
	UnpackedTarball/epoxy/src/dispatch_egl \
	UnpackedTarball/epoxy/src/egl_generated_dispatch \
))
else
$(eval $(call gb_Library_add_generated_cobjects,epoxy,\
	UnpackedTarball/epoxy/src/dispatch_glx \
	UnpackedTarball/epoxy/src/glx_generated_dispatch \
))
ifeq ($(ENABLE_GTK3),TRUE)
$(eval $(call gb_Library_add_cflags,epoxy,\
	-DPLATFORM_HAS_EGL=1 \
))
$(eval $(call gb_Library_add_generated_cobjects,epoxy,\
	UnpackedTarball/epoxy/src/dispatch_egl \
	UnpackedTarball/epoxy/src/egl_generated_dispatch \
))
else
$(eval $(call gb_Library_add_cflags,epoxy,\
	-DPLATFORM_HAS_EGL=0 \
))
endif
endif

# vim: set noet sw=4 ts=4:
