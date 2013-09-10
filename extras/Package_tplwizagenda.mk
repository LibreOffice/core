# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplwizagenda,$(SRCDIR)/extras/source/templates/wizard/agenda))

$(eval $(call gb_Package_set_outdir,extras_tplwizagenda,$(gb_INSTROOT)))

$(eval $(call gb_Package_add_files,extras_tplwizagenda,$(LIBO_SHARE_FOLDER)/template/common/wizard/agenda,\
    10grey.ott \
    1simple.ott \
    2elegant.ott \
    3modern.ott \
    4classic.ott \
    5blue.ott \
    6orange.ott \
    7red.ott \
    8green.ott \
    9colorful.ott \
    aw-10grey.ott \
    aw-1simple.ott \
    aw-2elegant.ott \
    aw-3modern.ott \
    aw-4classic.ott \
    aw-5blue.ott \
    aw-6orange.ott \
    aw-7red.ott \
    aw-8green.ott \
    aw-9colorful.ott \
))

# vim: set noet sw=4 ts=4:
