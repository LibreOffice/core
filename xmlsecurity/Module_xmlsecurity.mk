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
	Library_xsec_fw \
	$(if $(filter-out ANDROID IOS,$(OS)),Library_xsec_xmlsec) \
))

$(eval $(call gb_Module_add_slowcheck_targets,xmlsecurity,\
    CppunitTest_xmlsecurity_signing \
))

$(eval $(call gb_Module_add_l10n_targets,xmlsecurity,\
	AllLangResTarget_xsec \
	UIConfig_xmlsec \
))

#FIXME: ^^^, get nss&libxmlsec building on ios and android

# failing
#$(eval $(call gb_Module_add_check_targets,xmlsecurity,\
	CppunitTest_qa_certext \
))

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,xmlsecurity,\
    CppunitTest_xmlsecurity_dialogs_test \
))

# vim: set noet sw=4 ts=4:
