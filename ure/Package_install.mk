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

ifneq (WNT,$(OS))
$(eval $(call gb_Package_add_file,ure_install,ure/bin/startup.sh,startup.sh))
$(eval $(call gb_Package_add_symbolic_link,ure_install,ure/bin/uno,startup.sh))
endif

ifeq (MACOSX,$(OS))
$(eval $(call gb_Package_add_symbolic_link,ure_install,ure-link/bin/urelibs,../lib))
else ifneq (WNT,$(OS))
$(eval $(call gb_Package_add_symbolic_link,ure_install,ure-link,ure))
endif

$(eval $(call gb_Package_add_files,ure_install,$(LIBO_URE_LIB_FOLDER),\
	$(call gb_Helper_get_rcfile,jvmfwk3) \
	$(call gb_Helper_get_rcfile,uno) \
))

# vim:set noet sw=4 ts=4:
