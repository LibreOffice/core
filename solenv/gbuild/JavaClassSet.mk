# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

gb_JavaClassSet_JAVACCOMMAND := $(JAVACOMPILER) $(JAVAFLAGS)
gb_JavaClassSet_JAVACDEBUG :=

ifneq ($(gb_DEBUGLEVEL),0)
gb_JavaClassSet_JAVACDEBUG := -g
endif

define gb_JavaClassSet__command
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,\
		$(call gb_Helper_convert_native,\
		$(filter-out $(JARDEPS),$(4)))) && \
	$(if $(3),$(gb_JavaClassSet_JAVACCOMMAND) \
		$(gb_JavaClassSet_JAVACDEBUG) \
		-cp "$(T_CP)$(gb_CLASSPATHSEP)$(call gb_JavaClassSet_get_classdir,$(2))" \
		-d $(call gb_JavaClassSet_get_classdir,$(2)) \
		@$$RESPONSEFILE &&) \
	rm -f $$RESPONSEFILE && \
	touch $(1))

endef

$(call gb_JavaClassSet_get_target,%) :
	$(call gb_Output_announce,$*,$(true),JCS,3)
	$(call gb_JavaClassSet__command,$@,$*,$?,$^)

$(call gb_JavaClassSet_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),JCS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(dir $(call gb_JavaClassSet_get_target,$*)))

# UGLY: cannot use target local variable for REPO because it's needed in prereq
# No idea what above comment means, and whether still relevant
define gb_JavaClassSet_JavaClassSet
$(call gb_JavaClassSet_get_target,$(1)) : JARDEPS :=
endef

define gb_JavaClassSet__get_sourcefile
$(SRCDIR)/$(1).java
endef

define gb_JavaClassSet_add_sourcefile
$(eval $(call gb_JavaClassSet_get_target,$(1)) : \
   $(call gb_JavaClassSet__get_sourcefile,$(2)))
endef

define gb_JavaClassSet_add_sourcefiles
$(foreach sourcefile,$(2),$(call gb_JavaClassSet_add_sourcefile,$(1),$(sourcefile)))

endef

define gb_JavaClassSet_set_classpath
$(eval $(call gb_JavaClassSet_get_target,$(1)) : T_CP := $(2))

endef

# problem: currently we can't get these dependencies to work
# build order dependency is a hack to get these prerequisites out of the way in the build command
define gb_JavaClassSet_add_jar
$(eval $(call gb_JavaClassSet_get_target,$(1)) : $(2))
$(eval $(call gb_JavaClassSet_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(strip $(2)))
$(eval $(call gb_JavaClassSet_get_target,$(1)) : JARDEPS += $(2))
endef

# this does not generate dependency on the jar
define gb_JavaClassSet_add_system_jar
$(eval $(call gb_JavaClassSet_get_target,$(1)) : T_CP := $$(T_CP)$$(gb_CLASSPATHSEP)$(strip $(2)))
$(eval $(call gb_JavaClassSet_get_target,$(1)) : JARDEPS += $(2))
endef

define gb_JavaClassSet_add_jars
$(foreach jar,$(2),$(call gb_JavaClassSet_add_jar,$(1),$(jar)))
endef

define gb_JavaClassSet_add_system_jars
$(foreach jar,$(2),$(call gb_JavaClassSet_add_system_jar,$(1),$(jar)))
endef

# this forwards to functions that must be defined in RepositoryExternal.mk.
# $(call gb_LinkTarget_use_external,library,external)
define gb_JavaClassSet_use_external
$(eval $(if $(value gb_JavaClassSet__use_$(2)),\
  $(call gb_JavaClassSet__use_$(2),$(1)),\
  $(error gb_JavaClassSet_use_external: unknown external: $(2))))
endef

define gb_JavaClassSet_use_externals
$(foreach external,$(2),$(call gb_JavaClassSet_use_external,$(1),$(external)))
endef

# vim: set noet sw=4:
