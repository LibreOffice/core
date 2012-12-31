# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,external/wine))

$(eval $(call gb_CustomTarget_register_targets,external/wine,\
	libgdiplus.dll.a \
	libmsi.dll.a \
	liburlmod.dll.a \
))

$(call gb_CustomTarget_get_workdir,external/wine)/lib%.dll.a : $(SRCDIR)/external/wine/lib/%.def
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),DLT,1)
	$(DLLTOOL) --kill-at --input-def=$< --output-lib=$@ --dllname=$*.dll

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
