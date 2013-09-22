# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_cfgusr,$(SRCDIR)/extras/source/misc_config))

$(eval $(call gb_Package_set_outdir,extras_cfgusr,$(INSTROOT)))

$(eval $(call gb_Package_add_files,extras_cfgusr,$(LIBO_SHARE_PRESETS_FOLDER)/config,\
    autotbl.fmt \
))

# vim: set noet sw=4 ts=4:
