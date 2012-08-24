# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,tplwizfax,$(SRCDIR)/extras/source/templates/wizard/fax))

$(eval $(call gb_Zip_add_files,tplwizfax,\
    bus-classic_f.ott \
    bus-classic-pri_f.ott \
    bus-modern_f.ott \
    bus-modern-pri_f.ott \
    pri-bottle_f.ott \
    pri-fax_f.ott \
    pri-lines_f.ott \
    pri-marine_f.ott \
))

# vim: set noet sw=4 ts=4:
