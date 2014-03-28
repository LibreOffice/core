# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,kdefilepicker))

$(eval $(call gb_Executable_add_defs,kdefilepicker,\
	$(KDE_CFLAGS) \
))

$(eval $(call gb_Executable_use_libraries,kdefilepicker,\
	sal \
))

$(eval $(call gb_Executable_add_libs,kdefilepicker,\
	$(KDE_LIBS) \
	-lkio -lX11 \
))

$(eval $(call gb_Executable_add_exception_objects,kdefilepicker,\
	vcl/unx/kde/fpicker/kdecommandthread \
	vcl/unx/kde/fpicker/kdefilepicker \
	vcl/unx/kde/fpicker/kdefpmain \
	vcl/unx/kde/fpicker/kdemodalityfilter \
))

$(eval $(call gb_Executable_add_generated_cxxobjects,kdefilepicker,\
	CustomTarget/vcl/unx/kde/fpicker/kdefilepicker.moc \
))

# KDE/Qt consider -Wshadow more trouble than benefit
$(eval $(call gb_Executable_add_cxxflags,kdefilepicker,\
	-Wno-shadow \
))

# vim: set noet sw=4 ts=4:
