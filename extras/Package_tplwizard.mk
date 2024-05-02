# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplwizard,$(gb_CustomTarget_workdir)/extras/source/templates/wizard))

$(eval $(call gb_Package_add_files_with_dir,extras_tplwizard,$(LIBO_SHARE_FOLDER)/template/common/wizard,\
    agenda/10grey.ott \
    agenda/1simple.ott \
    agenda/2elegant.ott \
    agenda/3modern.ott \
    agenda/4classic.ott \
    agenda/5blue.ott \
    agenda/6orange.ott \
    agenda/7red.ott \
    agenda/8green.ott \
    agenda/9colorful.ott \
    agenda/aw-10grey.ott \
    agenda/aw-1simple.ott \
    agenda/aw-2elegant.ott \
    agenda/aw-3modern.ott \
    agenda/aw-4classic.ott \
    agenda/aw-5blue.ott \
    agenda/aw-6orange.ott \
    agenda/aw-7red.ott \
    agenda/aw-8green.ott \
    agenda/aw-9colorful.ott \
    fax/bus-classic_f.ott \
    fax/bus-classic-pri_f.ott \
    fax/bus-modern_f.ott \
    fax/bus-modern-pri_f.ott \
    fax/pri-bottle_f.ott \
    fax/pri-fax_f.ott \
    fax/pri-lines_f.ott \
    fax/pri-marine_f.ott \
    letter/bus-elegant_l.ott \
    letter/bus-modern_l.ott \
    letter/bus-office_l.ott \
    letter/off-elegant_l.ott \
    letter/off-modern_l.ott \
    letter/off-office_l.ott \
    letter/pri-bottle_l.ott \
    letter/pri-mail_l.ott \
    letter/pri-marine_l.ott \
    letter/pri-redline_l.ott \
    report/cnt-011.ott \
    report/cnt-012.ott \
    report/cnt-01.ott \
    report/cnt-021.ott \
    report/cnt-022.ott \
    report/cnt-02.ott \
    report/cnt-031.ott \
    report/cnt-032.ott \
    report/cnt-03.ott \
    report/cnt-041.ott \
    report/cnt-042.ott \
    report/cnt-04.ott \
    report/cnt-051.ott \
    report/cnt-052.ott \
    report/cnt-05.ott \
    report/cnt-061.ott \
    report/cnt-062.ott \
    report/cnt-06.ott \
    report/cnt-default.ott \
    report/stl-01.ott \
    report/stl-02.ott \
    report/stl-03.ott \
    report/stl-04.ott \
    report/stl-05.ott \
    report/stl-06.ott \
    report/stl-07.ott \
    report/stl-08.ott \
    report/stl-09.ott \
    report/stl-default.ott \
    styles/blackberry.ots \
    styles/black_white.ots \
    styles/default.ots \
    styles/diner.ots \
    styles/fall.ots \
    styles/glacier.ots \
    styles/green_grapes.ots \
    styles/jeans.ots \
    styles/marine.ots \
    styles/millennium.ots \
    styles/nature.ots \
    styles/neon.ots \
    styles/night.ots \
    styles/nostalgic.ots \
    styles/pastell.ots \
    styles/pool.ots \
    styles/pumpkin.ots \
    styles/xos.ots \
))

# vim: set noet sw=4 ts=4:
