# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,reportbuilder_xcu,$(SRCDIR)/officecfg/registry/data/org/openoffice))

$(eval $(call gb_Package_add_file,reportbuilder_xcu,xml/registry/data/org/openoffice/Office/ReportDesign.xcu,Office/ReportDesign.xcu))
$(eval $(call gb_Package_add_file,reportbuilder_xcu,xml/registry/data/org/openoffice/Office/ExtendedColorScheme.xcu,Office/ExtendedColorScheme.xcu))

$(eval $(call gb_Package_add_file,reportbuilder_xcu,xml/registry/data/org/openoffice/Office/UI/DbReportWindowState.xcu,Office/UI/DbReportWindowState.xcu))
$(eval $(call gb_Package_add_file,reportbuilder_xcu,xml/registry/data/org/openoffice/Office/UI/ReportCommands.xcu,Office/UI/ReportCommands.xcu))

$(eval $(call gb_Package_add_file,reportbuilder_xcu,xml/registry/data/org/openoffice/TypeDetection/Filter.xcu,TypeDetection/Filter.xcu))
$(eval $(call gb_Package_add_file,reportbuilder_xcu,xml/registry/data/org/openoffice/TypeDetection/Types.xcu,TypeDetection/Types.xcu))

# vim: set noet sw=4 ts=4:
