# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libebook))

$(eval $(call gb_ExternalProject_use_autoconf,libebook,build))

$(eval $(call gb_ExternalProject_register_targets,libebook,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libebook,\
	boost_headers \
	icu \
	liblangtag \
	libxml2 \
	revenge \
	zlib \
))

$(call gb_ExternalProject_get_state_target,libebook,build) :
	$(call gb_Trace_StartRange,libebook,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--without-tools \
			--disable-tests \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			--disable-werror \
			--disable-weffc \
			CXXFLAGS="$(gb_CXXFLAGS) $(call gb_ExternalProject_get_build_flags,libebook)" \
			CPPFLAGS="$(CPPFLAGS) $(ICU_UCHAR_TYPE) $(BOOST_CPPFLAGS)" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,libebook)" \
			LANGTAG_CFLAGS="$(LIBLANGTAG_CFLAGS)" \
			LANGTAG_LIBS="$(LIBLANGTAG_LIBS)" \
			XML_CFLAGS="$(LIBXML_CFLAGS)" \
			XML_LIBS="$(LIBXML_LIBS)" \
			$(gb_CONFIGURE_PLATFORMS) \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libebook,EXTERNAL)

# vim: set noet sw=4 ts=4:
