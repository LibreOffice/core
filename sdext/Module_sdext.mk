# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,sdext))

$(eval $(call gb_Module_add_targets,sdext,\
    Library_PresenterScreen \
))

$(eval $(call gb_Module_add_targets,sdext,\
    Library_PresentationMinimizer \
))

ifeq ($(ENABLE_PDFIMPORT),TRUE)
$(eval $(call gb_Module_add_targets,sdext,\
    CustomTarget_pdfimport \
    Library_pdfimport \
))

ifeq ($(ENABLE_POPPLER),TRUE)
$(eval $(call gb_Module_add_targets,sdext,\
    Executable_xpdfimport \
    Package_pdfimport_xpdfimport \
))
endif

$(eval $(call gb_Module_add_check_targets,sdext,\
    CppunitTest_sdext_pdfimport \
    Executable_pdf2xml \
    Executable_pdfunzip \
))
endif

# vim:set noet sw=4 ts=4:
