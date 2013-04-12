# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_Package_Package,touch_inc,$(SRCDIR)/touch/inc))

$(eval $(call gb_Package_add_file,touch_inc,inc/touch/touch.h,touch/touch.h))
$(eval $(call gb_Package_add_file,touch_inc,inc/touch/libotouch.hxx,touch/libotouch.hxx))

# vim: set noet sw=4 ts=4:
