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

gb_JavaClassSet_REPOSITORYNAMES := $(gb_Helper_REPOSITORYNAMES)
gb_JavaClassSet_JAVACCOMMAND := $(JAVACOMPILER)

define gb_JavaClassSet__command
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(if $(3),$(gb_JavaClassSet_JAVACCOMMAND) -cp "$(CLASSPATH)" -d $(call gb_JavaClassSet_get_classdir,$(2)) $(3) &&) \
	touch $(1))

endef

define gb_JavaClassSet__rules
$$(call gb_JavaClassSet_get_repo_target,$(1),%) :
	$$(call gb_JavaClassSet__command,$$@,$$*,$$?)

$$(call gb_JavaClassSet_get_target,%) : $$(call gb_JavaClassSet_get_repo_target,$(1),%)
	$$(call gb_Output_announce,$$*,$$(true),JCS,3)
	$$(call gb_Helper_abbreviate_dirs,\
		touch $$@)

endef

$(call gb_JavaClassSet_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),JCS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(dir $(call gb_JavaClassSet_get_target,$*)))


$(foreach reponame,$(gb_JavaClassSet_REPOSITORYNAMES),$(eval $(call gb_JavaClassSet__rules,$(reponame))))

define gb_JavaClassSet_JavaClassSet
endef

define gb_JavaClassSet__get_sourcefile
$(1)/$(2).java
endef

define gb_JavaClassSet_add_sourcefile
$(foreach reponame,$(gb_JavaClassSet_REPOSITORYNAMES),\
	$(eval $(call gb_JavaClassSet_get_repo_target,$(reponame),$(1)) : $(call gb_JavaClassSet__get_sourcefile,$($(reponame)),$(2))))

endef

define gb_JavaClassSet_add_sourcefiles
$(foreach sourcefile,$(2),$(call gb_JavaClassSet_add_sourcefile,$(1),$(sourcefile)))

endef

define gb_JavaClassSet_set_classpath
$(call gb_JavaClassSet_get_target,$(1)) : CLASSPATH := $(2)

endef

# vim: set noet sw=4:
