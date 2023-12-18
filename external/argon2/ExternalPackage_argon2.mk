# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,argon2,argon2))

$(eval $(call gb_ExternalPackage_use_external_project,argon2,argon2))

ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,argon2,$(LIBO_LIB_FOLDER)/Argon2OptDll.dll,vs2015/build/Argon2OptDll.dll))
endif # $(COM)

# vim: set noet sw=4 ts=4:
