# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,xmlsecurity))

$(eval $(call gb_Module_add_targets,xmlsecurity,\
	Library_xmlsecurity \
	$(if $(ENABLE_NSS)$(ENABLE_OPENSSL),Library_xsec_xmlsec) \
	UIConfig_xmlsec \
))

$(eval $(call gb_Module_add_slowcheck_targets,xmlsecurity,\
    CppunitTest_xmlsecurity_pdfsigning \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,xmlsecurity,\
    CppunitTest_xmlsecurity_signing \
))

$(eval $(call gb_Module_add_l10n_targets,xmlsecurity,\
	AllLangMoTarget_xsc \
))

# failing
#$(eval $(call gb_Module_add_check_targets,xmlsecurity,\
	CppunitTest_qa_certext \
))

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,xmlsecurity,\
    CppunitTest_xmlsecurity_dialogs_test \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
ifneq (,$(or $(ENABLE_NSS),$(filter WNT,$(OS))))
$(eval $(call gb_Module_add_targets,xmlsecurity,\
    $(if $(DISABLE_DYNLOADING),,Executable_pdfverify) \
))
endif
endif

# vim: set noet sw=4 ts=4:
