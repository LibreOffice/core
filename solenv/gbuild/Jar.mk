# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
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

# if a jar prerequisite has changed, the ClassSet must be recompiled. How?

# Jar class

gb_Jar_JAVACOMMAND := $(JAVAINTERPRETER)
gb_Jar_JARCOMMAND := jar

# location of files going to be packed into .jar file
define gb_Jar_get_workdir
$(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,$(1)))
endef

# location of manifest file in workdir
define gb_Jar_get_manifest_target 
$(call gb_Jar_get_workdir,$(1))/META-INF/MANIFEST.MF
endef

# creates classset and META-INF folders if they don't exist
# adds manifest version, class path, solarversion and content from sources to manifest file 
# creates the target folder of the jar file if it doesn't exist
# creates the jar file
# jar program does not remove the target in case of error, so rm it manually
# XXX: PACKAGEDIRS need special treatment, because sometimes we need to
# add into the jar another class hierarchy created outside of our class
# set (e.g., by javamaker). Because jar does not allow two same root dirs
# when creating the archive, we work around this deficiency by creating
# the archive with the main class hierarchy and then updating it from
# the other one(s), which seems to work .-)
define gb_Jar__command
	$(call gb_Output_announce,$*,$(true),JAR,3)
	$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(call gb_Jar_get_workdir,$(1))/META-INF && \
	echo Manifest-Version: 1.0 > $(call gb_Jar_get_manifest_target,$(1)) && \
	$(if $(JARCLASSPATH),echo "Class-Path: $(strip $(JARCLASSPATH))" >> $(call gb_Jar_get_manifest_target,$(1)) &&) \
	echo "Solar-Version: $(RSCREVISION)" >> $(call gb_Jar_get_manifest_target,$(1)) && \
	cat $(if $(MANIFEST),$(MANIFEST),$(gb_Helper_MISCDUMMY)) >> $(call gb_Jar_get_manifest_target,$(1)) && \
	mkdir -p $(dir $(2)) && cd $(call gb_Jar_get_workdir,$(1)) && \
	$(gb_Jar_JARCOMMAND) cfm $(2) $(call gb_Jar_get_manifest_target,$(1)) \
		META-INF $(PACKAGEROOTS) $(PACKAGEFILES) \
	$(foreach root,$(PACKAGEDIRS),&& cd $(dir $(root)) && $(gb_Jar_JARCOMMAND) uf $(2) $(notdir $(root))) \
	|| (rm $(2); false) )
endef

# clean target reuses clean target of ClassSet
$(call gb_Jar_get_clean_target,%) : $(call gb_JavaClassSet_get_clean_target,$(call gb_Jar_get_classsetname,%))
	$(call gb_Output_announce,$*,$(false),JAR,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Jar_get_target,$*) $(call gb_Jar_get_outdir_target,$*))

# the outdir target depends on the workdir target and is built by delivering the latter
# the workdir target is created by cd'ing to the target directory and adding/updating the files

# rule for creating the jar file using the command defined above
$(call gb_Jar_get_target,%) : $(call gb_JavaClassSet_get_target,$(call gb_Jar_get_classsetname,%))
	$(call gb_Jar__command,$*,$@,$*,$?)

# resets scoped variables (see explanations where they are set)
# creates a class set and a dependency to it 
# registers target and clean target
# adds jar files to DeliverLogTarget
# adds dependency for outdir target to workdir target (pattern rule for delivery is in Package.mk)
define gb_Jar_Jar
$(call gb_Jar_get_target,$(1)) : MANIFEST :=
$(call gb_Jar_get_target,$(1)) : JARCLASSPATH :=
$(call gb_Jar_get_target,$(1)) : PACKAGEROOTS :=
$(call gb_Jar_get_target,$(1)) : PACKAGEDIRS :=
$(call gb_Jar_get_target,$(1)) : PACKAGEFILES :=
$(call gb_JavaClassSet_JavaClassSet,$(call gb_Jar_get_classsetname,$(1)))
$(call gb_JavaClassSet_set_classpath,$(call gb_Jar_get_classsetname,$(1)),$(value XCLASSPATH))
$(eval $(call gb_Module_register_target,$(call gb_Jar_get_outdir_target,$(1)),$(call gb_Jar_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_Jar_get_outdir_target,$(1)),$(call gb_Jar_get_target,$(1)),$(1))
$(call gb_Jar_get_outdir_target,$(1)) : $(call gb_Jar_get_target,$(1))

endef

# source files are forwarded to the ClassSet
define gb_Jar_add_sourcefile
$(call gb_JavaClassSet_add_sourcefile,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

# PACKAGEROOTS is the list of all root folders created by the JavaClassSet to pack into the jar (without META-INF as this is added automatically)
define gb_Jar_set_packageroot
$(call gb_Jar_get_target,$(1)) : PACKAGEROOTS := $(2)

endef
#
# PACKAGEDIRS is the list of additional root directories to pack into the jar
define gb_Jar_add_packagedir
$(call gb_Jar_get_target,$(1)) : PACKAGEDIRS += $(2)

endef

define gb_Jar_add_packagedirs
$(foreach packagedir,$(2),$(call gb_Jar_add_packagedir,$(1),$(packagedir)))

endef

# PACKAGEFILES is the list of all root files to pack into the jar
define gb_Jar_add_packagefile
$(call gb_Jar_get_target,$(1)) : PACKAGEFILES += $(2)
$(call gb_Jar_get_target,$(1)) : $(call gb_Jar_get_workdir,$(1))/$(strip $(2))
$(call gb_Jar_get_workdir,$(1))/$(strip $(2)) : $(3) $(call gb_JavaClassSet_get_target,$(call gb_Jar_get_classsetname,$(1)))
	mkdir -p $$(dir $$@)
	cp -rf $(3) $$@

endef

# gb_Jar_add_packagefiles jar target-dir file(s)
define gb_Jar_add_packagefiles
$(foreach file,$(3),$(call gb_Jar_add_packagefile,$(1),$(if $(strip $(2)),$(strip $(2))/)$(notdir $(file)),$(file)))

endef

define gb_Jar_add_sourcefiles
$(foreach sourcefile,$(2),$(call gb_Jar_add_sourcefile,$(1),$(sourcefile)))

endef

define gb_Jar_add_generated_sourcefile
$(call gb_JavaClassSet_add_generated_sourcefile,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

define gb_Jar_add_generated_sourcefiles
$(foreach sourcefile,$(2),$(call gb_Jar_add_generated_sourcefile,$(1),$(sourcefile)))

endef

define gb_JarTest_set_classpath
$(call gb_JavaClassSet_set_classpath,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

# JARCLASSPATH is the class path that is written to the manifest of the jar
define gb_Jar_set_jarclasspath
$(call gb_Jar_get_target,$(1)) : JARCLASSPATH := $(2)

endef

define gb_Jar_add_manifest_classpath
$(call gb_Jar_get_target,$(1)) : JARCLASSPATH += $(2)

endef

# provide a manifest template containing jar specific information to be written into the manifest
# it will be appended to the standard content that is written in the build command explicitly
# the jar file gets a dependency to the manifest template
define gb_Jar_set_manifest
$(call gb_Jar_get_target,$(1)) : MANIFEST := $(2)
$(call gb_Jar_get_target,$(1)) : $(2)

endef

define gb_Jar_add_jar
$$(call gb_Output_error,\
 gb_Jar_add_jar: use gb_Jar_use_jar instead.)
endef

# remember: classpath is "inherited" to ClassSet
define gb_Jar_use_jar
$(call gb_JavaClassSet_use_jar,$(call gb_Jar_get_classsetname,$(1)),$(2))
$(call gb_Jar_add_manifest_classpath,$(1),$(notdir $(2)))

endef

define gb_Jar_add_system_jar
$$(call gb_Output_error,\
 gb_Jar_add_system_jar: use gb_Jar_use_system_jar instead.)
endef

define gb_Jar_use_system_jar
$(call gb_JavaClassSet_use_system_jar,$(call gb_Jar_get_classsetname,$(1)),$(2))
$(call gb_Jar_add_manifest_classpath,$(1),$(call gb_Helper_make_url,$(2)))

endef

define gb_Jar_add_jars
$$(call gb_Output_error,\
 gb_Jar_add_jars: use gb_Jar_use_jars instead.)
endef

# specify jars with imported modules
define gb_Jar_use_jars
$(foreach jar,$(2),$(call gb_Jar_use_jar,$(1),$(jar)))

endef

define gb_Jar_add_system_jars
$$(call gb_Output_error,\
 gb_Jar_add_system_jars: use gb_Jar_use_system_jars instead.)
endef

define gb_Jar_use_system_jars
$(call gb_JavaClassSet_use_system_jars,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

# this forwards to functions that must be defined in RepositoryExternal.mk.
# $(eval $(call gb_Jar_use_external,jar,external))
define gb_Jar_use_external
$(if $(value gb_Jar__use_$(2)),\
  $(call gb_Jar__use_$(2),$(1)),\
  $(error gb_Jar_use_external: unknown external: $(2)))

endef

define gb_Jar_use_externals
$(foreach external,$(2),$(call gb_Jar_use_external,$(1),$(external)))

endef

define gb_Jar_add_customtarget_dependency
$$(call gb_Output_error,\
 gb_Jar_add_customtarget_dependency: use gb_Jar_use_customtarget instead.)
endef

define gb_Jar_use_customtarget
$(call gb_JavaClassSet_use_customtarget,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

define gb_Jar_add_customtarget_dependencies
$$(call gb_Output_error,\
 gb_Jar_add_customtarget_dependencies: use gb_Jar_use_customtargets instead.)
endef

define gb_Jar_use_customtargets
$(foreach customtarget,$(2),$(call gb_Jar_use_customtarget,$(1),$(customtarget)))

endef

# possible directories for jar files containing UNO services 
gb_Jar_COMPONENTPREFIXES := \
    OOO:vnd.sun.star.expand:\dLO_JAVA_DIR/ \
    URE:vnd.sun.star.expand:\dURE_INTERNAL_JAVA_DIR/ \
    OXT:./ \

# get component prefix from layer name ("OOO", "URE", "OXT", "INTERN")
gb_Jar__get_componentprefix = \
    $(patsubst $(1):%,%,$(or \
        $(filter $(1):%,$(gb_Jar_COMPONENTPREFIXES)), \
        $(call gb_Output_error,no ComponentTarget native prefix for layer '$(1)')))

# layer must be specified explicitly in this macro (different to libraries)
define gb_Jar_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,$(2),$(call gb_Jar__get_componentprefix,$(3)),$(notdir $(call gb_Jar_get_target,$(1))))
$(call gb_Jar_get_target,$(1)) : \
	$(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_Jar_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef

# vim: set noet sw=4 ts=4:
