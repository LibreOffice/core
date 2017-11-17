# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,glib2))

ifeq ($(OS),WNT)
$(eval $(call gb_ExternalProject_register_targets,glib2,\
	build \
))

$(eval $(call gb_ExternalProject_use_external,glib2,zlib))

# get local copies of zlib.h & zlib.lib (glib imposes fixed dir
# structure on that), run native msbuild for binaries then
$(call gb_ExternalProject_get_state_target,glib2,build):
	$(call gb_ExternalProject_run,build,\
		mkdir -p $(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)/etc/include && \
		cp $(WORKDIR)/UnpackedTarball/zlib/zlib.h $(WORKDIR)/UnpackedTarball/zlib/zconf.h */etc/include/ && \
		mkdir -p $(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)/etc/lib && \
		cp $(WORKDIR)/LinkTarget/StaticLibrary/zlib.lib */etc/lib/ && \
		msbuild.exe glib.sln \
			/p:GlibEtcInstallRoot=./$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)/etc \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
	,build/win32/vs10)
endif

# vim: set noet sw=4 ts=4:
