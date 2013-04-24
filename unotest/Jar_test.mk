# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,test))

$(eval $(call gb_Jar_add_manifest_classpath,test,\
	juh.jar \
	ridl.jar \
	unoil.jar \
))

$(eval $(call gb_Jar_use_jars,test,\
	juh \
	ridl \
	unoil \
	OOoRunner \
))

$(eval $(call gb_Jar_use_system_jars,test,\
	$(OOO_JUNIT_JAR) \
))

$(eval $(call gb_Jar_set_packageroot,test,org))

$(eval $(call gb_Jar_add_sourcefiles,test,\
	unotest/source/java/org/openoffice/test/Argument \
	unotest/source/java/org/openoffice/test/FileHelper \
	unotest/source/java/org/openoffice/test/OfficeConnection \
	unotest/source/java/org/openoffice/test/OfficeFileUrl \
	unotest/source/java/org/openoffice/test/UnoApiTest \
))

# vim: set noet sw=4 ts=4:
