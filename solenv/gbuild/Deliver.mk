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

gb_Deliver_GNUCOPY := $(GNUCOPY)

# if ($true) then old files will get removed from the target location before
# they are copied there. In multi-user environments, if this is needed you need
# to be the owner of the target file to be able to modify timestamps
ifeq ($(strip gb_Deliver_HARDLINK),)
gb_Deliver_CLEARONDELIVER := $(false)
else
gb_Deliver_CLEARONDELIVER := $(true)
endif


define gb_Deliver_init
gb_Deliver_DELIVERABLES :=
gb_Deliver_DELIVERABLES_INDEX := 

endef

define gb_Deliver_register_deliverable
gb_Deliver_DELIVERABLES_$(notdir $(3)) += $(2):$(1)
gb_Deliver_DELIVERABLES_INDEX := $$(sort $$(gb_Deliver_DELIVERABLES_INDEX) $(notdir $(3)))
$(if $(gb_LOWRESTIME),.LOW_RESOLUTION_TIME : $(1),)

endef

define gb_Deliver_add_deliverable
$$(if $(3),,$$(error - missing third parameter for deliverable $(1)))
ifeq ($(MAKECMDGOALS),showdeliverables)
$(call gb_Deliver_register_deliverable,$(1),$(2),$(3))
endif

endef

define gb_Deliver__deliver
$(if $(gb_Deliver_CLEARONDELIVER),rm -f $(2) &&) $(if $(gb_Deliver_HARDLINK),ln,cp -P -f) $(1) $(2) && touch -hr $(1) $(2)
endef

ifneq ($(strip $(gb_Deliver_GNUCOPY)),)
ifeq ($(strip $(gb_Deliver_HARDLINK)),)
define gb_Deliver__deliver
$(gb_Deliver_GNUCOPY) $(if $(gb_Deliver_CLEARONDELIVER),--remove-destination) --no-dereference --force --preserve=timestamps $(1) $(2)
endef
endif
endif

define gb_Deliver_deliver
$(if $(1),$(call gb_Deliver__deliver,$(1),$(2)),\
 $(error gb_Deliver_deliver:\
  file does not exist in solver, and cannot be delivered: $(2)))
endef

define gb_Deliver_print_deliverable
$(info $(1) $(2))
endef

showdeliverables :
	$(eval MAKEFLAGS := s)
	$(foreach deliverable,$(sort $(foreach list,$(gb_Deliver_DELIVERABLES_INDEX),$(gb_Deliver_DELIVERABLES_$(list)))),\
			$(call gb_Deliver_print_deliverable,$(firstword $(subst :, ,$(deliverable))),$(lastword $(subst :, ,$(deliverable)))))
	@true

# vim: set noet sw=4:
