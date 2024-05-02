# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,testtools/bridgetest))

testtools_BRIDGEDIR := $(gb_CustomTarget_workdir)/testtools/bridgetest

ifeq ($(OS),WNT)
testtools_BATCHSUFFIX := .bat
else
testtools_BATCHSUFFIX :=
endif

$(call gb_CustomTarget_get_target,testtools/bridgetest) : \
	$(testtools_BRIDGEDIR)/bridgetest_server$(testtools_BATCHSUFFIX) \
	$(testtools_BRIDGEDIR)/bridgetest_client$(testtools_BATCHSUFFIX) \
	$(if $(ENABLE_JAVA),\
		$(testtools_BRIDGEDIR)/bridgetest_javaserver$(testtools_BATCHSUFFIX))

$(testtools_BRIDGEDIR)/bridgetest_server$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	$(call gb_Helper_abbreviate_dirs,\
		echo \
		"$(call gb_Executable_get_target_for_build,uno)" \
		"-s com.sun.star.test.bridge.CppTestObject" \
		"-u 'uno:socket$(COMMA)host=127.0.0.1$(COMMA)port=2002;urp;test'" \
		"--singleaccept" \
		"-env:LO_BUILD_LIB_DIR=$(call gb_Helper_make_url,$(gb_Library_WORKDIR_FOR_BUILD))" \
		"-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target,uno_services))" \
		"-env:URE_MORE_TYPES=$(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)


testtools_MY_CLASSPATH := $(call gb_Jar_get_target,ridl)$(gb_CLASSPATHSEP)$(call gb_Jar_get_target,java_uno)$(gb_CLASSPATHSEP)$(call gb_Jar_get_target,juh)

$(testtools_BRIDGEDIR)/bridgetest_javaserver$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	$(call gb_Helper_abbreviate_dirs,\
		echo \
		"URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target,uno_services))" \
		"URE_MORE_TYPES=$(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)" \
		"java" \
		"-classpath $(testtools_MY_CLASSPATH)$(gb_CLASSPATHSEP)$(call gb_Jar_get_target,testComponent)" \
		"com.sun.star.comp.bridge.TestComponentMain" \
		\""uno:socket$(COMMA)host=127.0.0.1$(COMMA)port=2002;urp;test"\" \
		"singleaccept" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(testtools_BRIDGEDIR)/bridgetest_client$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	$(call gb_Helper_abbreviate_dirs,\
		echo \
		"$(call gb_Executable_get_target_for_build,uno)" \
		"-s com.sun.star.test.bridge.BridgeTest --" \
		"-u 'uno:socket$(COMMA)host=127.0.0.1$(COMMA)port=2002;urp;test'" \
		"-env:LO_BUILD_LIB_DIR=$(call gb_Helper_make_url,$(gb_Library_WORKDIR_FOR_BUILD))" \
		"-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target,uno_services))" \
		"-env:URE_MORE_TYPES=$(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

# vim: set noet sw=4 ts=4:
