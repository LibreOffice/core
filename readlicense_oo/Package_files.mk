# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,readlicense_oo_files,$(SRCDIR)/readlicense_oo))

# LICENSE (upper case) is copied without EOL conversion from license.txt
# license.txt is converted, prior to copy, see Package_license.mk
$(eval $(call gb_Package_add_file,readlicense_oo_files,bin/osl/LICENSE,txt/license.txt))
$(eval $(call gb_Package_add_file,readlicense_oo_files,bin/ure/LICENSE,txt/license.txt))

$(eval $(call gb_Package_add_file,readlicense_oo_files,bin/NOTICE,txt/NOTICE))

$(eval $(call gb_Package_add_file,readlicense_oo_files,bin/osl/LICENSE.odt,odt/LICENSE.odt))
$(eval $(call gb_Package_add_file,readlicense_oo_files,bin/LICENSE.odt,odt/LICENSE.odt))
$(eval $(call gb_Package_add_file,readlicense_oo_files,bin/CREDITS.odt,odt/CREDITS.odt))

$(eval $(call gb_Package_add_file,readlicense_oo_files,bin/THIRDPARTYLICENSEREADME.html,html/THIRDPARTYLICENSEREADME.html))

# vim: set noet sw=4 ts=4:
