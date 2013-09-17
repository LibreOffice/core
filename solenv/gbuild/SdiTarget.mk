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

# SdiTarget is evil, in that it does not support multiple repositories for now (hardcoded to SRCDIR)
# Also there is no way to cleanly deliver the generated header to OUTDIR.
# (This can be workarounded by using gb_Package, but really should not.)

# SdiTarget class
gb_SdiTarget_SVIDLDEPS := $(call gb_Executable_get_runtime_dependencies,svidl)
gb_SdiTarget_SVIDLCOMMAND := $(call gb_Executable_get_command,svidl)

$(call gb_SdiTarget_get_target,%) : $(SRCDIR)/%.sdi $(gb_SdiTarget_SVIDLDEPS)
	$(call gb_Output_announce,$*,$(true),SDI,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@))
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(gb_SdiTarget_SVIDLCOMMAND) -quiet \
			$(INCLUDE) \
			-fs$@.hxx \
			-fd$@.ilb \
			-fl$@.lst \
			-fz$@.sid \
			-fx$(EXPORTS) \
			-fm$@ \
			$(if $(gb_FULLDEPS),-fM$(call gb_SdiTarget_get_dep_target,$*)) \
			$<)

# rule necessary to rebuild cxx files that include the header
$(call gb_SdiTarget_get_target,%.hxx) : $(call gb_SdiTarget_get_target,%)
	@true

ifeq ($(gb_FULLDEPS),$(true))
$(dir $(call gb_SdiTarget_get_dep_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_SdiTarget_get_dep_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_SdiTarget_get_dep_target,%) :
	$(if $(wildcard $@),touch $@)
endif

.PHONY : $(call gb_SdiTarget_get_clean_target,%)
$(call gb_SdiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SDI,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(foreach ext,.hxx .ilb .lst .sid,\
			$(call gb_SdiTarget_get_target,$*)$(ext)) \
			$(call gb_SdiTarget_get_dep_target,$*) \
			$(call gb_SdiTarget_get_target,$*))

define gb_SdiTarget_SdiTarget
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := $$(subst -I. ,-I$$(dir $(SRCDIR)/$(1)) ,$$(SOLARINC))
$(call gb_SdiTarget_get_target,$(1)) : EXPORTS := $(SRCDIR)/$(2).sdi
ifeq ($(gb_FULLDEPS),$(true))
-include $(call gb_SdiTarget_get_dep_target,$(1))
$(call gb_SdiTarget_get_dep_target,$(1)) :| $(dir $(call gb_SdiTarget_get_dep_target,$(1))).dir
endif
endef

define gb_SdiTarget_set_include
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := $(2)

endef

# vim: set noet sw=4:
