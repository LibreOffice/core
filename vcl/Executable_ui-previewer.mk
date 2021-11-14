# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,ui-previewer))

$(eval $(call gb_Executable_use_api,ui-previewer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_libraries,ui-previewer,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
    tl \
    ucbhelper \
))

$(eval $(call gb_Executable_use_vclmain,ui-previewer))

$(eval $(call gb_Executable_add_exception_objects,ui-previewer,\
    vcl/source/uipreviewer/previewer \
))

$(eval $(call gb_Executable_add_defs,ui-previewer,\
    -DVCL_INTERNALS \
))

$(eval $(call gb_Executable_add_default_nativeres,ui-previewer))

# vim: set noet sw=4 ts=4:
