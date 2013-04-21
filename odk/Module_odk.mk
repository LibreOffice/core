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
	CustomTarget_odkcommon \
	$(if $(filter WNT,$(OS)),Package_cli) \
	$(if $(DOXYGEN),CustomTarget_doxygen) \
	CustomTarget_check \
	CustomTarget_lib \
	CustomTarget_settings \
	CustomTarget_autodoc \
	Executable_unoapploader \
	Package_bin \
	Package_examples \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,odk,\
	Package_macosx \
))
endif

ifneq ($(SOLAR_JAVA),)
$(eval $(call gb_Module_add_targets,odk,\
	CustomTarget_unowinreg \
	CustomTarget_classes \
	CustomTarget_javadoc \
	$(if $(filter WNT,$(OS)),Library_unowinreg) \
	Zip_uno_loader_classes \
))
endif

# This apparently needs to come last, as the various CustomTarget_* add to
# odkcommon_ZIPLIST that is used here:
$(eval $(call gb_Module_add_targets,odk,\
	Zip_odkcommon \
))


# vim: set noet sw=4 ts=4:
