# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_GeneratedPackage_GeneratedPackage,python3,$(call gb_UnpackedTarball_get_dir,python3)/python-inst/@__________________________________________________OOO))

$(eval $(call gb_GeneratedPackage_use_unpacked,python3,python3))

$(eval $(call gb_GeneratedPackage_use_external_project,python3,python3))

$(eval $(call gb_GeneratedPackage_add_dir,python3,$(INSTROOT)/$(if $(ENABLE_MACOSX_MACLIKE_APP_STRUCTURE),Frameworks,$(gb_Package_PROGRAMDIRNAME))/LibreOfficePython.framework,LibreOfficePython.framework))

# vim: set noet sw=4 ts=4:
