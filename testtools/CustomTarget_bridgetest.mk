# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,testtools/bridgetest))

testtools_BRIDGEDIR := $(call gb_CustomTarget_get_workdir,testtools/bridgetest)

ifeq ($(OS),WNT)
testtools_BATCHSUFFIX := .bat
else
testtools_BATCHSUFFIX :=
endif

$(call gb_CustomTarget_get_target,testtools/bridgetest) : \
	$(testtools_BRIDGEDIR)/bridgetest_server$(testtools_BATCHSUFFIX) \
	$(testtools_BRIDGEDIR)/bridgetest_client$(testtools_BATCHSUFFIX) \
	$(if $(ENABLE_JAVA),\
		$(testtools_BRIDGEDIR)/bridgetest_javaserver$(testtools_BATCHSUFFIX) \
		$(testtools_BRIDGEDIR)/bridgetest_inprocess_java$(testtools_BATCHSUFFIX))

$(testtools_BRIDGEDIR)/bridgetest_server$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Helper_abbreviate_dirs,\
		echo $(if $(filter MACOSX,$(OS)), "$(gb_Helper_LIBRARY_PATH_VAR)=\$${$(gb_Helper_LIBRARY_PATH_VAR):+\$$$(gb_Helper_LIBRARY_PATH_VAR):}$(OUTDIR)/lib") \
		"$(call gb_Executable_get_target_for_build,uno)" \
		"-s com.sun.star.test.bridge.CppTestObject" \
		"-u 'uno:socket$(COMMA)host=127.0.0.1$(COMMA)port=2002;urp;test'" \
		"--singleaccept" \
		"-env:LO_BUILD_LIB_DIR=$(call gb_Helper_make_url,$(gb_Library_WORKDIR_FOR_BUILD))" \
		"-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target,uno_services))" \
		"-env:URE_MORE_TYPES=$(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)


testtools_MY_CLASSPATH := $(OUTDIR)/bin/ridl.jar$(gb_CLASSPATHSEP)$(OUTDIR)/bin/java_uno.jar$(gb_CLASSPATHSEP)$(OUTDIR)/bin/jurt.jar$(gb_CLASSPATHSEP)$(OUTDIR)/bin/juh.jar

$(testtools_BRIDGEDIR)/bridgetest_javaserver$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Helper_abbreviate_dirs,\
		echo \
		"URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target,uno_services))" \
		"URE_MORE_TYPES=$(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)" \
		"java" \
		"-classpath $(testtools_MY_CLASSPATH)$(gb_CLASSPATHSEP)$(OUTDIR)/bin/testComponent.jar" \
		"com.sun.star.comp.bridge.TestComponentMain" \
		\""uno:socket$(COMMA)host=127.0.0.1$(COMMA)port=2002;urp;test"\" \
		"singleaccept" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)

$(testtools_BRIDGEDIR)/bridgetest_inprocess_java$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Helper_abbreviate_dirs,\
		echo "JAVA_HOME=$(JAVA_HOME)" \
		"$(gb_Helper_LIBRARY_PATH_VAR)=\$${$(gb_Helper_LIBRARY_PATH_VAR):+\$$$(gb_Helper_LIBRARY_PATH_VAR):}$(OUTDIR)/lib" \
		"$(call gb_Executable_get_target_for_build,uno)" \
		"-s com.sun.star.test.bridge.BridgeTest" \
		"-env:LO_BUILD_LIB_DIR=$(call gb_Helper_make_url,$(gb_Library_WORKDIR_FOR_BUILD))" \
		"-env:URE_INTERNAL_JAVA_DIR=file://$(OUTDIR)/bin" \
		"-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target,uno_services))" \
		"-env:URE_MORE_TYPES=$(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)" \
		"-- com.sun.star.test.bridge.JavaTestObject noCurrentContext" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)

$(testtools_BRIDGEDIR)/bridgetest_client$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Helper_abbreviate_dirs,\
		echo $(if $(filter MACOSX,$(OS)), "$(gb_Helper_LIBRARY_PATH_VAR)=\$${$(gb_Helper_LIBRARY_PATH_VAR):+\$$$(gb_Helper_LIBRARY_PATH_VAR):}$(OUTDIR)/lib") \
		"$(call gb_Executable_get_target_for_build,uno)" \
		"-s com.sun.star.test.bridge.BridgeTest --" \
		"-u 'uno:socket$(COMMA)host=127.0.0.1$(COMMA)port=2002;urp;test'" \
		"-env:LO_BUILD_LIB_DIR=$(call gb_Helper_make_url,$(gb_Library_WORKDIR_FOR_BUILD))" \
		"-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target,uno_services))" \
		"-env:URE_MORE_TYPES=$(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)

# vim: set noet sw=4 ts=4:
