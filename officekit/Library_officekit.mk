# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,officekit))

$(eval $(call gb_StaticLibrary_set_include,officekit,\
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_libs,officekit,\
	$(if $(filter $(OS),LINUX), \
		-ldl \
        -lpthread \
	) \
))

$(eval $(call gb_StaticLibrary_use_libraries,officekit,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_StaticLibrary_add_cobjects,officekit,\
    officekit/source/shim \
))

# vim: set noet sw=4 ts=4:
