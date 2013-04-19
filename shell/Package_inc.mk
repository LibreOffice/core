# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,shell_inc,$(SRCDIR)/shell/inc/shell))

$(eval $(call gb_Package_add_files,shell_inc,inc/shell,\
	kde_headers.h \
	tde_defines.h \
	tde_headers.h \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
