# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,avmedia))

$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmedia \
))

ifneq ($(USE_AVMEDIA_DUMMY),TRUE)
$(eval $(call gb_Module_add_l10n_targets,avmedia,\
    AllLangMoTarget_avmedia \
))

ifeq ($(ENABLE_GSTREAMER_1_0),TRUE)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediagst \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediaMacAVF \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediawin \
))
endif

endif

# vim: set noet sw=4 ts=4:
