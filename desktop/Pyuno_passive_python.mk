# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Pyuno_Pyuno,passive_python,$(SRCDIR)/desktop/test/deployment/passive))

$(eval $(call gb_Pyuno_add_files,passive_python,,\
    passive_python.py \
))

$(eval $(call gb_Pyuno_set_componentfile_full,passive_python,desktop/test/deployment/passive/passive_python,./,passive_python.py,passive_generic))

# vim: set noet sw=4 ts=4:
