# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,smoketest))

$(eval $(call gb_CppunitTest_abort_on_assertion,smoketest))

$(eval $(call gb_CppunitTest_add_exception_objects,smoketest,\
	smoketest/smoketest_too \
))

$(eval $(call gb_CppunitTest_use_external,smoketest,boost_headers))

$(eval $(call gb_CppunitTest_use_api,smoketest,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,smoketest,\
	cppu \
	cppuhelper \
	sal \
	unotest \
))

ifeq ($(OS),MACOSX)
smoketest_SOFFICE := path:$(gb_DEVINSTALLROOT)/MacOS/soffice
else
smoketest_SOFFICE := path:$(gb_DEVINSTALLROOT)/program/soffice
endif

$(eval $(call gb_CppunitTest_use_ure,smoketest))

$(eval $(call gb_CppunitTest_add_arguments,smoketest,\
	-env:arg-soffice=$(smoketest_SOFFICE) \
	-env:arg-user=$(WORKDIR)/CustomTarget/smoketest \
	-env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
	-env:arg-testarg.smoketest.doc=$(OUTDIR)/bin/smoketestdoc.sxw \
))

$(call gb_CppunitTest_get_target,smoketest): clean_CustomTarget_smoketest

clean_CustomTarget_smoketest:
	rm -rf $(WORKDIR)/CustomTarget/smoketest
	mkdir -p $(WORKDIR)/CustomTarget/smoketest

# vim: set noet sw=4 ts=4:
