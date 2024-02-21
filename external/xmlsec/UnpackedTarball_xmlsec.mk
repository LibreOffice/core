# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

xmlsec_patches :=
# Remove this when Ubuntu 20.04 is EOL in 2025.
xmlsec_patches += old-nss.patch.1
# Remove this when Windows 7 is no longer supported
xmlsec_patches += BCryptKeyDerivation.patch.1

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xmlsec))

$(eval $(call gb_UnpackedTarball_set_tarball,xmlsec,$(XMLSEC_TARBALL),,xmlsec))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,xmlsec))

$(eval $(call gb_UnpackedTarball_add_patches,xmlsec,\
	$(foreach patch,$(xmlsec_patches),external/xmlsec/$(patch)) \
))

# vim: set noet sw=4 ts=4:
