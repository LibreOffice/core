# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,apache-commons))

$(eval $(call gb_Module_add_targets,apache-commons,\
	ExternalPackage_apache_commons_logging \
	ExternalProject_apache_commons_logging \
	UnpackedTarball_apache_commons_logging \
))

# vim: set noet sw=4 ts=4:
