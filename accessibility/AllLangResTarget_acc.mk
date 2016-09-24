# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,acc))

$(eval $(call gb_AllLangResTarget_set_reslocation,acc,accessibility))

$(eval $(call gb_AllLangResTarget_add_srs,acc,\
    acc/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,acc/res))

$(eval $(call gb_SrsTarget_set_include,acc/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/accessibility/inc \
))

$(eval $(call gb_SrsTarget_add_files,acc/res,\
    accessibility/source/helper/accessiblestrings.src \
))

# vim:set noet sw=4 ts=4:
