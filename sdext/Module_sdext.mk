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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Module_Module,sdext))

ifeq ($(ENABLE_MINIMIZER),YES)
$(eval $(call gb_Module_add_targets,sdext,\
    Configuration_minimizer \
    Extension_minimizer \
    Library_minimizer \
    Rdb_minimizer \
))
endif

ifeq ($(ENABLE_PDFIMPORT),YES)
$(eval $(call gb_Module_add_targets,sdext,\
    CustomTarget_pdfimport \
    Executable_pdf2xml \
    Executable_pdfunzip \
    Executable_xpdfimport \
    Extension_pdfimport \
    Library_pdfimport \
    Rdb_pdfimport \
    StaticLibrary_pdfimport_s \
))

$(eval $(call gb_Module_add_check_targets,sdext,\
    CppunitTest_pdfimport \
))
endif

ifeq ($(ENABLE_PRESENTER_SCREEN),YES)
$(eval $(call gb_Module_add_targets,sdext,\
    Configuration_presenter \
    Extension_presenter \
    Library_presenter \
    Rdb_presenter \
))
endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
