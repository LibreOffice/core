# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,unotest))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,unotest,\
	Library_unobootstrapprotector \
	Library_unoexceptionprotector \
	Library_unotest \
))
endif

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_targets,unotest,\
	Jar_test-tools \
))

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_targets,unotest,\
	Jar_test \
))
endif
endif

# vim: set noet sw=4 ts=4:
