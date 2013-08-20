# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,cpp))

$(eval $(call gb_Executable_add_cobjects,cpp,\
    soltools/cpp/_cpp \
    soltools/cpp/_eval \
    soltools/cpp/_include \
    soltools/cpp/_lex \
    soltools/cpp/_macro \
    soltools/cpp/_mcrvalid \
    soltools/cpp/_nlist \
    soltools/cpp/_tokens \
    soltools/cpp/_unix \
))

ifneq ($(filter AIX MACOSX WNT,$(OS)),)
$(eval $(call gb_Executable_add_cobjects,cpp,\
    soltools/cpp/_getopt \
))
endif

# vim:set noet sw=4 ts=4:
