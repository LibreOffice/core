# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,expat))

$(eval $(call gb_UnpackedTarball_set_tarball,expat,$(EXPAT_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,expat,conftools))

$(eval $(call gb_UnpackedTarball_add_patches,expat,\
	external/expat/expat-winapi.patch \
))

# This is a bit hackish.

# When building for Windows (as 32-bit) we need to build it twice: as
# 32- and 64-bit code, to be able to produce a 64-bit Explorer
# ("shell") extension that is used when the 32-bit LibreOffice is
# installed on a 64-bit OS.

$(eval $(call gb_UnpackedTarball_set_post_action,expat,\
	$(if $(filter $(BUILD_X64),TRUE),         \
	  cp lib/xmlparse.c lib/xmlparse_x64.c && \
	  cp lib/xmltok.c lib/xmltok_x64.c     && \
	  cp lib/xmlrole.c lib/xmlrole_x64.c) \
))

# vim: set noet sw=4 ts=4:
