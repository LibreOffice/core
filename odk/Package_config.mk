# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_config,$(SRCDIR)/odk/config))

$(eval $(call gb_Package_set_outdir,odk_config,$(INSTDIR)))

ifeq ($(OS),WNT)
$(eval $(call gb_Package_add_files,odk_config,$(SDKDIRNAME),\
	cfgWin.js \
	setsdkenv_windows.template \
	setsdkname.bat \
))
else
$(eval $(call gb_Package_add_files,odk_config,$(SDKDIRNAME),\
	configure.pl \
	setsdkenv_unix \
	setsdkenv_unix.sh.in \
))
endif

# vim: set noet sw=4 ts=4:
