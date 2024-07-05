# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,tools_unique_id))


$(eval $(call gb_CppunitTest_set_include,tools_unique_id,\
    $$(INCLUDE) \
    -I$(SRCDIR)/tools/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,tools_unique_id, \
	tools/qa/cppunit/UniqueIdTest \
))

$(eval $(call gb_CppunitTest_use_libraries,tools_unique_id, \
    tl \
    test \
    unotest \
))


# vim: set noet sw=4 ts=4:
