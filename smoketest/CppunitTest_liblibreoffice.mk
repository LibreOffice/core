# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,liblibreoffice))

$(eval $(call gb_CppunitTest_abort_on_assertion,liblibreoffice))

$(eval $(call gb_CppunitTest_add_exception_objects,liblibreoffice,\
	smoketest/libtest \
))

$(eval $(call gb_CppunitTest_use_external,liblibreoffice,boost_headers))

$(eval $(call gb_CppunitTest_use_api,liblibreoffice,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,liblibreoffice,\
	cppu \
	cppuhelper \
	libreoffice \
	sal \
	unotest \
))

ifeq ($(OS),MACOSX)
liblibreoffice_SOFFICE_INST := path:$(DEVINSTALLDIR)/opt/LibreOffice.app/Contents/MacOS
else
liblibreoffice_SOFFICE_INST := path:$(DEVINSTALLDIR)/opt/program
endif

$(eval $(call gb_CppunitTest_use_ure,liblibreoffice))

$(eval $(call gb_CppunitTest_add_arguments,liblibreoffice,\
	-env:arg-soffice=$(liblibreoffice_SOFFICE_INST) \
))

# vim: set noet sw=4 ts=4:
