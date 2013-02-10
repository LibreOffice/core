# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,desktop,$(WORKDIR)/CustomTarget))

ifneq ($(filter deb,$(PKGFORMAT)),)
$(eval $(call gb_Package_add_files,desktop,bin/desktop-integration/deb,\
	$(foreach product,$(PRODUCTLIST),\
	sysui/deb/$(product)-desktop-integration.tar.gz) \
))
endif

ifneq ($(filter rpm,$(PKGFORMAT)),)
$(eval $(call gb_Package_add_files,desktop,bin/desktop-integration/rpm,\
	$(foreach product,$(PRODUCTLIST),\
	sysui/rpm/$(product)-desktop-integration.tar.gz) \
))
endif

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Package_add_files,desktop,bin/desktop-integration/pkg,\
	$(foreach product,$(PRODUCTLIST),\
	sysui/solaris/$(product)-desktop-integration.tar.gz) \
))
endif

# vim: set noet sw=4 ts=4:
