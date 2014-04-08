# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,glew))

$(eval $(call gb_UnpackedTarball_set_tarball,glew,$(GLEW_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,glew,0))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_UnpackedTarball_set_patchflags,glew,--binary))
$(eval $(call gb_UnpackedTarball_add_patches,glew,\
	external/glew/glew-fix-rc-error.patch.1 \
))
else
$(eval $(call gb_UnpackedTarball_add_patches,glew,\
	external/glew/glew_use_CC_variable.patch.1 \
	external/glew/0001-stop-linking-to-unneeded-Xmu-Xi-Xext-libraries.patch.1 \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_patches,glew,\
	external/glew/glew-macosx-install-name.patch.1 \
))
endif

# vim: set noet sw=4 ts=4:
