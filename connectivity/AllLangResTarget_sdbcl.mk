# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sdbcl))

$(eval $(call gb_AllLangResTarget_add_srs,sdbcl,\
	sdbcl/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sdbcl/res))

$(eval $(call gb_SrsTarget_set_include,sdbcl/res,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_SrsTarget_add_files,sdbcl/res,\
	connectivity/source/resource/conn_log_res.src \
))

# vim: set noet sw=4 ts=4:
