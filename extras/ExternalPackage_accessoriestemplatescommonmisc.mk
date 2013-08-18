# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriestemplatescommonmisc,templates-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriestemplatescommonmisc,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriestemplatescommonmisc,$(LIBO_SHARE_FOLDER)/template/common/misc,\
	templates/common/misc/White_Notebook_Paper_legal.ott \
	templates/common/misc/Yellow_Pad_Legal.ott \
))

# vim: set noet sw=4 ts=4:
