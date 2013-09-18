# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_lib,$(OUTDIR)/lib))

$(eval $(call gb_Package_set_outdir,odk_lib,$(INSTDIR)))

$(eval $(call gb_Package_add_files,odk_lib,$(gb_Package_SDKDIRNAME)/lib,\
	$(if $(filter LINUX,$(OS)),$(notdir $(call gb_StaticLibrary_get_target,salcpprt))) \
))

# vim: set noet sw=4 ts=4:
