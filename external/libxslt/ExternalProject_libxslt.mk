# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libxslt))

$(eval $(call gb_ExternalProject_use_external,libxslt,libxml2))

$(eval $(call gb_ExternalProject_register_targets,libxslt,\
	build \
))
ifeq ($(OS),WNT)
$(eval $(call gb_ExternalProject_use_nmake,libxslt,build))

$(call gb_ExternalProject_get_state_target,libxslt,build):
	$(call gb_Trace_StartRange,libxslt,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		cscript /e:javascript configure.js \
			$(if $(MSVC_USE_DEBUG_RUNTIME),cruntime=/MDd) \
			$(if $(filter TRUE,$(ENABLE_DBGUTIL)),debug=yes) \
			vcmanifest=yes \
			lib=$(gb_UnpackedTarball_workdir)/libxml2/win32/bin.msvc \
		&& nmake \
	,win32)
	$(call gb_Trace_EndRange,libxslt,EXTERNAL)
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,libxslt,build):
	$(call gb_Trace_StartRange,libxslt,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --without-crypto --without-python \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			LDFLAGS="$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN" -Wl$(COMMA)-noinhibit-exec) \
			$(if $(SYSBASE),$(if $(filter SOLARIS LINUX,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
			$(if $(SYSBASE),CPPFLAGS="-I$(SYSBASE)/usr/include") \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)), \
			$(if $(filter iOS,$(OS)),LIBS="-liconv") \
			--disable-shared,--disable-static) \
			$(if $(SYSTEM_LIBXML),,--with-libxml-src=$(gb_UnpackedTarball_workdir)/libxml2) \
		&& chmod 777 xslt-config \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/libxslt/.libs/libxslt.1.dylib \
		) \
	)
	$(call gb_Trace_EndRange,libxslt,EXTERNAL)
endif

# vim: set noet sw=4 ts=4:
