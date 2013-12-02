# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,package))

$(eval $(call gb_Module_add_targets,package,\
	Library_package2 \
	Library_xstor \
	Package_dtd \
))

$(eval $(call gb_Module_add_check_targets,package,\
    CppunitTest_package2_test \
))

# vim: set noet sw=4 ts=4:
