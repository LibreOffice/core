# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xml2))

$(eval $(call gb_UnpackedTarball_set_tarball,xml2,$(LIBXML_TARBALL),,libxml2))

$(eval $(call gb_UnpackedTarball_add_patches,xml2,\
	external/libxml2/libxml2-config.patch.1 \
	external/libxml2/libxml2-global-symbols.patch \
	external/libxml2/libxml2-vc10.patch \
	$(if $(filter ANDROID,$(OS)),external/libxml2/libxml2-android.patch) \
	external/libxml2/libxml2-icu.patch.0 \
	external/libxml2/ubsan.patch.0 \
	external/libxml2/0001-Fix-type-confusion-in-xmlValidateOneNamespace.patch.1 \
	external/libxml2/0001-Prevent-unwanted-external-entity-reference.patch.1 \
	external/libxml2/0001-Increase-buffer-space-for-port-in-HTTP-redirect-supp.patch.1 \
	external/libxml2/0001-Fix-buffer-size-checks-in-xmlSnprintfElementContent.patch.1 \
	external/libxml2/0001-Fix-handling-of-parameter-entity-references.patch.1 \
))

# vim: set noet sw=4 ts=4:
