# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,xmlsecurity_qa_certext))

$(eval $(call gb_CppunitTest_use_api,xmlsecurity_qa_certext,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,xmlsecurity_qa_certext,\
	cppu \
	neon \
	sal \
	svl \
	test \
	tl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,xmlsecurity_qa_certext,\
	xmlsecurity/qa/certext/SanCertExt \
))

# vim: set noet sw=4 ts=4:
