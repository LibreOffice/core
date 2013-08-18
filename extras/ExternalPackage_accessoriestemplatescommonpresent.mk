# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriestemplatescommonpresent,templates-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriestemplatescommonpresent,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriestemplatescommonpresent,$(LIBO_SHARE_FOLDER)/template/common/presnt,\
	templates/common/presnt/dummy_common_templates.txt \
))

# vim: set noet sw=4 ts=4:
