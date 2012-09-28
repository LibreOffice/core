# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Caolán McNamara, Red Hat, Inc. <caolanm@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,tools_test))

$(eval $(call gb_CppunitTest_add_exception_objects,tools_test, \
    tools/qa/cppunit/test_inetmime \
    tools/qa/cppunit/test_pathutils \
    tools/qa/cppunit/test_reversemap \
    tools/qa/cppunit/test_stream \
    tools/qa/cppunit/test_urlobj \
))

$(eval $(call gb_CppunitTest_use_api,tools_test, \
	udkapi \
	offapi \
))

$(eval $(call gb_CppunitTest_use_libraries,tools_test, \
    sal \
    tl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_static_libraries,tools_test, \
    ooopathutils \
))

$(eval $(call gb_CppunitTest_set_include,tools_test,\
    $$(INCLUDE) \
    -I$(SRCDIR)/tools/inc \
))

# vim: set noet sw=4 ts=4:
