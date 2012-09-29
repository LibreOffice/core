# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,apache_commons))

ifneq ($(SOLAR_JAVA),)
ifneq ($(filter APACHE_COMMONS,$(BUILD_TYPE)),)
$(eval $(call gb_Module_add_targets,apache_commons,\
	ExternalProject_apache_commons_codec \
	ExternalProject_apache_commons_httpclient \
	ExternalProject_apache_commons_lang \
	ExternalProject_apache_commons_logging \
	Package_apache_commons_codec \
	Package_apache_commons_httpclient \
	Package_apache_commons_lang \
	Package_apache_commons_logging \
	UnpackedTarball_apache_commons_codec \
	UnpackedTarball_apache_commons_httpclient \
	UnpackedTarball_apache_commons_lang \
	UnpackedTarball_apache_commons_logging \
))
endif
endif

# vim: set noet sw=4 ts=4:
