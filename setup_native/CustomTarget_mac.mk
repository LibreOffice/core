# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,setup_native/mac))

$(eval $(call gb_CustomTarget_register_target,setup_native/mac,macinstall.ulf))

#FIXME: generalize rule? ripped off from ScpMergeTarget
ifneq ($(WITH_LANG),)
$(call gb_CustomTarget_get_workdir,setup_native/mac)/macinstall.ulf: $(SRCDIR)/setup_native/source/mac/macinstall.ulf | $(call gb_Executable_get_runtime_dependencies,ulfex)
	$(call gb_Output_announce,$@,$(true),SUM,1)
	MERGEINPUT=`$(gb_MKTEMP)` && \
	echo $(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/setup_native/source/mac.po) > $${MERGEINPUT} && \
	$(call gb_Helper_abbreviate_dirs,\
	$(call gb_Executable_get_command,ulfex) -i $< -o $@ -m $${MERGEINPUT} -l all ) && \
	rm -rf $${MERGEINPUT}
else
$(call gb_CustomTarget_get_workdir,setup_native/mac)/macinstall.ulf: $(SRCDIR)/setup_native/source/mac/macinstall.ulf
	cp $< $@
endif

# vim: set noet sw=4 ts=4:
