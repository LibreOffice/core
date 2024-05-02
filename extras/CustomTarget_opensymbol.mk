# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/fonts))

$(eval $(call gb_CustomTarget_register_targets,extras/fonts,opens___.ttf))

ifneq (,$(FONTFORGE))
$(gb_CustomTarget_workdir)/extras/fonts/opens___.ttf : \
		$(SRCDIR)/extras/source/truetype/symbol/OpenSymbol.sfd
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),FNT,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),FNT)
	$(FONTFORGE) -lang=ff -c 'Open($$1); Generate($$2)' $< $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),FNT)
else
$(gb_CustomTarget_workdir)/extras/fonts/opens___.ttf : \
		$(TARFILE_LOCATION)/$(OPENSYMBOL_TTF)
	cp $< $@
endif
