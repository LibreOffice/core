# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,officebean))

$(eval $(call gb_Library_use_externals,officebean,\
	jawt \
))

ifneq ($(OS),WNT)
$(eval $(call gb_Library_add_cobjects,officebean,\
	bean/native/unix/com_sun_star_comp_beans_LocalOfficeWindow \
	bean/native/unix/com_sun_star_beans_LocalOfficeWindow \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_cobjects,officebean,\
	bean/native/win32/com_sun_star_comp_beans_LocalOfficeWindow \
	bean/native/win32/com_sun_star_beans_LocalOfficeWindow \
))
endif

$(eval $(call gb_Library_use_libraries,officebean,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_libs,officebean,\
	$(JAVALIB) \
	$(AWTLIB) \
))

# vim: set noet sw=4 ts=4:
