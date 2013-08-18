# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplwizreport,$(SRCDIR)/extras/source/templates/wizard/report))

$(eval $(call gb_Package_set_outdir,extras_tplwizreport,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_tplwizreport,$(LIBO_SHARE_FOLDER)/template/common/wizard/report,\
    cnt-011.ott \
    cnt-012.ott \
    cnt-01.ott \
    cnt-021.ott \
    cnt-022.ott \
    cnt-02.ott \
    cnt-031.ott \
    cnt-032.ott \
    cnt-03.ott \
    cnt-041.ott \
    cnt-042.ott \
    cnt-04.ott \
    cnt-051.ott \
    cnt-052.ott \
    cnt-05.ott \
    cnt-061.ott \
    cnt-062.ott \
    cnt-06.ott \
    cnt-default.ott \
    stl-01.ott \
    stl-02.ott \
    stl-03.ott \
    stl-04.ott \
    stl-05.ott \
    stl-06.ott \
    stl-07.ott \
    stl-08.ott \
    stl-09.ott \
    stl-default.ott \
))

# vim: set noet sw=4 ts=4:
