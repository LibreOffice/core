# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,lpsolve))

$(eval $(call gb_UnpackedTarball_set_tarball,lpsolve,$(LPSOLVE_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,lpsolve,3))

ifeq ($(OS_FOR_BUILD),WNT)

$(eval $(call gb_UnpackedTarball_set_patchflags,lpsolve,--binary))
$(eval $(call gb_UnpackedTarball_add_patches,lpsolve,\
	lpsolve/lp_solve_5.5-windows.patch \
))

else

$(eval $(call gb_UnpackedTarball_add_patches,lpsolve,\
	lpsolve/lp_solve-aix.patch \
	lpsolve/lp_solve-fixed-warn.patch \
	lpsolve/lp_solve_5.5.patch \
))

$(eval $(call gb_UnpackedTarball_add_file,lpsolve,lpsolve55/ccc.static,lpsolve/ccc.static))

endif
# vim: set noet sw=4 ts=4:


