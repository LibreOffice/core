# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

odk_WORKDIR := $(call gb_CustomTarget_get_workdir,odk/odkcommon)
odkcommon_ZIPLIST :=
odkcommon_ZIPDEPS :=

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon))

$(call gb_CustomTarget_get_target,odk/odkcommon): \
	$(call gb_CustomTarget_get_target,odk/odkcommon/docs/common/ref) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/classes) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/idl) \
	$(call gb_CustomTarget_get_target,odk/odkcommon/include)

# vim: set noet sw=4 ts=4:
