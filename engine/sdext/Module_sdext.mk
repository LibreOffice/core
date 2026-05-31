# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,sdext))

$(eval $(call gb_Module_add_targets,sdext,\
    Library_PresentationMinimizer \
))

$(eval $(call gb_Module_add_l10n_targets,sdext,\
       AllLangMoTarget_sdext \
))

ifeq ($(ENABLE_PDFIMPORT),TRUE)
$(eval $(call gb_Module_add_targets,sdext,\
    Library_pdfimport \
))

$(eval $(call gb_Module_add_check_targets,sdext,\
    Executable_pdfunzip \
))
endif

# vim:set noet sw=4 ts=4:
