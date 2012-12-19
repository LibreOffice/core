# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 David Ostrovsky <d.ostrovsky@gmx.de> (initial developer)
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
	$(if $(SOLAR_JAVA),\
		$(testtools_BRIDGEDIR)/bridgetest_javaserver$(testtools_BATCHSUFFIX) \
		$(testtools_BRIDGEDIR)/bridgetest_inprocess_java$(testtools_BATCHSUFFIX))

$(testtools_BRIDGEDIR)/bridgetest_server$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Helper_abbreviate_dirs,\
		echo "$(call gb_Executable_get_target_for_build,uno)" \
		"-s com.sun.star.test.bridge.CppTestObject" \
		"-u 'uno:socket$(COMMA)host=127.0.0.1$(COMMA)port=2002;urp;test'" \
		"--singleaccept" \
		"-env:UNO_SERVICES='$(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target,ure/services)) $(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target,uno_services))'" \
		"-env:UNO_TYPES='$(call gb_Helper_make_url,$(call gb_UnoApiMerge_get_target,ure/types)) $(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)'" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)


testtools_MY_CLASSPATH := $(OUTDIR)/bin/ridl.jar$(gb_CLASSPATHSEP)$(OUTDIR)/bin/java_uno.jar$(gb_CLASSPATHSEP)$(OUTDIR)/bin/jurt.jar$(gb_CLASSPATHSEP)$(OUTDIR)/bin/juh.jar

$(testtools_BRIDGEDIR)/bridgetest_javaserver$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Helper_abbreviate_dirs,\
		echo \
		"UNO_SERVICES='$(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target,ure/services)) $(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target,uno_services))'" \
		"UNO_TYPES='$(call gb_Helper_make_url,$(call gb_UnoApiMerge_get_target,ure/types)) $(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)'" \
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
		"LD_LIBRARY_PATH=$(OUTDIR)/lib" \
		"$(call gb_Executable_get_target_for_build,uno)" \
		"-s com.sun.star.test.bridge.BridgeTest" \
		"-env:UNO_SERVICES='$(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target,ure/services)) $(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target,uno_services))'" \
		"-env:UNO_TYPES='$(call gb_Helper_make_url,$(call gb_UnoApiMerge_get_target,ure/types)) $(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)'" \
		"-env:URE_INTERNAL_JAVA_DIR=file://$(OUTDIR)/bin" \
		"-env:URE_INTERNAL_LIB_DIR=file://$(OUTDIR)/lib" \
		"-- com.sun.star.test.bridge.JavaTestObject noCurrentContext" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)

$(testtools_BRIDGEDIR)/bridgetest_client$(testtools_BATCHSUFFIX) :| $(testtools_BRIDGEDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Helper_abbreviate_dirs,\
		echo "$(call gb_Executable_get_target_for_build,uno)" \
		"-s com.sun.star.test.bridge.BridgeTest --" \
		"-u 'uno:socket$(COMMA)host=127.0.0.1$(COMMA)port=2002;urp;test'" \
		"-env:UNO_SERVICES='$(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target,ure/services)) $(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target,uno_services))'" \
		"-env:UNO_TYPES='$(call gb_Helper_make_url,$(call gb_UnoApiMerge_get_target,ure/types)) $(call gb_Helper_make_url,$(WORKDIR)/UnoApiTarget/bridgetest.rdb)'" \
		> $@)
	$(if $(filter-out WNT,$(OS)),chmod +x $@)

# vim: set noet sw=4 ts=4:
