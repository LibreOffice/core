# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,ucpp))

$(eval $(call gb_Executable_set_warnings_not_errors,ucpp))

$(eval $(call gb_Executable_use_unpacked,ucpp,ucpp))

$(eval $(call gb_Executable_add_defs,ucpp,\
	-DNO_UCPP_BUF \
	-DUCPP_CONFIG \
	-DSTAND_ALONE \
))

$(eval $(call gb_Executable_add_generated_cobjects,ucpp,\
	UnpackedTarball/ucpp/assert \
	UnpackedTarball/ucpp/cpp \
	UnpackedTarball/ucpp/eval \
	UnpackedTarball/ucpp/hash \
	UnpackedTarball/ucpp/lexer \
	UnpackedTarball/ucpp/macro \
	UnpackedTarball/ucpp/mem \
	UnpackedTarball/ucpp/nhash \
))

# vim: set noet sw=4 ts=4:
