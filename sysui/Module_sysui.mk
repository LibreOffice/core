# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,sysui))

ifneq ($(OS),WNT)
ifneq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,sysui,\
	CustomTarget_share \
	CustomTarget_slackware \
	Package_share \
	Package_desktop \
	$(if $(filter rpm,$(PKGFORMAT)),CustomTarget_rpm) \
	$(if $(filter deb,$(PKGFORMAT)),CustomTarget_deb) \
	$(if $(filter SOLARIS,$(OS)),CustomTarget_solaris) \
))

else # OS=MACOSX
# CustomTarget_share builds documents.ulf which is needed in infoplist...
$(eval $(call gb_Module_add_targets,sysui,\
	CustomTarget_share \
	Package_osxicons \
	CustomTarget_infoplist \
	Package_infoplist \
))
endif
else # OS=WNT
$(eval $(call gb_Module_add_targets,sysui,\
	Package_icons \
))
endif

# vim: set noet sw=4 ts=4:
