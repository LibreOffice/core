# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libmar))

$(eval $(call gb_StaticLibrary_set_include,libmar,\
	-I$(SRCDIR)/onlineupdate/source/libmar/src/ \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,libmar,\
	-DMAR_NSS \
))

$(eval $(call gb_StaticLibrary_use_externals,libmar,\
	nss3 \
))

$(eval $(call gb_StaticLibrary_add_cobjects,libmar,\
	onlineupdate/source/libmar/src/mar_create \
	onlineupdate/source/libmar/src/mar_extract \
	onlineupdate/source/libmar/src/mar_read \
	onlineupdate/source/libmar/verify/mar_verify \
	onlineupdate/source/libmar/verify/cryptox \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
