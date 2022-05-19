# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libtiff))

$(eval $(call gb_ExternalProject_register_targets,libtiff,\
	build \
))

ifeq ($(COM),MSC)
$(eval $(call gb_ExternalProject_use_nmake,libtiff,build))

# Explicitly passing in ARCH (for the known architectures, at least) avoids
# workdir/UnpackedTarball/libtiff/Makefile.vc not being able to detect it when CC is clang-cl:
$(call gb_ExternalProject_get_state_target,libtiff,build):
	$(call gb_Trace_StartRange,libttiff,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		nmake -nologo -f Makefile.vc CFG=$(if $(MSVC_USE_DEBUG_RUNTIME),debug,release)-static RTLIBCFG=dynamic OBJDIR=output \
		    $(if $(filter INTEL,$(CPUNAME)),ARCH=x86, \
		    $(if $(filter X86_64,$(CPUNAME)),ARCH=x64, \
		    $(if $(filter AARCH64,$(CPUNAME)),ARCH=ARM))) \
	)
	$(call gb_Trace_EndRange,libtiff,EXTERNAL)
else
$(eval $(call gb_ExternalProject_use_autoconf,libtiff,build))

$(call gb_ExternalProject_get_state_target,libtiff,build) :
	$(call gb_Trace_StartRange,libtiff,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--enable-static \
			--with-pic \
			--disable-shared \
			--disable-jbig \
			--disable-jpeg \
			--disable-lzma \
			--disable-win32-io \
			--disable-zstd \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CFLAGS="$(CFLAGS) $(call gb_ExternalProject_get_build_flags,libtiff)" \
			CPPFLAGS="$(CPPFLAGS) $(BOOST_CPPFLAGS)" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,libtiff)" \
			$(gb_CONFIGURE_PLATFORMS) \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libtiff,EXTERNAL)
endif

# vim: set noet sw=4 ts=4:
