# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This is de facto just for Windows for now

$(eval $(call gb_StaticLibrary_StaticLibrary,fontconfig))

$(eval $(call gb_StaticLibrary_use_unpacked,fontconfig,fontconfig))

$(eval $(call gb_StaticLibrary_set_include,fontconfig,\
	-I$(gb_UnpackedTarball_workdir)/fontconfig \
	-I$(gb_UnpackedTarball_workdir)/freetype/include \
	-I$(gb_UnpackedTarball_workdir)/expat/lib \
	$$(INCLUDE) \
))

# Ignore warnings, warnings in this code is not our problem, or is it?

$(eval $(call gb_StaticLibrary_add_defs,fontconfig,\
	-wd4018 \
	-wd4090 \
	-wd4100 \
	-wd4189 \
	-wd4200 \
	-wd4245 \
	-wd4267 \
	-wd4456 \
	-wd4457 \
	-wd4701 \
	-wd4703 \
	-DHAVE_CONFIG_H \
	-DFC_CACHEDIR='"C:/Windows/Temp"' \
	-DCONFIGDIR='"Whatever"' \
	-DFONTCONFIG_PATH='"Whatever"' \
	-DFC_TEMPLATEDIR='"Whatever"' \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,fontconfig,\
	$(addprefix UnpackedTarball/fontconfig/src/, \
		fcatomic \
		fccache \
		fccfg \
		fccharset \
		fccompat \
		fcdbg \
		fcdefault \
		fcdir \
		fcformat \
		fcfreetype \
		fcfs \
		fcptrlist \
		fchash \
		fcinit \
		fclang \
		fclist \
		fcmatch \
		fcmatrix \
		fcname \
		fcobjs \
		fcpat \
		fcrange \
		fcserialize \
		fcstat \
		fcstr \
		fcweight \
		fcxml \
		ftglue \
	) \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,fontconfig,\
	$(addprefix UnpackedTarball/fontconfig/src/, \
		fcdwrite \
	) \
))

# vim: set noet sw=4 ts=4:
