# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libetonyek))

$(eval $(call gb_ExternalProject_use_autoconf,libetonyek,build))

$(eval $(call gb_ExternalProject_register_targets,libetonyek,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libetonyek,\
	boost_headers \
	glm_headers \
	libxml2 \
	mdds_headers \
	revenge \
	zlib \
))

$(call gb_ExternalProject_get_state_target,libetonyek,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) ./configure \
			--with-pic \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
			--without-docs \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			--disable-tests \
			--disable-werror \
			--disable-weffc \
			--without-tools \
			--with-mdds=0.x \
			$(if $(filter WNT,$(OS_FOR_BUILD)),MKDIR_P="$(shell cygpath -m /usr/bin/mkdir) -p") \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(filter LINUX,$(OS)), \
				'LDFLAGS=-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN') \
			CPPFLAGS="$(CPPFLAGS) $(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost))" \
			CXXFLAGS="$(CXXFLAGS) $(CXXFLAGS_CXX11)" \
			XML_CFLAGS="$(LIBXML_CFLAGS)" \
			XML_LIBS="$(LIBXML_LIBS)" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/src/lib/.libs/libetonyek-0.1.1.dylib \
		) \
	)

# vim: set noet sw=4 ts=4:
