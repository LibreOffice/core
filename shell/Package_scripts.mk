# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,shell_scripts,$(SRCDIR)/shell/source/unix/misc))

$(eval $(call gb_Package_add_file,shell_scripts,bin/cde-open-url,cde-open-url.sh))
$(eval $(call gb_Package_add_file,shell_scripts,bin/gnome-open-url,gnome-open-url.sh))
$(eval $(call gb_Package_add_file,shell_scripts,bin/kde-open-url,kde-open-url.sh))
$(eval $(call gb_Package_add_file,shell_scripts,bin/open-url,open-url.sh))
$(eval $(call gb_Package_add_file,shell_scripts,bin/senddoc,senddoc.sh))
$(eval $(call gb_Package_add_file,shell_scripts,bin/tde-open-url,tde-open-url.sh))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
