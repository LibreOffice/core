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

ifeq ($(ENABLE_MINIMIZER),TRUE)
$(eval $(call gb_Module_add_targets,sdext,\
    Configuration_minimizer \
    Extension_minimizer \
    Library_minimizer \
    Rdb_minimizer \
))
endif

ifeq ($(ENABLE_PDFIMPORT),TRUE)
$(eval $(call gb_Module_add_targets,sdext,\
    CustomTarget_pdfimport \
    Executable_xpdfimport \
    Library_pdfimport \
    Package_pdfimport_xcu \
    Package_pdfimport_xpdfimport \
    StaticLibrary_pdfimport_s \
))

$(eval $(call gb_Module_add_check_targets,sdext,\
    CppunitTest_pdfimport \
    Executable_pdf2xml \
    Executable_pdfunzip \
))
endif

# vim:set noet sw=4 ts=4:
