# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_autodoc,$(call gb_CustomTarget_get_workdir,odk/docs/common/ref)))

$(eval $(call gb_Package_set_outdir,odk_autodoc,$(INSTDIR)))

# indices
$(eval $(call gb_Package_add_files_with_dir,odk_autodoc,$(gb_Package_SDKDIRNAME)/docs/common/ref,\
	index-files/index-1.html \
	index-files/index-10.html \
	index-files/index-11.html \
	index-files/index-12.html \
	index-files/index-13.html \
	index-files/index-14.html \
	index-files/index-15.html \
	index-files/index-16.html \
	index-files/index-17.html \
	index-files/index-18.html \
	index-files/index-19.html \
	index-files/index-2.html \
	index-files/index-20.html \
	index-files/index-21.html \
	index-files/index-22.html \
	index-files/index-23.html \
	index-files/index-24.html \
	index-files/index-25.html \
	index-files/index-26.html \
	index-files/index-27.html \
	index-files/index-3.html \
	index-files/index-4.html \
	index-files/index-5.html \
	index-files/index-6.html \
	index-files/index-7.html \
	index-files/index-8.html \
	index-files/index-9.html \
))

# module lists that are not already covered by generated _doc packages
$(eval $(call gb_Package_add_files_with_dir,odk_autodoc,$(gb_Package_SDKDIRNAME)/docs/common/ref,\
	com/module-ix.html \
	com/sun/module-ix.html \
	module-ix.html \
	org/freedesktop/module-ix.html \
	org/module-ix.html \
))

# vim: set noet sw=4 ts=4:
