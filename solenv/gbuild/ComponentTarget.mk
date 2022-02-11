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

gb_ComponentTarget__ALLCOMPONENTS :=

gb_ComponentTarget_XSLT_CREATE_COMPONENT := $(SRCDIR)/solenv/bin/createcomponent.xslt
gb_ComponentTarget_XSLT_DUMP_OPTIONALS := $(SRCDIR)/solenv/bin/optionalimplementations.xslt
gb_ComponentTarget_get_source = $(SRCDIR)/$(1).component

# Some comment on the prerequisite handling for gb_ComponentTarget__command:
# The whole setup feels - once again - much more complicated then it should be; for an IMHO simple task.
# We can't just add all the $(call gb_ComponentTarget_get_target,%).* target commands to gb_ComponentTarget__command,
# because $(shell cat $(1).filtered) is then evaluated too early, cat'ing a non-existing file.
# Same happens if you add them to a gb_ComponentTarget__pre_command, run before the gb_ComponentTarget__command.
# The various other "macros" add new rules to "expand" the pattern rules with normal rules and prerequisites.
# As a result, the files from the pattern rules aren't cleaned up automagically, which I consider a plus point.
# So the intermediate files must be explicitly added to $(call gb_ComponentTarget_get_clean_target,%).

# In the DISABLE_DYNLOADING case we don't need any COMPONENTPREFIX, we
# put just the static library filename into the uri parameter. For
# each statically linked app using some subset of LO components, there
# is a mapping from library filenames to direct pointers to the
# corresponding PREFIX_component_getFactory functions.
define gb_ComponentTarget__command
$(if $(LIBFILENAME),,$(call gb_Output_error,No LIBFILENAME set at component target: $(1)))
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet \
		--stringparam uri '$(if $(filter TRUE,$(DISABLE_DYNLOADING)),,$(subst \d,$$,$(COMPONENTPREFIX)))$(LIBFILENAME)' \
		--stringparam cppu_env $(CPPU_ENV) \
		--stringparam filtered '$(shell cat $(1).filtered)' \
		-o $(1) $(gb_ComponentTarget_XSLT_CREATE_COMPONENT) $(COMPONENTSOURCE)
endef

$(call gb_ComponentTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CMP,1)
	rm -f $(call gb_ComponentTarget_get_target,$*) \
	    $(call gb_ComponentTarget_get_target,$*).allfiltered \
	    $(call gb_ComponentTarget_get_target,$*).filtered \
	    $(call gb_ComponentTarget_get_target,$*).optionals \

$(call gb_ComponentTarget_get_target,%).dir:
	mkdir -p $(dir $@)

# %.optionals : list of all optional implementations marked <optional/> in the component file
$(call gb_ComponentTarget_get_target,%).optionals : \
	    $(gb_ComponentTarget_XSLT_DUMP_OPTIONALS) \
	    | $(call gb_ComponentTarget_get_target,%).dir \
	      $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet \
	    $(gb_ComponentTarget_XSLT_DUMP_OPTIONALS) $(COMPONENTSOURCE) 2>&1 | LC_ALL=C $(SORT) > $@

# %.filtered : list of all optional implementations we don't build
.PRECIOUS: $(call gb_ComponentTarget_get_target,%).filtered
$(call gb_ComponentTarget_get_target,%).filtered : $(call gb_ComponentTarget_get_target,%).optionals
	cat $< $(COMPONENTIMPL) | sed -e '/^#/d' -e '/^[ 	]*$$/d' | LC_ALL=C $(SORT) | $(UNIQ) -u > $@

# %.allfiltered : contains all possible filtered components, which must match %.optionals
.PRECIOUS: $(call gb_ComponentTarget_get_target,%).allfiltered
$(call gb_ComponentTarget_get_target,%).allfiltered : $(call gb_ComponentTarget_get_target,%).optionals
	$(if $(ALLFILTEREDIMPL), \
	    cat $(ALLFILTEREDIMPL) | sed -e '/^#/d' -e '/^[ 	]*$$/d' | LC_ALL=C $(SORT) -u > $@.tmp, \
	    touch $@.tmp)
	$(DIFF) -u $< $@.tmp
	mv $@.tmp $@

# when a library is renamed, the component file needs to be rebuilt to match.
# hence simply depend on Repository{,Fixes}.mk since the command runs quickly.
$(call gb_ComponentTarget_get_target,%) : \
		$(SRCDIR)/Repository.mk \
		$(SRCDIR)/RepositoryFixes.mk \
		$(gb_ComponentTarget_XSLT_CREATE_COMPONENT) \
		$(call gb_ComponentTarget_get_target,%).allfiltered \
		$(call gb_ComponentTarget_get_target,%).filtered \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$*,$(true),CMP,1)
	$(call gb_Trace_StartRange,$*,CMP)
	$(call gb_ComponentTarget__command,$@)
	$(call gb_Trace_EndRange,$*,CMP)

gb_ComponentTarget__init_source = $(call gb_ComponentTarget_get_source,$(patsubst CppunitTest/%,%,$(1)))
gb_ComponentTarget__init_allfiltered = $(wildcard $(call gb_ComponentTarget__init_source,$(1)).*)

define gb_ComponentTarget_ComponentTarget
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTPREFIX := $(2)
$(call gb_ComponentTarget_get_target,$(1)) : LIBFILENAME := $(3)
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTSOURCE := $(call gb_ComponentTarget__init_source,$(1))
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTIMPL :=
$(call gb_ComponentTarget_get_target,$(1)) : ALLFILTEREDIMPL := $(call gb_ComponentTarget__init_allfiltered,$(1))

$(call gb_ComponentTarget_get_target,$(1)) : $(call gb_ComponentTarget__init_source,$(1))
$(call gb_ComponentTarget_get_target,$(1)).optionals : $(call gb_ComponentTarget__init_source,$(1))
$(call gb_ComponentTarget_get_target,$(1)).allfiltered : $(call gb_ComponentTarget__init_allfiltered,$(1))

$(call gb_Helper_make_userfriendly_targets,$(1),ComponentTarget,$(call gb_ComponentTarget_get_target,$(1)))

ifneq ($(4),)
$$(eval $$(call gb_Rdb_add_component,$(4),$(1)))
endif
$(if $(4),$(eval gb_ComponentTarget__ALLCOMPONENTS += $(1)))

endef

# call gb_ComponentTarget_add_componentimpl,componentfile,implid
define gb_ComponentTarget_add_componentimpl
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTIMPL += $(call gb_ComponentTarget_get_source,$(1)).$(2)
$(call gb_ComponentTarget_get_target,$(1)).filtered : $(call gb_ComponentTarget_get_source,$(1)).$(2)

endef

# vim: set noet sw=4:
