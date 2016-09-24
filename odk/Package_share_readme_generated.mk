# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_share_readme_generated,$(WORKDIR)/CustomTarget/readlicense_oo))

$(eval $(call gb_Package_set_outdir,odk_share_readme_generated,$(INSTDIR)))

$(eval $(call gb_Package_add_files,odk_share_readme_generated,$(SDKDIRNAME)/share/readme,\
	$(if $(filter WNT,$(OS)),license/license.txt) \
	readme/$(call gb_README,en-US) \
))

# vim: set noet sw=4 ts=4:
