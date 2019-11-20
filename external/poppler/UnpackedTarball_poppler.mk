# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,poppler))

$(eval $(call gb_UnpackedTarball_set_tarball,poppler,$(POPPLER_TARBALL),,poppler))

$(eval $(call gb_UnpackedTarball_add_patches,poppler,\
	external/poppler/poppler-config.patch.1 \
	external/poppler/poppler-c++11.patch.1 \
	external/poppler/0001-Revert-Make-the-mul-tables-be-calculated-at-compile-.patch.1 \
))

# std::make_unique is only available in C++14
# use "env -i" to avoid Cygwin "environment is too large for exec"
# Mac OS X sed says "sed: RE error: illegal byte sequence"; Apple clang should
# be happy with std::make_unique so just skip it
ifneq ($(OS_FOR_BUILD),MACOSX)
$(eval $(call gb_UnpackedTarball_set_post_action,poppler,\
	env -i PATH="$(if $(filter WNT,$(OS)),/usr/bin,$(PATH))" $(FIND) . -name '*.cc' -exec sed -i -e 's/std::make_unique/o3tl::make_unique/' {} \\; \
))
endif

# vim: set noet sw=4 ts=4:
