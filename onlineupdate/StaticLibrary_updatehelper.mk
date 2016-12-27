# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,updatehelper))

$(eval $(call gb_StaticLibrary_set_include,updatehelper,\
	-I$(SRCDIR)/onlineupdate/inc/ \
	-I$(SRCDIR)/onlineupdate/source/update/common \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,updatehelper,\
	-DUNICODE \
	-DNSS3 \
	-DVERIFY_MAR_SIGNATURE \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,updatehelper,\
	onlineupdate/source/update/common/pathhash \
	onlineupdate/source/update/common/readstrings \
	onlineupdate/source/update/common/uachelper \
	onlineupdate/source/update/common/updatehelper \
	onlineupdate/source/update/common/updatelogging \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
