# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,mkdepend))

$(eval $(call gb_Executable_add_exception_objects,mkdepend,\
    soltools/mkdepend/collectdircontent \
))

$(eval $(call gb_Executable_add_defs,mkdepend,\
    -DNO_X11 \
    -DXP_PC \
    -DHW_THREADS \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Executable_add_defs,mkdepend,\
    -wd4100 \
    -wd4131 \
    -wd4706DNO_X11 \
))

$(eval $(call gb_Executable_use_libraries,mkdepend,\
    msvcprt \
))
endif

$(eval $(call gb_Executable_add_cobjects,mkdepend,\
    soltools/mkdepend/cppsetup \
    soltools/mkdepend/ifparser \
    soltools/mkdepend/include \
    soltools/mkdepend/main \
    soltools/mkdepend/parse \
    soltools/mkdepend/pr \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
