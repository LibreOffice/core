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

$(eval $(call gb_Jar_Jar,reportbuilderwizard))

$(eval $(call gb_Jar_use_jars,reportbuilderwizard,\
	ridl \
	unoil \
	jurt \
	juh \
	java_uno \
	commonwizards \
	report \
))

$(eval $(call gb_Jar_set_packageroot,reportbuilderwizard,com))

$(eval $(call gb_Jar_set_manifest,reportbuilderwizard,$(SRCDIR)/wizards/com/sun/star/wizards/reportbuilder/MANIFEST.MF))

$(eval $(call gb_Jar_add_sourcefiles,reportbuilderwizard,\
	wizards/com/sun/star/wizards/reportbuilder/ReportBuilderImplementation \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarSingleColumn \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarThreeColumns \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarTwoColumns \
	wizards/com/sun/star/wizards/reportbuilder/layout/DesignTemplate \
	wizards/com/sun/star/wizards/reportbuilder/layout/InBlocksLabelsAbove \
	wizards/com/sun/star/wizards/reportbuilder/layout/InBlocksLabelsLeft \
	wizards/com/sun/star/wizards/reportbuilder/layout/LayoutConstants \
	wizards/com/sun/star/wizards/reportbuilder/layout/ReportBuilderLayouter \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionEmptyObject \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionLabel \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionObject \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionTextField \
	wizards/com/sun/star/wizards/reportbuilder/layout/Tabular \
))

# vim: set noet sw=4 ts=4:
