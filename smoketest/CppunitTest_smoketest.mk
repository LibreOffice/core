# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_CppunitTest_CppunitTest,smoketest))

$(eval $(call gb_CppunitTest_add_exception_objects,smoketest,\
	smoketest/smoketest \
))

$(eval $(call gb_CppunitTest_add_api,smoketest,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_add_linked_libs,smoketest,\
	cppu \
	cppuhelper \
	sal \
	unotest \
))

ifeq ($(OS),MACOSX)
my_soffice:=path:$(OUTDIR)/installation/opt/LibreOffice.app/Contents/MacOS/soffice
else
my_soffice:=path:$(OUTDIR)/installation/opt/program/soffice
endif

$(eval $(call gb_CppunitTest_set_args,smoketest,\
	-env:UNO_SERVICES=$(call gb_CppunitTarget__make_url,$(OUTDIR)/xml/ure/services.rdb) \
	-env:UNO_TYPES=$(call gb_CppunitTarget__make_url,$(OUTDIR)/bin/types.rdb) \
	-env:arg-soffice=$(my_soffice) \
	-env:arg-user=$(WORKDIR)/CustomTarget/smoketest \
	-env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
	-env:arg-testarg.smoketest.doc=$(OUTDIR)/bin/smoketestdoc.sxw \
	--protector $(call gb_Library_get_target,unoexceptionprotector) \
	unoexceptionprotector \
))

$(call gb_CppunitTest_get_target,smoketest): clean_CustomTarget_smoketest

clean_CustomTarget_smoketest:
	rm -rf $(WORKDIR)/CustomTarget/smoketest
	mkdir -p $(WORKDIR)/CustomTarget/smoketest

# vim: set noet sw=4 ts=4:
