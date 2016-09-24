# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sdberr))

$(eval $(call gb_AllLangResTarget_add_srs,sdberr,\
	sdberr/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sdberr/res))

$(eval $(call gb_SrsTarget_add_files,sdberr/res,\
	connectivity/source/resource/conn_error_message.src \
))

# vim: set noet sw=4 ts=4:
