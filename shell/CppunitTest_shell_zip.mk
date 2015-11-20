# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,shell_zip))

$(eval $(call gb_CppunitTest_add_exception_objects,shell_zip, \
    shell/qa/zip/testzipimpl \
    shell/qa/zip/ziptest \
))


$(eval $(call gb_CppunitTest_set_include,shell_zip,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_externals,shell_zip,\
	expat \
	zlib \
))

$(eval $(call gb_CppunitTest_use_system_win32_libs,shell_zip, \
	ole32 \
	kernel32 \
	msvcprt \
))

$(eval $(call gb_CppunitTest_use_static_libraries,shell_zip,\
	shell_xmlparser \
	shlxthandler_common \
))

# vim: set noet sw=4 ts=4:
