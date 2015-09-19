# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(gb_FULLDEPS),$(true))

define gb_External__update_stamp_impl
echo $(SYSTEM_$(2)) > $(3).tmp && \
if ! test -f $(3); then \
	mv $(3).tmp $(3); \
elif cmp -s $(3) $(3).tmp; then \
	rm $(3).tmp; \
else \
	rm $(3) && mv $(3).tmp $(3); \
fi
endef

define gb_External__update_stamp
$(call gb_External__update_stamp_impl,$(firstword $(1)),$(lastword $(1)),$(call gb_External_get_target,$(firstword $(1))))
endef

$(WORKDIR)/External/done : $(BUILDDIR)/config_$(gb_Side).mk
	mkdir -p $(dir $@) \
		$(foreach external,$(gb_Externals_REGISTERED),&& $(call gb_External__update_stamp,$(subst :, ,$(external)))) \
	&& touch $@

# Force the update to run before the actual build, so stuff is rebuilt
# correctly where needed.
-include $(WORKDIR)/External/done

endif

# vim: set noet sw=4 ts=4:
