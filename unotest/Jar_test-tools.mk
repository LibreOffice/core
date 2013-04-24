# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,test-tools))

$(eval $(call gb_Jar_add_manifest_classpath,test-tools,\
	juh.jar \
	ridl.jar \
	unoil.jar \
))

$(eval $(call gb_Jar_use_jars,test-tools,\
	juh \
	ridl \
	unoil \
))

$(eval $(call gb_Jar_set_packageroot,test-tools,org))

$(eval $(call gb_Jar_add_sourcefiles,test-tools,\
	unotest/source/java/org/openoffice/test/tools/DocumentType \
	unotest/source/java/org/openoffice/test/tools/OfficeDocument \
	unotest/source/java/org/openoffice/test/tools/OfficeDocumentView \
	unotest/source/java/org/openoffice/test/tools/SpreadsheetDocument \
	unotest/source/java/org/openoffice/test/tools/SpreadsheetView \
))

# vim: set noet sw=4 ts=4:
