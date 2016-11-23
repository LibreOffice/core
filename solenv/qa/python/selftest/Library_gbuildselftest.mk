# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,gbuildselftest))

$(eval $(call gb_Library_set_include,gbuildselftest,\
    -Igbuildtojsontestinclude\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,gbuildselftest,\
	-DGBUILDSELFTESTDEF \
))

$(eval $(call gb_Library_add_cxxflags,gbuildselftest,\
	-DGBUILDSELFTESTCXXFLAG \
))

$(eval $(call gb_Library_add_cflags,gbuildselftest,\
	-DGBUILDSELFTESTCFLAG \
))

$(eval $(call gb_Library_use_libraries,gbuildselftest,\
	gbuildselftestdep \
))

$(eval $(call gb_Library_add_exception_objects,gbuildselftest,\
	solenv/qa/python/selftest/selftestobject \
))

# vim: set noet sw=4 ts=4:
