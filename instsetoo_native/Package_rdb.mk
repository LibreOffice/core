# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,instsetoo_native_rdb,$(OUTDIR)))

$(eval $(call gb_Package_set_outdir,instsetoo_native_rdb,$(INSTDIR)))

$(eval $(call gb_Package_add_file,instsetoo_native_rdb,program/services/services.rdb,xml/services.rdb))
$(eval $(call gb_Package_add_file,instsetoo_native_rdb,program/types/offapi.rdb,bin/offapi.rdb))
$(eval $(call gb_Package_add_file,instsetoo_native_rdb,ure/share/misc/services.rdb,xml/ure/services.rdb))
$(eval $(call gb_Package_add_file,instsetoo_native_rdb,ure/share/misc/types.rdb,bin/udkapi.rdb))

# vim: set noet sw=4 ts=4:
