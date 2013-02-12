# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,setup_native/scripts,$(SRCDIR)/setup_native/scripts))

$(eval $(call gb_Package_add_file,setup_native/scripts,bin/admin.pl,admin.pl))
$(eval $(call gb_Package_add_file,setup_native/scripts,bin/mac_install.script,mac_install.script))
$(eval $(call gb_Package_add_file,setup_native/scripts,bin/osx_install_languagepack.applescript,osx_install_languagepack.applescript))
$(eval $(call gb_Package_add_file,setup_native/scripts,bin/langpackscript.sh,langpackscript.sh))
$(eval $(call gb_Package_add_file,setup_native/scripts,bin/javaloader.sh,javaloader.sh))
$(eval $(call gb_Package_add_file,setup_native/scripts,bin/update,update.sh))
$(eval $(call gb_Package_add_file,setup_native/scripts,bin/downloadscript.sh,downloadscript.sh))

ifeq ($(ENABLE_ONLINE_UPDATE),TRUE)
$(eval $(call gb_Package_add_file,setup_native/scripts,bin/unpack_update,unpack_update.sh))
endif

# vim: set noet sw=4 ts=4:
