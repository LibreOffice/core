# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,connectivity_postgresql-sdbc,$(SRCDIR)/connectivity/source/drivers/postgresql))

$(eval $(call gb_Package_add_file,connectivity_postgresql-sdbc,$(LIBO_ETC_FOLDER)/postgresql-sdbc.ini,postgresql-sdbc.ini))

# vim: set noet sw=4 ts=4:
