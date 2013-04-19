# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sdbt))

$(eval $(call gb_AllLangResTarget_set_reslocation,sdbt,dbaccess))

$(eval $(call gb_AllLangResTarget_add_srs,sdbt,\
    dbaccess/sdbt \
))

$(eval $(call gb_SrsTarget_SrsTarget,dbaccess/sdbt))

$(eval $(call gb_SrsTarget_use_packages,dbaccess/sdbt,\
))

$(eval $(call gb_SrsTarget_set_include,dbaccess/sdbt,\
    $$(INCLUDE) \
    -I$(SRCDIR)/dbaccess/source/sdbtools/inc \
))

$(eval $(call gb_SrsTarget_add_files,dbaccess/sdbt,\
    dbaccess/source/sdbtools/resource/sdbt_strings.src \
))

# vim: set noet sw=4 ts=4:
