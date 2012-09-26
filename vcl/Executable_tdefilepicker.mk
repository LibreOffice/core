# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
# Copyright (C) 2012 Timothy Pearson (TDE)
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

$(eval $(call gb_Executable_Executable,tdefilepicker))

$(eval $(call gb_Executable_add_defs,tdefilepicker,\
	$(TDE_CFLAGS) \
	-DENABLE_TDE \
))

$(eval $(call gb_Executable_set_include,tdefilepicker,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/solenv/inc \
    -I$(SRCDIR)/vcl/inc/unx/tde \
))

$(eval $(call gb_Executable_use_libraries,tdefilepicker,\
	sal \
))

$(eval $(call gb_Executable_add_standard_system_libs,tdefilepicker))

$(eval $(call gb_Executable_add_libs,tdefilepicker,\
	$(TDE_LIBS) \
	-lkio -lX11 \
))

$(eval $(call gb_Executable_add_exception_objects,tdefilepicker,\
	vcl/unx/kde/fpicker/kdecommandthread \
	vcl/unx/kde/fpicker/kdefilepicker \
	vcl/unx/kde/fpicker/kdefpmain \
	vcl/unx/kde/fpicker/kdemodalityfilter \
))

$(eval $(call gb_Executable_add_generated_cxxobjects,tdefilepicker,\
	CustomTarget/vcl/unx/kde/fpicker/kdefilepicker.moc \
))

$(eval $(call gb_Executable_set_warnings_not_errors,tdefilepicker))

# TDE/TQt consider -Wshadow more trouble than benefit
$(eval $(call gb_Executable_add_cxxflags,kdefilepicker,\
	-Wno-shadow \
))

# vim: set noet sw=4 ts=4:
