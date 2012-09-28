# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CppunitTest_CppunitTest,cppu_checkapi))

$(eval $(call gb_CppunitTest_add_exception_objects,cppu_checkapi,\
    cppu/qa/checkapi/strings \
))

$(eval $(call gb_CppunitTest_set_include,cppu_checkapi,\
	$$(INCLUDE) \
	-I$(SRCDIR)/cppu/inc \
))

$(eval $(call gb_CppunitTest_use_internal_comprehensive_api,cppu_checkapi, \
	cppu \
	udkapi \
))

# strings.cxx includes generated strings.hxx
$(call gb_CxxObject_get_target,cppu/qa/checkapi/strings) : \
	INCLUDE += -I$(call gb_CustomTarget_get_workdir,cppu/allheaders)
$(call gb_CxxObject_get_target,cppu/qa/checkapi/strings) :| \
	$(call gb_CustomTarget_get_workdir,cppu/allheaders)/cppu_allheaders.hxx

$(eval $(call gb_CppunitTest_use_libraries,cppu_checkapi,\
    cppu \
	$(gb_UWINAPI) \
))

# vim: set noet sw=4 ts=4:
