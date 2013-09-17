# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,solenv_gdb,$(call gb_CustomTarget_get_workdir,solenv/gdb)))

# keep in sync with solenv/bin/install-gdb-printers
$(eval $(call gb_Package_add_files,solenv_gdb,lib,\
	$(if $(MERGELIBS), \
		$(call gb_Library_get_runtime_filename,merged)-gdb.py \
		$(if $(filter ALL,$(MERGELIBS)), \
			$(call gb_Library_get_runtime_filename,urelibs)-gdb.py \
		, \
			$(call gb_Library_get_runtime_filename,cppu)-gdb.py \
			$(call gb_Library_get_runtime_filename,sal)-gdb.py \
			$(call gb_Library_get_runtime_filename,sw)-gdb.py \
		) \
	, \
		$(call gb_Library_get_runtime_filename,basegfx)-gdb.py \
		$(call gb_Library_get_runtime_filename,cppu)-gdb.py \
		$(call gb_Library_get_runtime_filename,sal)-gdb.py \
		$(call gb_Library_get_runtime_filename,svl)-gdb.py \
		$(call gb_Library_get_runtime_filename,sw)-gdb.py \
		$(call gb_Library_get_runtime_filename,tl)-gdb.py \
	) \
	$(call gb_Library_get_runtime_filename,writerfilter)-gdb.py \
))

$(eval $(call gb_Package_use_custom_target,solenv_gdb,solenv/gdb))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
