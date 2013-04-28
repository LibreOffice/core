# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,solenv_gdb_install,$(call gb_CustomTarget_get_workdir,solenv/gdb)))

$(eval $(call gb_Package_set_outdir,solenv_gdb_install,$(INSTDIR)))

$(eval $(call gb_Package_add_files,solenv_gdb_install,program,\
	$(call gb_Library_get_runtime_filename,svl)-gdb.py \
	$(call gb_Library_get_runtime_filename,sw)-gdb.py \
	$(call gb_Library_get_runtime_filename,tl)-gdb.py \
))

$(eval $(call gb_Package_add_files,solenv_gdb_install,ure/lib,\
	$(call gb_Library_get_runtime_filename,cppu).3-gdb.py \
	$(call gb_Library_get_runtime_filename,sal).3-gdb.py \
))

$(eval $(call gb_Package_use_custom_target,solenv_gdb_install,solenv/gdb))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
