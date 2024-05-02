# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/allheaders))

odk_allheaders_DIR := $(gb_CustomTarget_workdir)/odk/allheaders

$(call gb_CustomTarget_get_target,odk/allheaders) : \
	$(odk_allheaders_DIR)/allheaders.hxx

define odk_genincludesheader
// Generated list of sal includes
#ifdef _WIN32
#include <windows.h>
#endif

endef

define odk_geninclude
$(if $(2),#ifdef _WIN32)
#include <$(subst $(INSTDIR)/$(SDKDIRNAME)/include/,,$(1))>
$(if $(2),#endif)

endef

$(odk_allheaders_DIR)/allheaders.hxx : \
			  $(call gb_Package_get_target,odk_headers) \
			  $(call gb_Package_get_target,odk_headers_generated) \
            | $(odk_allheaders_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	$(file >$@,\
		$(call odk_genincludesheader) \
		$(foreach file,$(shell cat $^),$(call odk_geninclude,$(file),$(findstring /win32/,$(file)))))
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

# vim: set noet sw=4 ts=4:
