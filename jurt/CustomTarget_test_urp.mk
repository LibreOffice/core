# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,jurt/test/com/sun/star/lib/uno/protocols/urp))

jurt_TESTURP := $(call gb_CustomTarget_get_workdir,jurt/test/com/sun/star/lib/uno/protocols/urp)

$(call gb_CustomTarget_get_target,jurt/test/com/sun/star/lib/uno/protocols/urp) : $(jurt_TESTURP)/done

$(jurt_TESTURP)/done : \
		$(call gb_UnoApiTarget_get_target,test_urp) \
		$(OUTDIR)/bin/udkapi.rdb \
		$(call gb_Executable_get_runtime_dependencies,javamaker)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JVM,1)
	rm -rf $(jurt_TESTURP) && \
	$(call gb_Helper_execute,javamaker -O$(jurt_TESTURP) -nD $< -X$(OUTDIR)/bin/udkapi.rdb) && \
	touch $@

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
