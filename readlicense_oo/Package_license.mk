# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,readlicense_oo_license,$(call gb_CustomTarget_get_workdir,readlicense_oo/license)))

$(eval $(call gb_Package_add_file,readlicense_oo_license,bin/osl/license.txt,license.txt))

# vim: set noet sw=4 ts=4:
