# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

readlicense_oo_DIR := $(gb_CustomTarget_workdir)/readlicense_oo/license
$(eval $(call gb_Package_Package,odk_share_readme,$(readlicense_oo_DIR)))

$(eval $(call gb_Package_set_outdir,odk_share_readme,$(INSTDIR)))

$(eval $(call gb_Package_add_files,odk_share_readme,$(SDKDIRNAME)/share/readme,\
	LICENSE.html \
))

# for WNT see Package_share_readme_generated.mk
ifneq ($(OS),WNT)
$(eval $(call gb_Package_add_file,odk_share_readme,$(SDKDIRNAME)/share/readme/LICENSE,LICENSE))
endif

# vim: set noet sw=4 ts=4:
