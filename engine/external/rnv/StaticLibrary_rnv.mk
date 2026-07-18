# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,rnv))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,rnv))

$(eval $(call gb_StaticLibrary_use_unpacked,rnv,rnv))

$(eval $(call gb_StaticLibrary_add_defs,rnv,\
	-DUNISTD_H="<unistd.h>" \
))

$(eval $(call gb_StaticLibrary_set_include,rnv,\
	-I$(gb_UnpackedTarball_workdir)/rnv \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,rnv,\
	UnpackedTarball/rnv/ary \
	UnpackedTarball/rnv/drv \
	UnpackedTarball/rnv/dsl \
	UnpackedTarball/rnv/dxl \
	UnpackedTarball/rnv/er \
	UnpackedTarball/rnv/ht \
	UnpackedTarball/rnv/m \
	UnpackedTarball/rnv/rn \
	UnpackedTarball/rnv/rnc \
	UnpackedTarball/rnv/rnd \
	UnpackedTarball/rnv/rnl \
	UnpackedTarball/rnv/rnv \
	UnpackedTarball/rnv/rnx \
	UnpackedTarball/rnv/rx \
	UnpackedTarball/rnv/s \
	UnpackedTarball/rnv/sc \
	UnpackedTarball/rnv/u \
	UnpackedTarball/rnv/xmlc \
	UnpackedTarball/rnv/xsd \
	UnpackedTarball/rnv/xsd_tm \
))

# vim: set noet sw=4 ts=4:
