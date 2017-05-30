# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

xmlsec_patches :=
xmlsec_patches += xmlsec1-configure.patch.1
xmlsec_patches += xmlsec1-vc.patch.1
xmlsec_patches += xmlsec1-1.2.14_fix_extern_c.patch.1
xmlsec_patches += xmlsec1-customkeymanage.patch.1
# Backport of <https://github.com/lsh123/xmlsec/pull/112>.
xmlsec_patches += xmlsec1-mscrypto-fix-signing-regression.patch.1

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xmlsec))

$(eval $(call gb_UnpackedTarball_set_tarball,xmlsec,$(LIBXMLSEC_TARBALL),,libxmlsec))

$(eval $(call gb_UnpackedTarball_add_patches,xmlsec,\
	$(foreach patch,$(xmlsec_patches),external/libxmlsec/$(patch)) \
))

# vim: set noet sw=4 ts=4:
