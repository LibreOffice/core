# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,textconv_dict))

$(eval $(call gb_Library_set_include,textconv_dict,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
))

$(eval $(call gb_Library_use_sdk_api,textconv_dict))

$(eval $(call gb_Library_use_libraries,textconv_dict,\
	$(gb_UWINAPI) \
	sal \
	cppu \
	cppuhelper \
))

$(eval $(call gb_Library_add_generated_exception_objects,textconv_dict,\
	$(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/textconversion/data/*.dic),\
		CustomTarget/i18npool/textconversion/$(notdir $(basename $(txt)))) \
))

# vim: set noet sw=4 ts=4:
