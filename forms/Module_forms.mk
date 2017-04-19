# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,forms))

$(eval $(call gb_Module_add_targets,forms,\
    Library_frm \
))

$(eval $(call gb_Module_add_l10n_targets,forms,\
    AllLangMoTarget_frm \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,forms,\
    JunitTest_forms_unoapi_1 \
    JunitTest_forms_unoapi_2 \
    JunitTest_forms_unoapi_3 \
    JunitTest_forms_unoapi_4 \
    JunitTest_forms_complex \
))

# vim: set noet sw=4 ts=4:
