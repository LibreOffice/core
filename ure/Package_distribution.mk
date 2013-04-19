# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,ure_distribution,$(SRCDIR)/ure/source))

$(eval $(call gb_Package_add_file,ure_distribution,bin/ure/README,README))
$(eval $(call gb_Package_add_file,ure_distribution,bin/ure/THIRDPARTYLICENSEREADME.html,THIRDPARTYLICENSEREADME.html))
$(eval $(call gb_Package_add_file,ure_distribution,bin/ure/jvmfwk3.ini,jvmfwk3.ini))
$(eval $(call gb_Package_add_file,ure_distribution,bin/ure/jvmfwk3rc,jvmfwk3rc))
$(eval $(call gb_Package_add_file,ure_distribution,bin/ure/startup.sh,startup.sh))
$(eval $(call gb_Package_add_file,ure_distribution,bin/ure/uno.ini,uno.ini))
$(eval $(call gb_Package_add_file,ure_distribution,bin/ure/unorc,unorc))

# vim:set noet sw=4 ts=4:
