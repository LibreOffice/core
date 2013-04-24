# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,binaryurp))

$(eval $(call gb_Module_add_targets,binaryurp,\
	Library_binaryurp \
))

$(eval $(call gb_Module_add_check_targets,binaryurp,\
	CppunitTest_binaryurp_test-cache \
	CppunitTest_binaryurp_test-unmarshal \
))

# vim: set noet sw=4 ts=4:
