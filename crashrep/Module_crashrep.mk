# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,crashrep))

ifeq ($(ENABLE_CRASHDUMP),TRUE)

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,crashrep,\
	CustomTarget_crashrep_res \
	Executable_crashrep \
	WinResTarget_crashrep \
))
else ifneq ($(PRODUCT),)
$(eval $(call gb_Module_add_targets,crashrep,\
	Executable_crashrep \
))
endif
endif

# vim: set noet sw=4 ts=4:
