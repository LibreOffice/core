# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,ure_install,$(SRCDIR)/ure/source))

$(eval $(call gb_Package_set_outdir,ure_install,$(INSTDIR)))

$(eval $(call gb_Package_add_file,ure_install,ure/bin/startup.sh,startup.sh))
$(eval $(call gb_Package_add_file,ure_install,ure/lib/$(call gb_Helper_get_rcfile,jvmfwk3),jvmfwk3rc))
$(eval $(call gb_Package_add_file,ure_install,ure/lib/$(call gb_Helper_get_rcfile,uno),unorc))

# vim:set noet sw=4 ts=4:
