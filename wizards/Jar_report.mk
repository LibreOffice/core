# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Jar_Jar,report))

$(eval $(call gb_Jar_use_jars,report,\
	libreoffice \
	java_uno \
	commonwizards \
))

$(eval $(call gb_Jar_set_manifest,report,$(SRCDIR)/wizards/com/sun/star/wizards/report/MANIFEST.MF))

$(eval $(call gb_Jar_add_manifest_classpath,report,reportbuilderwizard.jar))

$(eval $(call gb_Jar_set_packageroot,report,com))

$(eval $(call gb_Jar_add_sourcefiles,report,\
	wizards/com/sun/star/wizards/report/CallReportWizard \
	wizards/com/sun/star/wizards/report/Dataimport \
	wizards/com/sun/star/wizards/report/DBColumn \
	wizards/com/sun/star/wizards/report/GroupFieldHandler \
	wizards/com/sun/star/wizards/report/IReportBuilderLayouter \
	wizards/com/sun/star/wizards/report/IReportDefinitionReadAccess \
	wizards/com/sun/star/wizards/report/IReportDocument \
	wizards/com/sun/star/wizards/report/RecordTable \
	wizards/com/sun/star/wizards/report/ReportFinalizer \
	wizards/com/sun/star/wizards/report/ReportImplementationHelper \
	wizards/com/sun/star/wizards/report/ReportLayouter \
	wizards/com/sun/star/wizards/report/ReportTextDocument \
	wizards/com/sun/star/wizards/report/ReportTextImplementation \
	wizards/com/sun/star/wizards/report/ReportWizard \
))

$(eval $(call gb_Jar_set_componentfile,report,wizards/com/sun/star/wizards/report/report,OOO,services))

# vim: set noet sw=4 ts=4:
