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

$(eval $(call gb_CppunitTest_use_ure,smoketest))

ifeq ($(ENABLE_MACOSX_SANDBOX),TRUE)
userinstallation=$(shell $(gb_DEVINSTALLROOT)/MacOS/soffice --nstemporarydirectory)
else
userinstallation=$(WORKDIR)/CustomTarget/smoketest
endif

$(eval $(call gb_CppunitTest_add_arguments,smoketest,\
	-env:arg-soffice=$(gb_JunitTest_SOFFICEARG) \
	-env:arg-user=$(userinstallation) \
	-env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
	-env:arg-testarg.smoketest.doc=$(WORKDIR)/Zip/smoketestdoc.sxw \
))

$(call gb_CppunitTest_get_target,smoketest): \
	clean_CustomTarget_smoketest \
	$(WORKDIR)/Zip/smoketestdoc.sxw

clean_CustomTarget_smoketest:
	rm -rf $(WORKDIR)/CustomTarget/smoketest
	mkdir -p $(WORKDIR)/CustomTarget/smoketest

$(WORKDIR)/Zip/smoketestdoc.sxw: $(call gb_Zip_get_target,smoketestdoc)
	cp $< $@

# vim: set noet sw=4 ts=4:
