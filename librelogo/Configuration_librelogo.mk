# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Configuration_Configuration,librelogo,nodeliver))

$(eval $(call gb_Configuration_use_configuration,librelogo,registry))

$(eval $(call gb_Configuration_add_localized_datas,librelogo,librelogo/source/registry/data,\
	org/openoffice/Office/Addons.xcu \
	org/openoffice/Office/UI/StartModuleWindowState.xcu \
	org/openoffice/Office/UI/WriterWindowState.xcu \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
