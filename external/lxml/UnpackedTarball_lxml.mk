# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,lxml))

$(eval $(call gb_UnpackedTarball_set_tarball,lxml,$(LXML_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,lxml,0))

$(eval $(call gb_UnpackedTarball_add_patches,lxml, \
	external/lxml/0001-Make-regexp-string-raw-to-correct-its-escape-sequenc.patch.1 \
	external/lxml/replace-setuptools-with-distutils.patch.1 \
	external/lxml/Wincompatible-function-pointer-types.patch \
	external/lxml/Wincompatible-pointer-types.patch \
))

# vim: set noet sw=4 ts=4:
