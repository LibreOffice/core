# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,fontconfig))

$(eval $(call gb_UnpackedTarball_set_tarball,fontconfig,$(FONTCONFIG_TARBALL),,fontconfig))

$(eval $(call gb_UnpackedTarball_set_patchlevel,fontconfig,0))

$(eval $(call gb_UnpackedTarball_add_patches,fontconfig,\
	external/fontconfig/fontconfig-2.12.1.patch.1 \
	external/fontconfig/nowarnnoconfig.patch.1 \
))

ifneq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_patches,fontconfig,\
	external/fontconfig/libfontconfig-bundled-soname.patch.0 \
))
endif

ifeq ($(COM),MSC)

$(eval $(call gb_UnpackedTarball_add_patches,fontconfig,\
	external/fontconfig/windowsfonts.patch \
))

$(eval $(call gb_UnpackedTarball_set_post_action,fontconfig,\
	sed -e 's/@CACHE_VERSION@/9/' < fontconfig/fontconfig.h.in > fontconfig/fontconfig.h \
))


$(eval $(call gb_UnpackedTarball_add_file,fontconfig,fonts.conf,external/fontconfig/configs/wnt_fonts.conf))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,config.h,external/fontconfig/configs/wnt_fontconfig_config.h))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,fc-case/fccase.h,external/fontconfig/configs/wnt_fontconfig_fccase.h))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,fc-lang/fclang.h,external/fontconfig/configs/wnt_fontconfig_fclang.h))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,src/fcalias.h,external/fontconfig/configs/wnt_fontconfig_fcalias.h))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,src/fcaliastail.h,external/fontconfig/configs/wnt_fontconfig_fcaliastail.h))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,src/fcftalias.h,external/fontconfig/configs/wnt_fontconfig_fcftalias.h))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,src/fcftaliastail.h,external/fontconfig/configs/wnt_fontconfig_fcftaliastail.h))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,src/fcobjshash.h,external/fontconfig/configs/wnt_fontconfig_fcobjshash.h))
$(eval $(call gb_UnpackedTarball_add_file,fontconfig,src/fcstdint.h,external/fontconfig/configs/wnt_fontconfig_fcstdint.h))

endif

# vim: set noet sw=4 ts=4:
