# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libodfgen))

$(eval $(call gb_UnpackedTarball_set_tarball,libodfgen,$(ODFGEN_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libodfgen,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libodfgen))

# * external/libodfgen/ellipticalarc.patch reported upstream as
#   <https://sourceforge.net/p/libwpd/mailman/message/37329517/> "[Libwpd-devel] [PATCH] Avoid
#   division by zero for empty elliptical arc":
# * external/libodfgen/0001-const-up-libodfgen.patch.1
#   <https://sourceforge.net/p/libwpd/libodfgen/merge-requests/1/> upstreaming attempt
$(eval $(call gb_UnpackedTarball_add_patches,libodfgen, \
    external/libodfgen/ellipticalarc.patch \
    external/libodfgen/0001-const-up-libodfgen.patch.1 \
))

ifeq ($(SYSTEM_REVENGE),)
$(eval $(call gb_UnpackedTarball_add_patches,libodfgen, \
    external/libodfgen/rpath.patch \
))
endif

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,libodfgen, \
    external/libodfgen/libodfgen-bundled-soname.patch.0 \
))
endif
endif

# vim: set noet sw=4 ts=4:
