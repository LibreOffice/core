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

$(eval $(call gb_Executable_use_custom_headers,ucpp,\
	ucpp/source \
))

$(eval $(call gb_Executable_add_defs,ucpp,\
	-DNO_UCPP_BUF \
	-DUCPP_CONFIG \
	-DSTAND_ALONE \
))

$(eval $(call gb_Executable_add_generated_cobjects,ucpp,\
	CustomTarget/ucpp/source/assert \
	CustomTarget/ucpp/source/cpp \
	CustomTarget/ucpp/source/eval \
	CustomTarget/ucpp/source/hash \
	CustomTarget/ucpp/source/lexer \
	CustomTarget/ucpp/source/macro \
	CustomTarget/ucpp/source/mem \
	CustomTarget/ucpp/source/nhash \
))

# vim: set noet sw=4 ts=4:
