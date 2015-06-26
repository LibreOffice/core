# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,updater))

$(eval $(call gb_Executable_set_include,updater,\
	-I$(SRCDIR)/onlineupdate/source/update/src \
	-I$(SRCDIR)/onlineupdate/source/update/inc \
	-I$(SRCDIR)/onlineupdate/source/update/common \
	-I$(SRCDIR)/onlineupdate/source/update/updater/libbz2/src \
	-I$(SRCDIR)/onlineupdate/source/update/updater/xpcom/glue \
	-lpthread \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_add_libs,updater,\
    -lX11 \
    -lXext \
    -lXrender \
    -lSM \
    -lICE \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Executable_set_include,updater,\
	-lgtk \
))
$(eval $(call gb_Exectuable_add_libs,updater,\
	-lpthread \
))
else ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_set_include,updater,\
	-lgtk \
))
$(eval $(call gb_Exectuable_add_libs,updater,\
	-lpthread \
))
endif

$(eval $(call gb_Executable_add_cxxflags,updater,\
    $$(GTK3_CFLAGS) \
))

$(eval $(call gb_Executable_add_libs,updater,\
    $(GTK3_LIBS) \
))

$(eval $(call gb_Executable_use_libraries,updater,\
	libreofficekitgtk \
))

$(eval $(call gb_Executable_use_externals,updater,\
	gtk \
))

$(eval $(call gb_Executable_add_exception_objects,updater,\
	onlineupdate/source/update/updater/xpcom/glue/nsVersionComparator \
	onlineupdate/source/update/updater/archivereader \
	onlineupdate/source/update/updater/bspatch \
	onlineupdate/source/update/updater/loaddlls \
	onlineupdate/source/update/updater/progressui_gtk \
	onlineupdate/source/update/updater/progressui_null \
	onlineupdate/source/update/updater/progressui_win \
	onlineupdate/source/update/updater/updater \
	onlineupdate/source/update/updater/win_dirent \
	onlineupdate/source/update/common/pathhash \
	onlineupdate/source/update/common/readstrings \
	onlineupdate/source/update/common/uachelper \
	onlineupdate/source/update/common/updatehelper \
	onlineupdate/source/update/common/updatelogging \
))

$(eval $(call gb_Executable_add_cobjects,updater,\
	onlineupdate/source/update/src/mar_create \
	onlineupdate/source/update/src/mar_extract \
	onlineupdate/source/update/src/mar_read \
	onlineupdate/source/update/updater/libbz2/src/blocksort \
	onlineupdate/source/update/updater/libbz2/src/bzlib \
	onlineupdate/source/update/updater/libbz2/src/compress \
	onlineupdate/source/update/updater/libbz2/src/crctable \
	onlineupdate/source/update/updater/libbz2/src/decompress \
	onlineupdate/source/update/updater/libbz2/src/huffman \
	onlineupdate/source/update/updater/libbz2/src/randtable \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
