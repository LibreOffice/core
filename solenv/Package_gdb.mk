# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,solenv_gdb,$(call gb_CustomTarget_get_workdir,solenv/gdb)))

$(eval $(call gb_Package_add_files,solenv_gdb,lib,\
	$(if $(MERGELIBS), \
	$(if $(filter ALL,$(MERGELIBS)), \
		$(call gb_Library_get_runtime_filename,urelibs)-gdb.py, \
		$(call gb_Library_get_runtime_filename,cppu).3-gdb.py \
		$(call gb_Library_get_runtime_filename,sal).3-gdb.py \
		$(call gb_Library_get_runtime_filename,sw)-gdb.py \
	) \
	$(call gb_Library_get_runtime_filename,merged)-gdb.py, \
	$(call gb_Library_get_runtime_filename,cppu).3-gdb.py \
	$(call gb_Library_get_runtime_filename,sal).3-gdb.py \
	$(call gb_Library_get_runtime_filename,svl)-gdb.py \
	$(call gb_Library_get_runtime_filename,sw)-gdb.py \
	$(call gb_Library_get_runtime_filename,tl)-gdb.py) \
))

$(eval $(call gb_Package_use_custom_target,solenv_gdb,solenv/gdb))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
