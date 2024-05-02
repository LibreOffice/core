# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,ridljar/test/com/sun/star/lib/uno/protocols/urp))

ridljar_TESTURP := $(gb_CustomTarget_workdir)/ridljar/test/com/sun/star/lib/uno/protocols/urp

$(call gb_CustomTarget_get_target,ridljar/test/com/sun/star/lib/uno/protocols/urp) : $(ridljar_TESTURP)/done

$(ridljar_TESTURP)/done : \
		$(call gb_UnoApiTarget_get_target,test_urp) \
		$(call gb_UnoApi_get_target,udkapi) \
		$(call gb_Executable_get_runtime_dependencies,javamaker)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JVM,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),JVM)
	rm -rf $(ridljar_TESTURP) && \
	$(call gb_Helper_execute,javamaker -O$(ridljar_TESTURP) -nD $< \
		-X$(call gb_UnoApi_get_target,udkapi)) && \
	touch $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),JVM)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
