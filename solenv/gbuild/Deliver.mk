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

# gb_Deliver_GNUCOPY is set by the platform

# if ($true) then old files will get removed from the target location before
# they are copied there. In multi-user environments, this is needed you need to
# be the owner of the target file to be able to modify timestamps
gb_Deliver_CLEARONDELIVER := $(true)

gb_Deliver__deliverprefix = mkdir -p $(dir $(1)) &&

ifeq ($(gb_Deliver_CLEARONDELIVER),$(true))
gb_Deliver__deliverprefix += rm -rf $(1) &&
endif

define gb_Deliver_init
gb_Deliver_DELIVERABLES :=

endef

# FIXME: this does not really work for real multi repository builds, but the
# deliver.log format is broken in that case anyway
define gb_Deliver_add_deliverable
gb_Deliver_DELIVERABLES += $$(patsubst $(SOURCE_ROOT_DIR)/%,%,$(2)):$$(patsubst $(SOURCE_ROOT_DIR)/%,%,$(1))

endef

define gb_Deliver_deliver
$(call gb_Deliver__deliverprefix,$(2)) $(gb_Deliver_GNUCOPY) -f $(1) $(2)
endef

# We are currently only creating a deliver.log, if only one module gets build.
# As it is possible to add gbuild modules into other (which is done for example for
# the toplevel ooo module already) it does not make sense to create a deliver.log once
# fully migrated. The whole process should be rethought then.
# We need the trailing whitespace so that the newline of echo does not become part of the last record.
define gb_Deliver_setdeliverlogcommand
ifeq ($$(words $(gb_Module_ALLMODULES)),1)
$$(eval $$(call gb_Output_announce,$$(strip $$(gb_Module_ALLMODULES)),$$(true),LOG,1))
deliverlog : COMMAND := mkdir -p $$(OUTDIR)/inc/$$(strip $$(gb_Module_ALLMODULES)) &&
deliverlog : COMMAND += echo "$$(sort $$(gb_Deliver_DELIVERABLES)) " | awk -f $$(GBUILDDIR)/processdelivered.awk > $$(OUTDIR)/inc/$$(strip $(gb_Module_ALLMODULES))/deliver.log
else
$$(eval $$(call gb_Output_announce,more than one module - creating no deliver.log,$$(true),LOG,1))
deliverlog : COMMAND := true
endif
endef

.PHONY : deliverlog
deliverlog:
    $(eval $(call gb_Deliver_setdeliverlogcommand))
    $(call gb_Helper_abbreviate_dirs, $(COMMAND))

all : deliverlog

# vim: set noet sw=4 ts=4:
