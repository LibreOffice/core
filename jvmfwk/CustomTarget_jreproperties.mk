# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,jvmfwk/jreproperties))

$(call gb_CustomTarget_get_target,jvmfwk/jreproperties) : $(call gb_CustomTarget_get_workdir,jvmfwk/jreproperties)/JREProperties.class

$(call gb_CustomTarget_get_workdir,jvmfwk/jreproperties)/JREProperties.class : \
		$(SRCDIR)/jvmfwk/plugins/sunmajor/pluginlib/JREProperties.java \
		| $(call gb_CustomTarget_get_workdir,jvmfwk/jreproperties)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JCS,1)
	$(call gb_Helper_abbreviate_dirs, \
	cd $(dir $@) && $(gb_JavaClassSet_JAVACCOMMAND) $(gb_JavaClassSet_JAVACDEBUG) -d $(dir $@) $^)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
