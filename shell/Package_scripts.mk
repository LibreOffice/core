# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,shell_scripts,$(SRCDIR)/shell/source/unix/misc))

$(eval $(call gb_Package_set_outdir,shell_scripts,$(INSTDIR)))

$(eval $(call gb_Package_add_file,shell_scripts,program/cde-open-url,cde-open-url.sh))
$(eval $(call gb_Package_add_file,shell_scripts,program/open-url,open-url.sh))
$(eval $(call gb_Package_add_file,shell_scripts,program/tde-open-url,tde-open-url.sh))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
