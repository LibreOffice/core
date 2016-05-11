# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sax2_xmlimport))

$(eval $(call gb_CppunitTest_add_exception_objects,sax2_xmlimport, \
sax2/qa/xmlimport \
))

$(eval $(call gb_CppunitTest_use_libraries,sax2_xmlimport, \
	test \
	cppu \
	comphelper \
	sal \
	cppuhelper \
	sax \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,sax2_xmlimport,\
    offapi \
    udkapi \
))
$(eval $(call gb_CppunitTest_use_external,sax2_xmlimport,boost_headers))


$(eval $(call gb_CppunitTest_use_ure,sax2_xmlimport))
$(eval $(call gb_CppunitTest_use_vcl,sax2_xmlimport))

$(eval $(call gb_CppunitTest_use_components,sax2_xmlimport,\
    configmgr/source/configmgr \
    sax/source/expatwrap/expwrap \
))


$(eval $(call gb_CppunitTest_use_configuration,sax2_xmlimport))

# vim: set noet sw=4 ts=4:
