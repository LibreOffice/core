# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,poppler,poppler))

$(eval $(call gb_ExternalPackage_use_external_project,poppler,poppler))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_ExternalPackage_add_file,poppler,lib/poppler.lib,poppler/.libs/libpoppler.lib))
$(eval $(call gb_ExternalPackage_add_file,poppler,lib/fofi.lib,fofi/.libs/libfofi.lib))
$(eval $(call gb_ExternalPackage_add_file,poppler,lib/goo.lib,goo/.libs/libgoo.lib))
else
$(eval $(call gb_ExternalPackage_add_file,poppler,lib/libpoppler.a,poppler/.libs/libpoppler.a))
$(eval $(call gb_ExternalPackage_add_file,poppler,lib/libfofi.a,fofi/.libs/libfofi.a))
$(eval $(call gb_ExternalPackage_add_file,poppler,lib/libgoo.a,goo/.libs/libgoo.a))
endif

# vim: set noet sw=4 ts=4:
