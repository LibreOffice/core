# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,openldap))

$(eval $(call gb_UnpackedTarball_set_tarball,openldap,$(OPENLDAP_TARBALL),,openldap))

$(eval $(call gb_UnpackedTarball_set_patchlevel,openldap,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,openldap,\
	build \
	contrib/ldapc++ \
))

$(eval $(call gb_UnpackedTarball_add_patches,openldap,\
	external/openldap/openldap-2.4.44.patch.1 \
))

# vim: set noet sw=4 ts=4:
