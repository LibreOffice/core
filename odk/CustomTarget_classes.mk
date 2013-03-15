# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/classes))

$(eval $(call gb_JavaClassSet_JavaClassSet,loader))

$(eval $(call gb_JavaClassSet_add_sourcefiles,loader,\
	odk/source/com/sun/star/lib/loader/Loader \
	odk/source/com/sun/star/lib/loader/InstallationFinder \
	odk/source/com/sun/star/lib/loader/WinRegKey \
	odk/source/com/sun/star/lib/loader/WinRegKeyException \
))

odkcommon_ZIPLIST += classes/com/sun/star/lib/loader
odkcommon_ZIPDEPS += $(odk_WORKDIR)/classes/java.done

$(call gb_CustomTarget_get_target,odk/odkcommon/classes): $(odk_WORKDIR)/classes/java.done
$(odk_WORKDIR)/classes/java.done: $(call gb_JavaClassSet_get_target,loader)
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp -a $(call gb_JavaClassSet_get_classdir,loader)/com $(dir $@)
	touch $@

odkcommon_ZIPLIST += classes/win/unowinreg.dll
$(eval $(call gb_CustomTarget_register_target,odk/odkcommon/classes,win/unowinreg.dll))
$(odk_WORKDIR)/classes/win/unowinreg.dll: $(call gb_CustomTarget_get_workdir,odk/unowinreg)/unowinreg.dll
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp $< $@

# vim: set noet sw=4 ts=4:
