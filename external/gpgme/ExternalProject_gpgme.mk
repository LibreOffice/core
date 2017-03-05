# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,gpgme))

$(eval $(call gb_ExternalProject_register_targets,gpgme,\
	build \
))

$(eval $(call gb_ExternalProject_use_autoconf,gpgme,build))

$(eval $(call gb_ExternalProject_use_externals,gpgme,\
       libgpg-error \
       libassuan \
))

$(call gb_ExternalProject_get_state_target,gpgme,build):
	$(call gb_ExternalProject_run,build,\
		autoreconf \
		&& ./configure \
		   --enable-languages="cl cpp" \
		   GPG_ERROR_CFLAGS="$(GPG_ERROR_CFLAGS)" \
		   GPG_ERROR_LIBS="$(GPG_ERROR_LIBS)" \
		   LIBASSUAN_CFLAGS="$(LIBASSUAN_CFLAGS)" \
		   LIBASSUAN_LIBS="$(LIBASSUAN_LIBS)" \
		   $(if $(filter LINUX,$(OS)), \
				'LDFLAGS=-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN') \
		   $(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	  && $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
