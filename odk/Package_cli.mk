# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_cli,$(INSTROOT)))

$(eval $(call gb_Package_set_outdir,odk_cli,$(INSTDIR)))

$(eval $(call gb_Package_add_files,odk_cli,$(SDKDIRNAME)/cli,\
	$(LIBO_URE_LIB_FOLDER)/cli_cppuhelper.dll \
	$(LIBO_URE_LIB_FOLDER)/cli_basetypes.dll \
	$(LIBO_URE_LIB_FOLDER)/cli_ure.dll \
	$(LIBO_URE_LIB_FOLDER)/cli_uretypes.dll \
	$(LIBO_LIB_FOLDER)/cli_oootypes.dll \
))

# vim: set noet sw=4 ts=4:
