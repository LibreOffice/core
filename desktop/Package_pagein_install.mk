# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,desktop_pagein_install,$(OUTDIR)/bin))

$(eval $(call gb_Package_set_outdir,desktop_pagein_install,$(INSTDIR)))

$(eval $(call gb_Package_add_files,desktop_pagein_install,$(gb_Pyuno_PROGRAMDIRNAME),\
    pagein-calc \
    pagein-common \
    pagein-draw \
    pagein-impress \
    pagein-writer \
))

# vim: set noet sw=4 ts=4:
