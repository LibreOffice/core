# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,jvmaccess))

ifeq ($(ENABLE_JAVA),TRUE)

$(eval $(call gb_Module_add_targets,jvmaccess,\
))

$(eval $(call gb_Module_add_targets,jvmaccess,\
    Library_jvmaccess \
))

endif

# vim:set noet sw=4 ts=4:
