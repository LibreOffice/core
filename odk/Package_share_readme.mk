# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_share_readme,$(SRCDIR)))

$(eval $(call gb_Package_set_outdir,odk_share_readme,$(INSTDIR)))

$(eval $(call gb_Package_add_files,odk_share_readme,$(SDKDIRNAME)/share/readme,\
	readlicense_oo/license/LICENSE.html \
))

# for WNT see Package_share_readme_generated.mk
ifneq ($(OS),WNT)
$(eval $(call gb_Package_add_file,odk_share_readme,$(SDKDIRNAME)/share/readme/LICENSE,readlicense_oo/license/LICENSE))
endif

# vim: set noet sw=4 ts=4:
