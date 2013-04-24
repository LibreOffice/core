# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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

$(eval $(call gb_Executable_add_libs,tdefilepicker,\
	$(TDE_LIBS) \
	-ltdeio -lX11 \
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
