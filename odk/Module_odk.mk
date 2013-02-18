# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,odk))

$(eval $(call gb_Module_add_targets,odk,\
	CustomTarget_check \
	CustomTarget_odkcommon \
	CustomTarget_gendocu \
	Executable_unoapploader \
	Zip_odkcommon \
	Zip_odkexamples \
))

ifneq ($(SOLAR_JAVA),)
$(eval $(call gb_Module_add_targets,odk,\
	CustomTarget_unowinreg \
	CustomTarget_javadollar \
	$(if $(filter WNT,$(OS)),Library_unowinreg) \
	Zip_uno_loader_classes \
))
endif


# vim: set noet sw=4 ts=4:
