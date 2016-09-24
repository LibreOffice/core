# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,embedserv))

ifeq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,embedserv,\
	$(if $(filter WNT-TRUE,$(OS)-$(DISABLE_ATL)),,Library_inprocserv) \
))

ifeq ($(DISABLE_ATL),)
$(eval $(call gb_Module_add_targets,embedserv,\
	Library_emser \
))
endif

endif

# vim: set noet sw=4 ts=4:
