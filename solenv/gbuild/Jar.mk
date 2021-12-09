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

# Jar class

gb_Jar_JARCOMMAND := jar

gb_Jar_LAYER_DIRS := \
	URE:$(INSTROOT)/$(LIBO_URE_SHARE_JAVA_FOLDER) \
	OOO:$(INSTROOT)/$(LIBO_SHARE_JAVA_FOLDER) \
	OXT:$(WORKDIR)/Jar \
	NONE:$(WORKDIR)/Jar \


# location of files going to be packed into .jar file
define gb_Jar_get_workdir
$(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,$(1)))
endef

# location of manifest file in workdir
define gb_Jar_get_manifest_target
$(call gb_Jar_get_workdir,$(1))/META-INF/MANIFEST.MF
endef

gb_Jar__get_layer = $(strip $(foreach group,$(gb_Jar_VALIDGROUPS),$(if $(filter $(1),$(gb_Jar_$(group))),$(group))))
gb_Jar__get_dir_for_layer = $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Jar_LAYER_DIRS)))
gb_Jar_get_install_target = $(call gb_Jar__get_dir_for_layer,$(call gb_Jar__get_layer,$(1)))/$(1).jar

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
	$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(call gb_Jar_get_workdir,$(1))/META-INF && \
	echo Manifest-Version: 1.0 > $(call gb_Jar_get_manifest_target,$(1)) && \
	$(if $(JARCLASSPATH),$(SRCDIR)/solenv/bin/write_classpath.sh "$(call gb_Jar_get_manifest_target,$(1))" $(strip $(JARCLASSPATH)) &&) \
	echo "Solar-Version: $(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)" >> $(call gb_Jar_get_manifest_target,$(1)) && \
	$(if $(MANIFEST),cat $(MANIFEST) >> $(call gb_Jar_get_manifest_target,$(1)) &&) \
	mkdir -p $(dir $(2)) && cd $(call gb_Jar_get_workdir,$(1)) && \
	$(gb_Jar_JARCOMMAND) cfm $(2) $(call gb_Jar_get_manifest_target,$(1)) \
		META-INF $(PACKAGEROOTS) $(PACKAGEFILES) \
	$(foreach root,$(PACKAGEDIRS),&& cd $(dir $(root)) && $(gb_Jar_JARCOMMAND) uf $(2) $(notdir $(root))) \
	|| (rm -f $(2); false) )
endef

# clean target reuses clean target of ClassSet
.PHONY : $(call gb_Jar_get_clean_target,%)
$(call gb_Jar_get_clean_target,%) : $(call gb_JavaClassSet_get_clean_target,$(call gb_Jar_get_classsetname,%))
	$(call gb_Output_announce,$*,$(false),JAR,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Jar_get_target,$*))

# the workdir target is created by cd'ing to the target directory and adding/updating the files

# rule for creating the jar file using the command defined above
$(WORKDIR)/Jar/%.jar :
	$(call gb_Output_announce,$*,$(true),JAR,3)
	$(call gb_Trace_StartRange,$*,JAR)
	$(call gb_Jar__command,$*,$@)
	$(call gb_Trace_EndRange,$*,JAR)

# call gb_Jar__make_installed_rule,jar
define gb_Jar__make_installed_rule
$(call gb_Jar_get_target,$(1)) :
	$$(call gb_Jar__command,$(1),$(call gb_Jar_get_target,$(1)))

endef

# resets scoped variables (see explanations where they are set)
# creates a class set and a dependency to it
# registers target and clean target
# adds jar files to DeliverLogTarget
# call gb_Jar_Jar,jarname,java9modulename
define gb_Jar_Jar
ifeq (,$$(findstring $(1),$$(gb_Jar_KNOWN)))
$$(eval $$(call gb_Output_info,Currently known jars are: $(sort $(gb_Jar_KNOWN)),ALL))
$$(eval $$(call gb_Output_error,Jar $(1) must be registered in Repository.mk or RepositoryExternal.mk))
endif
$(call gb_Jar_get_target,$(1)) : MANIFEST :=
$(call gb_Jar_get_target,$(1)) : JARCLASSPATH :=
$(call gb_Jar_get_target,$(1)) : PACKAGEROOTS :=
$(call gb_Jar_get_target,$(1)) : PACKAGEDIRS :=
$(call gb_Jar_get_target,$(1)) : PACKAGEFILES :=
$(call gb_Jar_get_target,$(1)) : \
	$(call gb_JavaClassSet_get_target,$(call gb_Jar_get_classsetname,$(1)))
$(call gb_JavaClassSet_JavaClassSet,$(call gb_Jar_get_classsetname,$(1)),$(2))
$(eval $(call gb_Module_register_target,$(call gb_Jar_get_target,$(1)),$(call gb_Jar_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Jar,$(call gb_Jar_get_target,$(1)))

# installed jars need a rule to build!
$(if $(findstring $(INSTDIR),$(call gb_Jar_get_target,$(1))),$(call gb_Jar__make_installed_rule,$(1)))

endef

# source files are forwarded to the ClassSet
define gb_Jar_add_sourcefile
$(call gb_JavaClassSet_add_sourcefile,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

define gb_Jar_add_sourcefile_java9
$(call gb_JavaClassSet_add_sourcefile_java9,$(call gb_Jar_get_classsetname,$(1)),$(2))

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

define gb_Jar_add_sourcefiles_java9
$(foreach sourcefile,$(2),$(call gb_Jar_add_sourcefile_java9,$(1),$(sourcefile)))

endef

define gb_Jar_add_generated_sourcefile
$(call gb_JavaClassSet_add_generated_sourcefile,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

define gb_Jar_add_generated_sourcefiles
$(foreach sourcefile,$(2),$(call gb_Jar_add_generated_sourcefile,$(1),$(sourcefile)))

endef

# JARCLASSPATH is the class path that is written to the manifest of the jar
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

# URE jars are not added to manifest classpath:
gb_Jar_default_jars := $(gb_Jar_URE)

# remember: classpath is "inherited" to ClassSet
define gb_Jar_use_jar
$(call gb_JavaClassSet_use_jar,$(call gb_Jar_get_classsetname,$(1)),$(2))
$(if $(filter-out $(gb_Jar_default_jars),$(2)),\
  $(call gb_Jar_add_manifest_classpath,$(1),$(2).jar))

endef

define gb_Jar_use_system_jar
$(call gb_JavaClassSet_use_system_jar,$(call gb_Jar_get_classsetname,$(1)),$(2))
$(call gb_Jar_add_manifest_classpath,$(1),$(call gb_Helper_make_url,$(2)))

endef

# call gb_Jar_use_external_jar,jar,externaljarfullpath,manifestentry
define gb_Jar_use_external_jar
$(if $(3),,$(call gb_Output_error,gb_Jar_use_external_jar: manifest entry missing))
$(call gb_JavaClassSet_use_system_jar,$(call gb_Jar_get_classsetname,$(1)),$(2))
$(call gb_Jar_add_manifest_classpath,$(1),$(3))

endef

# specify jars with imported modules
define gb_Jar_use_jars
$(foreach jar,$(2),$(call gb_Jar_use_jar,$(1),$(jar)))

endef

define gb_Jar_use_system_jars
$(foreach jar,$(2),$(call gb_Jar_use_system_jar,$(1),$(jar)))

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

define gb_Jar_use_customtarget
$(call gb_JavaClassSet_use_customtarget,$(call gb_Jar_get_classsetname,$(1)),$(2))

endef

define gb_Jar_use_customtargets
$(foreach customtarget,$(2),$(call gb_Jar_use_customtarget,$(1),$(customtarget)))

endef

# Add a dependency on an ExternalProject.
#
# call gb_Jar_use_external_project,jar,externalproject
define gb_Jar_use_external_project
$(call gb_JavaClassSet_use_external_project,$(call gb_Jar_get_classsetname,$(1)),$(2))
endef

# possible directories for jar files containing UNO services
gb_Jar_COMPONENTPREFIXES := \
    OOO:vnd.sun.star.expand:\dLO_JAVA_DIR/ \
    URE:vnd.sun.star.expand:\dURE_INTERNAL_JAVA_DIR/ \
    OXT:./ \
    NONE:$(call gb_Helper_make_url,$(WORKDIR)/Jar/) \

# get component prefix from layer name ("OOO", "URE", "OXT", "NONE")
gb_Jar__get_componentprefix = \
    $(patsubst $(1):%,%,$(or \
        $(filter $(1):%,$(gb_Jar_COMPONENTPREFIXES)), \
        $(call gb_Output_error,no ComponentTarget native prefix for layer '$(1)')))

# layer must be specified explicitly in this macro (different to libraries)
define gb_Jar_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,$(2),$(call gb_Jar__get_componentprefix,$(3)),$(notdir $(call gb_Jar_get_target,$(1))),$(4))
$(call gb_Jar_get_target,$(1)) : $(call gb_ComponentTarget_get_target,$(2))
$(call gb_Jar_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef

# vim: set noet sw=4 ts=4:
