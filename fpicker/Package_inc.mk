# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,fpicker_inc,$(SRCDIR)/fpicker/inc))
$(eval $(call gb_Package_add_file,fpicker_inc,inc/fpicker/fpicker.hrc,fpicker/fpicker.hrc))

# vim: set noet sw=4 ts=4:
