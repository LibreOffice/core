#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(filter usedcxxfiles,$(MAKECMDGOALS)),)
ifneq ($(filter-out usedcxxfiles,$(MAKECMDGOALS)),)
$(error usedcxxfiles only works as a single target)
endif

gb_UsedCxxFiles_Filelist := $(shell mktemp)

define gb_LinkTarget_add_cxxobject
$(shell echo "$(2)" >> $(gb_UsedCxxFiles_Filelist))
endef

usedcxxfiles:
	@cat $(gb_UsedCxxFiles_Filelist)|sort|uniq
	@rm $(gb_UsedCxxFiles_Filelist)

endif
# vim: set noet ts=4 sw=4:
