# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_bin,$(OUTDIR)/bin))

$(eval $(call gb_Package_set_outdir,odk_bin,$(INSTDIR)))

$(eval $(call gb_Package_add_files,odk_bin,$(gb_Package_SDKDIRNAME)/bin,\
    $(addsuffix $(gb_Executable_EXT),\
	autodoc \
	$(if $(filter WNT,$(OS)),climaker) \
	cppumaker \
	idlc \
	javamaker \
	regcompare \
	$(if $(SYSTEM_UCPP),,ucpp) \
	uno-skeletonmaker \
	unoapploader \
    ) \
))

# vim: set noet sw=4 ts=4:
