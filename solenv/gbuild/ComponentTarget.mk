#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

gb_ComponentTarget_REPOS := $(gb_REPOS)

gb_ComponentTarget_XSLTCOMMANDFILE := $(SOLARENV)/bin/createcomponent.xslt
gb_ComponentTarget_get_source = $(1)/$(2).component

define gb_ComponentTarget__command
$(call gb_Output_announce,$(3),$(true),CMP,1)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_XSLTPROC) --nonet --stringparam uri \
		'$(subst \d,$$,$(COMPONENTPREFIX))$(LIBFILENAME)' -o $(1) \
		$(gb_ComponentTarget_XSLTCOMMANDFILE) $(realpath $(2)))

endef

define gb_ComponentTarget__rules
$$(call gb_ComponentTarget_get_target,%) : $$(call gb_ComponentTarget_get_source,$(1),%) | $(gb_XSLTPROCTARGET)
	$$(call gb_ComponentTarget__command,$$@,$$<,$$*)

$$(call gb_ComponentTarget_get_clean_target,%) :
	$$(call gb_Output_announce,$$*,$(false),CMP,1)
	rm -f $$(call gb_ComponentTarget_get_outdir_target,$$*) $$(call gb_ComponentTarget_get_target,$$*)

endef

$(foreach repo,$(gb_ComponentTarget_REPOS),$(eval $(call gb_ComponentTarget__rules,$(repo))))

$(call gb_ComponentTarget_get_target,%) :
	$(eval $(call gb_Outpt_error,Unable to find component file $(call gb_ComponentTarget_get_source,,$*) in the repositories: $(gb_ComponentTarget_REPOS) or xlstproc is missing.))

$(call gb_ComponentTarget_get_external_target,%) :
	$(call gb_Deliver_deliver,$<,$@)

define gb_ComponentTarget_ComponentTarget
$(call gb_ComponentTarget_get_target,$(1)) : LIBFILENAME := $(3)
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTPREFIX := $(2)
$(call gb_ComponentTarget_get_outdir_target,$(1)) : $(call gb_ComponentTarget_get_target,$(1))
$(call gb_Deliver_add_deliverable,$(call gb_ComponentTarget_get_outdir_target,$(1)),$(call gb_ComponentTarget_get_target,$(1)))

endef

# vim: set noet sw=4:
