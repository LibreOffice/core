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

gb_ComponentTarget_XSLTCOMMANDFILE := $(SOLARENV)/bin/createcomponent.xslt
gb_ComponentTarget_get_source = $(SRCDIR)/$(1).component

# In the DISABLE_DYNLOADING case we don't need any COMPONENTPREFIX, we
# put just the static library filename into the uri parameter. For
# each statically linked app using some subset of LO components, there
# is a mapping from library filenames to direct pointers to the
# corresponding PREFIX_component_getFactory functions.
define gb_ComponentTarget__command
$(call gb_Output_announce,$(3),$(true),CMP,1)
$(if $(LIBFILENAME),,$(call gb_Output_error,No LIBFILENAME set at component target: $(1)))
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet --stringparam uri \
		'$(if $(filter TRUE,$(DISABLE_DYNLOADING)),,$(subst \d,$$,$(COMPONENTPREFIX)))$(LIBFILENAME)' -o $(1) \
		$(gb_ComponentTarget_XSLTCOMMANDFILE) $(2))
endef


$(call gb_ComponentTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CMP,1)
	rm -f $(call gb_ComponentTarget_get_target,$*) \


# when a library is renamed, the component file needs to be rebuilt to match.
# hence simply depend on Repository{,Fixes}.mk since the command runs quickly.
$(call gb_ComponentTarget_get_target,%) : \
		$(call gb_ComponentTarget_get_source,%) \
		$(SRCDIR)/Repository.mk \
		$(SRCDIR)/RepositoryFixes.mk \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_ComponentTarget__command,$@,$<,$*)

define gb_ComponentTarget_ComponentTarget
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTPREFIX := $(2)
$(call gb_ComponentTarget_get_target,$(1)) : LIBFILENAME := $(3)

endef

# vim: set noet sw=4:
