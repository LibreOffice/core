# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,dkt))

$(eval $(call gb_AllLangResTarget_set_reslocation,dkt,desktop))

$(eval $(call gb_AllLangResTarget_add_srs,dkt,\
    dkt/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,dkt/res))

$(eval $(call gb_SrsTarget_use_packages,dkt/res,\
    svl_inc \
))

$(eval $(call gb_SrsTarget_add_files,dkt/res,\
    desktop/source/app/desktop.src \
))

# vim: set ts=4 sw=4 et:
