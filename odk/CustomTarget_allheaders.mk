# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/allheaders))

odk_allheaders_DIR := $(call gb_CustomTarget_get_workdir,odk/allheaders)

$(call gb_CustomTarget_get_target,odk/allheaders) : \
	$(odk_allheaders_DIR)/allheaders.hxx

$(odk_allheaders_DIR)/allheaders.hxx : \
			  $(call gb_PackageSet_get_target,odk_headers) \
            | $(odk_allheaders_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	printf '// Generated list of sal includes\n' > $@ && \
	printf '#ifdef WNT\n' >> $@ && \
	printf '#include <windows.h>\n' >> $@ && \
	printf '#endif\n' >> $@ \
	$(foreach file,$(shell cat $^),\
		$(if $(findstring /win32/,$(file)),&& printf '#ifdef WNT\n' >> $@) \
	    && printf '#include <%s>\n' $(subst $(INSTDIR)/$(SDKDIRNAME)/include/,,$(file)) >> $@ \
		$(if $(findstring /win32/,$(file)),&& printf '#endif // WNT\n' >> $@) \
	)

# vim: set noet sw=4 ts=4:
