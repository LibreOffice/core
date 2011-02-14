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

gb_CustomTarget_REPOSITORYNAMES := $(gb_Helper_REPOSITORYNAMES)

# N.B.: putting the "+" there at the start activates the GNU make job server
define gb_CustomTarget__command
+$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(call gb_CustomTarget_get_workdir,$(2)) && \
    $(MAKE) -C $(call gb_CustomTarget_get_workdir,$(2)) -f $< && \
    touch $(1))

endef

define gb_CustomTarget__rules
$$(call gb_CustomTarget_get_repo_target,$(1),%) :
    $$(call gb_Output_announce,$$*,$$(true),MAK,3)
    $$(call gb_CustomTarget__command,$$@,$$*)

$$(call gb_CustomTarget_get_target,%) : $$(call gb_CustomTarget_get_repo_target,$(1),%)
    $$(call gb_Helper_abbreviate_dirs,\
        touch $$@)

endef

.PHONY: $(call gb_CustomTarget_get_clean_target,%)
$(call gb_CustomTarget_get_clean_target,%) :
    $(call gb_Output_announce,$*,$(false),MAK,3)
    $(call gb_Helper_abbreviate_dirs,\
        rm -rf $(call gb_CustomTarget_get_workdir,$*) && \
        rm -f $(call gb_CustomTarget_get_target,$*) \
            $(foreach reponame,$(gb_CustomTarget_REPOSITORYNAMES),$(call gb_CustomTarget_get_repo_target,$(reponame),$*)))


$(foreach reponame,$(gb_CustomTarget_REPOSITORYNAMES),$(eval $(call gb_CustomTarget__rules,$(reponame))))

define gb_CustomTarget__get_makefile
$(1)/$(2)/Makefile
endef

define gb_CustomTarget_CustomTarget
$(foreach reponame,$(gb_CustomTarget_REPOSITORYNAMES),\
    $(eval $(call gb_CustomTarget_get_repo_target,$(reponame),$(1)) : $(call gb_CustomTarget__get_makefile,$($(reponame)),$(1))))

$(call gb_CustomTarget_get_workdir,$(1))/% : $(call gb_CustomTarget_get_target,$(1))

endef


define gb_CustomTarget_add_dependency
$(foreach reponame,$(gb_CustomTarget_REPOSITORYNAMES),\
    $(eval $(call gb_CustomTarget_get_repo_target,$(reponame),$(1)) : $($(reponame))/$(2)))

endef

define gb_CustomTarget_add_dependencies
$(foreach dependency,$(2),$(call gb_CustomTarget_add_dependency,$(1),$(dependency)))

endef

define gb_CustomTarget_add_outdir_dependency
$(foreach reponame,$(gb_CustomTarget_REPOSITORYNAMES),\
    $(eval $(call gb_CustomTarget_get_repo_target,$(reponame),$(1)) : $(2)))

endef

define gb_CustomTarget_add_outdir_dependencies
$(foreach dependency,$(2),$(call gb_CustomTarget_add_outdir_dependency,$(1),$(dependency)))

endef

# vim: set noet sw=4 ts=4:
