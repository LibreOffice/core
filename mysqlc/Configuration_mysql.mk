# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Configuration_Configuration,mysql,nodeliver))

$(eval $(call gb_Configuration_add_localized_datas,mysql,mysqlc/source/registry/data,\
	org/openoffice/Office/DataAccess/Drivers.xcu \
))

