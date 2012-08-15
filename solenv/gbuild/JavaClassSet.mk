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

gb_JavaClassSet_JAVACCOMMAND := $(JAVACOMPILER) $(JAVAFLAGS) \
    -source $(JAVA_SOURCE_VER) -target $(JAVA_TARGET_VER)
gb_JavaClassSet_JAVACDEBUG :=

# Enforces correct dependency order for possibly generated stuff:
# generated sources, jars/classdirs etc.
gb_JavaClassSet_get_preparation_target = $(WORKDIR)/JavaClassSet/$(1).prepared

ifneq ($(gb_DEBUGLEVEL),0)
gb_JavaClassSet_JAVACDEBUG := -g
endif

define gb_JavaClassSet__command
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(if $(filter-out $(JARDEPS),$(4)), \
		rm -rf $(call gb_JavaClassSet_get_classdir,$(2))/* && \
		RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,\
			$(filter-out $(JARDEPS),$(4))) && \
		$(if $(3),$(gb_JavaClassSet_JAVACCOMMAND) \
			$(gb_JavaClassSet_JAVACDEBUG) \
			-cp "$(T_CP)$(gb_CLASSPATHSEP)$(call gb_JavaClassSet_get_classdir,$(2))" \
			-d $(call gb_JavaClassSet_get_classdir,$(2)) \
			@$$RESPONSEFILE &&) \
		rm -f $$RESPONSEFILE &&) \
	touch $(1))

endef

$(call gb_JavaClassSet_get_target,%) :
	$(call gb_Output_announce,$*,$(true),JCS,3)
	$(call gb_JavaClassSet__command,$@,$*,$?,$^)

$(call gb_JavaClassSet_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),JCS,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(dir $(call gb_JavaClassSet_get_target,$*))) \
			$(call gb_JavaClassSet_get_preparation_target,$*)

$(call gb_JavaClassSet_get_preparation_target,%) :
	mkdir -p $(dir $@) && touch $@

define gb_JavaClassSet_JavaClassSet
$(call gb_JavaClassSet_get_target,$(1)) : $(call gb_JavaClassSet_get_preparation_target,$(1))
$(call gb_JavaClassSet_get_target,$(1)) : JARDEPS := $(call gb_JavaClassSet_get_preparation_target,$(1))

endef

define gb_JavaClassSet__get_sourcefile
$(SRCDIR)/$(1).java
endef

define gb_JavaClassSet__get_generated_sourcefile
$(WORKDIR)/$(1).java
endef

define gb_JavaClassSet_add_sourcefile
$(call gb_JavaClassSet_get_target,$(1)) : $(call gb_JavaClassSet__get_sourcefile,$(2))

endef

define gb_JavaClassSet_add_sourcefiles
$(foreach sourcefile,$(2),$(call gb_JavaClassSet_add_sourcefile,$(1),$(sourcefile)))

endef

define gb_JavaClassSet_add_generated_sourcefile
$(call gb_JavaClassSet_get_target,$(1)) : $(call gb_JavaClassSet__get_generated_sourcefile,$(2))
$(call gb_JavaClassSet__get_generated_sourcefile,$(2)) :| $(call gb_JavaClassSet_get_preparation_target,$(1))

endef

define gb_JavaClassSet_add_generated_sourcefiles
$(foreach sourcefile,$(2),$(call gb_JavaClassSet_add_generated_sourcefile,$(1),$(sourcefile)))

endef

define gb_JavaClassSet_set_classpath
$$(call gb_Output_error,\
 gb_JavaClassSet_set_classpath: use gb_JavaClassSet_add_classpath instead.)
endef

define gb_JavaClassSet_add_classpath
$(call gb_JavaClassSet_get_target,$(1)) : T_CP := $$(if $$(T_CP),$$(T_CP)$$(gb_CLASSPATHSEP))$(strip $(2))

endef

define gb_JavaClassSet_add_jar
$$(call gb_Output_error,\
 gb_JavaClassSet_add_jar: use gb_JavaClassSet_use_jar instead.)
endef

# problem: currently we can't get these dependencies to work
# build order dependency is a hack to get these prerequisites out of the way in the build command
define gb_JavaClassSet_use_jar
$(call gb_JavaClassSet_get_target,$(1)) : $(2)
$(call gb_JavaClassSet_get_target,$(1)) : JARDEPS += $(2)
$(call gb_JavaClassSet_add_classpath,$(1),$(2))
$(2) :| $(gb_Helper_PHONY)

endef

# this does not generate dependency on the jar
define gb_JavaClassSet_add_system_jar
$$(call gb_Output_error,\
 gb_JavaClassSet_add_system_jar: use gb_JavaClassSet_use_system_jar instead.)
endef

define gb_JavaClassSet_use_system_jar
$(call gb_JavaClassSet_add_classpath,$(1),$(2))

endef

define gb_JavaClassSet_add_jars
$$(call gb_Output_error,\
 gb_JavaClassSet_add_jars: use gb_JavaClassSet_use_jars instead.)
endef

define gb_JavaClassSet_use_jars
$(foreach jar,$(2),$(call gb_JavaClassSet_use_jar,$(1),$(jar)))

endef

define gb_JavaClassSet_add_system_jars
$$(call gb_Output_error,\
 gb_JavaClassSet_add_system_jars: use gb_JavaClassSet_use_system_jars instead.)
endef

define gb_JavaClassSet_use_system_jars
$(foreach jar,$(2),$(call gb_JavaClassSet_use_system_jar,$(1),$(jar)))

endef

define gb_JavaClassSet_add_jar_classset
$$(call gb_Output_error,\
 gb_JavaClassSet_add_jar_classset: use gb_JavaClassSet_use_jar_classset instead.)
endef

# gb_JavaClassSet_use_jar_classset: Like gb_JavaClassSet_use_jar, but instead of
# using the jar, use the directory tree with the class files that make up the
# jar.  This is sometimes necessary in JunitTests that have test classes in
# packages that belong to a sealed jar.
# $1: token identifying this JavaClassSet
# $2: token identifying the Jar being used
define gb_JavaClassSet_use_jar_classset
$(call gb_JavaClassSet_get_target,$(1)) : $(call gb_JavaClassSet_get_target,$(call gb_Jar_get_classsetname,$(2)))
$(call gb_JavaClassSet_get_target,$(1)) : JARDEPS += $(call gb_JavaClassSet_get_target,$(call gb_Jar_get_classsetname,$(2)))
$(call gb_JavaClassSet_add_classpath,$(1),$(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,$(2))))

endef

define gb_JavaClassSet_use_external
$$(call gb_Output_error,\
 gb_JavaClassSet_use_external: use gb_Jar_use_external instead.)
endef

define gb_JavaClassSet_use_externals
$(foreach external,$(2),$(call gb_JavaClassSet_use_external,$(1),$(external)))

endef

define gb_JavaClassSet_add_customtarget_dependency
$$(call gb_Output_error,\
 gb_JavaClassSet_add_customtarget_dependency: use gb_JavaClassSet_use_customtarget instead.)
endef

define gb_JavaClassSet_use_customtarget
$(call gb_JavaClassSet_get_preparation_target,$(1)) : \
	$(call gb_CustomTarget_get_target,$(2))

endef

# vim: set noet sw=4:
