# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,setup_native/misc,$(SRCDIR)/setup_native/source))

$(eval $(call gb_Package_set_outdir,setup_native/misc,$(OUTDIR)))

$(eval $(call gb_Package_add_file,setup_native/misc,bin/Info.plist.langpack,mac/Info.plist.langpack))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/ooo3_installer.icns,mac/ooo3_installer.icns))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/osl/DS_Store,mac/ooo/DS_Store))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/osl/DS_Store_Dev,mac/ooo/DS_Store_Dev))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/osl/DS_Store_Langpack,mac/ooo/DS_Store_Langpack))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/osl/osxdndinstall.png,mac/ooo/osxdndinstall.png))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/root3.dat,linux/root3.dat))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/root4.dat,linux/root4.dat))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/root5.dat,linux/root5.dat))
$(eval $(call gb_Package_add_file,setup_native/misc,bin/desktophelper.txt,win32/desktophelper.txt))

# vim: set noet sw=4 ts=4:
