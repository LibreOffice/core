# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,twain_dsm,twain_dsm))

$(eval $(call gb_ExternalPackage_use_external_project,twain_dsm,twain_dsm))

ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,twain_dsm,$(LIBO_BIN_FOLDER)/TWAINDSM.dll,PCBuild/out/TWAINDSM.dll))
endif

# headers are not delivered, but used from unpacked dir pub/include/

# vim: set noet sw=4 ts=4:
