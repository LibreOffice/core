# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,unowinreg))

ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_libs,unowinreg,\
	-lstdc++ \
	-lgcc_eh \
	-lgcc \
	-lmingw32 \
	-lmoldname \
	-lmsvcrt \
))
else
$(eval $(call gb_Library_add_ldflags,unowinreg,\
	/DEF:$(SRCDIR)/odk/source/unowinreg/win/unowinreg.def \
	/MANIFEST:NO \
))
endif

$(eval $(call gb_Library_add_exception_objects,unowinreg,\
   odk/source/unowinreg/win/unowinreg \
))

$(eval $(call gb_Library_use_system_win32_libs,unowinreg,\
	kernel32 \
	advapi32 \
))

# vim: set noet sw=4 ts=4:
