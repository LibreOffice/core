# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
#       Miklos Vajna <vmiklos@frugalware.org>
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

$(eval $(call gb_CppunitTest_CppunitTest,writerfilter_rtftok))

$(eval $(call gb_CppunitTest_add_exception_objects,writerfilter_rtftok, \
	writerfilter/qa/cppunittests/rtftok/testrtftok \
))

$(eval $(call gb_CppunitTest_add_linked_libs,writerfilter_rtftok, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	ucbhelper \
	vcl \
	writerfilter \
	$(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_add_api,writerfilter_rtftok,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_uses_ure,writerfilter_rtftok))

$(eval $(call gb_CppunitTest_add_type_rdbs,writerfilter_rtftok,\
	types \
))

$(eval $(call gb_CppunitTest_add_service_rdbs,writerfilter_rtftok,\
	writerfilter_rtftok \
))

$(eval $(call gb_CppunitTest_set_args,writerfilter_rtftok,\
	--headless \
	--invisible \
	--protector unoexceptionprotector$(gb_Library_DLLEXT) unoexceptionprotector \
))

$(eval $(call gb_RdbTarget_RdbTarget,writerfilter_rtftok))

$(eval $(call gb_RdbTarget_add_components,writerfilter_rtftok,\
	writerfilter/util/writerfilter \
))

$(eval $(call gb_RdbTarget_add_old_components,writerfilter_rtftok,\
	ucb1 \
	ucpfile1 \
))

# we need to explicitly depend on library rtftok and writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,writerfilter_rtftok) : $(call gb_Library_get_target,rtftok) $(call gb_Library_get_target,writerfilter)

# vim: set noet sw=4 ts=4:
