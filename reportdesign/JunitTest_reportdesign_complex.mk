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

$(eval $(call gb_JunitTest_JunitTest,reportdesign_complex))

$(eval $(call gb_JunitTest_set_defs,reportdesign_complex,\
	$$(DEFS) \
	-Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/reportdesign/qa/complex/reportdesign/test_documents \
))

$(eval $(call gb_JunitTest_add_sourcefiles,reportdesign_complex,\
	reportdesign/qa/complex/reportdesign/ReportDesignerTest \
	reportdesign/qa/complex/reportdesign/TestDocument \
	reportdesign/qa/complex/reportdesign/FileURL \
))

$(eval $(call gb_JunitTest_use_jars,reportdesign_complex,\
	OOoRunner \
	ridl \
	test \
	unoil \
	jurt \
))

$(eval $(call gb_JunitTest_add_classes,reportdesign_complex,\
	complex.reportdesign.ReportDesignerTest \
))

# vim: set noet sw=4 ts=4:
