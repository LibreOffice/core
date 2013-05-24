# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,testtools/uno_test))

# this target is phony to run it every time
.PHONY : $(call gb_CustomTarget_get_target,testtools/uno_test)

$(call gb_CustomTarget_get_target,testtools/uno_test) : \
		$(call gb_Executable_get_runtime_dependencies,uno) \
		$(call gb_InternalUnoApi_get_target,bridgetest) \
		$(call gb_Rdb_get_target,uno_services) \
		$(call gb_Rdb_get_target,ure/services) \
		$(call gb_UnoApi_get_target,udkapi)
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Executable_get_command,uno) \
		-s com.sun.star.test.bridge.BridgeTest \
		-- com.sun.star.test.bridge.CppTestObject \
		-env:LO_BUILD_LIB_DIR=$(call gb_Helper_make_url,$(gb_Helper_OUTDIR_FOR_BUILDLIBDIR)) \
		-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target,uno_services)) \
		-env:URE_MORE_TYPES=$(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb))

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
