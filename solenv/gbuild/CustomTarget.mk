# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# the .dir is for make 3.81, which ignores trailing /
$(call gb_CustomTarget_get_workdir,%)/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CustomTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),CUS,3)
	touch $@

.PHONY: $(call gb_CustomTarget_get_clean_target,%)
$(call gb_CustomTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CUS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_CustomTarget_get_workdir,$*) && \
		rm -f $(call gb_CustomTarget_get_target,$*))

define gb_CustomTarget_CustomTarget
$(eval $(call gb_Module_register_target,$(call gb_CustomTarget_get_target,$(1)),$(call gb_CustomTarget_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),CustomTarget)
$(call gb_CustomTarget_get_target,$(1)) :| $(dir $(call gb_CustomTarget_get_target,$(1))).dir

endef

define gb_CustomTarget_register_target
$(call gb_CustomTarget_get_target,$(1)) : $(call gb_CustomTarget_get_workdir,$(1))/$(2)
$(call gb_CustomTarget_get_workdir,$(1))/$(2) :| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define gb_CustomTarget_register_targets
$(foreach target,$(2),$(call gb_CustomTarget_register_target,$(1),$(target)))

endef

ifneq ($(WITH_LANG),)

# $(call gb_CustomTarget_ulfex__command,ulftarget,ulfsource,pofiles)
define gb_CustomTarget_ulfex__command
$(call gb_Output_announce,$(1),$(true),ULF,1)
MERGEINPUT=`$(gb_MKTEMP)` && \
echo $(3) > $${MERGEINPUT} && \
$(call gb_Helper_abbreviate_dirs,\
	$(call gb_Executable_get_command,ulfex) -i $(2) -o $(1) -m $${MERGEINPUT} -l all) && \
rm -rf $${MERGEINPUT}
endef

else

define gb_CustomTarget_ulfex__command
cp $(2) $(1)
endef

endif

# $(call gb_CustomTarget_ulfex_rule,ulftargetpattern,ulfsource,pofiles)
define gb_CustomTarget_ulfex_rule
$(1) : $(2) $(if $(WITH_LANG),$(call gb_Executable_get_runtime_dependencies,ulfex)) | $(dir $(1)).dir
	$$(call gb_CustomTarget_ulfex__command,$$@,$(subst %,$$*,$(strip $(2))),$(strip $(3)))

endef

# vim: set noet sw=4:
