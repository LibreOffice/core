# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/classes))

$(eval $(call gb_CustomTarget_register_targets,odk/classes,\
	java.done \
))

$(eval $(call gb_JavaClassSet_JavaClassSet,loader))

$(eval $(call gb_JavaClassSet_add_sourcefiles,loader,\
	odk/source/com/sun/star/lib/loader/Loader \
	odk/source/com/sun/star/lib/loader/InstallationFinder \
	odk/source/com/sun/star/lib/loader/WinRegKey \
	odk/source/com/sun/star/lib/loader/WinRegKeyException \
))

$(gb_CustomTarget_workdir)/odk/classes/java.done: $(call gb_JavaClassSet_get_target,loader)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp -a $(call gb_JavaClassSet_get_classdir,loader)/com $(dir $@) && \
	touch $@

# vim: set noet sw=4 ts=4:
