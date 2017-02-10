# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,raptor))

$(eval $(call gb_ExternalProject_use_external,raptor,libxml2))

$(eval $(call gb_ExternalProject_register_targets,raptor,\
	build \
))

$(call gb_ExternalProject_get_state_target,raptor,build):
	$(call gb_ExternalProject_run,build,\
		$(if $(filter IOS,$(OS)),LIBS="-liconv") \
		CFLAGS="$(CFLAGS) $(if $(debug),-g,-O) $(if $(filter TRUE,$(DISABLE_DYNLOADING)),-fvisibility=hidden) \
			$(if $(filter GCCLINUXPOWERPC64,$(COM)$(OS)$(CPUNAME)),-mminimal-toc)" \
		LDFLAGS=" \
			$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN") \
			$(if $(SYSBASE),$(if $(filter LINUX SOLARIS,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
		CPPFLAGS="$(if $(SYSBASE),-I$(SYSBASE)/usr/include)" \
		./configure --disable-gtk-doc \
			 --enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" \
			--with-www=xml \
			--without-xslt-config \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			$(if $(filter INTEL ARM,$(CPUNAME)),ac_cv_c_bigendian=no)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
			$(if $(SYSTEM_LIBXML),,--with-xml2-config=$(call gb_UnpackedTarball_get_dir,xml2)/xml2-config) \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
