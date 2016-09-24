# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,beanshell,beanshell))

$(eval $(call gb_ExternalPackage_use_external_project,beanshell,beanshell))

$(eval $(call gb_ExternalPackage_add_file,beanshell,$(LIBO_SHARE_JAVA_FOLDER)/bsh.jar,dist/bsh-2.0b6.jar))

# vim: set noet sw=4 ts=4:
