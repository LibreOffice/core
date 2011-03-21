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

endef

define gb_Deliver_add_deliverable
gb_Deliver_DELIVERABLES += $$(patsubst $(REPODIR)/%,%,$(2)):$$(patsubst $(REPODIR)/%,%,$(1))
$(if $(gb_LOWRESTIME),.LOW_RESOLUTION_TIME : $(1),)

endef

ifeq ($(strip $(gb_Deliver_GNUCOPY)),)
define gb_Deliver_deliver
mkdir -p $(dir $(2)) && $(if $(gb_Deliver_CLEARONDELIVER),rm -f $(2) &&) $(if $(gb_Deliver_HARDLINK),ln,cp -f) $(1) $(2) && touch -r $(1) $(2)
endef
else
define gb_Deliver_deliver
mkdir -p $(dir $(2)) && $(gb_Deliver_GNUCOPY) $(if $(gb_Deliver_CLEARONDELIVER),--remove-destination) $(if $(gb_Deliver_HARDLINK),--link) --force --preserve=timestamps $(1) $(2)
endef
endif


# We are currently only creating a deliver.log, if only one module gets build.
# As it is possible to add gbuild modules into other (which is done for example for
# the toplevel ooo module already) it does not make sense to create a deliver.log once
# fully migrated. The whole process should be rethought then.
# We need the trailing whitespace so that the newline of echo does not become part of the last record.
define gb_Deliver_setdeliverlogcommand
ifeq ($$(words $(gb_Module_ALLMODULES)),1)
$$(eval $$(call gb_Output_announce,$$(strip $$(gb_Module_ALLMODULES)),$$(true),LOG,1))
deliverlog : COMMAND := \
 mkdir -p $$(OUTDIR)/inc/$$(strip $$(gb_Module_ALLMODULES)) \
 && RESPONSEFILE=$$(call var2file,$(shell $(gb_MKTEMP)),100,$$(sort $$(gb_Deliver_DELIVERABLES))) \
 && $(gb_AWK) -f $$(GBUILDDIR)/processdelivered.awk < $$$${RESPONSEFILE} \
		> $$(OUTDIR)/inc/$$(strip $(gb_Module_ALLMODULES))/gb_deliver.log \
 && rm -f $$$${RESPONSEFILE}
else
$$(eval $$(call gb_Output_announce,more than one module - creating no deliver.log,$$(true),LOG,1))
deliverlog : COMMAND := true
endif
endef

# FIXME: this does not really work for real multi repository builds, but the
# deliver.log format is broken in that case anyway
.PHONY : deliverlog showdeliverables
deliverlog:
	$(eval $(call gb_Deliver_setdeliverlogcommand))
	$(call gb_Helper_abbreviate_dirs, $(COMMAND))

# all : deliverlog

define gb_Deliver_print_deliverable
$(info $(1) $(patsubst $(OUTDIR)/%,%,$(2)))
endef

showdeliverables :
	$(eval MAKEFLAGS := s)
	$(foreach deliverable,$(sort $(gb_Deliver_DELIVERABLES)),\
			$(call gb_Deliver_print_deliverable,$(REPODIR)/$(firstword $(subst :, ,$(deliverable))),$(REPODIR)/$(lastword $(subst :, ,$(deliverable)))))
	true
# vim: set noet sw=4:
