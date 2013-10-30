# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,setup_native/packinfo,$(SRCDIR)/setup_native/source/packinfo))

$(eval $(call gb_Package_set_outdir,setup_native/packinfo,$(OUTDIR)))

$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/solariscopyrightfile,solariscopyrightfile))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/linuxcopyrightfile,linuxcopyrightfile))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/libreoffice.pcp,libreoffice.pcp))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/openofficelanguagepack.pcp,openofficelanguagepack.pcp))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/ure.pcp,ure.pcp))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/finals_instsetoo.txt,finals_instsetoo.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/package.txt,package.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/package_names.txt,package_names.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/package_names_ext.txt,package_names_ext.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/packinfo_accessories.txt,packinfo_accessories.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/packinfo_brand.txt,packinfo_brand.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/packinfo_extensions.txt,packinfo_extensions.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/packinfo_office.txt,packinfo_office.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/packinfo_office_help.txt,packinfo_office_help.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/packinfo_office_lang.txt,packinfo_office_lang.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/packinfo_sdkoo.txt,packinfo_sdkoo.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/packinfo_ure.txt,packinfo_ure.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/shellscripts_module.txt,shellscripts_module.txt))
$(eval $(call gb_Package_add_file,setup_native/packinfo,bin/spellchecker_selection.txt,spellchecker_selection.txt))

# vim: set noet sw=4 ts=4:
