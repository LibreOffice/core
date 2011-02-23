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

# location of manifest file in workdir
define gb_Jar_get_manifest_target 
$(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,$(1)))/META-INF/MANIFEST.MF
endef

# creates classset and META-INF folders if they don't exist
# adds manifest version, class path, solarversion and content from sources to manifest file 
# creates the target folder of the jar file if it doesn't exist
# creates the jar file
define gb_Jar__command
	$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,$(1)))/META-INF && \
	echo Manifest-Version: 1.0 > $(call gb_Jar_get_manifest_target,$(1)) && \
	echo "Class-Path: $(JARCLASSPATH)" >> $(call gb_Jar_get_manifest_target,$(1)) && \
	echo "Solar-Version: $(RSCREVISION)" >> $(call gb_Jar_get_manifest_target,$(1)) && \
	cat $(MANIFEST) >> $(call gb_Jar_get_manifest_target,$(1)) && \
	mkdir -p $(dir $(2)) && \
	cd $(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,$(1))) && $(gb_Jar_JARCOMMAND) cfm $(2) $(call gb_Jar_get_manifest_target,$(1)) META-INF $(PACKAGEROOTS) )
endef

# clean target reuses clean target of ClassSet
$(call gb_Jar_get_clean_target,%) : $(call gb_JavaClassSet_get_clean_target,$(call gb_Jar_get_classsetname,%))
	$(call gb_Output_announce,$*,$(false),JAR,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(dir $(call gb_Jar_get_target,$*)) && \
		rm -f $(call gb_Jar_get_final_target,$*) && \
		rm -f $(call gb_Jar_get_outdir_target,$*))

# the final target is a touch target; we use it as registered targets should be in workdir, not in outdir
# the outdir target depends on the workdir target and is built by delivering the latter
# the workdir target is created by cd'ing to the target directory and adding/updating the files
$(call gb_Jar_get_final_target,%) : $(call gb_Jar_get_outdir_target,%)
	$(call gb_Helper_abbreviate_dirs,\
		touch $@)

# rule for creating the jar file using the command defined above
$(call gb_Jar_get_target,%) : $(call gb_JavaClassSet_get_target,$(call gb_Jar_get_classsetname,%))
	$(call gb_Jar__command,$*,$@,$*,$?)

# resets scoped variables (see explanations where they are set)
# creates a class set and a dependency to it 
# registers target and clean target
# adds jar files to DeliverLogTarget
# adds dependency for outdir target to workdir target (pattern rule for delivery is in Package.mk)
define gb_Jar_Jar
$(call gb_Jar_get_target,$(1)) : CLASSPATH := $(value XCLASSPATH)
$(call gb_Jar_get_target,$(1)) : MANIFEST :=
$(call gb_Jar_get_target,$(1)) : JARCLASSPATH :=
$(call gb_Jar_get_target,$(1)) : PACKAGEROOTS :=
$(call gb_JavaClassSet_JavaClassSet,$(call gb_Jar_get_classsetname,$(1)))
$(eval $(call gb_Module_register_target,$(call gb_Jar_get_final_target,$(1)),$(call gb_Jar_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_Jar_get_outdir_target,$(1)),$(call gb_Jar_get_target,$(1)))
$(call gb_Jar_get_outdir_target,$(1)) : $(call gb_Jar_get_target,$(1))

endef

# source files are forwarded to the ClassSet
define gb_Jar_add_sourcefile
$(call gb_JavaClassSet_add_sourcefile,$(call gb_Jar_get_classsetname,$(1)),$(2))
endef

# PACKAGEROOTS is the list of all root folders to pack into the jar (without META-INF as this is added automatically)
define gb_Jar_set_packageroot
$(call gb_Jar_get_target,$(1)) : PACKAGEROOTS := $(2)
endef

define gb_Jar_add_sourcefiles
$(foreach sourcefile,$(2),$(call gb_Jar_add_sourcefile,$(1),$(sourcefile)))
endef

# don't forward it to ClassSet; the ClassSet "inherits" the classpath from the Jar
define gb_JarTest_set_classpath
$(call gb_Jar_get_target,$(1)) : CLASSPATH := $(2)
endef

# JARCLASSPATH is the class path that is written to the manifest of the jar
define gb_Jar_set_jarclasspath

$(call gb_Jar_get_target,$(1)) : JARCLASSPATH := $(2)
endef

# provide a manifest template containing jar specific information to be written into the manifest
# it will be appended to the standard content that is written in the build command explicitly
# the jar file gets a dependency to the manifest template
define gb_Jar_set_manifest
$(call gb_Jar_get_target,$(1)) : MANIFEST := $(2)
$(call gb_Jar_get_target,$(1)) : $(2)

endef

# remember: classpath is "inherited" to ClassSet
define gb_Jar_add_jar
$(call gb_Jar_get_target,$(1)) : CLASSPATH := $$(CLASSPATH)$(gb_CLASSPATHSEP)$(2)
$(call gb_JavaClassSet_add_jar,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

# specify jars with imported modules
define gb_Jar_add_jars
$(foreach jar,$(2),$(call gb_Jar_add_jar,$(1),$(jar)))
endef

# possible directories for jar files containing UNO services 
gb_Jar_COMPONENTPREFIXES := \
    OOO:vnd.sun.star.expand:\dOOO_BASE_DIR/program/classes/ \
    URE:vnd.sun.star.expand:\dURE_INTERNAL_JAVA_DIR/ \
    INTERN:vnd.sun.star.expand:\dOOO_INBUILD_JAVA_DIR/

# get component prefix from layer name ("OOO", "URE", "INTERN")
gb_Jar__get_componentprefix = \
    $(patsubst $(1):%,%,$(or \
        $(filter $(1):%,$(gb_Jar_COMPONENTPREFIXES)), \
        $(call gb_Output_error,no ComponentTarget native prefix for layer '$(1)')))

# layer must be specified explicitly in this macro (different to libraries)
define gb_Jar_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,$(2),$(call gb_Jar__get_componentprefix,$(3)),$(notdir $(call gb_Jar_get_target,$(1))))
$(call gb_Jar_get_target,$(1)) : $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_Jar_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef


# vim: set noet sw=4 ts=4:
