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

# SdiTarget is evil, in that it does not support multiple repositories for now (hardcoded to SRCDIR)
# Also there is no way to cleanly deliver the generated header to OUTDIR.
# (This can be workarounded by using gb_Package, but really should not.)

# SdiTarget class
gb_SdiTarget_SVIDLDEPS := $(call gb_Executable_get_runtime_dependencies,svidl)
gb_SdiTarget_SVIDLCOMMAND := LD_LIBRARY_PATH=$(INSTDIR)/ure/lib $(call gb_Executable_get_command,svidl)

$(call gb_SdiTarget_get_target,%) : $(SRCDIR)/%.sdi $(gb_SdiTarget_SVIDLDEPS)
	$(call gb_Output_announce,$*,$(true),SDI,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) $(dir $(call gb_SdiTarget_get_dep_target,$*)))
	$(call gb_Helper_abbreviate_dirs,\
		cd $(dir $<) && \
		$(gb_SdiTarget_SVIDLCOMMAND) -quiet \
			$(INCLUDE) \
			-fs$@.hxx \
			-fd$@.ilb \
			-fl$@.lst \
			-fz$@.sid \
			-fx$(EXPORTS) \
			-fm$@ \
			-fM$(call gb_SdiTarget_get_dep_target,$*) \
			$<)

# rule necessary to rebuild cxx files that include the header
$(call gb_SdiTarget_get_target,%.hxx) : $(call gb_SdiTarget_get_target,%)
	@true

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SdiTarget_get_dep_target,%) :
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_SdiTarget_get_target,$*)))
endif

.PHONY : $(call gb_SdiTarget_get_clean_target,%)
$(call gb_SdiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SDI,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(foreach ext,.hxx .ilb .lst .sid,\
			$(call gb_SdiTarget_get_target,$*)$(ext)) \
			$(call gb_SdiTarget_get_dep_target,$*) \
			$(call gb_SdiTarget_get_target,$*))

define gb_SdiTarget_SdiTarget
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := $$(subst -I. ,-I$$(dir $(SRCDIR)/$(1)) ,$$(SOLARINC))
$(call gb_SdiTarget_get_target,$(1)) : EXPORTS := $(SRCDIR)/$(2).sdi
ifeq ($(gb_FULLDEPS),$(true))
-include $(call gb_SdiTarget_get_dep_target,$(1))
endif
endef

define gb_SdiTarget_set_include
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := $(2)

endef

# vim: set noet sw=4:
