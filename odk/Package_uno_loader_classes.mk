# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_uno_loader_classes,$(call gb_CustomTarget_get_workdir,odk/classes)))

$(eval $(call gb_Package_set_outdir,odk_uno_loader_classes,$(INSTDIR)))

$(eval $(call gb_Package_add_files_with_dir,odk_uno_loader_classes,$(gb_Package_SDKDIRNAME)/classes,\
	com/sun/star/lib/loader/InstallationFinder$$StreamGobbler.class \
	com/sun/star/lib/loader/InstallationFinder.class \
	com/sun/star/lib/loader/Loader$$CustomURLClassLoader.class \
	com/sun/star/lib/loader/Loader$$Drain.class \
	com/sun/star/lib/loader/Loader$$Drain.class \
	com/sun/star/lib/loader/Loader.class \
	com/sun/star/lib/loader/WinRegKey.class \
	com/sun/star/lib/loader/WinRegKeyException.class \
))

# vim: set noet sw=4 ts=4:
