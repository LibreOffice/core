# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,apache_commons_lang))

$(eval $(call gb_UnpackedTarball_set_tarball,apache_commons_lang,$(APACHE_COMMONS_LANG_TARBALL),,apache-commons))

$(eval $(call gb_UnpackedTarball_add_patches,apache_commons_lang,\
        $(if $(filter TRUE,$(HAVE_JAVA6)) \
		,external/apache-commons/patches/lang.patch \
	) \
))

# vim: set noet sw=4 ts=4:
