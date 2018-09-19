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
# Backport of <https://github.com/lsh123/xmlsec/pull/192>.
xmlsec_patches += mscng-fixes.patch.1
# Backport of <https://github.com/lsh123/xmlsec/pull/197>.
xmlsec_patches += xmlsec1-macos.patch.1

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xmlsec))

$(eval $(call gb_UnpackedTarball_set_tarball,xmlsec,$(XMLSEC_TARBALL),,xmlsec))

$(eval $(call gb_UnpackedTarball_add_patches,xmlsec,\
	$(foreach patch,$(xmlsec_patches),external/xmlsec/$(patch)) \
))

# vim: set noet sw=4 ts=4:
