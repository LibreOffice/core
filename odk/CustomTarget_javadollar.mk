# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

#FIXME: gb_Zip really dosen't like $ signs

$(eval $(call gb_CustomTarget_CustomTarget,odk/javadollar))

odk_DOLLARCLASSES := \
	$(call gb_JavaClassSet_get_classdir,loader)/com/sun/star/lib/loader/Loader$$CustomURLClassLoader.class \
	$(call gb_JavaClassSet_get_classdir,loader)/com/sun/star/lib/loader/Loader$$Drain.class \
	$(call gb_JavaClassSet_get_classdir,loader)/com/sun/star/lib/loader/InstallationFinder$$StreamGobbler.class

$(call gb_CustomTarget_get_target,odk/javadollar): $(call gb_JavaClassSet_get_target,loader) $(call gb_Zip_get_target,odkcommon) $(call gb_Zip_get_target,uno_loader_classes)
	$(foreach x,$(odk_DOLLARCLASSES),\
		cp '$(x)' $(odk_WORKDIR)/classes/com/sun/star/lib/loader/;)
	$(foreach x,$(odk_DOLLARCLASSES),\
		cd $(odk_WORKDIR) && zip -q $(call gb_Zip_get_target,odkcommon) \
		'classes/com/sun/star/lib/loader/$(notdir $(x))';)
	$(foreach x,$(odk_DOLLARCLASSES),\
		cd $(odk_WORKDIR) && zip -q $(call gb_Zip_get_target,uno_loader_classes) \
		'classes/com/sun/star/lib/loader/$(notdir $(x))';)
	touch $@


# vim: set noet sw=4 ts=4:
