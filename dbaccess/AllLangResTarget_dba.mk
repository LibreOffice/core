# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,dba))

$(eval $(call gb_AllLangResTarget_set_reslocation,dba,dbaccess))

$(eval $(call gb_AllLangResTarget_add_srs,dba,\
    dbaccess/dba \
))

$(eval $(call gb_SrsTarget_SrsTarget,dbaccess/dba))

$(eval $(call gb_SrsTarget_use_packages,dbaccess/dba,\
	svl_inc \
))

$(eval $(call gb_SrsTarget_set_include,dbaccess/dba,\
    $$(INCLUDE) \
	-I$(SRCDIR)/dbaccess/source/core/inc \
))

$(eval $(call gb_SrsTarget_add_files,dbaccess/dba,\
    dbaccess/source/core/resource/strings.src \
))

# vim: set noet sw=4 ts=4:
