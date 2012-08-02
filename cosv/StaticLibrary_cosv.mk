# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,cosv))

$(eval $(call gb_StaticLibrary_use_packages,cosv,\
	cosv_inc \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,cosv,\
	cosv/source/comphelp/badcast \
	cosv/source/service/comdline \
	cosv/source/service/comfunc \
	cosv/source/service/csv_ostream \
    cosv/source/service/std_outp \
    cosv/source/storage/dirchain \
    cosv/source/storage/file \
    cosv/source/storage/persist \
    cosv/source/storage/ploc \
    cosv/source/storage/ploc_dir \
    cosv/source/storage/plocroot \
    cosv/source/strings/streamstr \
    cosv/source/strings/string \
    cosv/source/strings/str_types \
))

# vim: set noet sw=4 ts=4:
