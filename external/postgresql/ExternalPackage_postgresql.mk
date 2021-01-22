# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,postgresql,postgresql))

$(eval $(call gb_ExternalPackage_use_external_project,postgresql,postgresql))

$(eval $(call gb_ExternalPackage_add_file,postgresql,$(LIBO_LIB_FOLDER)/libpq.dll,$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)/libpq/libpq.dll))

# vim: set noet sw=4 ts=4:
