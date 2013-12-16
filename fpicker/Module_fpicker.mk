# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,fpicker))

$(eval $(call gb_Module_add_targets,fpicker,\
	Library_fps_office \
	UIConfig_fps \
))

$(eval $(call gb_Module_add_l10n_targets,fpicker,\
	AllLangResTarget_fps_office \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,fpicker,\
	Library_fps_aqua \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,fpicker,\
	Library_fps \
	WinResTarget_fps \
))
endif

# vim: set noet sw=4 ts=4:
