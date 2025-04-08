# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,graphite))

$(eval $(call gb_UnpackedTarball_set_tarball,graphite,$(GRAPHITE_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,graphite,0))

$(eval $(call gb_UnpackedTarball_add_patches,graphite, \
    external/graphite/ubsan.patch \
    external/graphite/enumarith.patch \
))

# cannot use post_action since $(file ..) would be run when the recipe is parsed, i.e. would always
# happen before the tarball is unpacked
$(gb_UnpackedTarball_workdir)/graphite/graphite2-uninstalled.pc: $(call gb_UnpackedTarball_get_target,graphite)
	$(file >$@,$(call gb_pkgconfig_file,graphite2,1.3.14,$(GRAPHITE_CFLAGS),$(GRAPHITE_LIBS)))

# vim: set noet sw=4 ts=4:
