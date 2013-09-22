# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplwizletter,$(SRCDIR)/extras/source/templates/wizard/letter))

$(eval $(call gb_Package_set_outdir,extras_tplwizletter,$(INSTROOT)))

$(eval $(call gb_Package_add_files,extras_tplwizletter,$(LIBO_SHARE_FOLDER)/template/common/wizard/letter,\
    bus-elegant_l.ott \
    bus-modern_l.ott \
    bus-office_l.ott \
    off-elegant_l.ott \
    off-modern_l.ott \
    off-office_l.ott \
    pri-bottle_l.ott \
    pri-mail_l.ott \
    pri-marine_l.ott \
    pri-redline_l.ott \
))

# vim: set noet sw=4 ts=4:
