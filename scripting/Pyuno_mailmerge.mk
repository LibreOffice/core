# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Pyuno_Pyuno,mailmerge,$(SRCDIR)/scripting/source/pyprov))

$(eval $(call gb_Pyuno_set_componentfile,mailmerge,scripting/source/pyprov/mailmerge,services))

$(eval $(call gb_Pyuno_add_files,mailmerge,,\
	mailmerge.py \
	msgbox.py \
))

# vim: set noet sw=4 ts=4:
