# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,cui))

$(eval $(call gb_Module_add_targets,cui,\
	Library_cui \
))

$(eval $(call gb_Module_add_l10n_targets,cui,\
	AllLangResTarget_cui \
	UIConfig_cui \
))

$(eval $(call gb_Module_add_slowcheck_targets,cui,\
    CppunitTest_cui_dialogs_test \
))

# vim: set noet sw=4 ts=4:
