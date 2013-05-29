# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,avmedia))

$(eval $(call gb_Module_add_targets,avmedia,\
	AllLangResTarget_avmedia \
	Library_avmedia \
))

ifeq ($(ENABLE_GSTREAMER),TRUE)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediagst \
))
endif

ifeq ($(ENABLE_GSTREAMER_0_10),TRUE)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediagst_0_10 \
))
endif

ifeq ($(ENABLE_VLC),TRUE)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediavlc \
))
endif

ifeq ($(OS),MACOSX)
ifneq ($(CPUNAME),X86_64)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediaQuickTime \
))
endif
endif

ifneq ($(ENABLE_DIRECTX),)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediawin \
))
endif

# vim: set noet sw=4 ts=4:
