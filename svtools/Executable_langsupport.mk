# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,langsupport))

$(eval $(call gb_Executable_set_targettype_gui,langsupport,YES))

$(eval $(call gb_Executable_set_include,langsupport,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svtools/inc \
    -I$(SRCDIR)/svtools/inc/svtools \
    -I$(SRCDIR)/svtools/source/inc \
))

$(eval $(call gb_Executable_use_external,langsupport,boost_headers))

$(eval $(call gb_Executable_use_sdk_api,langsupport))

$(eval $(call gb_Executable_use_libraries,langsupport,\
    comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
    sal \
    svt \
    tl \
    ucbhelper \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,langsupport,\
    svtools/langsupport/langsupport \
))

# vim: set noet sw=4 ts=4:
