# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,javadep))

ifeq ($(COM),MSC)
$(eval $(call gb_Executable_use_packages,javadep,\
    soltools_inc \
))
endif

$(eval $(call gb_Executable_add_cobjects,javadep,\
    soltools/javadep/javadep \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
