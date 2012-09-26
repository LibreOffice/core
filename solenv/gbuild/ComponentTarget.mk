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
	$(gb_XSLTPROC) --nonet --stringparam uri \
		'$(if $(filter TRUE,$(DISABLE_DYNLOADING)),,$(subst \d,$$,$(COMPONENTPREFIX)))$(LIBFILENAME)' -o $(1) \
		$(gb_ComponentTarget_XSLTCOMMANDFILE) $(2))
endef


$(call gb_ComponentTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CMP,1)
	rm -f $(call gb_ComponentTarget_get_outdir_target,$*) \
		$(call gb_ComponentTarget_get_target,$*) \


$(call gb_ComponentTarget_get_target,%) : \
		$(call gb_ComponentTarget_get_source,%) $(gb_XSLTPROCTARGET)
	$(call gb_ComponentTarget__command,$@,$<,$*)

# the .dir is for make 3.81, which ignores trailing /
$(dir $(call gb_ComponentTarget_get_outdir_target,))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ComponentTarget_get_outdir_target,%) :
	$(call gb_Deliver_deliver,$<,$@)

define gb_ComponentTarget_ComponentTarget
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTPREFIX := $(2)
$(call gb_ComponentTarget_get_target,$(1)) : LIBFILENAME := $(3)
$(call gb_ComponentTarget_get_outdir_target,$(1)) : \
	$(call gb_ComponentTarget_get_target,$(1)) \
	| $(dir $(call gb_ComponentTarget_get_outdir_target,$(1))).dir
$(call gb_Deliver_add_deliverable,$(call gb_ComponentTarget_get_outdir_target,$(1)),$(call gb_ComponentTarget_get_target,$(1)),$(1))

endef

# vim: set noet sw=4:
