# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,uwinapi))

$(eval $(call gb_Library_use_system_win32_libs,uwinapi,\
	$(if $(filter $(COM),MSC), \
		kernel32 \
		msvcrt \
		shlwapi \
		user32 \
		version \
	) \
))

$(eval $(call gb_Library_add_defs,uwinapi,\
	$(if $(filter $(COM),GCC), \
		-Wno-unused-parameter -Wno-return-type) \
	$(LFS_CFLAGS) \
))

$(eval $(call gb_Library_add_cobjects,uwinapi,\
	sal/systools/win32/uwinapi/snprintf \
	sal/systools/win32/uwinapi/snwprintf \
))

ifeq ($(COM),MSC)


$(eval $(call gb_Library_add_cobjects,uwinapi,\
	sal/systools/win32/uwinapi/legacy \
))

# VS 2015 implements ISO Standard for snprintf and friends
ifneq ($(filter 140,$(VCVER)),)
$(eval $(call gb_Library_add_ldflags,uwinapi,\
	/DEF:$(SRCDIR)/sal/systools/win32/uwinapi/uwinapi.def \
))
endif

endif


# vim: set noet sw=4 ts=4:
