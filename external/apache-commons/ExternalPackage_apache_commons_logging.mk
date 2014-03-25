# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,apache_commons_logging,apache_commons_logging))

$(eval $(call gb_ExternalPackage_use_external_project,apache_commons_logging,apache_commons_logging))

$(eval $(call gb_ExternalPackage_add_file,apache_commons_logging,$(LIBO_SHARE_JAVA_FOLDER)/commons-logging-1.1.3.jar,target/commons-logging-1.1.3.jar))

# vim: set noet sw=4 ts=4:
