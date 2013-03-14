#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_CppunitTest_CppunitTest,svl_qa_cppunit))

$(eval $(call gb_CppunitTest_use_api,svl_qa_cppunit, \
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_externals,svl_qa_cppunit, \
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,svl_qa_cppunit, \
	svl/qa/unit/svl \
))

$(eval $(call gb_CppunitTest_use_libraries,svl_qa_cppunit, \
	comphelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
	sal \
	sot \
	svl \
	tl \
	unoidl \
	unotest \
	utl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,svl_qa_cppunit,\
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_components,svl_qa_cppunit,\
	i18npool/util/i18npool \
	configmgr/source/configmgr \
))

$(eval $(call gb_CppunitTest_use_ure,svl_qa_cppunit))
$(eval $(call gb_CppunitTest_use_configuration,svl_qa_cppunit))

# vim: set noet sw=4:
