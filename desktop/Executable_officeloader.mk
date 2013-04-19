# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,officeloader))

$(eval $(call gb_Executable_use_libraries,officeloader,\
    sal \
	$(gb_UWINAPI) \
))

# I'm not suer wht was the intent, but that break the link
#$(eval $(call gb_Executable_set_ldflags,officeloader,\
#    $(filter -bind_at_load,$$(LDFLAGS)) \
#))

$(eval $(call gb_Executable_add_noexception_objects,officeloader,\
    desktop/unx/source/officeloader/officeloader \
))

# vim: set ts=4 sw=4 et:
