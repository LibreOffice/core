# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,ucb))

$(eval $(call gb_Module_add_targets,ucb,\
	Library_cached1 \
	Library_srtrs1 \
	Library_ucb1 \
	$(if $(ENABLE_LIBCMIS),Library_ucpcmis1) \
	$(if $(WITH_WEBDAV),Library_ucpdav1) \
	$(if $(ENABLE_GDRIVE),Library_ucpgdrive) \
	Library_ucpdropbox \
	Library_ucpexpand1 \
	Library_ucpext \
	Library_ucpfile1 \
	Library_ucphier1 \
	Library_ucpimage \
	Library_ucppkg1 \
	Library_ucptdoc1 \
))

$(eval $(call gb_Module_add_check_targets,ucb,\
	$(if $(ENABLE_GDRIVE),CppunitTest_ucb_gdrive) \
))

ifeq ($(ENABLE_GIO),TRUE)
$(eval $(call gb_Module_add_targets,ucb,\
	Library_ucpgio1 \
))
endif

ifneq ($(WITH_WEBDAV),)

$(eval $(call gb_Module_add_check_targets,ucb,\
    CppunitTest_ucb_webdav_core \
))

endif

$(eval $(call gb_Module_add_subsequentcheck_targets,ucb,\
	JunitTest_ucb_unoapi \
	$(if $(WITH_WEBDAV),JunitTest_ucb_webdav_unoapi) \
))

# vim: set noet sw=4 ts=4:
