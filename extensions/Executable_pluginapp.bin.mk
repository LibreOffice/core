# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Executable_Executable,pluginapp.bin))

$(eval $(call gb_Executable_use_external,pluginapp.bin,npapi_headers))

$(eval $(call gb_Executable_set_include,pluginapp.bin,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/source/plugin/inc \
))

$(eval $(call gb_Executable_use_sdk_api,pluginapp.bin))

$(eval $(call gb_Executable_add_exception_objects,pluginapp.bin,\
	extensions/source/plugin/unx/npwrap \
	extensions/source/plugin/unx/npnapi \
))

$(eval $(call gb_Executable_use_static_libraries,pluginapp.bin,\
	plugcon \
))

$(eval $(call gb_Executable_use_libraries,pluginapp.bin,\
	sal \
))

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Executable_add_libs,pluginapp.bin,\
	-lXm \
	-lXt \
	-lXext \
	-lX11 \
	-ldl \
))
else ifeq ($(filter-out FREEBSD NETBSD OPENBSD DRAGONFLY,$(OS)),)
$(eval $(call gb_Executable_add_libs,pluginapp.bin,\
	-lXt \
	-lXext \
	-lX11 \
))
else ifeq ($(OS),AIX)
$(eval $(call gb_Executable_add_libs,pluginapp.bin,\
	-lXpm \
	-lXmu \
	-lXt \
	-lXext \
	-lX11 \
	-ldl \
))
else
$(eval $(call gb_Executable_add_libs,pluginapp.bin,\
	-lXt \
	-lXext \
	-lX11 \
	-ldl \
))
endif


ifeq ($(ENABLE_GTK),TRUE)
$(eval $(call gb_Executable_use_externals,pluginapp.bin,\
    gthread \
    gtk \
))

# the orignal dmakefile said: don't ask, it's ugly
ifeq ($(OS),SOLARIS)
$(eval $(call gb_Executable_set_ldflags,pluginapp.bin,\
	-z nodefs \
))
endif

endif

# vim:set noet sw=4 ts=4:
