# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,reportbuilder_xcs,$(SRCDIR)/officecfg/registry/schema/org/openoffice/Office))

$(eval $(call gb_Package_add_file,reportbuilder_xcs,xml/registry/schema/org/openoffice/Office/ReportDesign.xcs,ReportDesign.xcs))

$(eval $(call gb_Package_add_file,reportbuilder_xcs,xml/registry/schema/org/openoffice/Office/UI/DbReportWindowState.xcs,UI/DbReportWindowState.xcs))
$(eval $(call gb_Package_add_file,reportbuilder_xcs,xml/registry/schema/org/openoffice/Office/UI/ReportCommands.xcs,UI/ReportCommands.xcs))


# vim: set noet sw=4 ts=4:
