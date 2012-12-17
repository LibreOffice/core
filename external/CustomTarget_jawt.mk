# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,external/jawt))

$(eval $(call gb_CustomTarget_register_target,external/jawt,libjawt.dll.a))

$(call gb_CustomTarget_get_workdir,external/jawt)/jawt.def:| \
	$(call gb_CustomTarget_get_workdir,external/jawt)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo EXPORTS > $@
	echo "JAWT_GetAWT@8" >> $@

$(call gb_CustomTarget_get_target,external/jawt)/libjawt.dll.a: \
	$(call gb_CustomTarget_get_workdir,external/jawt)/jawt.def
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),DLT,1)
	$(DLLTOOL) --input-def=$< --output-lib=$@ --dllname=jawt.dll
	
# vim:set shiftwidth=4 tabstop=4 noexpandtab:
