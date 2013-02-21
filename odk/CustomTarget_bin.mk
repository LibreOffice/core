# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/bin))

odk_EXELIST := cppumaker \
	regcompare \
	idlc \
	javamaker \
	autodoc \
	unoapploader \
	uno-skeletonmaker \
	$(if $(SYSTEM_UCPP),,ucpp) \
	$(if $(filter WNT,$(OS)),climaker)

define odk_exe
odkcommon_ZIPLIST += bin/$(1)$(gb_Executable_EXT)
$(call gb_CustomTarget_get_target,odk/odkcommon/bin): $(odk_WORKDIR)/bin/$(1)$(gb_Executable_EXT)
$(odk_WORKDIR)/bin/$(1)$(gb_Executable_EXT): $(call gb_Executable_get_target,$(1))
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach exe,$(odk_EXELIST),$(eval $(call odk_exe,$(exe))))

ifeq ($(OS),MACOSX)
odkcommon_ZIPLIST += bin/addsym-macosx.sh
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/bin,addsym-macosx.sh))
$(odk_WORKDIR)/bin/addsym-macosx.sh: $(SRCDIR)/odk/pack/copying/addsym-macosx.sh
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp $< $@
	chmod 755 $@
endif

# vim: set noet sw=4 ts=4:
