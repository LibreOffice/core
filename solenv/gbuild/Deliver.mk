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

gb_Deliver_GNUCOPY := $(GNUCOPY)

# if ($true) then old files will get removed from the target location before
# they are copied there. In multi-user environments, this is needed you need to
# be the owner of the target file to be able to modify timestamps
gb_Deliver_CLEARONDELIVER := $(true)

define gb_Deliver_init
gb_Deliver_DELIVERABLES :=
gb_Deliver_DELIVERABLES_INDEX := 

endef

define gb_Deliver_do_add
$$(if $(3),,$$(error - missing third parameter for deliverable $(1)))
gb_Deliver_DELIVERABLES_$(notdir $(3)) += $$(patsubst $(REPODIR)/%,%,$(2)):$$(patsubst $(REPODIR)/%,%,$(1))
gb_Deliver_DELIVERABLES_INDEX := $(sort $(gb_Deliver_DELIVERABLES_INDEX) $(notdir $(3)))
$(if $(gb_LOWRESTIME),.LOW_RESOLUTION_TIME : $(1),)

endef

define gb_Deliver_add_deliverable
ifeq ($(MAKECMDGOALS),showdeliverables)
$(call gb_Deliver_do_add,$(OUTDIR)/$(1),$(2),$(3))
else
$(call gb_Deliver_do_add,$(OUTDIR)/$(1),$(2),$(3))
endif

endef

ifeq ($(strip $(gb_Deliver_GNUCOPY)),)
define gb_Deliver__deliver
mkdir -p $(dir $(2)) && $(if $(gb_Deliver_CLEARONDELIVER),rm -f $(2) &&) $(if $(gb_Deliver_HARDLINK),ln,cp -f) $(1) $(2) && touch -r $(1) $(2)
endef
else
define gb_Deliver__deliver
mkdir -p $(dir $(2)) && $(gb_Deliver_GNUCOPY) $(if $(gb_Deliver_CLEARONDELIVER),--remove-destination) $(if $(gb_Deliver_HARDLINK),--link) --force --preserve=timestamps $(1) $(2)
endef
endif

define gb_Deliver_deliver
$(if $(1),$(call gb_Deliver__deliver,$(1),$(2)),\
 $(error gb_Deliver_deliver:\
  file does not exist in solver, and cannot be delivered: $(2)))
endef

define gb_Deliver_print_deliverable
$(info $(1) $(patsubst $(OUTDIR)/%,%,$(2)))
endef

showdeliverables :
	$(eval MAKEFLAGS := s)
	$(foreach deliverable,$(sort $(foreach list,$(gb_Deliver_DELIVERABLES_INDEX),$(gb_Deliver_DELIVERABLES_$(list)))),\
			$(call gb_Deliver_print_deliverable,$(REPODIR)/$(firstword $(subst :, ,$(deliverable))),$(REPODIR)/$(lastword $(subst :, ,$(deliverable)))))
	true

# vim: set noet sw=4:
