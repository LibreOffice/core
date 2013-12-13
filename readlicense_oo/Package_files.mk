# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,readlicense_oo_files,$(SRCDIR)/readlicense_oo/license))

# LICENSE (upper case) is copied without EOL conversion
# license.txt is converted, prior to copy, see Package_license.mk
ifneq ($(OS),WNT)
$(eval $(call gb_Package_add_file,readlicense_oo_files,LICENSE,LICENSE))
endif

$(eval $(call gb_Package_add_file,readlicense_oo_files,NOTICE,NOTICE))

$(eval $(call gb_Package_add_file,readlicense_oo_files,LICENSE.fodt,LICENSE.fodt))
$(eval $(call gb_Package_add_file,readlicense_oo_files,CREDITS.fodt,CREDITS.fodt))

$(eval $(call gb_Package_add_file,readlicense_oo_files,LICENSE.html,LICENSE.html))

# vim: set noet sw=4 ts=4:
