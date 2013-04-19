# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,tubes_inc,$(SRCDIR)/tubes/inc))

$(eval $(call gb_Package_add_file,tubes_inc,inc/tubes/collaboration.hxx,tubes/collaboration.hxx))
$(eval $(call gb_Package_add_file,tubes_inc,inc/tubes/manager.hxx,tubes/manager.hxx))
$(eval $(call gb_Package_add_file,tubes_inc,inc/tubes/tubesdllapi.h,tubes/tubesdllapi.h))

# vim: set noet sw=4 ts=4:
