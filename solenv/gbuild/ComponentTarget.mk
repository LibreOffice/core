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

gb_ComponentTarget_XLSTCOMMAND := xslt
gb_ComponentTarget_XLSTCOMMANDFILE := $(SOLARENV)/bin/createcomponent.xslt
gb_ComponentTarget_get_source = $(1)/$(2).component

# gb_ComponentTarget_PREFIXBASISNATIVE is set by the platform

define gb_ComponentTarget__command
$(call gb_Helper_announce,Processing $(2) ...)
$(call gb_Helper_abbreviate_dirs_native,\
    mkdir -p $(dir $(1)) && \
    $(gb_ComponentTarget_XLSTCOMMAND) --nonet --stringparam uri \
        '$(gb_ComponentTarget_PREFIXBASISNATIVE)$(COMPONENTNAME)' -o $(1) \
        $(gb_ComponentTarget_XLSTCOMMANDFILE) $(2))

endef

define gb_ComponentTarget__rules
$$(call gb_ComponentTarget_get_target,%) : $$(call gb_ComponentTarget_get_source,$(1),%)
    $$(call gb_ComponentTarget__command,$$@,$$<)

endef

$(foreach repo,$(gb_ComponentTarget_REPOS),$(eval $(call gb_ComponentTarget__rules,$(repo))))

$(call gb_ComponentTarget_get_target,%) :
    $(error unable to find component file $(call gb_ComponentTarget_get_source,,$*) in the repositories: $(gb_ComponentTarget_REPOS))

define gb_ComponentTarget_ComponentTarget
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTNAME := $(2)

endef

# vim: set noet sw=4 ts=4:
