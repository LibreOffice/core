# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,liborcus,$(call gb_UnpackedTarball_get_dir,orcus)))

$(eval $(call gb_Package_use_external_project,liborcus,liborcus))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Package_add_file,liborcus,lib/orcus.lib,vsprojects/liborcus-static-nozip/Release/orcus.lib))
else
$(eval $(call gb_Package_add_file,liborcus,lib/liborcus-0.2.a,src/liborcus/.libs/liborcus-0.2.a))
$(eval $(call gb_Package_add_file,liborcus,lib/liborcus.a,src/liborcus/.libs/liborcus-0.2.a))
endif

# vim: set noet sw=4 ts=4:
